//
//  uss_local_client.h
//  uss_project
//
//  Created by dinglonghao on 16-7-15.
//  Copyright (c) 2016年 dinglonghao. All rights reserved.
//

#ifndef uss_local_client_h
#define uss_local_client_h

#include "uss_common.h"

#ifdef USS_HAVE_M_LC

uss_int uss_lc_set_valid_peer(uss_context *ctx, const uss_void *peer);

typedef enum _USS_LC_CLOUD_DATA_TYPE
{
    USS_LC_CD_USERTOKEN = 1, // 一般是手机测模式
    USS_LC_CD_DEVICEID = 2, // 一般是ugw模式
} USS_LC_CLOUD_DATA_TYPE;
typedef struct
{
    USS_LC_CLOUD_DATA_TYPE type;
    union
    {
        uss_string usertoken;
        uss_string deviceid;
    };
    uss_void *reserved;
} uss_lc_cloud_data;

// 在uss_context_init后调用
// 调用后，即表示ctx只有唯一一个设备
// 那么后续入参deviceid将被忽略
//
// 如果USS_CTX_PRO_VER_0_DOT_5和USS_CTX_PRO_VER_1，则必须调用这个方法
// USS_CTX_PRO_VER_2,不是必须调用;如果不调用，则后续入参deviceid必须有值
uss_int uss_lc_bind_sole_device(uss_context *ctx, const uss_string deviceid);

uss_int uss_lc_set_cloud_data(uss_context *ctx, const uss_lc_cloud_data cloud_data);

uss_int uss_lc_set_localkey(uss_context *ctx,
                            const uss_string deviceid,
                            const uss_local_key *localkey);

USS_ERROR uss_lc_handshake(uss_context *ctx, const uss_string deviceid);

typedef void (*uss_lc_bindinfo_completion_cb)(uss_void *context,
                                                const uss_string deviceid,
                                                uss_int error,
                                                const uss_bind_info* bindinfo);
uss_int uss_lc_get_bindinfo(uss_context *ctx,
                            uss_void *context,
                            const uss_string deviceid,
                            uss_lc_bindinfo_completion_cb cb);

// packet_buf uss 内部申请内存，填充数据；外部释放内存
// deviceid uss内部申请内存，uss负责管理内存，外部只能使用
USS_ERROR uss_lc_read_packet(uss_context *ctx,
                             const uss_char **deviceid,
                             uss_byte **packet_buf,
                             uss_uint *out_packet_len);
// packet_buf和deviceid 外部申请，填充数据，管理释放内存；uss只负责使用
USS_ERROR uss_lc_write_packet(uss_context *ctx,
                              const uss_string deviceid,
                              const uss_byte *packet_buf,
                              uss_uint packet_buf_len);

// 对设备的handshake的当前状态恢复到起始状态
// deviceid = NULL，表示ctx下的所有设备，全部恢复到起始状态
uss_int uss_lc_reset_handshake(uss_context *ctx, const uss_string deviceid);

// 对设备所有状态全部恢复到起始状态，数据全部清空
// 加入cloud参数,是为了清理对应的device的localkey缓存数据
// deviceid = NULL，表示ctx下的所有设备
uss_int uss_lc_reset_device(uss_context *ctx, const uss_lc_cloud_data cloud_data, const uss_string deviceid);

#endif // USS_HAVE_M_LC

#endif
