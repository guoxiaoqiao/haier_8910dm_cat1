//
//  uss_cloud_user.h
//  uss
//
//  Created by dinglonghao on 16-4-23.
//  Copyright (c) 2016年 dinglonghao. All rights reserved.
//

#ifndef uss_uss_cloud_user_h
#define uss_uss_cloud_user_h

#include "uss_cloud_common.h"

#ifdef USS_HAVE_M_CU

typedef void (*uss_cu_notify_new_localkey_callback)(uss_context *ctx,
        const uss_string usertoken,
        const uss_string device_id,
        const uss_int error,
        const uss_local_key *key,        
        uss_void *cb_param);

// 目前实现，ctx最多对应一个usertoken，多次调用，最后一次覆盖前面的
uss_int uss_cu_add_user_data(uss_context *ctx,
        const uss_string usertoken,
        uss_cu_notify_new_localkey_callback cb,
        const uss_void *cb_param);

uss_int uss_cu_rmv_user_data(uss_context *ctx,
        const uss_string usertoken);

uss_int uss_cu_do_get_localkey(uss_context *ctx);

uss_int uss_cu_request_device_localkey(uss_context *ctx,
        const uss_string device_id);

#endif

#endif
