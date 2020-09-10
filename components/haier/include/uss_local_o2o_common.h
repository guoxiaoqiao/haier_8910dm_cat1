//
//  uss_local_o2o_common.h
//  uss
//
//  Created by dinglonghao on 16-5-5.
//  Copyright (c) 2016年 dinglonghao. All rights reserved.
//

#ifndef uss_uss_local_o2o_common_h
#define uss_uss_local_o2o_common_h

#include "uss_common.h"

#if (defined USS_HAVE_M_LC_O2O || defined USS_HAVE_M_LS_O2O)

uss_int uss_l_o2o_bind_device(uss_context *ctx,
        const uss_string deviceid);

uss_int uss_l_o2o_set_localkey(uss_context *ctx,
        const uss_local_key *localkey);

USS_ERROR uss_l_o2o_handshake(uss_context *ctx);

// *packet_buf uss 内部申请内存，外部释放
USS_ERROR uss_l_o2o_read_packet(uss_context *ctx,
        uss_byte **packet_buf,
        uss_uint *out_packet_len);
USS_ERROR uss_l_o2o_write_packet(uss_context *ctx,
        const uss_byte *packet_buf,
        uss_uint packet_buf_len);

#endif

#endif
