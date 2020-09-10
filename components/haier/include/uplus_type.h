
/*!
 * \file uplus_type.h
 * \brief 基础数据类型定义。
 *
 * \date 2017-01-11
 * \author fanming
 *
 */

#ifndef __UPLUS_TYPE_H__
#define __UPLUS_TYPE_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char uplus_u8;
typedef unsigned short uplus_u16;
typedef unsigned int uplus_u32;
typedef char uplus_s8;
typedef short uplus_s16;
typedef int uplus_s32;
typedef uplus_u32 uplus_time;
typedef unsigned int uplus_size_t;
typedef int uplus_socklen_t;
typedef void * uplus_ctx_id;
typedef void * uplus_task_id;
typedef void * uplus_mutex_id;
typedef void * uplus_sem_id;
typedef long uplus_max_size_type;
typedef unsigned long long uplus_u64;

#define UPLUS_COMPARE_U64(a, b) (a) > (b)
#define UPLUS_SET_U64(a, b) (a) = (b)

#ifndef NULL
#define NULL ((void*) 0)
#endif

#ifdef __cplusplus
}
#endif

#endif /*__UPLUS_TYPE_H__*/

