//
//  uss_local_server.h
//  uss_project
//
//  Created by dinglonghao on 16-7-15.
//  Copyright (c) 2016年 dinglonghao. All rights reserved.
//

#ifndef uss_local_server_h
#define uss_local_server_h

#include "uss_common.h"

#ifdef USS_HAVE_M_LS

uss_int uss_ls_add_device(uss_context *ctx,
        const uss_string deviceid);

uss_int uss_ls_set_localkey(uss_context *ctx,
        const uss_string deviceid,
        const uss_local_key *localkey);

typedef void (*uss_ls_handshake_result_callback)
(const uss_void *peer,
        const uss_char *deviceid,
        USS_ERROR result,
        uss_void *cb_param);

// handshake 参数只有一ctx，持续调用，只有不可恢复等
// 异常，才返回错误。只要返回错误，必须关闭ctx，新建ctx。
// 内部进行握手处理，最后通过回调方式，通知握手成功，失败
// 通知参数，ctx，peer，deviceid。
// 最后讨论，直接返回结构体了，没有回调方式了。
//USS_ERROR uss_ls_handshake(uss_context *ctx, uss_void *peer, const uss_string deviceid);

// peer,deviceid都是出参，外部模块只能使用，内存管理由uss负责
USS_ERROR uss_ls_handshake(uss_context *ctx,
        uss_ls_handshake_result_callback cb,
        const uss_void *cb_param);

/*
 typedef void (*uss_ls_handshake_device_add_cb)(uss_context *ctx, uss_void *peer,
 const uss_string deviceid, uss_void *cb_param);
 typedef void (*uss_ls_handshake_device_rmv_cb)(uss_context *ctx, uss_void *peer,
 const uss_string deviceid, uss_void *cb_param);

 int uss_ls_set_handshake_device_cb(uss_context *ctx, uss_void *peer,
 uss_ls_handshake_device_add_cb add_cb, uss_void *add_cb_param,
 uss_ls_handshake_device_rmv_cb rmv_cb, uss_void *rmv_cb_param);
 */

// 触发uss 检查当前使用的localkey是否有效，及bindkey的更新任务等
uss_int uss_ls_do(uss_context *ctx);

// packet_buf: uss内部申请内存，填充数据；外部使用，释放内存
// deviceid: uss负责管理内存，外部只能使用
// peer:uss内部申请内存，填充数据；外部使用，释放内存
// 如果是内部数据，*peer，*deviceid，*packet_buf都是null。
USS_ERROR uss_ls_read_packet(uss_context *ctx,
        uss_void **peer,
        const uss_char **deviceid,
        uss_byte **packet_buf,
        uss_uint *out_packet_len);
// packet_buf和deviceid和peer 外部申请，填充数据，管理释放内存；uss只负责使用
USS_ERROR uss_ls_write_packet(uss_context *ctx,
        const uss_void *peer,
        const uss_string deviceid,
        const uss_byte *packet_buf,
        uss_uint packet_buf_len);

uss_int uss_ls_shutdown_peer(uss_context *ctx,
        const uss_void *peer);

// 如果peer=null，则表示需要关闭所有peer的，指定设备
uss_int uss_ls_shutdown_device(uss_context *ctx,
        const uss_void *peer,
        const uss_string deviceid);
uss_void uss_test_ls_set_localkey(uss_context *ctx, const uss_string deviceid, const uss_local_key *localkey);

USS_ERROR uss_ls_get_bindinfo(uss_context *ctx,
                              const uss_string deviceid,
                              const uss_byte *in_bindinfo_buf,
                              uss_int32 in_buf_len,
                              uss_byte **out_bindinfo_buf,
                              uss_int32 *out_buf_len);
#endif // USS_HAVE_M_LS

#endif
