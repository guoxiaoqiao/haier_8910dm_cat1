#include "h_ssl.h"

//zhangkai 20/04/28
uint8_t read_or_pending;

static int32_t SSL_SocketTx(int32_t Socketfd, const unsigned char *Buf, size_t TxLen)
{
    struct timeval tm;
    fd_set WriteSet;
	int32_t Result;
	int32_t ret = -1;

	//uplus_sys_log("socket=%d:%dbyte need send", Socketfd, TxLen);

	Result = send(Socketfd, (uint8_t *)Buf, TxLen, 0);

	if (Result < 0)
	{
		uplus_sys_log("[zk ssl]SSL_SocketTx:Result=%d error=%d", Result, uplus_net_last_error_get(Socketfd, NULL));
		return -1;
	}
    FD_ZERO(&WriteSet);
    FD_SET(Socketfd, &WriteSet);
    tm.tv_sec = 10;
    tm.tv_usec = 0;
    ret = select(Socketfd + 1, NULL, &WriteSet, NULL, &tm);
    if(ret > 0)
    {
    	uplus_sys_log("[zk ssl]SSL_SocketTx: OK! %dbyte", TxLen);
		return Result;
    }
    else
    {
        uplus_sys_log("[zk ssl]SSL_SocketTx:ERROR ret=%d len=%d", ret, TxLen);
        return 0;
    }
}

static int32_t SSL_SocketRx(int32_t Socketfd, unsigned char *Buf, size_t RxLen)
{
    struct timeval tm;
    fd_set ReadSet;
	int32_t Result;
	
    FD_ZERO(&ReadSet);
    FD_SET(Socketfd, &ReadSet);

	if(read_or_pending)
	{
		tm.tv_sec = 5;
		tm.tv_usec = 0;
	}
	else
	{
		tm.tv_sec = 10;
		tm.tv_usec = 0;
	}
	
    Result = select(Socketfd + 1, &ReadSet, NULL, NULL, &tm);
    if(Result > 0)
    {
    	Result = recv(Socketfd, Buf, RxLen, 0);
        if(Result == 0)
        {
        	uplus_sys_log("[zk ssl]SSL_SocketRx_1: remote socket close!");
            return -1;
        }
        else if(Result < 0)
        {
        	uplus_sys_log("[zk ssl]SSL_SocketRx_2:recv error %d", uplus_net_last_error_get(Socketfd, NULL));
            return -1;
        }
		uint8_t *p = Buf;
        uplus_sys_log("[zk ssl]SSL_SocketRx_3:recv %d %x %x %x %x %x", Result, p[0], p[1], p[2], p[3], p[4]);
		return Result;
    }
    else
    {
    	uplus_sys_log("[zk ssl]SSL_SocketRx_4:recv error %d", uplus_net_last_error_get(Socketfd, NULL));
    	return 0;
    }
}

int mbedtls_send( void *ctx, const unsigned char *buf, size_t len )
{
	int fd = ((mbedtls_context *) ctx)->fd;

	if( fd < 0 )
	{
		uplus_sys_log("[zk ssl] mbedtls_send_0:socket id error");
		
        return( -0x0045 );
	}

	return SSL_SocketTx(fd, buf, len);
}

int mbedtls_read( void *ctx, unsigned char *buf, size_t len )
{
	int fd = ((mbedtls_context *) ctx)->fd;

	if( fd < 0 )
	{
		uplus_sys_log("[zk ssl] mbedtls_read_0:socket id error");
		
        return( -0x0045 );
	}

	return SSL_SocketRx(fd, buf, len);
}
   
/***************************** ssl *********************************/
mbedtls_ssl_context* uplus_ssl_init(void)
{
	mbedtls_ssl_context *ssl = malloc(sizeof(mbedtls_ssl_context));
	if(ssl == NULL)
	{
		uplus_sys_log("[zk ssl] uplus_ssl_init:malloc fail");
		return NULL;
	}
	mbedtls_ssl_init(ssl);
	return ssl;
}

void uplus_ssl_deinit(mbedtls_ssl_context *ssl)
{
	if(ssl != NULL)
	{
		mbedtls_ssl_free(ssl);

		free(ssl);

		uplus_sys_log("[zk ssl] uplus_ssl_deinit ok");
	}
}

/***************************** ssl_config *********************************/
mbedtls_ssl_config* uplus_ssl_config_init(void)
{
	mbedtls_ssl_config *conf = malloc(sizeof(mbedtls_ssl_config));
	if(conf == NULL)
	{
		uplus_sys_log("[zk ssl] uplus_ssl_config_init:malloc fail");
		return NULL;
	}
	mbedtls_ssl_config_init(conf);
	return conf;
}

void uplus_ssl_config_deinit(mbedtls_ssl_config *conf)
{
	if(conf != NULL)
	{
		mbedtls_ssl_config_free(conf);
	
		free(conf);

		uplus_sys_log("[zk ssl] uplus_ssl_config_deinit ok");
	}
}

/***************************** x509_crt *********************************/
mbedtls_x509_crt* uplus_x509_crt_init(void)
{
	mbedtls_x509_crt *cacert = malloc(sizeof(mbedtls_x509_crt));
	if(cacert == NULL)
	{
		uplus_sys_log("[zk ssl] uplus_x509_crt_init:malloc fail");
		return NULL;
	}
	mbedtls_x509_crt_init(cacert);
	return cacert;
}

void uplus_x509_crt_deinit(mbedtls_x509_crt *cacert)
{
	if(cacert != NULL)
	{
		mbedtls_x509_crt_free(cacert);

		free(cacert);

		uplus_sys_log("[zk ssl] uplus_x509_crt_deinit ok");
	}
}

/***************************** ctr_drbg *********************************/
mbedtls_ctr_drbg_context* uplus_ctr_drbg_init(void)
{
	mbedtls_ctr_drbg_context *ctr_drbg = malloc(sizeof(mbedtls_ctr_drbg_context));
	if(ctr_drbg == NULL)
	{
		uplus_sys_log("[zk ssl] uplus_ctr_drbg_init:malloc fail");
		return NULL;
	}
	mbedtls_ctr_drbg_init(ctr_drbg);
	return ctr_drbg;
}

void uplus_ctr_drbg_deinit(mbedtls_ctr_drbg_context *ctr_drbg)
{
	if(ctr_drbg != NULL)
	{
		mbedtls_ctr_drbg_free(ctr_drbg);

		free(ctr_drbg);

		uplus_sys_log("[zk ssl] uplus_ctr_drbg_deinit ok");
	}
}

/***************************** entropy *********************************/
mbedtls_entropy_context *uplus_entropy_init(void)
{
 	mbedtls_entropy_context *entropy = malloc(sizeof(mbedtls_entropy_context));
	if(entropy == NULL)
	{
		uplus_sys_log("[zk ssl] uplus_entropy_init:malloc fail");
		return NULL;
	}
	mbedtls_entropy_init(entropy);
	return entropy;
}

void uplus_entropy_deinit(mbedtls_entropy_context *entropy)
{
	if(entropy != NULL)
	{
	 	mbedtls_entropy_free(entropy);

		free(entropy);

		uplus_sys_log("[zk ssl] uplus_entropy_deinit ok");
	}
}

uplus_ctx_id net_ssl_client_create(uplus_s32 fd, struct uplus_ca_chain *root_ca, uplus_u8 root_ca_num)
{
	int32_t ret = -1;
	char *error_buf = NULL;
	ssl_session_ctx *ssl_ctx = NULL;
	const char *pers = "uplus_ssl_client";

	ssl_ctx = calloc(1, sizeof(ssl_session_ctx));
	if(ssl_ctx == NULL)
	{
		return NULL;
	}

	ssl_ctx->server_fd.fd = fd;

	ssl_ctx->ssl = uplus_ssl_init();
	if(ssl_ctx->ssl == NULL)
	{
		 goto exit;
	}

	ssl_ctx->conf = uplus_ssl_config_init();
	if(ssl_ctx->conf == NULL)
	{
		 goto exit;
	}

	ssl_ctx->cacert = uplus_x509_crt_init();
	if(ssl_ctx->cacert == NULL)
	{
		 goto exit;
	}

	ssl_ctx->ctr_drbg = uplus_ctr_drbg_init();
	if(ssl_ctx->ctr_drbg == NULL)
	{
		 goto exit;
	}

	ssl_ctx->entropy = uplus_entropy_init();
	if(ssl_ctx->entropy == NULL)
	{
		 goto exit;
	}
	
    if( ( ret = mbedtls_ctr_drbg_seed( ssl_ctx->ctr_drbg, mbedtls_entropy_func, ssl_ctx->entropy, (const unsigned char *) pers, strlen( pers ) ) ) != 0 )
    {
        uplus_sys_log( "[zk ssl] failed  ! mbedtls_ctr_drbg_seed returned %d", ret );
        goto exit;
    }

    uplus_sys_log( "[zk ssl] entropy ok" );

    /*
     * 0. Initialize certificates
     */
    /*uplus_sys_log( "[zk sll] Loading the CA root certificate ..." );
   

	if(root_ca != NULL)
	{
		ssl_ctx->cacert = uplus_x509_crt_init();
		if(ssl_ctx->cacert == NULL)
		{
			 goto exit;
		}
		
		//uplus_sys_log("root ca %d,%s",strlen(root_ca->ca),(const char *)root_ca->ca);
		
	    ret = mbedtls_x509_crt_parse( ssl_ctx->cacert, root_ca, root_ca_num);
	    if( ret < 0 )
	    {
	        uplus_sys_log( "[zk ssl]failed  !  mbedtls_x509_crt_parse returned -0x%x", (unsigned int) -ret );
	        goto exit;
	    }
	}
	else*/
	{
	    if( ( ret = mbedtls_ssl_config_defaults(ssl_ctx->conf, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT ) ) != 0 )
	    {
	        uplus_sys_log( "[zk ssl]failed  ! mbedtls_ssl_config_defaults returned %d", ret );
	        goto exit;
	    }
	    uplus_sys_log( "[zk ssl] config ok" );

	    /* OPTIONAL is not optimal for security,
	     * but makes interop easier in this simplified example */
	    mbedtls_ssl_conf_authmode( ssl_ctx->conf, MBEDTLS_SSL_VERIFY_NONE );
	    mbedtls_ssl_conf_ca_chain( ssl_ctx->conf, ssl_ctx->cacert, NULL );
	    mbedtls_ssl_conf_rng( ssl_ctx->conf, mbedtls_ctr_drbg_random,  ssl_ctx->ctr_drbg);
	   // mbedtls_ssl_conf_dbg( &conf, my_debug, stdout );

	    if( ( ret = mbedtls_ssl_setup( ssl_ctx->ssl, ssl_ctx->conf ) ) != 0 )
	    {
	        uplus_sys_log( "[zk ssl]failed ! mbedtls_ssl_setup returned %d", ret );
	        goto exit;
	    }

	    if( ( ret = mbedtls_ssl_set_hostname( ssl_ctx->ssl, appSysTem.uplus_hostname ) ) != 0 )
	    {
	        uplus_sys_log( "[zk ssl]failed ! mbedtls_ssl_set_hostname returned %d", ret );
	        goto exit;
	    }

	    //mbedtls_ssl_set_bio( &ssl, &server_fd, mbedtls_net_send, mbedtls_net_recv, NULL );
	    
	    mbedtls_ssl_set_bio( ssl_ctx->ssl, &ssl_ctx->server_fd, mbedtls_send, mbedtls_read, NULL );
	}
	
	return (uplus_ctx_id)ssl_ctx;

exit:

#ifdef MBEDTLS_ERROR_C
	error_buf = calloc(1,100);
	if(error_buf != NULL)
	{
		mbedtls_strerror( ret, error_buf, 100 );
		uplus_sys_log("[zk ssl]Last error was: %d - %s", ret, error_buf);

		free(error_buf);
	}
#endif

	net_ssl_client_close(ssl_ctx);

   /* uplus_x509_crt_deinit(ssl_ctx->cacert);
	uplus_ssl_deinit(ssl_ctx->ssl);
	uplus_ssl_config_deinit(ssl_ctx->conf);
  	uplus_ctr_drbg_deinit(ssl_ctx->ctr_drbg);
	uplus_entropy_deinit(ssl_ctx->entropy);
	free(ssl_ctx);*/
	
	return NULL;
}


/*!
 * \brief SSL握手。
 * \param [in] id SSL会话标识。
 * \return 成功返回0，失败返回-1，继续握手返回1。
 */
uplus_s32 net_ssl_client_handshake(uplus_ctx_id id)
{
	int32_t ret;
	ssl_session_ctx *ssl_ctx = (ssl_session_ctx *)id;

	if(ssl_ctx == NULL)
	{
		uplus_sys_log( "[zk ssl]net_ssl_client_handshake:param error" );
		return -1;
	}

	while( ( ret = mbedtls_ssl_handshake( ssl_ctx->ssl) ) != 0 )
	{
	   if( ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE )
	   {
		   uplus_sys_log( "[zk ssl]failed ! mbedtls_ssl_handshake returned -0x%x", (unsigned int) -ret );
		   return -1;
	   }
	}
	
	uplus_sys_log( "[zk ssl] handshake ok" );

	 /*
     * 5. Verify the server certificate
     */
   // uplus_sys_log( "  . Verifying peer X.509 certificate..." );

    /* In real life, we probably want to bail out when ret != 0 */
   /* if( ( flags = mbedtls_ssl_get_verify_result( &ssl ) ) != 0 )
    {
        char vrfy_buf[512];

        uplus_sys_log( " failed" );

        mbedtls_x509_crt_verify_info( vrfy_buf, sizeof( vrfy_buf ), "  ! ", flags );

        uplus_sys_log( "%s", vrfy_buf );
    }
    else
        uplus_sys_log( " ok" );*/

	return 0;
}

uplus_s32 net_ssl_client_close(uplus_ctx_id id)
{
	ssl_session_ctx *ssl_ctx = (ssl_session_ctx *)id;

	mbedtls_ssl_close_notify( ssl_ctx->ssl);
	 
	uplus_x509_crt_deinit(ssl_ctx->cacert);
	uplus_ssl_deinit(ssl_ctx->ssl);
	uplus_ssl_config_deinit(ssl_ctx->conf);
  	uplus_ctr_drbg_deinit(ssl_ctx->ctr_drbg);
	uplus_entropy_deinit(ssl_ctx->entropy);

	free(ssl_ctx);

	uplus_sys_log("[zk ssl] ssl client close suc");
	
	return 0;
}

static SSL_RECV ssl_recv;
static void ssl_recv_clr(void)
{
	ssl_recv.buffLen = 0;
	ssl_recv.rx = 0;
	
	if(ssl_recv.ssl_recv_buf != NULL)
	{
		free(ssl_recv.ssl_recv_buf);
		
		ssl_recv.ssl_recv_buf = NULL;
	}
	uplus_sys_log("[zk ssl] ssl_recv_clr");
}

static void ssl_recv_set(uint8_t *buff, uint16_t len)
{
	if((buff == NULL) || (len ==0))
		return;
	
	ssl_recv_clr();
	
	ssl_recv.ssl_recv_buf = malloc(len+1);
	if(ssl_recv.ssl_recv_buf != NULL)
	{	
		memcpy(ssl_recv.ssl_recv_buf, buff, len);
		ssl_recv.buffLen = len;
		ssl_recv.rx = 0;

		uplus_sys_log("[zk ssl] recv_set:ok len=%d", len);
	}
	else
	{
		uplus_sys_log("[zk ssl] recv_set:malloc fial len=%d", len);
		ssl_recv_clr();
	}
}

uplus_s32 net_ssl_pending(uplus_ctx_id id)
{
	int32_t ret;
	uplus_size_t readlen= 0;
	uint8_t *readbuf;
	ssl_session_ctx *ssl_ctx = (ssl_session_ctx *)id;

	read_or_pending = 1;

	readbuf = calloc(1, 1024);
	if(readbuf == NULL)
	{
		uplus_sys_log( "[zk ssl]ssl_pending:malloc fail");
		return 0;
	}
	do
    {
        ret = mbedtls_ssl_read( ssl_ctx->ssl, readbuf, 1023 );

        if( ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE )
            continue;

        if( ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY )
            break;

        if( ret < 0 )
        {
            uplus_sys_log( "[zk ssl]ssl_pending:failed ! mbedtls_ssl_read returned %d", ret );
            break;
        }

        if( ret == 0 )
        {
            uplus_sys_log( "[zk ssl]ssl_pending:EOF" );
            break;
        }
		readlen = ret;
        uplus_sys_log( "[zk ssl]ssl_pending:%d bytes read", readlen);
    }
    while( 0 );
	
	ssl_recv_set(readbuf, readlen);
	
	read_or_pending = 0;

	free(readbuf);
	
	return readlen;
}

uplus_s32 net_ssl_read(uplus_ctx_id id, uplus_u8 *buf, uplus_size_t len)
{
	int32_t ret = 0;
	uplus_size_t readlen= 0;
	ssl_session_ctx *ssl_ctx = (ssl_session_ctx *)id;

	if((ssl_recv.buffLen != 0) && (ssl_recv.ssl_recv_buf != NULL))
	{
		uplus_sys_log("[zk ssl] ssl_read_2:len=%d bufflen=%d", len, ssl_recv.buffLen);
		if(len >= ssl_recv.buffLen)
		{
			memcpy(buf, ssl_recv.ssl_recv_buf+ssl_recv.rx, ssl_recv.buffLen);
			uint16_t bufflen = ssl_recv.buffLen;
			//缓冲区数据已经全部读完，则清楚缓冲区
			ssl_recv_clr();

			return bufflen; 
		}
		else
		{
			memcpy(buf, ssl_recv.ssl_recv_buf+ssl_recv.rx, len);
			
			ssl_recv.rx += len;
			ssl_recv.buffLen -= len;
			if(ssl_recv.buffLen == 0)
			{
				//缓冲区数据已经全部读完，则清楚缓冲区
				ssl_recv_clr();
			}
			return len;
		}
	}
	 
    uplus_sys_log( "[zk ssl] < Read from server:" );
	
    do
    {
        ret = mbedtls_ssl_read( ssl_ctx->ssl, buf, len );

        if( ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE )
            continue;

        if( ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY )
            break;

        if( ret < 0 )
        {
            uplus_sys_log( "[zk ssl]ssl_read:failed ! mbedtls_ssl_read returned %d", ret );
            break;
        }

        if( ret == 0 )
        {
            uplus_sys_log( "[zk ssl]ssl_read:EOF" );
            break;
        }
		readlen = ret;
        uplus_sys_log( "[zk ssl]ssl_read_0:%d bytes read", readlen);
    }
    while( 0 );

	return readlen;
}

uplus_s32 net_ssl_write(uplus_ctx_id id, uplus_u8 *buf, uplus_size_t len)
{
	int32_t ret;
	ssl_session_ctx *ssl_ctx = (ssl_session_ctx *)id;
	
    uplus_sys_log( "[zk ssl] > Write to server:" );
	
	while( ( ret = mbedtls_ssl_write( ssl_ctx->ssl, buf, len ) ) <= 0 )
	{
		if( ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE )
		{
			uplus_sys_log( "[zk ssl]failed ! mbedtls_ssl_write returned %d", ret );
			
			#ifdef MBEDTLS_ERROR_C
				char *error_buf = NULL;
				error_buf = calloc(1,100);
				if(error_buf != NULL)
				{
					mbedtls_strerror( ret, error_buf, 100 );
					uplus_sys_log("[zk ssl]Last error was: %d - %s", ret, error_buf );

					free(error_buf);
				}
			#endif
			
			return ret;
		}
	}
	uplus_sys_log( "[zk ssl] net_ssl_write:%d bytes written", ret);

	return ret;
}
