//
//  uss_cloud_device_v2.h
//  uss_project
//
//  Created by dinglonghao on 16-9-19.
//  Copyright (c) 2016年 dinglonghao. All rights reserved.
//

#ifndef uss_project_uss_cloud_device_v2_h
#define uss_project_uss_cloud_device_v2_h

#include "uss_cloud_device.h"

#ifdef USS_HAVE_M_CD_V2

typedef uss_void (*uss_cd_notify_new_localkey_callback_v2)(const uss_string device_id,
        const uss_local_key *key,
        uss_void *localkey_cb_param);

typedef uss_void (*uss_cd_notify_get_bindkey_callback_v2)(const uss_string device_id,
        uss_void *bindkey_cb_param);

// 设置主deviceid，根据这个主deviceid，能获取到其它设备的localkey，
// 供内部获取，及uss_cd_reg_external_localkey_notify_callback_ext 上报
uss_void uss_cd_set_master_device_v2(uss_context *ctx,
        const uss_string device_id);

uss_int uss_cd_add_device_data_v2(uss_context *ctx,
        const uss_string device_id);
uss_int uss_cd_rmv_device_data_v2(uss_context *ctx,
        const uss_string device_id);

/*
 uss_cd_notify_new_localkey_callback_ext:提示dev存储localkey
 uss_cd_notify_get_bindkey_callback_ext：提示dev，uss收到了bindkey；
 具体应用dev配置时30s收到bindkey时才允许被搜索
 */
uss_void uss_cd_reg_self_notify_callback_v2(uss_context *ctx,
        uss_cd_notify_new_localkey_callback_v2 localkey_cb,
        const uss_void* localkey_cb_param,
        uss_cd_notify_get_bindkey_callback_v2 bindkey_cb,
        const uss_void* bindkey_cb_param);

// 非自身的设备，其它设备localkey的变化通知
uss_void uss_cd_reg_external_localkey_notify_callback_v2(uss_context *ctx,
        uss_cd_notify_new_localkey_callback_v2 localkey_cb,
        const uss_void* localkey_cb_param);

uss_int uss_cd_do_v2(uss_context *ctx);

/*
 device,判断localkey为NULL来做为是设备强制要localkey的依据，
 另外对比新的localkey是否跟旧的一样，来确定是否存储
 */
uss_void uss_cd_set_localkey_v2(uss_context *ctx,
        const uss_string device_id,
        const uss_local_key *localkey);


typedef struct _uss_set_bind_time_window_ret
{
    uss_string deviceid;
    uss_int error;
} uss_set_bind_time_window_ret;
typedef void (*uss_cd_set_bind_time_window_completion_cb_v2)(uss_void *context,
                                                const uss_set_bind_time_window_ret *retlist,
                                                uss_int retlist_num);
/*
 开启绑定时间窗口
 */
uss_int uss_cd_set_bind_time_window_v2(uss_context *ctx,
                            uss_void *context,
                            const uss_string *device_id_list, uss_int device_id_num,
                            uss_cd_set_bind_time_window_completion_cb_v2 cb);



/**
 * @brief 获取bindkey结果
 * @param deviceid 设备id
 * @param error 结果错误码, 具体参见设备侧接入协议
 * @param bind_key 数据信息(数据+长度)
 */
typedef struct _uss_get_bind_key_ret_v2
{
    uss_string deviceid;
    uss_int error;
    uss_bind_key bind_key;
} uss_get_bind_key_ret_v2;
/**
 * @brief 获取bindkey结果通知
 * @param context 应用传入的上下文参数
 * @param retlist 结果列表
 * @param retlist_num 结果列表个数
 */
typedef void (*uss_cd_get_bind_key_completion_cb_v2)(uss_void *context,
                                            const uss_get_bind_key_ret_v2 *retlist,
                                            uss_int retlist_num);
/**
 * @brief 获取多个设备的bindkey数据
 * @param ctx ctx句柄
 * @param device_id_list 需要获取bindkey的设备集合
 * @param device_id_num 设备集合数
 * @param cb 结果通知回调函数指针
 * @return 0: 成功， <0: 失败
 */
uss_int uss_cd_get_bind_key_v2(uss_context *ctx,
                               uss_void *context,
                               const uss_string *device_id_list, uss_int device_id_num,
                               uss_cd_get_bind_key_completion_cb_v2 cb);

#endif // USS_HAVE_M_CD_V2

#endif
