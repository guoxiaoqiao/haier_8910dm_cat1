//
//  uss_cloud_common.h
//  uss
//
//  Created by dinglonghao on 16-4-29.
//  Copyright (c) 2016年 dinglonghao. All rights reserved.
//

#ifndef uss_uss_cloud_common_h
#define uss_uss_cloud_common_h

#include "uss_common.h"

#if (defined USS_HAVE_M_CU || defined USS_HAVE_M_CD || defined USS_HAVE_M_CD_V2)

USS_ERROR uss_c_connect(uss_context *ctx);

// *packet_buf uss 内部申请内存，外部释放
USS_ERROR uss_c_read_packet(uss_context *ctx,
        uss_byte **packet_buf,
        uss_uint *out_packet_len);
// 
USS_ERROR uss_c_write_packet(uss_context *ctx,
        const uss_byte *packet_buf,
        uss_uint packet_buf_len);

#endif

#endif
