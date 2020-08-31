#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
//#include <getopt.h>
//#include <libgen.h>
#include "EdpKit.h"
#include "Common.h"
#if defined _ENCRYPT || defined _ENCRYPT_OPENSSL
#include "Openssl.h"
#endif
/*
 * [˵��]
 * Main.c ��Ϊ�˲���EdpKit��д��, Ҳ�Ǹ��ͻ�չʾ���ʹ��EdpKit
 * Main.c ʹ�õ���c & linux socket
 *
 * ���԰����ˣ�
 *      ���EDP�������ͣ������豸������, ��������, ת������, �洢json����, �洢bin����
 *      ���ղ�����EDP���������豸��������Ӧ, ����������Ӧ, ת������, �洢json����, �洢bin����
 *
 * [ע��]
 * Main.c����������EDP SDK��һ����, �ͻ�����Ӧ�ø����Լ���ϵͳд����Main.c�Ĵ���
 * �ͻ�����Ӧ��ֻ����Common.h, cJSON.* �� EdpKit.*, ����Ӧ�ð���Main.c
 * 
 * �ӽ���������openssl��ʵ�ֵģ������openssl�⣬�����ֱ������Openssl.*�ļ����ṩ
 * �ĺ���ʵ�ּӽ��ܡ�����Ӧ���Լ�ʵ��Openssl.h�еĺ�����
 * �����Ҫ���ܹ��ܣ���ο�Makefile�е�˵����ȡ����Ӧ�е�ע�͡�
 */

/*----------------------------������-----------------------------------------*/
#define ERR_CREATE_SOCKET -1
#define ERR_HOSTBYNAME -2
#define ERR_CONNECT -3
#define ERR_SEND -4
#define ERR_TIMEOUT -5
#define ERR_RECV -6
/*---------------ͳһlinux��windows�ϵ�Socket api----------------------------*/
#ifndef htonll
#ifdef _BIG_ENDIAN
#define htonll(x) (x)
#define ntohll(x) (x)
#else
#define htonll(x) ((((uint64)htonl(x)) << 32) + htonl(x >> 32))
#define ntohll(x) ((((uint64)ntohl(x)) << 32) + ntohl(x >> 32))
#endif
#endif
/* linux������Ҫ����_LINUX */

#define Socket(a, b, c) socket(a, b, c)
#define Connect(a, b, c) connect(a, b, c)
#define Close(a) close(a)
#define Read(a, b, c) read(a, b, c)
#define Recv(a, b, c, d) recv(a, (void *)b, c, d)
#define Select(a, b, c, d, e) select(a, b, c, d, e)
#define Send(a, b, c, d) send(a, (const int8 *)b, c, d)
#define Write(a, b, c) write(a, b, c)
#define GetSockopt(a, b, c, d, e) getsockopt((int)a, (int)b, (int)c, (void *)d, (socklen_t *)e)
#define SetSockopt(a, b, c, d, e) setsockopt((int)a, (int)b, (int)c, (const void *)d, (int)e)
#define GetHostByName(a) gethostbyname((const char *)a)
#define bzero(a, b) memset(a, 0, b)
#define bcopy(a, b, c) memcpy(b, a, c)

static int g_is_encrypt = 0;

/*
 * buffer��ʮ���������
 */
void hexdump(const unsigned char *buf, uint32 num)
{
    printf("begin hexdump");

    uint32 i = 0;
    for (; i < num; i++)
    {
        printf("%02X ", buf[i]);
        if ((i + 1) % 8 == 0)
            printf("\n");
    }
    printf("\n");
}
/* 
 * ������:  Open
 * ����:    ����socket�׽��ֲ����ӷ����
 * ����:    addr    ip��ַ
 *          protno  �˿ں�
 * ˵��:    ����ֻ�Ǹ���һ������socket���ӷ���˵�����, ������ʽ���ѯ���socket api
 * ���socket api:  
 *          socket, gethostbyname, connect
 * ����ֵ:  ���� (int32)
 *          <=0     ����socketʧ��
 *          >0      socket������
 */
int32 Open(const uint8 *addr, int16 portno)
{
#if 0
    int32 sockfd;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    /* ����socket�׽��� */
    sockfd = Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0) {
        fprintf(stderr, "ERROR opening socket\n");
        return ERR_CREATE_SOCKET; 
    }
    server = GetHostByName(addr);
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        return ERR_HOSTBYNAME;
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
          (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);
    serv_addr.sin_port = htons(portno);
    /* �ͻ��� ������TCP������������ */
    if (Connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
    {
        fprintf(stderr, "ERROR connecting\n");
        return ERR_CONNECT;
    }
#ifdef _DEBUG
    printf("[%s] connect to server %s:%d succ!...\n", __func__, addr, portno);
#endif
    return sockfd;
#endif
    int ret;
    struct addrinfo hints, *addr_list, *cur;
    int32 sockfd;

    /* Do name resolution with both IPv6 and IPv4 */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    char portstr[32] = {
        0,
    };
    sprintf(portstr, "%d", portno);
    if (getaddrinfo(addr, portstr, &hints, &addr_list) != 0)
        return (ERR_HOSTBYNAME);

    /* Try the sockaddrs until a connection succeeds */
    ret = ERR_HOSTBYNAME;
    for (cur = addr_list; cur != NULL; cur = cur->ai_next)
    {
        sockfd = (int)socket(cur->ai_family, cur->ai_socktype,
                             cur->ai_protocol);
        if (sockfd < 0)
        {
            ret = ERR_CREATE_SOCKET;
            continue;
        }

        if (connect(sockfd, cur->ai_addr, cur->ai_addrlen) == 0)
        {
            ret = sockfd;
            break;
        }

        close(sockfd);
        ret = ERR_CONNECT;
    }

    freeaddrinfo(addr_list);

    return (ret);
}
/* 
 * ������:  DoSend
 * ����:    ��buffer�е�len�ֽ�����д��(����)socket������sockfd, �ɹ�ʱ����д��(���͵�)�ֽ���.
 * ����:    sockfd  socket������ 
 *          buffer  �跢�͵��ֽ�
 *          len     �跢�͵ĳ���
 * ˵��:    ����ֻ�Ǹ�����һ���������ݵ�����, ������ʽ���ѯ���socket api
 *          һ����˵, ���Ͷ���Ҫѭ������, ����Ϊ��Ҫ���͵��ֽ��� > socket��д������ʱ, һ��send�Ƿ��Ͳ����.
 * ���socket api:  
 *          send
 * ����ֵ:  ���� (int32)
 *          <=0     ����ʧ��
 *          >0      �ɹ����͵��ֽ���
 */
int32 DoSend(int32 sockfd, const char *buffer, uint32 len)
{
    int32 total = 0;
    int32 n = 0;
    while (len != total)
    {
        /* ���ŷ���len - total���ֽڵ����� */
        n = Send(sockfd, buffer + total, len - total, 0);
        if (n <= 0)
        {
            fprintf(stderr, "ERROR writing to socket\n");
            return n;
        }
        /* �ɹ�������n���ֽڵ����� */
        total += n;
        printf("send %d bytes", n);
    }
    /* wululu test print send bytes */
    hexdump((const unsigned char *)buffer, len);
    return total;
}
/* 
 * ������:  recv_thread_func
 * ����:    �����̺߳���
 * ����:    arg     socket������
 * ˵��:    ����ֻ�Ǹ�����һ����socket�������ݵ�����, ������ʽ���ѯ���socket api
 *          һ����˵, ���ն���Ҫѭ������, ����Ϊ��Ҫ���յ��ֽ��� > socket�Ķ�������ʱ, һ��recv�ǽ��ղ����.
 * ���socket api:  
 *          recv
 * ����ֵ:  ��
 */
void recv_thread_func(void *arg)
{
    int sockfd = *(int *)arg;
    int error = 0;
    int n, rtn;
    uint8 mtype, jsonorbin;
    char buffer[1024];
    RecvBuffer *recv_buf = NewBuffer();
    EdpPacket *pkg;

    char *src_devid;
    char *push_data;
    uint32 push_datalen;

    cJSON *save_json;
    char *save_json_str;

    cJSON *desc_json;
    char *desc_json_str;
    char *save_bin;
    uint32 save_binlen;
    unsigned short msg_id;
    unsigned char save_date_ret;

    char *cmdid;
    uint16 cmdid_len;
    char *cmd_req;
    uint32 cmd_req_len;
    EdpPacket *send_pkg;
    char *ds_id;
    double dValue = 0;
    int iValue = 0;
    char *cValue = NULL;

    char *simple_str = NULL;
    char cmd_resp[] = "ok";
    unsigned cmd_resp_len = 0;

    DataTime stTime = {0};

    FloatDPS *float_data = NULL;
    int count = 0;
    int i = 0;

    struct UpdateInfoList *up_info = NULL;

    printf("[%s] recv thread start ...\n", __func__);

    while (error == 0)
    {
        /* ���Ž���1024���ֽڵ����� */
        n = Recv(sockfd, buffer, 1024, 0);
        if (n <= 0)
        {
            printf("recv from server, error");
            break;
        }
        printf("recv from server, bytes: %d\n", n);
        /* wululu test print send bytes */
        hexdump((const unsigned char *)buffer, n);
        /* �ɹ�������n���ֽڵ����� */
        WriteBytes(recv_buf, buffer, n);
        while (1)
        {
            /* ��ȡһ����ɵ�EDP�� */
            if ((pkg = GetEdpPacket(recv_buf)) == 0)
            {
                printf("need more bytes...\n");
                break;
            }
            /* ��ȡ���EDP������Ϣ���� */
            mtype = EdpPacketType(pkg);
#ifdef _ENCRYPT
            if (mtype != ENCRYPTRESP)
            {
                if (g_is_encrypt)
                {
                    SymmDecrypt(pkg);
                }
            }
#endif
            /* �������EDP������Ϣ����, �ֱ���EDP������ */
            switch (mtype)
            {
#ifdef _ENCRYPT
            case ENCRYPTRESP:
                UnpackEncryptResp(pkg);
                break;
#endif
            case CONNRESP:
                /* ����EDP�� - ������Ӧ */
                rtn = UnpackConnectResp(pkg);
                printf("recv connect resp, rtn: %d\n", rtn);
                break;
            case PUSHDATA:
                /* ����EDP�� - ����ת�� */
                UnpackPushdata(pkg, &src_devid, &push_data, &push_datalen);
                printf("recv push data, src_devid: %s, push_data: %s, len: %d\n",
                       src_devid, push_data, push_datalen);
                free(src_devid);
                free(push_data);
                break;
            case UPDATERESP:
                UnpackUpdateResp(pkg, &up_info);
                while (up_info)
                {
                    printf("name = %s\n", up_info->name);
                    printf("version = %s\n", up_info->version);
                    printf("url = %s\nmd5 = ", up_info->url);
                    for (i = 0; i < 32; ++i)
                    {
                        printf("%c", (char)up_info->md5[i]);
                    }
                    printf("\n");
                    up_info = up_info->next;
                }
                FreeUpdateInfolist(up_info);
                break;

            case SAVEDATA:
                /* ����EDP�� - ���ݴ洢 */
                if (UnpackSavedata(pkg, &src_devid, &jsonorbin) == 0)
                {
                    if (jsonorbin == kTypeFullJson || jsonorbin == kTypeSimpleJsonWithoutTime || jsonorbin == kTypeSimpleJsonWithTime)
                    {
                        printf("json type is %d\n", jsonorbin);
                        /* ����EDP�� - json���ݴ洢 */
                        /* UnpackSavedataJson(pkg, &save_json); */
                        /* save_json_str=cJSON_Print(save_json); */
                        /* printf("recv save data json, src_devid: %s, json: %s\n", */
                        /*     src_devid, save_json_str); */
                        /* free(save_json_str); */
                        /* cJSON_Delete(save_json); */

                        /* UnpackSavedataInt(jsonorbin, pkg, &ds_id, &iValue); */
                        /* printf("ds_id = %s\nvalue= %d\n", ds_id, iValue); */

                        UnpackSavedataDouble(jsonorbin, pkg, &ds_id, &dValue);
                        printf("ds_id = %s\nvalue = %f\n", ds_id, dValue);

                        /* UnpackSavedataString(jsonorbin, pkg, &ds_id, &cValue); */
                        /* printf("ds_id = %s\nvalue = %s\n", ds_id, cValue); */
                        /* free(cValue); */

                        free(ds_id);
                    }
                    else if (jsonorbin == kTypeBin)
                    { /* ����EDP�� - bin���ݴ洢 */
                        UnpackSavedataBin(pkg, &desc_json, (uint8 **)&save_bin, &save_binlen);
                        desc_json_str = cJSON_Print(desc_json);
                        printf("recv save data bin, src_devid: %s, desc json: %s, bin: %s, binlen: %d\n",
                               src_devid, desc_json_str, save_bin, save_binlen);
                        free(desc_json_str);
                        cJSON_Delete(desc_json);
                        free(save_bin);
                    }
                    else if (jsonorbin == kTypeString)
                    {
                        UnpackSavedataSimpleString(pkg, &simple_str);

                        printf("%s\n", simple_str);
                        free(simple_str);
                    }
                    else if (jsonorbin == kTypeStringWithTime)
                    {
                        UnpackSavedataSimpleStringWithTime(pkg, &simple_str, &stTime);

                        printf("time:%u-%02d-%02d %02d-%02d-%02d\nstr val:%s\n",
                               stTime.year, stTime.month, stTime.day, stTime.hour, stTime.minute, stTime.second, simple_str);
                        free(simple_str);
                    }
                    else if (jsonorbin == kTypeFloatWithTime)
                    {
                        if (UnpackSavedataFloatWithTime(pkg, &float_data, &count, &stTime))
                        {
                            printf("UnpackSavedataFloatWithTime failed!\n");
                        }

                        printf("read time:%u-%02d-%02d %02d-%02d-%02d\n",
                               stTime.year, stTime.month, stTime.day, stTime.hour, stTime.minute, stTime.second);
                        printf("read float data count:%d, ptr:[%p]\n", count, float_data);

                        for (i = 0; i < count; ++i)
                        {
                            printf("ds_id=%u,value=%f\n", float_data[i].ds_id, float_data[i].f_data);
                        }

                        free(float_data);
                        float_data = NULL;
                    }
                    free(src_devid);
                }
                else
                {
                    printf("error\n");
                }
                break;
            case SAVEACK:
                UnpackSavedataAck(pkg, &msg_id, &save_date_ret);
                printf("save ack, msg_id = %d, ret = %d\n", msg_id, save_date_ret);
                break;
            case CMDREQ:
                if (UnpackCmdReq(pkg, &cmdid, &cmdid_len,
                                 &cmd_req, &cmd_req_len) == 0)
                {
                    /*
                     * �û������Լ������������أ���Ӧ��Ϣ�����Ϊ�գ��˴����践��2���ַ�"ok"��
                     * ���������Ҫ�ͷ�
                     */
                    cmd_resp_len = strlen(cmd_resp);
                    send_pkg = PacketCmdResp(cmdid, cmdid_len,
                                             cmd_resp, cmd_resp_len);
#ifdef _ENCRYPT
                    if (g_is_encrypt)
                    {
                        SymmEncrypt(send_pkg);
                    }
#endif
                    DoSend(sockfd, (const char *)send_pkg->_data, send_pkg->_write_pos);
                    DeleteBuffer(&send_pkg);

                    free(cmdid);
                    free(cmd_req);
                }
                break;
            case PINGRESP:
                /* ����EDP�� - ������Ӧ */
                UnpackPingResp(pkg);
                printf("recv ping resp\n");
                break;

            default:
                /* δ֪��Ϣ���� */
                error = 1;
                printf("recv failed...\n");
                break;
            }
            DeleteBuffer(&pkg);
        }
    }
    DeleteBuffer(&recv_buf);

#ifdef _DEBUG
    printf("[%s] recv thread end ...\n", __func__);
#endif
}
void usage(char *name)
{
    printf("Usage:edpclient [options]\n");
    printf("-h List help document\n");
    printf("-i The ip of the edpacc\n");
    printf("-p The port of the edpacc\n");
    printf("-s Assign the dev_id of the source\n");
    printf("-d Assign the dev_id of the destination\n");
    printf("-a Assign the API key of the source\n");
    printf("-l Assign the name of the datastream for test 'save json data'\n");
    printf("-v Assign the value of the datastream for test 'save json data'\n");
    printf("-t Assign the time of the datastream for test 'save string data'\n");
    printf("-E Encrypt\n");
    exit(0);
}
#define EDP_THREAD_STACKSIZE 4096
#define EDP_THREAD_PRIO TCPIP_THREAD_PRIO
static HANDLE edp_handle;

int edp_main(int argc, char *argv[])
{
    char opt;
    int sockfd, n, ret;
    //pthread_t id_1;
    EdpPacket *send_pkg;
    char c;
    char push_data[] = {'a', 'b', 'c'};
    char text1[] = "{\"name\": \"Jack\"}";
    /* cJSON����ֻ֧��unicode����   */
    char text2[] = "{\"ds_id\": \"temperature\"}";
    cJSON *save_json, *desc_json;
    char save_bin[] = {'c', 'b', 'a'};
    char cmdline[255] = "";
    for (int i = 0; i < argc; i++)
    {
        strcat(cmdline, argv[i]);
        strcat(cmdline, " ");
    }
    printf("entering edp_main:%s\n", cmdline);
    char *ip = NULL;
    char *port = NULL;
    char *src_dev = NULL;
    char *dst_dev = NULL;
    char *src_api_key = NULL;
    char *ds_for_send = NULL;
    char *method_for_send = NULL;
    double value_for_send = 0.0;
    DataTime save_time = {0};
    char send_str[] = ",;temperature,2015-03-22 22:31:12,22.5;humidity,35%;pm2.5,89;1001";
    FloatDPS send_float[] = {{1, 0.5}, {2, 0.8}, {3, -0.5}};
    SaveDataType data_type;
    struct UpdateInfoList *up_info = NULL;
    /* 
     * ˵��: ����ֻ��Ϊ�˲���EdpKit��д������, �ͻ�����Ӧ�ø����Լ����������д����
     * ���ݱ�׼���� ����ͬ�Ĵ��� 
     * 0 ���� ��������EDP��
     * 1 ���� ת����������EDP��
     * 2 ���� �洢json����EDP��
     * 3 ���� �洢bin����EDP��
     * 4 ���� �洢��ʱ��Json����EDP��
     */
    char msg[][50] = {"send ping to server",
                      "send pushdata to server",
                      "send savedata full json to server",
                      "send savedata bin to server",
                      "send savedata simple json without time to server",
                      "send savedata simple json with time to server",
                      "send string split by simicolon",
                      "send string with time to server",
                      "send float with time to server",
                      "send update info to server"};

    while ((opt = getopt(argc, argv, "hi:p:s:d:a:l:v:t:M:E")) != -1)
    {
        switch (opt)
        {
        case 'i':
            ip = optarg;
            break;

        case 'p':
            port = optarg;
            break;

        case 's':
            src_dev = optarg;
            break;

        case 'd':
            dst_dev = optarg;
            break;

        case 'a':
            src_api_key = optarg;
            break;

        case 'l':
            ds_for_send = optarg;
            break;

        case 'v':
            value_for_send = atof(optarg);
            break;

        case 't':
            /*20160405144601*/
            if (strlen(optarg) != 14)
            {
                break;
            }
            sscanf(optarg, "%4d%2d%2d%2d%2d%2d",
                   &save_time.year, &save_time.month, &save_time.day,
                   &save_time.hour, &save_time.minute, &save_time.second);
            break;
        case 'M':
            method_for_send = optarg;
            break;
        case 'E':
#ifndef _ENCRYPT
            printf("Sorry the option 'E' is not supported right now\n");
            printf("Please check your compile flags, \n");
            printf("uncomment this line 'CFLAGS+=-D_ENCRYPT -lcrypto'\n");
            printf("and this line 'CLIENT_OBJ += Openssl.o'\n");
            printf("and try it again\n");
            exit(0);
#endif
            g_is_encrypt = 1;
            break;

        case 'h':
        default:
            usage(argv[0]);
            return 0;
        }
    }

    if (!ip || !port || !src_dev || !dst_dev || !src_api_key || !ds_for_send)
    {
        usage(argv[0]);
        return 0;
    }

    /* create a socket and connect to server */
    sockfd = Open((const uint8 *)ip, atoi(port));
    if (sockfd < 0)
        exit(0);
    printf("Open sock successed sockfd=%d\n", sockfd);
    /* create a recv thread */
    //ret=pthread_create(&id_1,NULL,(void *(*) (void *))recv_thread_func, &sockfd);
    edp_handle = sys_thread_new("edp_handle", recv_thread_func, &sockfd, EDP_THREAD_STACKSIZE, EDP_THREAD_PRIO);
#ifdef _ENCRYPT
    if (g_is_encrypt)
    {
        send_pkg = PacketEncryptReq(kTypeAes);
        /* ���豸�Ʒ��ͼ������� */
        printf("send encrypt to server, bytes: %d\n", send_pkg->_write_pos);
        ret = DoSend(sockfd, (const char *)send_pkg->_data, send_pkg->_write_pos);
        DeleteBuffer(&send_pkg);
        sleep(1);
    }
#endif
    /* connect to server */
    /*    send_pkg = PacketConnect1(src_dev, "Bs04OCJioNgpmvjRphRak15j7Z8=");*/
    send_pkg = PacketConnect1(src_dev, src_api_key);
#ifdef _ENCRYPT
    if (g_is_encrypt)
    {
        SymmEncrypt(send_pkg);
    }
#endif
    /* send_pkg = PacketConnect2("433223", "{ \"SYS\" : \"0DEiuApATHgLurKNEl6vY4bLwbQ=\" }");*/
    /* send_pkg = PacketConnect2("433223", "{ \"13982031959\" : \"888888\" }");*/

    /* ���豸�Ʒ����������� */
    printf("send connect to server, bytes: %d\n", send_pkg->_write_pos);
    ret = DoSend(sockfd, (const char *)send_pkg->_data, send_pkg->_write_pos);
    DeleteBuffer(&send_pkg);

    COS_Sleep(1000);
    printf("\n[0] send ping\n[1] send push data\n[2] send save json\n[3] send save bin\n");
    printf("[4] send save json simple format without time\n");
    printf("[5] send save json simple format with time\n");
    printf("[6] send simple format (string) \n");
    printf("[7] send simple format (string) with datetime \n");
    printf("[8] send float data with datetime \n");
    printf("[9] send update info \n");

    c = *method_for_send;
    switch (c)
    {
    case '0':
        send_pkg = PacketPing();
        break;

    case '1':
        send_pkg = PacketPushdata(dst_dev, push_data, sizeof(push_data));
        break;

    case '2':
    case '4':
    case '5':
        if (c == '2')
            data_type = kTypeFullJson;
        if (c == '4')
            data_type = kTypeSimpleJsonWithoutTime;
        if (c == '5')
            data_type = kTypeSimpleJsonWithTime;

        send_pkg = PacketSavedataInt(data_type, dst_dev, ds_for_send, 1234, 0, 0);
        /* send_pkg = PacketSavedataDouble(data_type, dst_dev, ds_for_send, value_for_send, 0, 0); */
        /* send_pkg = PacketSavedataString(data_type, dst_dev, ds_for_send, "test12345678", 0, 1); */
        break;

    case '3':
        desc_json = cJSON_Parse(text2);
        send_pkg = PacketSavedataBin(dst_dev, desc_json, (const uint8 *)save_bin, sizeof(save_bin), 0);
        break;

    case '6':
        send_pkg = PacketSavedataSimpleString(dst_dev, send_str, 0);
        break;

    case '7':
        if (save_time.year > 0)
            send_pkg = PacketSavedataSimpleStringWithTime(dst_dev, send_str, &save_time, 0);
        else
            send_pkg = PacketSavedataSimpleStringWithTime(dst_dev, send_str, NULL, 0);
        break;

    case '8':
        if (save_time.year > 0)
            send_pkg = PackSavedataFloatWithTime(dst_dev, send_float, 3, &save_time, 0);
        else
            send_pkg = PackSavedataFloatWithTime(dst_dev, send_float, 3, NULL, 0);
        break;

    case '9':
        /* up_info = (struct UpdateInfoList*)malloc(sizeof(struct UpdateInfoList)); */
        /* up_info->name = "file.txt"; */
        /* up_info->version = "v1.0"; */
        /* up_info->next = NULL; */

        /* up_info->next = (struct UpdateInfoList*)malloc(sizeof(struct UpdateInfoList)); */
        /* up_info->next->name = "EdpKit.h"; */
        /* up_info->next->version = "v1.0"; */
        /* up_info->next->next = NULL; */

        send_pkg = PacketUpdateReq(up_info);
        hexdump(send_pkg->_data, send_pkg->_write_pos);
        free(up_info);
        break;

    default:
        //getchar();	/* ��ȡ�س��������� */
        printf("input error, please try again\n");
        break;
    }
#ifdef _ENCRYPT
    if (g_is_encrypt)
    {
        SymmEncrypt(send_pkg);
    }
#endif
    printf("%s, bytes: %d\n", msg[c - '0'], send_pkg->_write_pos);
    DoSend(sockfd, (const char *)send_pkg->_data, send_pkg->_write_pos);
    DeleteBuffer(&send_pkg);

    /* close socket */
    Close(sockfd);

    return 0;
}
