/* -*- Mode: C; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

/* coap-client -- simple CoAP client
 *
 * Copyright (C) 2010--2016 Olaf Bergmann <bergmann@tzi.org>
 *
 * This file is part of the CoAP library libcoap. Please see README for terms of
 * use.
 */

#include "coap_config.h"

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <osi_log.h>

#include "coap.h"
#include "coap_mbeddtls.h"
#include "coap_list.h"
#include "coap_async_api.h"

#define exit(x) do{ OSI_LOGI(0,"exit with %d\n",x);result = -1;goto exit;} while(0)
#define MAX_USER 128 /* Maximum length of a user name (i.e., PSK
                      * identity) in bytes. */
#define MAX_KEY   64 /* Maximum length of a key (i.e., PSK) in bytes. */

int flags = 0;

static unsigned char _token_data[8];
str the_token = { 0, _token_data };

#define FLAGS_BLOCK 0x01

static coap_list_t *optlist = NULL;
/* Request URI.
 * TODO: associate the resources with transaction id and make it expireable */
static coap_uri_t uri;
static str proxy = { 0, NULL };
static unsigned short proxy_port = COAP_DEFAULT_PORT;

/* reading is done when this flag is set */
static int ready = 0;

static str output_file = { 0, NULL };   /* output file name */
static FILE *file = NULL;               /* output file stream */

static str payload = { 0, NULL };       /* optional payload to send */

unsigned char msgtype = COAP_MESSAGE_CON; /* usually, requests are sent confirmable */

typedef unsigned char method_t;
method_t method = 1;                    /* the method we are using in our requests */

coap_block_t block = { .num = 0, .m = 0, .szx = 6 };

unsigned int wait_seconds = 90;         /* default timeout in seconds */
coap_tick_t max_wait;                   /* global timeout (changed by set_timeout()) */

unsigned int obs_seconds = 30;          /* default observe time */
coap_tick_t obs_wait = 0;               /* timeout for current subscription */
int observe = 0;                        /* set to 1 if resource is being observed */

u32_t total_file_size = 0;

coap_incoming_data_cb_t coap_data_handler;
void *coap_data_handler_param;
#define min(a,b) ((a) < (b) ? (a) : (b))

#ifndef UNUSED_PARAM
#ifdef __GNUC__
#define UNUSED_PARAM __attribute__ ((unused))
#else /* not a GCC */
#define UNUSED_PARAM
#endif /* GCC */
#endif

static inline void
set_timeout(coap_tick_t *timer, const unsigned int seconds) {
  *timer = seconds * 1000;
}

static int
append_to_output(const unsigned char *data, size_t len, int num) {
//    size_t written;
#if 0
  if (!file) {
    if (!output_file.s || (output_file.length && output_file.s[0] == '-'))
      file = stdout;
    else {
      if (!(file = fopen((char *)output_file.s, "w"))) {
        perror("fopen");
        return -1;
      }
    }
  }

  do {
    written = fwrite(data, 1, len, file);
    len -= written;
    data += written;
  } while ( written && len );
  fflush(file);
#else
    if (coap_data_handler != NULL)
        coap_data_handler(coap_data_handler_param,data,len,num);
#endif
  return 0;
}

static void
close_output(void) {
  if (file) {

    /* add a newline before closing in case were writing to stdout */
    if (!output_file.s || (output_file.length && output_file.s[0] == '-'))
      fwrite("\n", 1, 1, file);

    fflush(file);
    fclose(file);
  }
}

static int
order_opts(void *a, void *b) {
  coap_option *o1, *o2;

  if (!a || !b)
    return a < b ? -1 : 1;

  o1 = (coap_option *)(((coap_list_t *)a)->data);
  o2 = (coap_option *)(((coap_list_t *)b)->data);

  return (COAP_OPTION_KEY(*o1) < COAP_OPTION_KEY(*o2))
    ? -1
    : (COAP_OPTION_KEY(*o1) != COAP_OPTION_KEY(*o2));
}

static coap_pdu_t *
coap_new_request(coap_context_t *ctx,
                 method_t m,
                 coap_list_t **options,
                 unsigned char *data,
                 size_t length) {
  coap_pdu_t *pdu;
  coap_list_t *opt;

  if ( ! ( pdu = coap_new_pdu() ) )
    return NULL;

  pdu->hdr->type = msgtype;
  pdu->hdr->id = coap_new_message_id(ctx);
  pdu->hdr->code = m;

  pdu->hdr->token_length = the_token.length;
  if ( !coap_add_token(pdu, the_token.length, the_token.s)) {
    debug("cannot add token to request\n");
  }

  coap_show_pdu(pdu);

  if (options) {
    /* sort options for delta encoding */
    LL_SORT((*options), order_opts);

    LL_FOREACH((*options), opt) {
      coap_option *o = (coap_option *)(opt->data);
      coap_add_option(pdu,
                      COAP_OPTION_KEY(*o),
                      COAP_OPTION_LENGTH(*o),
                      COAP_OPTION_DATA(*o));
    }
  }

  if (length) {
    if ((flags & FLAGS_BLOCK) == 0)
      coap_add_data(pdu, length, data);
    else
      coap_add_block(pdu, length, data, block.num, block.szx);
  }

  return pdu;
}

static coap_tid_t
clear_obs(coap_context_t *ctx,
          const coap_endpoint_t *local_interface,
          const coap_address_t *remote) {
  coap_pdu_t *pdu;
  coap_list_t *option;
  coap_tid_t tid = COAP_INVALID_TID;
  unsigned char buf[2];

  /* create bare PDU w/o any option  */
  pdu = coap_pdu_init(msgtype,
                      COAP_REQUEST_GET,
                      coap_new_message_id(ctx),
                      COAP_MAX_PDU_SIZE);

  if (!pdu) {
    return tid;
  }

  if (!coap_add_token(pdu, the_token.length, the_token.s)) {
    coap_log(LOG_CRIT, "cannot add token");
    goto error;
  }

  for (option = optlist; option; option = option->next ) {
    coap_option *o = (coap_option *)(option->data);
    if (COAP_OPTION_KEY(*o) == COAP_OPTION_URI_HOST) {
      if (!coap_add_option(pdu,
          COAP_OPTION_KEY(*o),
          COAP_OPTION_LENGTH(*o),
          COAP_OPTION_DATA(*o))) {
        goto error;
      }
      break;
    }
  }

  if (!coap_add_option(pdu,
      COAP_OPTION_OBSERVE,
      coap_encode_var_bytes(buf, COAP_OBSERVE_CANCEL),
      buf)) {
    coap_log(LOG_CRIT, "cannot add option Observe: %u", COAP_OBSERVE_CANCEL);
    goto error;
  }

  for (option = optlist; option; option = option->next ) {
    coap_option *o = (coap_option *)(option->data);
    switch (COAP_OPTION_KEY(*o)) {
    case COAP_OPTION_URI_PORT :
    case COAP_OPTION_URI_PATH :
    case COAP_OPTION_URI_QUERY :
      if (!coap_add_option (pdu,
                            COAP_OPTION_KEY(*o),
                            COAP_OPTION_LENGTH(*o),
                            COAP_OPTION_DATA(*o))) {
        goto error;
      }
      break;
      default:
      ;
    }
  }

  coap_show_pdu(pdu);

  if (pdu->hdr->type == COAP_MESSAGE_CON)
    tid = coap_send_confirmed(ctx, local_interface, remote, pdu);
  else
    tid = coap_send(ctx, local_interface, remote, pdu);

  if (tid == COAP_INVALID_TID) {
    debug("clear_obs: error sending new request");
    coap_delete_pdu(pdu);
  } else if (pdu->hdr->type != COAP_MESSAGE_CON)
    coap_delete_pdu(pdu);

  return tid;
 error:

  coap_delete_pdu(pdu);
  return tid;
}

static int
resolve_address(const str *server, struct sockaddr *dst) {

  struct addrinfo *res, *ainfo;
  struct addrinfo hints;
  static char addrstr[256];
  int error, len=-1;

  memset(addrstr, 0, sizeof(addrstr));
  if (server->length)
    memcpy(addrstr, server->s, server->length);
  else
    memcpy(addrstr, "localhost", 9);

  memset ((char *)&hints, 0, sizeof(hints));
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_family = AF_UNSPEC;

  error = getaddrinfo(addrstr, NULL, &hints, &res);

  if (error != 0) {
    fprintf(stderr, "getaddrinfo: %d\n", error);
    return -1;
  }

  for (ainfo = res; ainfo != NULL; ainfo = ainfo->ai_next) {
    switch (ainfo->ai_family) {
    case AF_INET6:
    case AF_INET:
      len = ainfo->ai_addrlen;
      memcpy(dst, ainfo->ai_addr, len);
      goto finish;
    default:
      ;
    }
  }

 finish:
  freeaddrinfo(res);
  return len;
}

#define HANDLE_BLOCK1(Pdu)                                        \
  ((method == COAP_REQUEST_PUT || method == COAP_REQUEST_POST) && \
   ((flags & FLAGS_BLOCK) == 0) &&                                \
   ((Pdu)->hdr->code == COAP_RESPONSE_CODE(201) ||                \
    (Pdu)->hdr->code == COAP_RESPONSE_CODE(204)))

static inline int
check_token(coap_pdu_t *received) {
  return received->hdr->token_length == the_token.length &&
    memcmp(received->hdr->token, the_token.s, the_token.length) == 0;
}

static void
message_handler(struct coap_context_t *ctx,
                const coap_endpoint_t *local_interface,
                const coap_address_t *remote,
                coap_pdu_t *sent,
                coap_pdu_t *received,
                const coap_tid_t id UNUSED_PARAM) {

  coap_pdu_t *pdu = NULL;
  coap_opt_t *block_opt;
  coap_opt_iterator_t opt_iter;
  unsigned char buf[4];
  coap_list_t *option;
  size_t len;
  unsigned char *databuf;
  coap_tid_t tid;

#ifndef NDEBUG
  if (LOG_DEBUG <= coap_get_log_level()) {
    debug("** process incoming %d.%02d response:\n",
          (received->hdr->code >> 5), received->hdr->code & 0x1F);
    coap_show_pdu(received);
  }
#endif

  /* check if this is a response to our original request */
  if (!check_token(received)) {
    /* drop if this was just some message, or send RST in case of notification */
    if (!sent && (received->hdr->type == COAP_MESSAGE_CON ||
                  received->hdr->type == COAP_MESSAGE_NON))
      coap_send_rst(ctx, local_interface, remote, received);
    return;
  }

  if (received->hdr->type == COAP_MESSAGE_RST) {
    info("got RST\n");
    return;
  }

  /* output the received data, if any */
  if (COAP_RESPONSE_CLASS(received->hdr->code) == 2) {

    /* set obs timer if we have successfully subscribed a resource */
    if (sent && coap_check_option(received, COAP_OPTION_SUBSCRIPTION, &opt_iter)) {
      debug("observation relationship established, set timeout to %d\n", obs_seconds);
      set_timeout(&obs_wait, obs_seconds);
      observe = 1;
    }

    /* Got some data, check if block option is set. Behavior is undefined if
     * both, Block1 and Block2 are present. */
    block_opt = coap_check_option(received, COAP_OPTION_BLOCK2, &opt_iter);
    if (block_opt) { /* handle Block2 */
      unsigned short blktype = opt_iter.type;

      /* TODO: check if we are looking at the correct block number */
    if (coap_get_data(received, &len, &databuf)) {
        debug("output get data!!!\n");

        int num = coap_opt_block_num(block_opt);
        debug("output get data!!!,num=%d\n",num);
        append_to_output(databuf, len, num);
    }
      if(COAP_OPT_BLOCK_MORE(block_opt)) {
        /* more bit is set */
        debug("found the M bit, block size is %u, block nr. %u\n",
              COAP_OPT_BLOCK_SZX(block_opt),
              coap_opt_block_num(block_opt));

        /* create pdu with request for next block */
        pdu = coap_new_request(ctx, method, NULL, NULL, 0); /* first, create bare PDU w/o any option  */
        if ( pdu ) {
          /* add URI components from optlist */
          for (option = optlist; option; option = option->next ) {
            coap_option *o = (coap_option *)(option->data);
            switch (COAP_OPTION_KEY(*o)) {
              case COAP_OPTION_URI_HOST :
              case COAP_OPTION_URI_PORT :
              case COAP_OPTION_URI_PATH :
              case COAP_OPTION_URI_QUERY :
                coap_add_option (pdu,
                                 COAP_OPTION_KEY(*o),
                                 COAP_OPTION_LENGTH(*o),
                                 COAP_OPTION_DATA(*o));
                break;
              default:
                ;     /* skip other options */
            }
          }

          /* finally add updated block option from response, clear M bit */
          /* blocknr = (blocknr & 0xfffffff7) + 0x10; */
          debug("query block %d\n", (coap_opt_block_num(block_opt) + 1));
          coap_add_option(pdu,
                          blktype,
                          coap_encode_var_bytes(buf,
                                 ((coap_opt_block_num(block_opt) + 1) << 4) |
                                  COAP_OPT_BLOCK_SZX(block_opt)), buf);

          if (pdu->hdr->type == COAP_MESSAGE_CON)
            tid = coap_send_confirmed(ctx, local_interface, remote, pdu);
          else
            tid = coap_send(ctx, local_interface, remote, pdu);

          if (tid == COAP_INVALID_TID) {
            debug("message_handler: error sending new request");
            coap_delete_pdu(pdu);
          } else {
            //set_timeout(&max_wait, wait_seconds);
            if (pdu->hdr->type != COAP_MESSAGE_CON)
              coap_delete_pdu(pdu);
          }

          return;
        }
      }
    } else { /* no Block2 option */
      block_opt = coap_check_option(received, COAP_OPTION_BLOCK1, &opt_iter);
      debug("no more blockblock_opt=%d\n",block_opt);

      if (block_opt) { /* handle Block1 */
        unsigned int szx = COAP_OPT_BLOCK_SZX(block_opt);
//        unsigned int num = coap_opt_block_num(block_opt);
        debug("found Block1 option, block size is %u, block nr. %u\n", szx, num);
        if (szx != block.szx) {
          unsigned int bytes_sent = ((block.num + 1) << (block.szx + 4));
          if (bytes_sent % (1 << (szx + 4)) == 0) {
            /* Recompute the block number of the previous packet given the new block size */
            block.num = (bytes_sent >> (szx + 4)) - 1;
            block.szx = szx;
            debug("new Block1 size is %u, block number %u completed\n", (1 << (block.szx + 4)), block.num);
          } else {
            debug("ignoring request to increase Block1 size, "
            "next block is not aligned on requested block size boundary. "
            "(%u x %u mod %u = %u != 0)\n",
                  block.num + 1, (1 << (block.szx + 4)), (1 << (szx + 4)),
                  bytes_sent % (1 << (szx + 4)));
          }
        }

        if (payload.length <= (block.num+1) * (1 << (block.szx + 4))) {
          debug("upload ready\n");
          ready = 1;
          return;
        }

        /* create pdu with request for next block */
        pdu = coap_new_request(ctx, method, NULL, NULL, 0); /* first, create bare PDU w/o any option  */
        if (pdu) {

          /* add URI components from optlist */
          for (option = optlist; option; option = option->next ) {
            coap_option *o = (coap_option *)(option->data);
            switch (COAP_OPTION_KEY(*o)) {
              case COAP_OPTION_URI_HOST :
              case COAP_OPTION_URI_PORT :
              case COAP_OPTION_URI_PATH :
              case COAP_OPTION_CONTENT_FORMAT :
              case COAP_OPTION_URI_QUERY :
                coap_add_option (pdu,
                                 COAP_OPTION_KEY(*o),
                                 COAP_OPTION_LENGTH(*o),
                                 COAP_OPTION_DATA(*o));
                break;
              default:
              ;     /* skip other options */
            }
          }

          /* finally add updated block option from response, clear M bit */
          /* blocknr = (blocknr & 0xfffffff7) + 0x10; */
          block.num++;
          block.m = ((block.num+1) * (1 << (block.szx + 4)) < payload.length);

          debug("send block %d\n", block.num);
          coap_add_option(pdu,
                          COAP_OPTION_BLOCK1,
                          coap_encode_var_bytes(buf,
                          (block.num << 4) | (block.m << 3) | block.szx), buf);

          coap_add_block(pdu,
                         payload.length,
                         payload.s,
                         block.num,
                         block.szx);
          coap_show_pdu(pdu);
          if (pdu->hdr->type == COAP_MESSAGE_CON)
            tid = coap_send_confirmed(ctx, local_interface, remote, pdu);
          else
            tid = coap_send(ctx, local_interface, remote, pdu);

          if (tid == COAP_INVALID_TID) {
            debug("message_handler: error sending new request");
            coap_delete_pdu(pdu);
          } else {
            //set_timeout(&max_wait, wait_seconds);
            if (pdu->hdr->type != COAP_MESSAGE_CON)
              coap_delete_pdu(pdu);
          }

          return;
        }
      } else {
        /* There is no block option set, just read the data and we are done. */
        if (coap_get_data(received, &len, &databuf))
            append_to_output(databuf, len, 0);
      }
    }
  } else {      /* no 2.05 */

    /* check if an error was signaled and output payload if so */
    if (COAP_RESPONSE_CLASS(received->hdr->code) >= 4) {
        char errbuf[11];
        memset(errbuf, 0, sizeof(errbuf));
        sprintf(errbuf, "error %d.%02d", (received->hdr->code >> 5), received->hdr->code & 0x1F);
        append_to_output((const unsigned char *)errbuf, strlen(errbuf), 0);

      fprintf(stderr, "%d.%02d",
              (received->hdr->code >> 5), received->hdr->code & 0x1F);
      if (coap_get_data(received, &len, &databuf)) {
        fprintf(stderr, " ");
        while(len--)
        fprintf(stderr, "%c", *databuf++);
      }
      fprintf(stderr, "\n");
    }

  }

  /* finally send new request, if needed */
  if (pdu && coap_send(ctx, local_interface, remote, pdu) == COAP_INVALID_TID) {
    debug("message_handler: error sending response");
  }
  coap_delete_pdu(pdu);

  /* our job is done, we can exit at any time */
  ready = coap_check_option(received, COAP_OPTION_SUBSCRIPTION, &opt_iter) == NULL;
  debug("coap_check_option done");
}

static void
usage( const char *program, const char *version) {
  const char *p;

  p = strrchr( program, '/' );
  if ( p )
    program = ++p;

  fprintf( stderr, "%s v%s -- a small CoAP implementation\n"
     "(c) 2010-2015 Olaf Bergmann <bergmann@tzi.org>\n\n"
     "usage: %s [-A type...] [-t type] [-b [num,]size] [-B seconds] [-e text]\n"
     "\t\t[-m method] [-N] [-o file] [-P addr[:port]] [-p port]\n"
     "\t\t[-s duration] [-O num,text] [-T string] [-v num] [-a addr] \n\n"
     "\t\t[-u user] [-k key] URI\n\n"
     "\tURI can be an absolute or relative coap URI,\n"
     "\t-a addr\tthe local interface address to use\n"
     "\t-A type...\taccepted media types as comma-separated list of\n"
     "\t\t\tsymbolic or numeric values\n"
     "\t-t type\t\tcontent format for given resource for PUT/POST\n"
     "\t-b [num,]size\tblock size to be used in GET/PUT/POST requests\n"
     "\t       \t\t(value must be a multiple of 16 not larger than 1024)\n"
     "\t       \t\tIf num is present, the request chain will start at\n"
     "\t       \t\tblock num\n"
     "\t-B seconds\tbreak operation after waiting given seconds\n"
     "\t\t\t(default is %d)\n"
     "\t-e text\t\tinclude text as payload (use percent-encoding for\n"
     "\t\t\tnon-ASCII characters)\n"
     "\t-f file\t\tfile to send with PUT/POST (use '-' for STDIN)\n"
     "\t-k key\t\tPre-shared key for the specified user. This argument\n"
     "\t       \t\trequires DTLS with PSK to be available.\n"
     "\t-m method\trequest method (get|put|post|delete), default is 'get'\n"
     "\t-N\t\tsend NON-confirmable message\n"
     "\t-o file\t\toutput received data to this file (use '-' for STDOUT)\n"
     "\t-p port\t\tlisten on specified port\n"
     "\t-s duration\tsubscribe for given duration [s]\n"
     "\t-u user\t\tuser identity for pre-shared key mode. This argument\n"
     "\t       \t\trequires DTLS with PSK to be available.\n"
     "\t-v num\t\tverbosity level (default: 3)\n"
     "\t-O num,text\tadd option num with contents text to request\n"
     "\t-P addr[:port]\tuse proxy (automatically adds Proxy-Uri option to\n"
     "\t\t\trequest)\n"
     "\t-T token\tinclude specified token\n"
     "\n"
     "examples:\n"
     "\tcoap-client -m get coap://[::1]/\n"
     "\tcoap-client -m get coap://[::1]/.well-known/core\n"
     "\tcoap-client -m get -T cafe coap://[::1]/time\n"
     "\techo 1000 | coap-client -m put -T cafe coap://[::1]/time -f -\n"
     ,program, version, program, wait_seconds);
}

static coap_list_t *
new_option_node(unsigned short key, unsigned int length, unsigned char *data) {
  coap_list_t *node;

  node = coap_malloc(sizeof(coap_list_t) + sizeof(coap_option) + length);

  if (node) {
    coap_option *option;
    option = (coap_option *)(node->data);
    COAP_OPTION_KEY(*option) = key;
    COAP_OPTION_LENGTH(*option) = length;
    memcpy(COAP_OPTION_DATA(*option), data, length);
  } else {
    coap_log(LOG_DEBUG, "new_option_node: malloc\n");
  }

  return node;
}

typedef struct {
  unsigned char code;
  char *media_type;
} content_type_t;

static void
cmdline_content_type(char *arg, unsigned short key) {
  static content_type_t content_types[] = {
    {  0, "plain" },
    {  0, "text/plain" },
    { 40, "link" },
    { 40, "link-format" },
    { 40, "application/link-format" },
    { 41, "xml" },
    { 41, "application/xml" },
    { 42, "binary" },
    { 42, "octet-stream" },
    { 42, "application/octet-stream" },
    { 47, "exi" },
    { 47, "application/exi" },
    { 50, "json" },
    { 50, "application/json" },
    { 60, "cbor" },
    { 60, "application/cbor" },
    { 255, NULL }
  };
  coap_list_t *node;
  unsigned char i, value[10];
  int valcnt = 0;
  unsigned char buf[2];
  char *p, *q = arg;

  while (q && *q) {
    p = strchr(q, ',');

    if (isdigit((unsigned char)*q)) {   
      if (p)
        *p = '\0';
      value[valcnt++] = atoi(q);
    } else {
      for (i=0;
           content_types[i].media_type &&
           strncmp(q, content_types[i].media_type, p ? (size_t)(p-q) : strlen(q)) != 0 ;
           ++i)
      ;

      if (content_types[i].media_type) {
        value[valcnt] = content_types[i].code;
        valcnt++;
      } else {
        warn("W: unknown content-format '%s'\n",arg);
      }
    }

    if (!p || key == COAP_OPTION_CONTENT_TYPE)
      break;

    q = p+1;
  }

  for (i = 0; i < valcnt; ++i) {
    node = new_option_node(key, coap_encode_var_bytes(buf, value[i]), buf);
    if (node) {
      LL_PREPEND(optlist, node);
    }
  }
}

static unsigned short
get_default_port(const coap_uri_t *u) {
  return coap_uri_scheme_is_secure(u) ? COAPS_DEFAULT_PORT : COAP_DEFAULT_PORT;
}

static int
cmdline_uri(char *arg) {
  unsigned char portbuf[2];
#define BUFSIZE 40
  unsigned char _buf[BUFSIZE];
  unsigned char *buf = _buf;
  size_t buflen;
  int res;

  if (proxy.length) {   /* create Proxy-Uri from argument */
    size_t len = strlen(arg);
    while (len > 270) {
      coap_insert(&optlist,
                  new_option_node(COAP_OPTION_PROXY_URI,
                  270,
                  (unsigned char *)arg));

      len -= 270;
      arg += 270;
    }

    coap_insert(&optlist,
                new_option_node(COAP_OPTION_PROXY_URI,
                len,
                (unsigned char *)arg));

  } else {      /* split arg into Uri-* options */
    if (coap_split_uri((unsigned char *)arg, strlen(arg), &uri) < 0) {
      return -1;
    }

    if (uri.port != get_default_port(&uri)) {
      coap_insert(&optlist,
                  new_option_node(COAP_OPTION_URI_PORT,
                  coap_encode_var_bytes(portbuf, uri.port),
                  portbuf));
    }

    if (uri.path.length) {
      buflen = BUFSIZE;
      res = coap_split_path(uri.path.s, uri.path.length, buf, &buflen);

      while (res--) {
        coap_insert(&optlist,
                    new_option_node(COAP_OPTION_URI_PATH,
                    COAP_OPT_LENGTH(buf),
                    COAP_OPT_VALUE(buf)));

        buf += COAP_OPT_SIZE(buf);
      }
    }

    if (uri.query.length) {
      buflen = BUFSIZE;
      buf = _buf;
      res = coap_split_query(uri.query.s, uri.query.length, buf, &buflen);

      while (res--) {
        coap_insert(&optlist,
                    new_option_node(COAP_OPTION_URI_QUERY,
                    COAP_OPT_LENGTH(buf),
                    COAP_OPT_VALUE(buf)));

        buf += COAP_OPT_SIZE(buf);
      }
    }
  }

  return 0;
}

#if 0
static int
cmdline_blocksize(char *arg) {
  unsigned short size;

  again:
  size = 0;
  while(*arg && *arg != ',')
    size = size * 10 + (*arg++ - '0');

  if (*arg == ',') {
    arg++;
    block.num = size;
    goto again;
  }

  if (size)
    block.szx = (coap_fls(size >> 4) - 1) & 0x07;

  flags |= FLAGS_BLOCK;
  return 1;
}
#endif
/* Called after processing the options from the commandline to set
 * Block1 or Block2 depending on method. */
static void
set_blocksize(void) {
  static unsigned char buf[4];	/* hack: temporarily take encoded bytes */
  unsigned short opt;
  unsigned int opt_length;

  if (method != COAP_REQUEST_DELETE) {
    opt = method == COAP_REQUEST_GET ? COAP_OPTION_BLOCK2 : COAP_OPTION_BLOCK1;

    block.m = (opt == COAP_OPTION_BLOCK1) &&
      ((1u << (block.szx + 4)) < payload.length);

    opt_length = coap_encode_var_bytes(buf,
          (block.num << 4 | block.m << 3 | block.szx));

    coap_insert(&optlist, new_option_node(opt, opt_length, buf));
  }
}

#if 0
static void
cmdline_subscribe(char *arg) {
  obs_seconds = atoi(arg);
  coap_insert(&optlist, new_option_node(COAP_OPTION_SUBSCRIPTION, 0, NULL));
}

static int
cmdline_proxy(char *arg) {
  char *proxy_port_str = strrchr((const char *)arg, ':'); /* explicit port ? */
  if (proxy_port_str) {
    char *ipv6_delimiter = strrchr((const char *)arg, ']');
    if (!ipv6_delimiter) {
      if (proxy_port_str == strchr((const char *)arg, ':')) {
        /* host:port format - host not in ipv6 hexadecimal string format */
        *proxy_port_str++ = '\0'; /* split */
        proxy_port = atoi(proxy_port_str);
      }
    } else {
      arg = strchr((const char *)arg, '[');
      if (!arg) return 0;
      arg++;
      *ipv6_delimiter = '\0'; /* split */
      if (ipv6_delimiter + 1 == proxy_port_str++) {
        /* [ipv6 address]:port */
        proxy_port = atoi(proxy_port_str);
      }
    }
  }

  proxy.length = strlen(arg);
  if ( (proxy.s = coap_malloc(proxy.length + 1)) == NULL) {
    proxy.length = 0;
    return 0;
  }

  memcpy(proxy.s, arg, proxy.length+1);
  return 1;
}
#endif

static inline void
cmdline_token(char *arg) {
  strncpy((char *)the_token.s, arg, min(sizeof(_token_data), strlen(arg)));
  the_token.length = strlen(arg);
}

#if 0
static void
cmdline_option(char *arg) {
  unsigned int num = 0;

  while (*arg && *arg != ',') {
    num = num * 10 + (*arg - '0');
    ++arg;
  }
  if (*arg == ',')
    ++arg;

  coap_insert(&optlist,
              new_option_node(num, strlen(arg), (unsigned char *)arg));
}
#endif
/**
 * Calculates decimal value from hexadecimal ASCII character given in
 * @p c. The caller must ensure that @p c actually represents a valid
 * heaxdecimal character, e.g. with isxdigit(3).
 *
 * @hideinitializer
 */
#define hexchar_to_dec(c) ((c) & 0x40 ? ((c) & 0x0F) + 9 : ((c) & 0x0F))

/**
 * Decodes percent-encoded characters while copying the string @p seg
 * of size @p length to @p buf. The caller of this function must
 * ensure that the percent-encodings are correct (i.e. the character
 * '%' is always followed by two hex digits. and that @p buf provides
 * sufficient space to hold the result. This function is supposed to
 * be called by make_decoded_option() only.
 *
 * @param seg     The segment to decode and copy.
 * @param length  Length of @p seg.
 * @param buf     The result buffer.
 */
static void
decode_segment(const unsigned char *seg, size_t length, unsigned char *buf) {

  while (length--) {

    if (*seg == '%') {
      *buf = (hexchar_to_dec(seg[1]) << 4) + hexchar_to_dec(seg[2]);

      seg += 2; length -= 2;
    } else {
      *buf = *seg;
    }

    ++buf; ++seg;
  }
}

/**
 * Runs through the given path (or query) segment and checks if
 * percent-encodings are correct. This function returns @c -1 on error
 * or the length of @p s when decoded.
 */
static int
check_segment(const unsigned char *s, size_t length) {

  size_t n = 0;

  while (length) {
    if (*s == '%') {
      if (length < 2 || !(isxdigit(s[1]) && isxdigit(s[2])))
        return -1;

      s += 2;
      length -= 2;
    }

    ++s; ++n; --length;
  }

  return n;
}

static int
cmdline_input(char *text, str *buf) {
  int len;
  len = check_segment((unsigned char *)text, strlen(text));

  if (len < 0)
    return 0;

  buf->s = (unsigned char *)coap_malloc(len);
  if (!buf->s)
    return 0;

  buf->length = len;
  decode_segment((unsigned char *)text, strlen(text), buf->s);
  return 1;
}

#if 0
static int
cmdline_input_from_file(char *filename, str *buf) {
return 1;

  FILE *inputfile = NULL;
  ssize_t len;
  int result = 1;
  struct stat statbuf;

  if (!filename || !buf)
    return 0;

  if (filename[0] == '-' && !filename[1]) { /* read from stdin */
    buf->length = 20000;
    buf->s = (unsigned char *)coap_malloc(buf->length);
    if (!buf->s)
      return 0;

    inputfile = stdin;
  } else {
    /* read from specified input file */
    inputfile = fopen(filename, "r");
    if ( !inputfile ) {
      perror("cmdline_input_from_file: fopen");
      return 0;
    }

    if (fstat(fileno(inputfile), &statbuf) < 0) {
      perror("cmdline_input_from_file: stat");
      fclose(inputfile);
      return 0;
    }

    buf->length = statbuf.st_size;
    buf->s = (unsigned char *)coap_malloc(buf->length);
    if (!buf->s) {
      fclose(inputfile);
      return 0;
    }
  }

  len = fread(buf->s, 1, buf->length, inputfile);

  if (len < 0 || ((size_t)len < buf->length)) {
    if (ferror(inputfile) != 0) {
      perror("cmdline_input_from_file: fread");
      coap_free(buf->s);
      buf->length = 0;
      buf->s = NULL;
      result = 0;
    } else {
      buf->length = len;
    }
  }

  if (inputfile != stdin)
    fclose(inputfile);

  return result;

}
#endif


static method_t
cmdline_method(char *arg) {
  static char *methods[] =
    { 0, "get", "post", "put", "delete", 0};
  unsigned char i;

  for (i=1; methods[i] && strcasecmp(arg,methods[i]) != 0 ; ++i)
    ;

  return i;     /* note that we do not prevent illegal methods */
}

static ssize_t
cmdline_read_user(char *arg, unsigned char *buf, size_t maxlen) {
  size_t len = strnlen(arg, maxlen);
  if (len) {
    memcpy(buf, arg, len);
    return len;
  }
  return -1;
}

static ssize_t
cmdline_read_key(char *arg, unsigned char *buf, size_t maxlen) {
  size_t len = strnlen(arg, maxlen);
  if (len) {
    memcpy(buf, arg, len);
    return len;
  }
  return -1;
}

static coap_context_t *
get_context(const char *node, const char *port, int secure) {
  coap_context_t *ctx = NULL;
  int s;
  struct addrinfo hints;
  struct addrinfo *result, *rp;
  int ep_type;

  ctx = coap_new_context(NULL);
  if (!ctx) {
    coap_log(LOG_ERR, "func[%s()] line[%d]:coap_new_context() return error.\n", __func__, __LINE__);
    return NULL;
  }

  ep_type = secure ? COAP_ENDPOINT_DTLS : COAP_ENDPOINT_NOSEC;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
  hints.ai_socktype = SOCK_DGRAM; /* Coap uses UDP */
  hints.ai_flags = AI_PASSIVE | AI_NUMERICHOST | AI_NUMERICSERV | AI_ALL;

  s = getaddrinfo(node, port, &hints, &result);
  if ( s != 0 ) {
    coap_log(LOG_ERR, "func[%s()] line[%d]:getaddrinfo() return s=%d, errno=%d.\n", __func__, __LINE__, s, errno);
    coap_free_context(ctx);
	ctx = NULL;
    return NULL;
  }

  /* iterate through results until success */
  for (rp = result; rp != NULL; rp = rp->ai_next) {
    coap_address_t addr;
    coap_endpoint_t *endpoint;

    if (rp->ai_addrlen <= sizeof(addr.addr)) {
      coap_address_init(&addr);
      addr.size = rp->ai_addrlen;
      memcpy(&addr.addr, rp->ai_addr, rp->ai_addrlen);

      endpoint = coap_new_endpoint(&addr, ep_type);
      if (endpoint) {
        coap_attach_endpoint(ctx, endpoint);
        goto finish;
      } else {
        coap_log(LOG_CRIT, "func[%s()] line[%d]:cannot create endpoint.\n", __func__, __LINE__);
        continue;
      }
    }
  }

  coap_log(LOG_ERR, "func[%s()] line[%d]:no context available for interface '%s'.\n", __func__, __LINE__, node);
  coap_free_context(ctx);
  ctx = NULL;

 finish:
  freeaddrinfo(result);
  return ctx;
}

static void coap_client_dinit()
{
    flags = 0;
    optlist = NULL;
    memset(&uri,0,sizeof(uri));
    proxy.length = 0;
    proxy.s = NULL;
    proxy_port = COAP_DEFAULT_PORT;
    
    /* reading is done when this flag is set */
    ready = 0;
        
    payload.length =0;
    if(payload.s != NULL)
        coap_free(payload.s);
    payload.s = NULL;
    
    msgtype = COAP_MESSAGE_CON; /* usually, requests are sent confirmable */
    
    method = 1;                    /* the method we are using in our requests */
    
    block.num =0;
    block.m = 0;
    block.szx = 6;
    
    wait_seconds = 90;         /* default timeout in seconds */
    max_wait = 0;                   /* global timeout (changed by set_timeout()) */

    obs_seconds = 30;          /* default observe time */
    obs_wait = 0;               /* timeout for current subscription */
    total_file_size = 0;
    coap_data_handler = NULL;
    coap_data_handler_param = NULL;
}

static void getoptrst(void)
{
    static char times = 1;
    if (times > 1)
    {
        opterr = 1;
        optind = 1;
        optopt = 0;
        optarg = NULL;
    }

    times++;
    if (times > 127)
    {
        times = 2;
    }

	return ;
}

int
coap_client_main(int argc, char **argv) {
  coap_context_t  *ctx = NULL;
  coap_address_t dst;
#if defined(LWIP_IPV4_ON) && defined(LWIP_IPV6_ON)
  static char addr[INET6_ADDRSTRLEN];
#else
#ifdef LWIP_IPV4_ON
  static char addr[INET_ADDRSTRLEN];
#else
  static char addr[INET6_ADDRSTRLEN];
#endif
#endif
  void *addrptr = NULL;
  int result = -1;
  coap_pdu_t  *pdu;
  static str server;
  unsigned short port = COAP_DEFAULT_PORT;
  char port_str[NI_MAXSERV] = "0";
  char node_str[NI_MAXHOST] = "";
  int opt, res;
  coap_log_t log_level = LOG_DEBUG;
  coap_tid_t tid = COAP_INVALID_TID;
  unsigned char user[MAX_USER], key[MAX_KEY];
  ssize_t user_length = 0, key_length = 0;
  //char cmdline[255]="";
  //for (int i=0;i<argc;i++){
  //  strcat(cmdline,argv[i]);
  //  strcat(cmdline," ");
  //}

  coap_clock_init();
  coap_tick_t start, now;
  coap_ticks(&start);

  //coap_log(LOG_INFO, "fun[%s()] line[%d]:cmdline=%s\n", __func__, __LINE__, cmdline);

  getoptrst();

  //Rda haven't param so much.
  //while ((opt = getopt(argc, argv, "Na:b:e:f:g:k:m:p:s:t:o:v:A:B:O:P:T:u:")) != -1) {
  //while ((opt = getopt(argc, argv, "Na:b:e:f:g:k:m:p:s:t:v:A:B:O:P:T:u:")) != -1) {
    while ((opt = getopt(argc, argv, "k:m:p:t:B:u:")) != -1) {
    switch (opt) {
    case 'k' :
      key_length = cmdline_read_key(optarg, key, MAX_KEY);
      break;
    case 'm' :
      method = cmdline_method(optarg);
      break;
    case 'p' :
      strncpy(port_str, optarg, NI_MAXSERV-1);
      port_str[NI_MAXSERV - 1] = '\0';
      break;
    case 't' :
      cmdline_content_type(optarg,COAP_OPTION_CONTENT_TYPE);
      break;
    case 'B' :
      wait_seconds = atoi(optarg);
      break;  
    case 'u' :
      user_length = cmdline_read_user(optarg, user, MAX_USER);
      break;
    default:
      usage( argv[0], PACKAGE_VERSION );
      exit( 1 );
    	}
    }
#if 0
    case 'a' :
      strncpy(node_str, optarg, NI_MAXHOST-1);
      node_str[NI_MAXHOST - 1] = '\0';
      break;
//    case 'b' :
//     cmdline_blocksize(optarg);
      break;
    case 'B' :
      wait_seconds = atoi(optarg);
      break;
    case 'e' :
      if (!cmdline_input(optarg,&payload))
        payload.length = 0;
      break;
    case 'f' :
      if (!cmdline_input_from_file(optarg,&payload))
        payload.length = 0;
      break;
    case 'k' :
      key_length = cmdline_read_key(optarg, key, MAX_KEY);
      break;
    case 'p' :
      strncpy(port_str, optarg, NI_MAXSERV-1);
      port_str[NI_MAXSERV - 1] = '\0';
      break;
    case 'm' :
      method = cmdline_method(optarg);
      break;
    case 'N' :
      msgtype = COAP_MESSAGE_NON;
      break;
    case 's' :
      cmdline_subscribe(optarg);
      break;
    //case 'o' :
      //output_file.length = strlen(optarg);
      //output_file.s = (unsigned char *)coap_malloc(output_file.length + 1);

      //if (!output_file.s) {
       //fprintf(stderr, "cannot set output file: insufficient memory\n");
        //exit(-1);
      //} else {
        /* copy filename including trailing zero */
        //memcpy(output_file.s, optarg, output_file.length + 1);
      //}
      //break;
    case 'A' :
      cmdline_content_type(optarg,COAP_OPTION_ACCEPT);
      break;
    case 't' :
      cmdline_content_type(optarg,COAP_OPTION_CONTENT_TYPE);
      break;
    case 'O' :
      cmdline_option(optarg);
      break;
    case 'P' :
      if (!cmdline_proxy(optarg)) {
        fprintf(stderr, "error specifying proxy address\n");
        exit(-1);
      }
      break;
    case 'T' :
      cmdline_token(optarg);
      break;
    case 'u' :
      user_length = cmdline_read_user(optarg, user, MAX_USER);
      break;
    case 'v' :
      log_level = strtol(optarg, NULL, 10);
      break;
    default:
      usage( argv[0], PACKAGE_VERSION );
      exit( 1 );
    }
  }
#endif

  coap_dtls_set_log_level(log_level);
  coap_set_log_level(log_level);

  if (optind < argc) {
    if (cmdline_uri(argv[optind]) < 0)
      coap_log(LOG_ERR, "invalid CoAP URI\n");
    if (coap_uri_scheme_is_secure(&uri) && !coap_dtls_is_supported()) {
      coap_log(LOG_EMERG, "coaps URI scheme not supported in this version of libcoap\n");
      exit(1);
    }
  } else {
    usage( argv[0], PACKAGE_VERSION );
    exit( 1 );
  }

  if (proxy.length) {
    server = proxy;
    port = proxy_port;
  } else {
    server = uri.host;
    port = uri.port;
  }

  /* resolve destination address where server should be sent */
  res = resolve_address(&server, &dst.addr.sa);

  if (res < 0) {
    fprintf(stderr, "failed to resolve address\n");
    exit(-1);
  }

  dst.size = res;
  dst.addr.sin.sin_port = htons(port);

  /* add Uri-Host if server address differs from uri.host */

  switch (dst.addr.sa.sa_family) {
  case AF_INET:
    addrptr = &dst.addr.sin.sin_addr;
    /* create context for IPv4 */
    ctx = get_context(node_str[0] == 0 ? "0.0.0.0" : node_str, port_str,
                      coap_uri_scheme_is_secure(&uri));//new coap_context,attch endport
    break;
#ifdef LWIP_IPV6_ON
  case AF_INET6:
    addrptr = &dst.addr.sin6.sin6_addr;

    /* create context for IPv6 */
    ctx = get_context(node_str[0] == 0 ? "::" : node_str, port_str,
                      coap_uri_scheme_is_secure(&uri));
    break;
#endif
  default:
    ;
  }

  if (!ctx) {
    coap_log(LOG_ERR, "func[%s()] line[%d]:cannot create context\n", __func__, __LINE__);
    goto finish;
  }

  if ((user_length < 0) || (key_length < 0)) {
    coap_log(LOG_CRIT, "Invalid user name or key specified\n");
    goto finish;
  }

  /* Create a new PSK item and add to keystore if talking to a secure
     resource. The user name or key may be empty. */
  if (coap_uri_scheme_is_secure(&uri)) {//dtls

        coap_keystore_item_t *psk;
        psk = coap_keystore_new_psk(NULL, 0,
                                    (user_length > 0) ? user : NULL,
                                    (size_t)user_length,
                                    (key_length > 0) ? key : NULL,
                                    (size_t)key_length, 0);
        if (!psk || !coap_keystore_store_item(ctx->keystore, psk, NULL)) {
            coap_log(LOG_WARNING, "cannot store key\n");//set and save keystore
        }
        mbedtls_timing_delay_context timer;
        if (0 != (result = coap_dtls_cfg_context(ctx,&dst,&timer)))//prepare to read/write:config mbedtls,connect socket and handshake
        {
            coap_log(LOG_WARNING, "coap_dtls_cfg_context failed,result=%d\n",result);
            result = -1;
            goto finish;
        }
    }
    coap_register_option(ctx, COAP_OPTION_BLOCK2);
    coap_register_response_handler(ctx, message_handler);

    /* construct CoAP message */
    if (!proxy.length && addrptr
        && (inet_ntop(dst.addr.sa.sa_family, addrptr, addr, sizeof(addr)) != 0)
        && (strlen(addr) != uri.host.length
        || memcmp(addr, uri.host.s, uri.host.length) != 0)) {
        /* add Uri-Host */

        coap_insert(&optlist,
                    new_option_node(COAP_OPTION_URI_HOST,
                    uri.host.length,
                    uri.host.s));
  }

  /* set block option if requested at commandline */
  if (flags & FLAGS_BLOCK)
    set_blocksize();

  if (! (pdu = coap_new_request(ctx, method, &optlist, payload.s, payload.length))) {
    goto finish;
  }

#ifndef NDEBUG
  if (LOG_DEBUG <= coap_get_log_level()) {
    debug("sending CoAP request:\n");
    coap_show_pdu(pdu);
  }
#endif

  if (pdu->hdr->type == COAP_MESSAGE_CON)
    tid = coap_send_confirmed(ctx, ctx->endpoint, &dst, pdu);
  else
    tid = coap_send(ctx, ctx->endpoint, &dst, pdu);

  if (pdu->hdr->type != COAP_MESSAGE_CON || tid == COAP_INVALID_TID)
    coap_delete_pdu(pdu);


  set_timeout(&max_wait, wait_seconds);
  debug("timeout is set to %d seconds\n", wait_seconds);

  coap_ticks(&now);
  max_wait -= (now - start);
  debug("real timeout is set to %d seconds\n", max_wait);
  if (observe)
    obs_wait -= (now - start);
  while (!(ready && coap_can_exit(ctx))) {
    unsigned int wait_ms = observe ? min(obs_wait, max_wait) : max_wait;
    result = coap_run_once(ctx, wait_ms);

    if (result >= 0) {
      if ((unsigned int)result <= obs_wait) {
        obs_wait -= result;
      } else if (observe) {
        debug("clear observation relationship\n");
        clear_obs(ctx, ctx->endpoint, &dst); /* FIXME: handle error case COAP_TID_INVALID */

        /* make sure that the obs timer does not fire again */
        obs_wait = 0;
        observe = 0;
      }

      coap_ticks(&now);
      if (start + wait_seconds * COAP_TICKS_PER_SECOND < now) {
        ready = 1;
        result = -1;
        goto finish;
      }

      if ((unsigned int)result < max_wait) {
        max_wait -= result;
      }

      if (COAP_INVALID_TID == coap_get_tid())
      {
          result = -1;
          coap_reset_tid();
      }
    }
  }


 finish:
  close_output();
  coap_delete_list(optlist);
  coap_free_context( ctx );
 exit:
  coap_client_dinit();
  debug("coap_client_main result = %d\n", result);
  return result;
}

u8_t coap_client_set_payload(void *data,unsigned int payload_length, unsigned char payload_type)
{
    if (payload_type == 0) {
        if (!cmdline_input((char *)data,&payload)) {
            payload.length = 0;
            return 0;
        }
        total_file_size = payload_length;
    } else if (payload_type == 1) {
        if (!payload.s){
            payload.s = (unsigned char *)coap_malloc(payload_length);
        }else{
            unsigned char *old_s = payload.s;
            payload.s = (unsigned char *)coap_malloc(payload_length + payload.length);
            if (!payload.s) {
                payload.s = old_s;
                return 0;
            }
            memcpy(payload.s, old_s, payload.length);
            coap_free(old_s);
        }
        if (!payload.s) {
          return 0;
        }
        if (data != NULL) {
            memcpy(payload.s+payload.length, data, payload_length);
            payload.length += payload_length;
            return payload.length;
        } else
            total_file_size = payload_length;
    }
    return 1;
}

u32_t coap_client_payload_size() {
    return payload.length;
}

void coap_client_set_datahander(coap_incoming_data_cb_t handler,void *param) {
    coap_data_handler = handler;
    coap_data_handler_param = param;
}
