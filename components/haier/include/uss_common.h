//
//  uss_common.h
//  uss
//
//  Created by dinglonghao on 16-4-22.
//  Copyright (c) 2016年 dinglonghao. All rights reserved.
//

// udp 引入 remoteEP
// 一个连接上，如果有多个localkey时，引入device_id

#ifndef uss_uss_common_h
#define uss_uss_common_h

//#define USS_HAVE_M_CU
//#define USS_HAVE_M_CD
//#define USS_HAVE_M_CD_V2
//#define USS_HAVE_M_LC_O2O
//#define USS_HAVE_M_LS_O2O
//#define USS_HAVE_M_LC
//#define USS_HAVE_M_LS

#define USS_LEVEL_DEBUG     0x01
#define USS_LEVEL_INFO      0x02
#define USS_LEVEL_WARN      0x04
#define USS_LEVEL_ERROR     0x08
#define USS_LEVEL_NOLOG     0x10

#ifdef USS_HAVE_M_LC
#ifndef USS_REQUIRE_DTL_BIO
#define USS_REQUIRE_DTL_BIO   
#endif
#endif

#ifdef USS_HAVE_M_LS
#ifndef USS_REQUIRE_DTL_BIO
#define USS_REQUIRE_DTL_BIO   
#endif
#endif

#define USS_RET_E_UNKNOWN_FATAL (-99)
#define USS_RET_E_MEMORY        (-98)
#define USS_RET_E_NOT_IMPL      (-96)
#define USS_RET_SUCCESS         (0)
#define USS_RET_NOT_FIND        (1)
#define USS_RET_E_ONLY_SUPPORT_ONE_DEV  (96)
#define USS_RET_E_PARAMS        (97)
#define USS_RET_E_NO_AUTH       (98)
#define USS_RET_E_UNKNOWN       (99)

typedef unsigned char uss_byte;
typedef char uss_char;
typedef enum
{
    uss_false, uss_true
} uss_bool;
typedef unsigned short int uss_ushort;
typedef unsigned int uss_uint;
typedef int uss_int;
typedef int uss_int32;
typedef short uss_short;
typedef void uss_void;
typedef char* uss_string;

typedef enum _USS_CTX_TYPE
{
    USS_CTX_TYPE_LOCAL_CLIENT,
    USS_CTX_TYPE_LOCAL_SERVER,
    USS_CTX_TYPE_CLOUD_USER,
    USS_CTX_TYPE_CLOUD_DEVICE,
} USS_CTX_TYPE;

typedef enum _USS_CTX_PRO_VER
{
    USS_CTX_PRO_VER_0_DOT_5 = 1,
    USS_CTX_PRO_VER_1 = 2,
    USS_CTX_PRO_VER_2 = 3,
} USS_CTX_PRO_VER;

typedef enum _USS_CTX_LOCAL_BIZ_VER
{
    USS_CTX_BIZ_VER_UWT = 1,
    USS_CTX_BIZ_VER_COAP = 2,
} USS_CTX_LOCAL_BIZ_VER;

typedef enum _USS_ERROR
{
    // < 0 无法恢复的异常，只能外部立刻断开连接，或者释放ctx等
    USS_ERROR_UNKNOWN_FATAL = -99,
    USS_ERROR_MEMORY_E = -98,
    USS_ERROR_IO_E = -97,
    USS_ERROR_NOT_IMPL = -96,
    USS_ERROR_PROTOCOL_VER = -95,
    USS_ERROR_CONNECT_FAIL = -94,
    USS_ERROR_HANDSHAKE_FAIL = -93,
    USS_ERROR_SESSION_FAIL = -92,
    USS_ERROR_RECV_SN_FAIL = -91,
    USS_ERROR_NOT_FIND = -90,
    USS_ERROR_TC_DECAP = -89,

    USS_ERROR_NONE = 0,
    USS_ERROR_NOT_SUPPORT_SOCKET_IO_TYPE = 1,

    USS_ERROR_CONNECT_READ_WANT = 2,
    USS_ERROR_CONNECT_WRITE_WANT = 3,

    USS_ERROR_HANDSHAKE_DOING = 4,
    USS_ERROR_HANDSHAKE_NO_KEY = 5,
    USS_ERROR_HANDSHAKE_OBTAINING_KEY_FROM_CLOUD = 6,
    USS_ERROR_HANDSHAKE_WAITING_ACK = 7,
    USS_ERROR_HANDSHAKE_SERVER_NOT_SUPPORT_DEVICE = 8, // local_client专用错误码

    USS_ERROR_READ_WANT = 9,
    USS_ERROR_READ_NO_AUTH = 10,
    USS_ERROR_READ_EAGAIN = 11,
    USS_ERROR_READ_DATA_CHECK_FAIL = 12,
    USS_ERROR_READ_HANDSHAKE_RESET = 13,

    USS_ERROR_WRITE_NO_AUTH = 14,
    USS_ERROR_WRITE_EAGAIN = 15,

    USS_ERROR_NO_BINDKEY = 16,
    USS_ERROR_CLOUD_BREAK = 17,

    USS_ERROR_SHUTDOWN = 18,

    USS_ERROR_LOCALKEY_VER_MISMATCH = 19,
    USS_ERROR_NO_LOCALKEY = 20,

    USS_ERROR_CONNECTING = 21,
    
    USS_ERROR_NO_BIND_TIME_WIN = 22,

    USS_ERROR_UD_DECAP = 89,

    USS_ERROR_ONLY_SUPPORT_ONE_DEV = 96,
    USS_ERROR_PARAMS = 97,
    USS_ERROR_UNKNOWN = 99,
} USS_ERROR;

typedef enum _USS_SOCKET_IO_TYPE
{
    USS_SOCKET_IO_ONE_TO_ONE, // tcp 模式
    USS_SOCKET_IO_ONE_TO_MANY, // udp 模式
} USS_SOCKET_IO_TYPE;

struct uss_context_t;
typedef struct uss_context_t uss_context;

typedef struct
{
        uss_byte* buf;
        uss_uint buf_len;
} uss_local_key;

typedef struct
{
        uss_byte* buf;
        uss_uint buf_len;
} uss_bind_info;

typedef struct
{
        uss_byte* buf;
        uss_uint buf_len;
} uss_bind_key;

// ************ io层接口规范 **************** //
// 返回值，参数，错误码等，完全等同于 linux socket的 send，recv，sendto，recvfrom方法
typedef enum _USS_IO_ERROR
{
    USS_IO_EWOULDBLOCK,
    USS_IO_EAGAIN,
    USS_IO_ECONNRESET,
    USS_IO_EINTR,
    USS_IO_EPIPE,
    USS_IO_ECONNREFUSED,
    USS_IO_ECONNABORTED,
    USS_IO_GENERAL, /* general/unknown error */
    USS_IO_EUSER_IGNORE, // 返回值是<0时，此错误码表示io正常
} USS_IO_ERROR;

// 注意 peer 实现ifs接口的外部模块申请空间、填充进数据，uss负责管理，释放;
// buf 由uss申请空间，管理释放，实现ifs接口的外部模块负责放入数据
typedef uss_int (*uss_io_recvfrom_ifs)(
    uss_void *io,
    uss_byte *buf,
    uss_uint n,
    uss_void **peer,
    USS_IO_ERROR *error);
// buf 由uss申请空间，填充数据，管理释放
typedef uss_int (*uss_io_sendto_ifs)(
    uss_void *io,
    const uss_byte *buf,
    uss_uint n,
    const uss_void *peer,
    USS_IO_ERROR *error);

typedef uss_int (*uss_io_recv_ifs)(
    uss_void *io,
    uss_byte *buf,
    uss_uint n,
    USS_IO_ERROR *error);
typedef uss_int (*uss_io_send_ifs)(
    uss_void *io,
    const uss_byte *buf,
    uss_uint n,
    USS_IO_ERROR *error);

typedef uss_bool (*uss_io_peer_is_equals)(
    const uss_void *a_peer,
    const uss_void *b_peer);
// peer 拷贝构造函数，uss调用后，得到peer副本
typedef uss_void* (*uss_io_peer_new)(
    const uss_void *peer);
// peer 释放函数
typedef uss_void (*uss_io_peer_delete)(
    uss_void *peer);

// 获取io是否非堵塞
typedef uss_bool (*uss_io_get_using_nonblock)(
    uss_void *io);
typedef uss_bool (*uss_io_get_peer_using_nonblock)(
    uss_void *io,
    const uss_void *peer);
// *********************************************************************************** //
typedef struct
{
        uss_io_recv_ifs io_read;
        uss_io_send_ifs io_write;
        uss_io_get_using_nonblock io_get_nonblock;
} uss_o2o_io_method;
typedef struct
{
        uss_io_recvfrom_ifs io_read;
        uss_io_sendto_ifs io_write;
        uss_io_peer_is_equals io_peer_is_equals;
        uss_io_peer_new io_peer_new;
        uss_io_peer_delete io_peer_delete;
        uss_io_get_peer_using_nonblock io_get_nonblock;
} uss_o2m_io_method;
//#pragma anon_unions
typedef struct
{
        uss_void *io;
        USS_SOCKET_IO_TYPE socket_io_type;
        union
        {
                uss_o2o_io_method o2o_io_method;
                uss_o2m_io_method o2m_io_method;
        };
        uss_void *reserved;
} uss_io_method;

typedef uss_int (*uss_printf_ifs)(
    const char *format,
    ...);
//typedef uss_int (*uss_uptime_ms_ifs)();
uss_int uss_set_printf_method(
    uss_printf_ifs printf_impl);
//uss_int uss_set_uptime_ms_method(uss_uptime_ms_ifs uptime_ms_impl);
void uss_set_log_level(
    uss_int level);

uss_context* uss_context_new(
    USS_CTX_TYPE type);

// PRO_VER特指USS本地通信协议
uss_int uss_context_init(
    uss_context *ctx,
    uss_io_method method); // 默认 USS_CTX_PRO_VER_1
uss_int uss_context_init_pro_ver(
    uss_context *ctx,
    uss_io_method method,
    USS_CTX_PRO_VER pro_ver);

// uss_context_init 和 uss_context_init_pro_ver 都是原有接口，
// USS_CTX_PRO_VER_0_DOT_5，USS_CTX_PRO_VER_1 则创建 uss_local_o2o_client,uss_local_o2o_server 的本地uss_ctx
// USS_CTX_PRO_VER_2 则创建uss_local_client,uss_local_server 的本地uss_ctx
// uss_local_o2o_client,uss_local_o2o_server 业务协议使用 uwt
// uss_local_client,uss_local_server 业务协议使用 coap

// 对应新的需求（201901）需要 uss_local_client,uss_local_server对应 不同的USS本地通信协议和业务协议（coap，uwt）
// 使用上面的 uss_context_init或者uss_context_init_pro_ver方法,那么USS本地通信协议和业务协议是无法改变的
// 基于此，需要调用新的方法uss_context_init_pro_ver_biz_ver
// 3个注意点
//       1:调用uss_context_init_pro_ver_biz_ver进行init，则只会创建uss_local_client,uss_local_server
//       2:目前 USS本地通信协议不支持USS_CTX_PRO_VER_0_DOT_5，也就是说uss_context_init_pro_ver_biz_ver
//          这个方法pro_ver=USS_CTX_PRO_VER_0_DOT_5，不被支持
//       3:ctx虽然可以支持多个peer，但是只有USS_CTX_PRO_VER_2，才支持一个peer上关联多个设备
uss_int uss_context_init_pro_ver_biz_ver(
                                         uss_context *ctx,
                                         uss_io_method method,
                                         USS_CTX_PRO_VER pro_ver,
                                         USS_CTX_LOCAL_BIZ_VER local_biz_ver);


// 1、内部的状态机恢复到初始状态
// 2、读写缓存区的数据清空
uss_int uss_shutdown(
    uss_context *ctx);
uss_void uss_context_free(
    uss_context *ctx);

//uss_bool uss_is_write_buf_have_data(uss_context *ctx);
//void uss_flush_write_buf(uss_context *ctx);

// handshake 只解析数据，不负责读写数据，所以进行handshake时，uss_read_packet等函数必须被调用
// 如果已经握手成功，则立刻返回成功，handshake只是返回uss内部的状态机，和启动一个认证开始的流程
// 等待握手ack数据，内部处理，内部有超时，如果超时，关闭session。超时时间，可能不准，因为没有都是被外部触发的。
// handshake 根据不同情况，参数不一样，所以参见各种的头文件

/* 定义到内部
 USS_ERROR uss_read(uss_context *ctx, uss_byte *packet_buf, uss_int n, uss_uint *out_buf_len, uss_string *pdeviceid);
 USS_ERROR uss_read_packet(uss_context *ctx, uss_byte *packet_buf, uss_uint *out_buf_len, uss_string *pdeviceid);
 USS_ERROR uss_write_packet(uss_context *ctx, uss_byte *packet_buf, uss_uint buf_len, uss_string pdeviceid);

 USS_ERROR uss_read_from(uss_context *ctx, uss_byte *packet_buf, uss_int n, uss_uint *out_buf_len,
 uss_void **premoteEP, uss_string *pdeviceid);
 USS_ERROR uss_read_packet_from(uss_context *ctx, uss_byte *packet_buf, uss_uint *out_buf_len,
 uss_void **premoteEP, uss_string *pdeviceid);
 USS_ERROR uss_write_packet_to(uss_context *ctx, uss_byte *packet_buf, uss_uint buf_len,
 uss_void *remoteEP, uss_string pdeviceid);
 */

/**
 * @brief 生成设备连接平台时的认证信息
 * @param key 产品密钥
 * @param key_len 产品密钥长度
 * @param in 产品的uplusID
 * @param in_len 产品的uplusID长度
 * @param out 认证信息
 * @param out_len 认证信息的长度
 * @return 0:成功， <0:失败
 */
uss_int uss_encrpyt_product_info(
    const uss_byte * key,
    const uss_int32 key_len,
    const uss_byte * in,
    const uss_int32 in_len,
    uss_byte ** out,
    uss_int32 * out_len);

/**
 * @brief 生成绑定信息
 * @param bind_key 设备的bind_key
 * @param bind_key_len 设备bind_key的长度
 * @param user_token 用户的token
 * @param user_token_len 用户token的长度
 * @param out 绑定信息字符串缓存，由调用者释放
 * @return
 */
uss_int uss_make_bound_info(
    const uss_byte * bind_key,
    const uss_int32 bind_key_len,
    const uss_byte * user_token,
    const uss_int32 user_token_len,
    uss_char ** out);

/**
 * @brief 生成随机数据，保存在缓存中
 * @param in_out 缓存
 * @param in_len 缓存长度
 * @return >=0: 成功, <0: 失败
 */
uss_int uss_gen_rand_bytes(
    uss_byte * in_out,
    uss_int32 in_len);

/*
 * @brief encrypt the in data use AES128
 * @param key, the raw key string, aes_key = md5(key)
 * @param key_len, length of the raw key
 * @param in, the data to be encrypted
 * @param in_len, length of in data
 * @param out, encrypted data, the caller should free it
 * @param out_len, length of out data, the caller should free it
 * @return 0: success, <0: failure
 */
uss_int uss_encrpyt_data(const uss_byte * key,
        const uss_int32 key_len,
        const uss_byte * in,
        const uss_int32 in_len,
        uss_byte ** out,
        uss_int32 * out_len);

/*
 * @brief decrypt the in data use AES128
 * @param key, the raw key string, aes_key = md5(key)
 * @param key_len, length of the raw key
 * @param in, the data to be decrypted
 * @param in_len, length of in data
 * @param out, decrypted data, the caller should free it
 * @param out_len, length of out data, the caller should free it
 * @return 0: success, <0: failure
 */
uss_int uss_decrpyt_data(const uss_byte * key,
        const uss_int32 key_len,
        const uss_byte * in,
        const uss_int32 in_len,
        uss_byte ** out,
        uss_int32 * out_len);

#define USS_PKS_AUTH_TYPE_UNK      0x00
#define USS_PKS_AUTH_TYPE_PSK0     0x01
#define USS_PKS_AUTH_TYPE_CERT0    0x02
#define USS_PKS_AUTH_TYPE_ECDHE0   0x03
struct uss_pks_auth_ctx;
typedef struct uss_pks_auth_ctx uss_pks_auth_ctx_t;
uss_pks_auth_ctx_t * uss_pks_auth_context_new(void);
uss_void uss_pks_auth_context_free(
    uss_pks_auth_ctx_t *ctx);
uss_void uss_pks_auth_context_set_type(
    uss_pks_auth_ctx_t *ctx,
    uss_int type);
uss_int uss_pks_auth_context_psk_set_param(
    uss_pks_auth_ctx_t *ctx,
    uss_char *dev_id_str,
    uss_byte *psk,
    uss_int psk_len);
void uss_pks_auth_context_psk_get_param(
    uss_pks_auth_ctx_t *ctx,
    const uss_char **dev_id_str,
    const uss_byte **psk,
    uss_int * psk_len);
uss_int uss_pks_auth_context_ecdh_get_secret(
    uss_pks_auth_ctx_t *ctx,
    uss_byte **secret,
    uss_int *p_secret_len);

uss_int uss_pks_auth_svr_parse_peer(
    uss_pks_auth_ctx_t *ctx,
    uss_byte *data,
    uss_int len);
uss_int uss_pks_auth_svr_make_mine(
    uss_pks_auth_ctx_t *ctx,
    uss_byte **data,
    uss_int *p_len);
uss_int uss_pks_auth_svr_verify(
    uss_pks_auth_ctx_t *ctx);

uss_int uss_pks_auth_cli_make_mine(
    uss_pks_auth_ctx_t *ctx,
    uss_byte **data,
    uss_int *p_len);
uss_int uss_pks_auth_cli_parse_peer(
    uss_pks_auth_ctx_t *ctx,
    uss_byte *data,
    uss_int len);
uss_int uss_pks_auth_cli_verify(
    uss_pks_auth_ctx_t *ctx);

#endif
