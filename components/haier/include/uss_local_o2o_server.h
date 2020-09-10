//
//  uss_local_o2o_server.h
//  uss
//
//  Created by dinglonghao on 16-5-5.
//  Copyright (c) 2016年 dinglonghao. All rights reserved.
//

#ifndef uss_uss_local_o2o_server_h
#define uss_uss_local_o2o_server_h

#include "uss_local_o2o_common.h"

#ifdef USS_HAVE_M_LS_O2O

//int uss_ls_o2o_set_bindkey(uss_context *ctx, uss_bind_key *key);

// 触发uss 检查当前使用的localkey是否有效
uss_int uss_ls_do_check_localkey(uss_context *ctx);

USS_ERROR uss_ls_o2o_get_bindinfo(uss_context *ctx,
                              const uss_byte *in_bindinfo_buf,
                              uss_int32 in_buf_len,
                              uss_byte **out_bindinfo_buf,
                              uss_int32 *out_buf_len);

USS_ERROR uss_ls_o2o_get_bindinfo_by_device_id(const uss_string device_id,
                                  const uss_byte *in_bindinfo_buf,
                                  uss_int32 in_buf_len,
                                  uss_byte **out_bindinfo_buf,
                                  uss_int32 *out_buf_len);

#endif

#endif
