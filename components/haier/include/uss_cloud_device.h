//
//  uss_cloud_device.h
//  uss
//
//  Created by dinglonghao on 16-4-23.
//  Copyright (c) 2016年 dinglonghao. All rights reserved.
//

#ifndef uss_uss_cloud_device_h
#define uss_uss_cloud_device_h

#include "uss_cloud_common.h"

#ifdef USS_HAVE_M_CD

// sn = 0,表示不是uss_cd_localkey_req， uss_cd_bindkey_req触发的，有uss内部触发
typedef uss_void (*uss_cd_notify_new_localkey_callback)(const uss_string device_id,
        const uss_local_key *key,
        uss_void *localkey_cb_param);

typedef uss_void (*uss_cd_notify_get_bindkey_callback)(const uss_string device_id,
        uss_void *bindkey_cb_param);

// 新加接口，设置主deviceid，根据这个主deviceid，能获取到其它设备的localkey

// add_device 只有ctx，device_id 2个参数
// 回调单独设置，2种回调
// 1、自身的设备的localkey，bindkey回调
// 2、获取的其它设备localkey回调，作用同uss_cu_notify_new_localkey_callback
uss_int uss_cd_add_device_data(uss_context *ctx,
        const uss_string device_id,
        uss_cd_notify_new_localkey_callback localkey_cb,
        const uss_void* localkey_cb_param,
        uss_cd_notify_get_bindkey_callback bindkey_cb,
        const uss_void* bindkey_cb_param);

uss_int uss_cd_rmv_device_data(uss_context *ctx,
        const uss_string deviceid);

// 触发uss内部处理（更新数据等），时间间隔属于uss内部业务，但是也依赖于外部调用的时间间隔
// 自动更新bindkey内部功能去掉 by dinglh 20181023
uss_int uss_cd_do(uss_context *ctx);

void uss_cd_set_localkey(uss_context *ctx,
        const uss_local_key *localkey);

uss_int uss_cd_get_timespan_localkey(uss_context *ctx);

uss_int uss_cd_get_timespan_bindkey(uss_context *ctx);


// get_sn 是由uss_cd_get_bind_key传出的 get_sn值，对应，以便调用者对应每次调用的结果
typedef void (*uss_cd_get_bind_key_callback)(uss_void *context,uss_int get_sn,
                                                uss_int error, const uss_bind_key bind_key);
// 设置获取bindkey的回调通知函数
uss_int uss_cd_set_get_bind_key_callback(uss_context *ctx, uss_void *context, uss_cd_get_bind_key_callback cb);
// 获取设备的bindkey，同时通过get_sn传出这次调用的sn，在uss_cd_get_bind_key_callback回调通知中和get_sn对应
// 获取设备bindkey后，同时调用uss_cd_notify_get_bindkey_callback回调通知
uss_int uss_cd_get_bind_key(uss_context *ctx, uss_void *context, uss_int *get_sn);


// 默认is_at_once = true,uss_cd_do调用后，立刻获取localkey
// is_at_once = false,即使uss_cd_do调用后，也是大概很长时间后，才开始获取localkey，这个时间段目前是2个半小时
uss_int uss_cd_set_get_localkey_at_once_flag(uss_context *ctx, uss_bool is_at_once);

#endif

#endif
