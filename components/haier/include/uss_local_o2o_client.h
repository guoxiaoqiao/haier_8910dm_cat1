//
//  uss_local_o2o_client.h
//  uss
//
//  Created by dinglonghao on 16-5-5.
//  Copyright (c) 2016年 dinglonghao. All rights reserved.
//

#ifndef uss_uss_local_o2o_client_h
#define uss_uss_local_o2o_client_h

#include "uss_local_o2o_common.h"

#ifdef USS_HAVE_M_LC_O2O

uss_int uss_lc_o2o_set_cloud_data(uss_context *ctx,
        const uss_string usertoken);

uss_int uss_lc_o2o_get_bindinfo(uss_context *ctx,
        uss_void *context,
        void (*completionHandler)(uss_void *context,
                uss_int error,
                const uss_bind_info* bindinfo));

uss_int uss_lc_o2o_set_cloud_data_master_deviceid(uss_context *ctx,
                                  const uss_string master_deviceid);

#endif

#endif
