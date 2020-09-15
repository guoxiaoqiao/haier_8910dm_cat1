#ifndef    _H_SSL_H_
#define    _H_SSL_H_

#include "haier_appmain.h"
#include "pal_common.h"

typedef struct {

		uint16_t rx;
		uint8_t *ssl_recv_buf;
		uint16_t buffLen;
		
}SSL_RECV;


typedef struct {
	
    int fd;      
	
}mbedtls_context;

typedef struct{

	//这是我自己添加的
	mbedtls_context server_fd;
  	//这是mbedtls库的一些结构体
    mbedtls_entropy_context *entropy;
    mbedtls_ctr_drbg_context *ctr_drbg;
    mbedtls_ssl_context *ssl;
    mbedtls_ssl_config *conf;
    mbedtls_x509_crt *cacert;
		
}ssl_session_ctx;

extern uplus_ctx_id net_ssl_client_create(uplus_s32 fd, struct uplus_ca_chain *root_ca, uplus_u8 root_ca_num);
extern uplus_s32 net_ssl_client_handshake(uplus_ctx_id id);
extern uplus_s32 net_ssl_client_close(uplus_ctx_id id);
extern uplus_s32 net_ssl_pending(uplus_ctx_id id);
extern uplus_s32 net_ssl_read(uplus_ctx_id id, uplus_u8 *buf, uplus_size_t len);
extern uplus_s32 net_ssl_write(uplus_ctx_id id, uplus_u8 *buf, uplus_size_t len);

#endif