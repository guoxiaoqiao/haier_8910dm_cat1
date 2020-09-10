
/**************************************************************************
 * SHA256 declarations 
 **************************************************************************/

#ifndef _ZK_SHA256_H
#define _ZK_SHA256_H

#include "haier_appmain.h"

#define SHA256_SIZE   32

typedef struct
{
    uint32_t total[2];
    uint32_t state[8];
    uint8_t buffer[64];
	
} zk_SHA256_CTX;

extern void zk_SHA256_Init(zk_SHA256_CTX *ctx);

extern void zk_SHA256_Update(zk_SHA256_CTX *ctx, const uint8_t * msg, int len);

extern void zk_SHA256_Final(uint8_t *digest, zk_SHA256_CTX *ctx);

#endif

