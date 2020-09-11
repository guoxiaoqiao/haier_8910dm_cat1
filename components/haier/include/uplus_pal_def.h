
/*!
 * \file uplus_pal_def.h
 * \brief 平台抽象层接口定义.
 *
 * \date 2018-04-19
 * \author fanming
 *
 */

#ifndef __UPLUS_PAL_DEF_H__
#define __UPLUS_PAL_DEF_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "uplus_type.h"
#include "string.h"

//#define uplus_sys_log iot_debug_print

/*! \def OP_XXX
 *
 * \brief PAL接口操作
 * SET 设置。
 * GET 查询。
 * START 开始。
 * STOP 结束。
 * ADD 增加。
 * DELETE 删除。
 *
 */
#define OP_SET 1
#define OP_GET 2
#define OP_START 1
#define OP_STOP 2
#define OP_ADD 1
#define OP_DELETE 2

/*!
 * \defgroup OS PAL APIs
 * \details OS相关PAL接口。
 * \note
 * \{
 */

/*! \def TIME_NO_WAIT/TIME_WAIT_FOREVER
 *
 * \brief 获取资源时的等待操作，用于信号量和队列操作。
 * TIME_NO_WAIT 如果资源不能获得，不等待。
 * TIME_WAIT_FOREVER 一直等待，知道获取到资源。
 *
 */
#define TIME_NO_WAIT 0
#define TIME_WAIT_FOREVER (-1)

/*! \def SEM_TAKE_TIMEOUT
 *
 * \brief 获取资源时的返回值，表示超时未获得资源。
 *
 */
#define SEM_TAKE_TIMEOUT (-2)

/*!
 * \typedef task_func
 * \brief 任务处理接口。
 * \param [in] para 参数。
 * \return N/A。
 */
typedef void (*task_func)(void *para);

/*!
 * \brief 创建任务。
 * \param [in] name 任务名称，以‘\0’结尾的字符串。
 * \param [in] stack_size 任务栈大小，单位字节。仅包含uplugSDK占用的栈空间，PAL占用的栈空间需要额外计算。
 * \param [in] priority 任务优先级，取值0-7，共8个优先级，0最高。
 * \param [in] func 任务处理函数。
 * \param [in] para 函数参数。
 * \param [out] id 如果创建成功，返回创建的任务ID，任务ID不能为NULL；否则不填写。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_os_task_create(uplus_s8 * name, uplus_u32 stack_size, uplus_u8 priority, task_func func, void *para, uplus_task_id *id);

/*!
 * \brief 删除任务。
 * \param [in] id 任务ID（成功创建任务返回的任务ID）。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_os_task_delete(uplus_task_id id);

/*!
 * \brief 任务睡眠（当前任务）。
 * \param [in] delay 任务睡眠时间，单位毫秒。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_os_task_sleep(uplus_u32 delay);

/*!
 * \brief 创建互斥信号量。
 * \param [out] id 如果创建成功，返回创建的信号量ID；否则不填写。信号量ID不能为NULL。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_os_mutex_create(uplus_mutex_id *id);

/*!
 * \brief 获取互斥信号量。
 * \param [in] id 信号量ID。
 * \param [in] time_wait 等待操作，TIME_NO_WAIT、TIME_WAIT_FOREVER或者等待时间（单位毫秒）。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_os_mutex_take(uplus_mutex_id id, uplus_s32 time_wait);

/*!
 * \brief 释放互斥信号量。
 * \param [in] id 信号量ID。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_os_mutex_give(uplus_mutex_id id);

/*!
 * \brief 删除互斥信号量。
 * \param [in] id 信号量ID。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_os_mutex_delete(uplus_mutex_id id);

/*!
 * \brief 创建同步信号量。
 * \param [out] id 如果创建成功，返回创建的信号量ID；否则不填写。信号量ID不能为NULL。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_os_sem_create(uplus_sem_id *id);

/*!
 * \brief 获取同步信号量。
 * \param id 信号量ID。
 * \param [in] time_wait 等待操作，TIME_NO_WAIT、TIME_WAIT_FOREVER或者等待时间（单位毫秒）。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_os_sem_take(uplus_sem_id id, uplus_s32 time_wait);

/*!
 * \brief 释放同步信号量。
 * \param [in] id 信号量ID。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_os_sem_give(uplus_sem_id id);

/*!
 * \brief 删除互斥信号量。
 * \param [in] id 信号量ID。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_os_sem_delete(uplus_sem_id id);

/*!
 * \brief 获取系统运行时间
 * \note 获取系统运行时间。uplugSDK不关心系统时间单位，但时间单位精度要小于100毫秒。
 * \return 系统运行时间。
 */
extern uplus_time uplus_os_current_time_get(void);

/*!
 * \brief 计算时间差。
 * \param [in] new_time 新时间（uplus_os_current_time_get返回的时间）。
 * \param [in] old_time 旧时间（uplus_os_current_time_get返回的时间）。
 * \return 时间差，单位毫秒。
 */
extern uplus_time uplus_os_diff_time_cal(uplus_time new_time, uplus_time old_time);

/*!
 * \}
 */

/*!
 * \defgroup TOOL PAL APIs
 * \details TOOL相关PAL接口。
 * \note
 * \{
 */

/*!
 * \brief C库memcpy。
 */
extern void *uplus_tool_memcpy(void *dest, const void *src, uplus_size_t n);

/*!
 * \brief C库memmove。
 */
extern void *uplus_tool_memmove(void *dest, const void *src, uplus_s32 n);

/*!
 * \brief C库memset。
 */
extern void *uplus_tool_memset(void *s, uplus_s32 c, uplus_s32 n);

/*!
 * \brief C库memcmp。
 */
extern uplus_s32 uplus_tool_memcmp(const void *s1, const void *s2, uplus_size_t n);

/*!
 * \brief C库strncmp。
 */
extern uplus_s32 uplus_tool_strncmp(const uplus_s8 *s1, const uplus_s8 *s2, uplus_size_t n);

/*!
 * \brief C库strcmp。
 */
extern uplus_s32 uplus_tool_strcmp(const uplus_s8 *s1, const uplus_s8 *s2);

/*!
 * \brief C库strncpy。
 */
extern uplus_s8 * uplus_tool_strncpy(uplus_s8 *dest, const uplus_s8 *src, uplus_size_t n);

/*!
 * \brief C库strlen。
 */
extern uplus_size_t uplus_tool_strlen(uplus_s8 *s);

/*!
 * \brief C库snprintf。
 */
extern uplus_s32 uplus_tool_snprintf(uplus_s8* str, uplus_size_t size, const uplus_s8* fmt, ...);

/*!
 * \brief C库rand。
 */
extern uplus_s32 uplus_tool_rand(void);

/*!
 * \brief C库srand。
 */
extern void uplus_tool_srand(uplus_u32 seed);

/*!
 * \brief C库strtol。
 */
extern uplus_s32 uplus_tool_strtol(const uplus_s8 *str, uplus_s8 **endptr, uplus_s32 base);

/*!
 * \brief C库strchr。
 */
extern uplus_s8 * uplus_tool_strchr(const uplus_s8 *str, uplus_s32 c);

/*!
 * \brief 申请内存。
 * \param [in] n 申请内存的大小，单位字节。
 * \return 成功返回内存指针，失败返回NULL。
 */
extern void *uplus_tool_malloc(uplus_size_t n);

/*!
 * \brief 申请数据内存。
 * \note 数据内存是可以与外设交互的内存，例如可用于DMA的内存，硬件加解密可以访问的内存等等。
 * \param [in] n 申请内存的大小，单位字节。
 * \return 成功返回内存指针，失败返回NULL。
 */
extern void *uplus_tool_data_malloc(uplus_size_t n);

/*!
 * \brief 释放内存。
 * \param [in] ptr，内存指针。
 * \return N/A。
 */
extern void uplus_tool_free(void *ptr);

/*!
 * \brief MD5上下文初始化。
 * \param [in] ctx 如果成功初始化，返回上下文标识；否则不填写。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_tool_md5_init(uplus_ctx_id *ctx);

/*!
 * \brief MD5增量计算。
 * \param [in] ctx 上下文标识。
 * \param [in] buf 数据指针。
 * \param [in] len 数据长度。
 * \return N/A。
 */
extern void uplus_tool_md5_update(uplus_ctx_id ctx, uplus_u8 *buf, uplus_s32 len);

/*!
 * \brief 结束MD5计算。
 * \param [in] ctx 上下文标识。
 * \param [out] output MD5计算的摘要值，16字节。
 * \return N/A。
 */
extern void uplus_tool_md5_finish(uplus_ctx_id ctx, uplus_u8 *output/*16*/);

/*!
 * \brief SHA256上下文初始化。
 * \param [in] ctx 如果成功初始化，返回上下文标识；否则不填写。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_tool_sha256_init(uplus_ctx_id *ctx);

/*!
 * \brief SHA256增量计算。
 * \param [in] ctx 上下文标识。
 * \param [in] buf 数据指针。
 * \param [in] len 数据长度。
 * \return N/A。
 */
extern void uplus_tool_sha256_update(uplus_ctx_id ctx, uplus_u8 *buf, uplus_s32 len);

/*!
 * \brief 结束SHA256计算。
 * \param [in] ctx 上下文标识。
 * \param [out] output SHA256计算的摘要值，32字节。
 * \return N/A。
 */
extern void uplus_tool_sha256_finish(uplus_ctx_id ctx, uplus_u8 *output/*32*/);

/*! \def MODE_ENCRYPT/MODE_DECRYPT
 *
 * \brief 加解密操作。
 */
#define MODE_ENCRYPT 1
#define MODE_DECRYPT 0

/*!
 * \brief AES CBC加解密计算。
 * \param [in] mode 操作类型，MODE_ENCRYPT/MODE_DECRYPT。
 * \param [in] key 密钥。
 * \param [in] key_len 密钥长度，128比特或者256比特。
 * \return N/A。
 */
extern void uplus_tool_aes_crypt_cbc(uplus_s32 mode, uplus_u8 * key, uplus_s32 key_len/*128 or 256*/, uplus_u8 * in, uplus_s32 in_len, uplus_u8 *out);

/*! \def UPLUS_RSA_XXX
 *
 * \brief RSA公私钥标识。
 *
 * \details - XXX包括PUBLIC和PRIVATE。
 */
#define UPLUS_RSA_PUBLIC 0
#define UPLUS_RSA_PRIVATE 1

/*!
 * \brief 初始化RSA公钥上下文。
 *
 * \param [in] key RSA公钥，通常是PEM格式。
 * \param [in] key_len RSA公钥长度。
 * \param [out] out_len 输出的报文长度。
 *
 * \return RSA公钥上下文。
 */
extern void * uplus_tool_rsa_public_init(const uplus_u8 *key, uplus_size_t key_len, uplus_size_t *out_len);

/*!
 * \brief RSA加密。
 *
 * \param [in] ctx RSA公钥/私钥上下文。
 * \param [in] mode UH_RSA_XXX，。
 * \param [in] in 待加密的数据。
 * \param [in] in_len 待加密的数据长度。
 * \param [out] out 输出，加密后的数据。
 *
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_tool_rsa_encrypt(
	void *ctx, uplus_s32 mode,
	const uplus_u8 *in, uplus_size_t in_len,
	uplus_u8 *out);

/*!
 * \brief RSA解密。
 *
 * \param [in] ctx RSA公钥/私钥上下文。
 * \param [in] mode UH_RSA_XXX，。
 * \param [in] in 待解密的数据。
 * \param [in] in_len 待解密的数据长度。
 * \param [out] out 输出，解密后的数据。
 * \param [in] out_max_len 存放解密数据缓存的最大长度。
 *
 * \return 成功返回UH_OK，失败返回UH_FAIL。
 */
extern uplus_s32 uplus_tool_rsa_decrypt(
	void *ctx, uplus_s32 mode,
	const uplus_u8 *in, uplus_size_t *out_len, uplus_u8 *out, uplus_size_t out_max_len);

/*!
 * \brief 释放RSA公钥上下文。
 *
 * \param [in] ctx RSA公钥/私钥上下文。
 *
 * \return N/A。
 */
extern void uplus_tool_rsa_public_finish(void *ctx);

/*!
 * \brief 生成随机数据。
 *
 * \param [out] output 输出的随机数据。
 * \param [in] output_len 输出缓存的大小。
 *
 * \return N/A。
 */
extern void uplus_tool_random_generate(uplus_u8 *output, uplus_size_t output_len);

/*!
 * \}
 */

/*!
 * \defgroup SYSTEM PAL APIs
 * \details SYSTEM相关PAL接口。
 * \note
 * \{
 */

/*! \def LED_XXX
 *
 * \brief 系统状态指示，可以用于指示灯控制。按比特表示。
 DEV_STATUS 串行设备（与底板通信）状态。 1-通信正常，0-通信错误。
 POWER_SAVE 省电模式。1-省电，0-正常。
 POWER_STATUS 电源指示。1-电源正常，0-电源故障。
 STATUS_MASK 状态掩码，3个比特。
 	STATUS_CONFIG 配置模式。
 	STATUS_NO_WIFI WIFI关闭。
 	STATUS_AP_FAIL 无法连接AP。
 	STATUS_AP_OK AP连接正常。
 	STATUS_SVR_OK 服务器连接正常。
 */
#define LED_DEV_STATUS 0x20 /*base board status*/
#define LED_POWER_SAVE 0x10
#define LED_POWER_STATUS 0x8 /*power status*/

#define LED_STATUS_MASK 0x7
#define LED_STATUS_CONFIG 0x7
#define LED_STATUS_NO_WIFI 0x0
#define LED_STATUS_AP_FAIL 0x4
#define LED_STATUS_AP_OK 0x5
#define LED_STATUS_SVR_OK 0x6

/*!
 * \brief 状态灯控制
 * \param [in] led_status_ind 状态指示，按比特定义不同含义，LED_XXX。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_sys_led_control(uplus_u16 led_status_ind);

/*!
 * \brief 启动看门狗。
 * \details 看门狗用于系统崩溃或者系统不响应时的恢复。
 * \param [in] timeout 看门狗超时时间，单位毫秒。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_sys_watchdog_start(uplus_u32 timeout);

/*!
 * \brief 喂狗。
 * \return N/A。
 */
extern void uplus_sys_watchdog_feed(void);

/*!
 * \brief 打开串行设备（与底板通信）。
 * \details 打开串行设备。串行设备硬件参数根据硬件平台和协议自行确定。串行设备发送缓存至少256字节，接收缓存至少1K字节。串行设备读数据需支持select机制。
 如果设备的读写是同一个描述符，则返回一个描述符；否则，需要返回两个描述符，第一个是用于读，第二个用于写，两个描述符都必须合法有效。
 * \param [in] fd 设备描述符，如果成功返回设备描述符；否则不填写。
 * \param [in] baudrate 波特率。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_sys_serial_open(uplus_s32 *fd,uplus_u32 baudrate);

/*!
 * \brief 关闭串行设备（与底板通信）。
 * \details 关闭串行设备。如果打开串行设备返回了两个描述符，则uplugSDK会主动调用两次本接口以关闭串行设备。
 * \param [in] fd 设备描述符。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_sys_serial_close(uplus_s32 fd);

/*!
 * \brief 从串行设备读取数据（与底板通信）。
 * \details 从串行设备读取数据。如果接收缓存中数据的长度大于len，返回len长度数据；否则返回实际数据长度。
 * \param [in] fd 设备描述符。
 * \param [out] buf 数据缓存。
 * \param [in] len 数据缓存大小（期望读取的数据长度）。
 * \return 成功返回实际读取的数据长度，失败返回-1。
 */
extern uplus_s32 uplus_sys_serial_read(uplus_s32 fd, uplus_u8 *buf, uplus_size_t len);

/*!
 * \brief 向串行设备写入数据（与底板通信）。
 * \param [in] fd 设备描述符。
 * \param [in] buf 数据缓存。
 * \param [in] len 数据缓存大小（期望写入的数据长度）。
 * \return 成功返回实际写入的数据长度，失败返回-1。
 */
extern uplus_s32 uplus_sys_serial_write(uplus_s32 fd, uplus_u8 *buf, uplus_size_t len);

/*!
 * \brief 系统复位。
 * \return N/A。
 */
extern void uplus_sys_system_reset(void);

/*!
 * \brief 获取系统平台SDK版本信息
 * \return 描述系统平台SDK版本信息的字符串。
 */
extern uplus_s8 *uplus_sys_sdk_ver_get(void);

/*!
 * \brief 关闭系统日志及调试输出，用于安全目的。
 * \param [in] on_off 开关，1-开，0-关。
 * \return N/A。
 */
extern void uplus_sys_log_on_off(uplus_u8 on_off);

/*! \def ZONE_0/ZONE_1
 *
 * \brief 互为备份的两个区域，用于配置数据和OTA镜像。
 */
#define ZONE_1 1
#define ZONE_2 2

/*!
 * \brief 读取配置，从非易失性存储器中读取配置到内存中。
 * \param [in] config_zone 配置区域，ZONE_1/ZONE_2。
 * \param [out] conf 内存指针，存放读取的配置数据。
 * \param [in] len 长度，读取的配置数据长度。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_sys_config_read(uplus_u8 config_zone, uplus_u8 *conf, uplus_size_t len);

/*!
 * \brief 写配置，将配置写入非易失性存储器中。
 * \param [in] config_zone 配置区域，ZONE_1/ZONE_2。
 * \param [in] conf 内存指针，存放待写入的配置数据。
 * \param [in] len 长度，待写入的配置数据长度。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_sys_config_write(uplus_u8 config_zone, uplus_u8 *conf, uplus_size_t len);

/*! \def ZONE_BOARD
 *
 * \brief 底板OTA镜像。
 */
#define ZONE_BOARD 3

/*! \def ZONE_RESERVED_MIN/ZONE_RESERVED_MAX
 *
 * \brief 最小/最大保留镜像。
 */
#define ZONE_RESERVED_MIN 4
#define ZONE_RESERVED_MAX 9

/*! \def ZONE_USER
 *
 * \brief 用户自定义镜像。大于等于ZONE_BOARD的镜像区域都是用户自定义区域。小于ZONE_BOARD的是uplugSDK定义区域。
 */
#define ZONE_USER 10

/*! \def ZONE_OTHER
 *
 * \brief 当前运行的镜像区域不是ZONE_1和ZONE_2。
 */
#define ZONE_OTHER 0

/*!
 * \brief 获取当前运行镜像区域。
 * \param [in] image_zone，当前运行的版本镜像区，ZONE_1/ZONE_2/ZONE_OTHER。如果成功，返回当前运行的镜像区域；否则不填写。如果当前运行版本既不是镜像1，也不是镜像2，返回ZONE_OTHER。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_sys_run_image_get(uplus_u8 *image_zone);

/*!
 * \brief 读取镜像数据。
 * \param image_zone，镜像区域，ZONE_XXX。
 * \param [in] offset 偏移量。
 * \param [out] buf 内存指针。
 * \param [in] len 读取的长度。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_sys_image_read(uplus_u8 image_zone, uplus_u32 offset, uplus_u8 *buf, uplus_size_t len);

/*!
 * \brief 写入镜像数据。
 * \param image_zone，镜像区域，ZONE_XXX。
 * \param [in] offset 偏移量。
 * \param [in] buf 待写入数据的内存指针。
 * \param [in] len 写入的长度。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_sys_image_write(uplus_u8 image_zone, uplus_u32 offset, uplus_u8 *buf, uplus_size_t len);

/*!
 * \brief 准备写入镜像数据。
 * \param [in] image_zone，镜像区域，ZONE_XXX。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_sys_image_start(uplus_u8 image_zone);

/*! \def OTA_IMAGE_OK/OTA_IMAGE_FAIL
 *
 * \brief 写入镜像数据的结果。
 */
#define OTA_IMAGE_OK 1
#define OTA_IMAGE_FAIL 0

/*!
 * \brief 完成写入镜像数据。
 * \param [in] image_zone，镜像区域，ZONE_XXX。
 * \param [in] result 写入镜像数据的结果，OTA_IMAGE_OK/OTA_IMAGE_FAIL
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_sys_image_finish(uplus_u8 image_zone, uplus_s32 result);

/*!
 * \brief 镜像版本切换
 * \param [in] image_zone 镜像区域，ZONE_1/ZONE_2。下一次启动的版本区域是image_zone。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_sys_image_switch(uplus_u8 image_zone);

/*!
 * \brief 打印调试信息。
 * \details 调试信息格式化输出。可根据需要，自由输出日志信息到串口，网络等。
 * \param [in] fmt 格式化字符串。
 * \param [in] ... 可变参数。
 * \return 成功返回实际输出信息的长度，失败返回-1。
 */
extern uplus_s32 uplus_sys_debug_printf(const uplus_s8 *fmt, ...);
/*!
 * \brief 系统日志输出。
 * \details 日志信息格式化输出。可根据需要，自由输出日志信息到串口，网络等。
 * \param [in] fmt 格式化字符串。
 * \param [in] ... 可变参数。
 * \return 成功返回实际输出信息的长度，失败返回-1。
 */
extern uplus_s32 uplus_sys_log(const uplus_s8 *fmt, ...);

/*!
 * \brief 系统相关初始化。
 * \details 执行与API接口相关的初始化动作。该接口会在uplugSDK初始化时调用。
 * \return N/A。
 */
extern uplus_s32 uplus_sys_init(void);

/*!
 * \brief 系统周期执行动作。
 * \details 周期是10秒。该接口每个周期被调用。
 * \return N/A。
 */
extern void uplus_sys_period(void);

/*! \def SYSTEM_RESET_TYPE_
 * \brief 系统复位类型。
 * UNDEF 未定义。
 * POWER_ON 上电。
 * WATCHDOG 狗复位。
 * RESET 主动复位。
 */
#define SYSTEM_RESET_TYPE_UNDEF -1
#define SYSTEM_RESET_TYPE_POWER_ON 0
#define SYSTEM_RESET_TYPE_WATCHDOG 1
#define SYSTEM_RESET_TYPE_RESET 2

/*!
 * \brief 获取系统复位类型。
 * \return 系统复位类型，如果不支持，返回SYSTEM_RESET_TYPE_UNDEF。
 */
extern uplus_s32 uplus_sys_reset_type_get(void);

/*!
 * \brief 获取系统保留区域大小。
 * \details 系统保留区域是软件重启不丢失数据的区域。
 * 如果不支持，返回0。
 * 如果支持，必须是256字节的整倍数。
 * param [out] res_area 如果支持直接访问，返回直接访问指针；否则返回NULL。
 * \return 系统保留区域大小。
 */
extern uplus_u16 uplus_sys_res_area_get_size(void **mem);

/*!
 * \brief 获取系统保留区域的数据。
 * \details 如果可以直接访问保留区域，实现空功能即可。
 * \param [in] offset 偏移量。
 * \param [out] buf 内存指针。
 * \param [in] len 读取的长度。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_sys_res_area_read(uplus_u32 offset, uplus_u8 *buf, uplus_size_t len);

/*!
 * \brief 向系统保留区域写入数据。
 * \details 如果可以直接访问保留区域，实现空功能即可。
 * \param [in] offset 偏移量。
 * \param [in] buf 待写入数据的内存指针。
 * \param [in] len 写入的长度。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_sys_res_area_write(uplus_u32 offset, uplus_u8 *buf, uplus_size_t len);

/*!
 * \brief 进入低功耗模式。
 * \details 进入低功耗后该接口不会返回，直到被唤醒。进入低功耗可能会有三种方式，
 * 一是被唤醒后，程序在原来的位置上继续运行。
 * 二是被唤醒后，软件重启。
 * 三是被唤醒后，重新上电。
 * \return N/A。
 */
extern void uplus_sys_low_power_enter(void);

/*!
 * \brief 退出低功耗模式。
 * \return N/A。
 */
extern void uplus_sys_low_power_exit(void);

/*!
 * \brief 打开SOC片选开关，用于外部关闭SOC芯片。
 * \param [in] enable 1-打开，0-关闭。
 * \return N/A。
 */
extern void uplus_sys_chip_enable(uplus_u8 enable);

#define FILE_TYPE_EPP_CONFIG 1

#define FILE_FLAGS_RDONLY 1
#define FILE_FLAGS_WRONLY 2
#define FILE_FLAGS_RDWR 3

/*!
 * \brief 打开文件。
 * \details 不要求系统支持文件系统。
 * \param [out] fd 文件描述符，能唯一标识文件。
 * \param [in] file_type 文件类型，参见FILE_TYPE_XXX。
 * \param [in] file_name 文件名称，通常是字符串形式的TYPEID。
 * \param [in] flags 文件标志，参见FILE_FLAGS_XXX。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_sys_file_open(uplus_s32 *fd, uplus_u8 file_type, const char *file_name, uplus_s32 flags);

/*!
 * \brief 关闭文件。
 * \param [in] fd 文件描述符。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_sys_file_close(uplus_s32 fd);

#define FILE_SEEK_SET 1
#define FILE_SEEK_CUR 2
#define FILE_SEEK_END 3

/*!
 * \brief 文件定位。
 * \param [in] fd 文件描述符。
 * \param [in] offset 相对于whence的偏移量，以字节为单位。
 * \param [in] whence 给定的位置，参见FILE_SEEK_XXX。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_u32 uplus_sys_file_seek(uplus_s32 fd, uplus_s32 offset, uplus_s32 whence);

/*!
 * \brief 读文件。
 * \details 从文件读取数据。如果接收缓存中数据的长度大于len，返回len长度数据；否则返回实际数据长度。
 * \param [in] fd 文件描述符。
 * \param [out] buf 数据缓存。
 * \param [in] len 数据缓存大小（期望读取的数据长度）。
 * \return 成功返回实际读取的数据长度，失败返回-1。
 */
extern uplus_s32 uplus_sys_file_read(uplus_s32 fd, uplus_u8 *buf, uplus_size_t len);

/*!
 * \brief 写文件。
 * \param [in] fd 文件描述符。
 * \param [in] buf 数据缓存。
 * \param [in] len 数据缓存大小（期望写入的数据长度）。
 * \return 成功返回实际写入的数据长度，失败返回-1。
 */
extern uplus_s32 uplus_sys_file_write(uplus_s32 fd, uplus_u8 *buf, uplus_size_t len);

/*!
 * \}
 */

/*!
 * \defgroup NET PAL APIs
 * \details NET相关PAL接口。
 * \note
 * \{
 */
#define IP_PROTO_TCP 6
#define IP_PROTO_UDP 17

struct uplus_in_addr
{
	uplus_u32 s_addr; /*network order*/
};

#define AF_DOMAIN_INET 2
#define AF_DOMAIN_INET6 3

#define SOCK_TYPE_STREAM 1
#define SOCK_TYPE_DGRAM 2

/*! For compatibility with BSD code */
struct uplus_sockaddr_in
{
	uplus_u16 sin_family;
	uplus_u16 sin_port; /*network order*/
	struct uplus_in_addr sin_addr; /*network order*/
	uplus_s8 sin_zero[8];
};

struct uplus_sockaddr
{
	uplus_u16 sa_family;
	uplus_s8 sa_data[14];
};

struct uplus_ipmreq
{
	struct uplus_in_addr imr_multiaddr; /* IP multicast address of group */
	struct uplus_in_addr imr_interface; /* local IP address of interface */
};

/*!
 * \brief Socket库socket create。
 */
extern uplus_s32 uplus_net_socket_create(uplus_s32 domain, uplus_s32 type, uplus_s32 protocol);
/*!
 * \brief
 * \param
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_net_socket_close(uplus_s32 fd);

/*!
 * \brief Socket库bind。
 */
extern uplus_s32 uplus_net_socket_bind(uplus_s32 sockfd, const struct uplus_sockaddr *addr, uplus_socklen_t addrlen);

/*!
 * \brief Socket库listen。
 */
extern uplus_s32 uplus_net_socket_listen(uplus_s32 sockfd, uplus_s32 backlog);

/*!
 * \brief Socket库accept。
 */
extern uplus_s32 uplus_net_socket_accept(uplus_s32 sockfd, struct uplus_sockaddr *addr, uplus_socklen_t *addrlen);

/*!
 * \brief Socket库connect。
 */
extern uplus_s32 uplus_net_socket_c(uplus_s32 sockfd, const struct uplus_sockaddr *addr, uplus_socklen_t addrlen);

/*!
 * \brief Socket库send。
 */
extern uplus_s32 uplus_net_socket_send(uplus_s32 sockfd, const void *buf, uplus_size_t len, uplus_s32 flags);

/*!
 * \brief Socket库sendto。
 */
extern uplus_s32 uplus_net_socket_sendto(uplus_s32 sockfd, const void *buf, uplus_size_t len, uplus_s32 flags, const struct uplus_sockaddr *addr, uplus_socklen_t addrlen);

/*!
 * \brief Socket库recv。
 */
extern uplus_s32 uplus_net_socket_recv(uplus_s32 sockfd, void *buf, uplus_size_t len, uplus_s32 flags);

/*!
 * \brief Socket库recvfrom。
 */
extern uplus_s32 uplus_net_socket_recvfrom(uplus_s32 sockfd, void *buf, int len, int flags, struct uplus_sockaddr *addr, uplus_socklen_t *addrlen);

#define SO_LEVEL_SOCKET 0
#define SO_LEVEL_IP 1
#define SO_LEVEL_TCP 2
#define SO_LEVEL_UDP 3

/*for SO_LEVEL_SOCKET*/
#define SO_OPT_KEEPALIVE 1
#define SO_OPT_BROADCAST 2
#define SO_OPT_REUSEADDR 3
#define SO_OPT_SNDTIMEO 4
#define SO_OPT_RCVTIMEO 5

#define SO_OPT_NONBLOCK 6

/*for SO_LEVEL_IP*/
#define SO_IP_MULTICAST_TTL   1
#define SO_IP_MULTICAST_IF    2
#define SO_IP_MULTICAST_LOOP  3
#define SO_IP_ADD_MEMBERSHIP  4
#define SO_IP_DROP_MEMBERSHIP 5

/*for SO_LEVEL_TCP*/
#define SO_TCP_NODELAY    1
#if 0
#define SO_TCP_KEEPALIVE  2 /*may not be supported in some systems*/
#define SO_TCP_KEEPIDLE   3
#define SO_TCP_KEEPINTVL  4
#define SO_TCP_KEEPCNT    5
#endif

/*!
 * \brief 设置socket选项。
 * \param [in] sockfd socket描述符。
 * \param [in] level SO_LEVEL_XXX。
 * \param [in] optname SO_OPT_XXX。
 * \param [in] optval 指向设置值的指针。
 * \param [in] optlen 数据长度。
 * \return 成功返回0，不支持返回0，其他错误返回-1。
 */
extern uplus_s32 uplus_net_socket_opt_set (uplus_s32 sockfd, uplus_s32 level, uplus_s32 optname, const void *optval, uplus_s32 optlen);
 
/*!
 * \brief 字节序转换htonl
 * \param [in] hostlong 主机序4字节整数。
 * \return 网络序4字节整数。
 */
extern uplus_u32 uplus_net_htonl(uplus_u32 hostlong);

/*!
 * \brief 字节序转换ntohl
 * \param [in] netlong 网络序4字节整数。
 * \return 主机序4字节整数。
 */
extern uplus_u32 uplus_net_ntohl(uplus_u32 netlong);

/*!
 * \brief 字节序转换htons
 * \param [in] hostshort 主机序2字节整数。
 * \return 网络序2字节整数。
 */
extern uplus_u16 uplus_net_htons(uplus_u16 hostshort);

/*!
 * \brief 字节序转换ntohs
 * \param [in] netshort 网络序2字节整数。
 * \return 主机序2字节整数。
 */
extern uplus_u16 uplus_net_ntohs(uplus_u16 netshort);

/*!
 * \brief IP地址转换为字符串。
 * \param [in] in 网络字节序的IP地址。
 * \return 点分十进制表示的IP地址字符串。
 */
extern uplus_s8 *uplus_net_inet_ntoa(struct uplus_in_addr in);

/*!
 * \brief 字符串转换为IP地址。
 * \param [in] cp 点分十进制表示的IP地址字符串。
 * \return 网络序4字节整数表示的IP地址。
 */
extern uplus_u32 uplus_net_inet_addr(const uplus_s8 *cp);

/*!
 * \struct uplus_timeval
 * \brief Structure of timeval.
 */
struct uplus_timeval
{
	uplus_u32 tv_sec; /*!<seconds.*/
	uplus_u32 tv_usec; /*!<microseconds*/
};

/*!
 * \brief IO复用。
 * \param [in] nfds 描述集合中所有文件描述符，最大值加1。
 * \param [inout] readfds 指向fd set的指针。监视文件描述是否可读。NULL表示不关心。
 * \param [inout] writefds 指向fd set的指针。监视文件描述是否可写。NULL表示不关心。
 * \param [inout] exceptfds 指向fd set的指针。监视文件描述是否异常。NULL表示不关心。
 * \param [in] timeout select超时时间。NULL表示select处于阻塞状态。
 * \return >0，一些文件描述符可读、可写或者异常。<0，错误。=0，超时。
 */
extern uplus_s32 uplus_net_select(uplus_s32 nfds, void *readfds, void *writefds, void *exceptfds, struct uplus_timeval *timeout);

/*!
 * \brief 清空FD集合。
 * \param [in] set 指向fd set的指针。
 * \return N/A。
 */
extern void uplus_net_fd_zero(void *set);

/*!
 * \brief 清除FD集合中指定的文件描述符。
 * \param [in] fd 文件描述符。
 * \param [in] set 指向fd set的指针。
 * \return N/A。
 */
extern void uplus_net_fd_clr(uplus_s32 fd, void *set);

/*!
 * \brief 设置FD集合中指定的文件描述符。
 * \param [in] fd 文件描述符。
 * \param [in] set 指向fd set的指针。
 * \return N/A。
 */
extern void uplus_net_fd_set(uplus_s32 fd, void *set);

/*!
 * \brief 判断指定的文件描述符是否就绪。
 * \param [in] fd 文件描述符。
 * \param [in] set 指向fd set的指针。
 * \return 1表示fd已就绪，0表示fd未就绪。
 */
extern uplus_s32 uplus_net_fd_isset(uplus_s32 fd, void *set);

/*!
 * \brief 获取fd set的大小。
 * \return Fd set的大小。
 */
extern uplus_u32 uplus_net_fd_size(void);

/*!
 * \brief 申请可用于select的文件描述符。
 * \return 成功返回FD，失败返回-1。
 */
extern uplus_s32 uplus_net_alloc_fd(void);

/*!
 * \brief 释放文件描述符。
 * \param [in] fd 文件描述符。
 * \return N/A。
 */
extern void uplus_net_free_fd(uplus_s32 fd);

/*!
 * \brief 通知select有数据可读。
 * \param [in] fd 文件描述符。
 * \return N/A。
 */
extern void uplus_net_fd_rcv_plus(uplus_s32 fd);

/*!
 * \brief 数据读取后清除可读标记。
 * \param [in] fd 文件描述符。
 * \return N/A。
 */
extern void uplus_net_fd_rcv_minus(uplus_s32 fd);

/*! \def NETIF_TYPE_STA/NETIF_TYPE_AP
 *
 * \brief 网络接口类型，在不同的系统上，WIFI STA和WIFI AP可能是同一个网络接口，也有可能是不同的网络接口。
 * NETIF_TYPE_STA WIFI STA对应的网络接口。
 * NETIF_TYPE_AP WIFI AP对应的网络接口。
 *
 */
#define NETIF_TYPE_STA 0
#define NETIF_TYPE_AP 1

/*!
 * \brief IPv4地址设置与查询
 * \param [in] netif_type 网络接口类型，NETIF_TYPE_XXX。
 * \param [in] op 操作方式，OP_SET/OP_GET。
 * \param [inout] local_ip_addr 点分十进制表示的IP地址。
 * \param [inout] net_mask 点分十进制表示的IP掩码。
 * \param [inout] gateway_ip_addr 点分十进制表示的IP网关。
 * \return 成功返回0，未获取到IP地址也返回0，其他错误返回-1。
 */
extern uplus_s32 uplus_net_ip_config(uplus_u8 netif_type, uplus_u8 op, uplus_s8 *local_ip_addr, uplus_s8 *net_mask, uplus_s8 *gateway_ip_addr);

/*!
 * \brief DNS服务器设置与查询。
 * \param [in] op 操作方式，OP_SET/OP_GET。
 * \param [in] dns_server，域名服务器，点分十进制的IP地址。当操作方式是查询时，如果成功填写域名服务器；否则不填写。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_net_dns_config(uplus_u8 op, uplus_s8 *dns_server);

/*!
 * \brief DNS域名解析
 * \note 阻塞方式，超时时间不超过5秒。
 * \param [in] hostname 待解析的域名，以’\0’结尾的字符串。
 * \param [in] ip_addr 域名的IP地址。成功放回解析域名获得的IP地址；失败不填写。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_net_dns_request(const uplus_s8 *hostname, uplus_s8 *ip_addr);

/*!
 * \brief 提供给DHCP的IP地址。
 * \param [in] 在通常情况下，同一设备会连接相同的路由器，通过DHCP获取到的IP地址也是相同的。提供上次获得的IP地址，可以加快DHCP的通信流程。
 * \param [in] offered_ip 上次获得的IP地址，点分十进制表示。
 * \return N/A。
 */
//extern void uplus_net_dhcp_offer_ip(uplus_s8 * offered_ip);

/*! \def DHCP_MODE_XXX
 *
 * \brief DHCP模式。
 CLIENT DHCP客户端
 SERVER DHCP服务端
 */
#define DHCP_MODE_CLIENT 1
#define DHCP_MODE_SERVER 2

/*!
 * \brief DHCP模式配置。
 * \param [in] op 操作方式，OP_START/OP_STOP。
 * \param [in] mode DHCP模式，DHCP_MODE_XXX。
 * \return 成功返回0，失败返回-1。
 */
//extern uplus_s32 uplus_net_dhcp_config(uplus_u8 op, uplus_u8 mode);

/*!
 * \brief DHCP地址池设置。
 * \param [in] op 操作方式，OP_ADD/OP_DELETE。
 * \param [in] address_pool_start DHCP地址池起始地址，点分十进制表示的IP地址。
 * \param [in] address_pool_end DHCP地址池结束地址，点分十进制表示的IP地址。
 * \return 成功返回0，失败返回-1。
 */
//extern uplus_s32 uplus_net_dhcp_pool_set(uplus_u8 op, uplus_s8 *address_pool_start, uplus_s8 *address_pool_end);

struct uplus_ca_chain
{
	const uplus_u8 * ca;
	uplus_u32 ca_len;
};

/*!
 * \brief 打开SSL客户端。
 * \param fd 已连接的socket描述符。
 * \param [in] root_ca 根证书，如果为空，表示不验证服务器证书。PEM证书格式。
 * \param [in] root_ca_num 根证书的数量。
 * \return 成功返回SSL会话标识，失败返回NULL。
 */
extern uplus_ctx_id uplus_net_ssl_client_create(uplus_s32 fd, struct uplus_ca_chain *root_ca, uplus_u8 root_ca_num);

/*!
 * \brief SSL握手。
 * \param [in] id SSL会话标识。
 * \return 成功返回0，失败返回-1，继续握手返回1。
 */
extern uplus_s32 uplus_net_ssl_client_handshake(uplus_ctx_id id);

/*!
 * \brief 关闭SSL客户端。
 * \note 此接口不能关闭已绑定的socket fd。
 * \param [in] id SSL会话标识。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_net_ssl_client_close(uplus_ctx_id id);

/*!
 * \brief 获取SSL可读数据的长度。
 * \param [in] id SSL会话标识。
 * \return 返回SSL会话可读数据的长度，如果无可读数据，返回0。
 */
extern uplus_s32 uplus_net_ssl_pending(uplus_ctx_id id);

/*!
 * \brief 读取SSL数据
 * \param [in] id SSL会话标识。
 * \param [out] buf 数据缓存指针。
 * \param [in] len 数据长度。
 * \return 成功返回实际写入的数据长度，失败返回-1。
 */
extern uplus_s32 uplus_net_ssl_read(uplus_ctx_id id, uplus_u8 *buf, uplus_size_t len);

/*!
 * \brief 写入SSL数据。
 * \param [in] id SSL会话标识。
 * \param [in] buf 数据缓存指针。
 * \param [in] len 数据长度。
 * \return 成功返回实际读取的数据长度，失败返回-1。
 */
extern uplus_s32 uplus_net_ssl_write(uplus_ctx_id id, uplus_u8 *buf, uplus_size_t len);

/*error*/
#define ERR_OTHERS 1
#define ERR_EAGAIN 2
/*!
 * \brief 获取socket接口或者ssl接口调用返回的错误码。
 * \param [in] fd socket描述符，SSL会话标识为NULL时有效。
 * \param [in] ssl_id SSL会话标识。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_net_last_error_get(uplus_s32 fd, uplus_ctx_id ssl_id);

#define MAX_ERRNO_NUM 139
/*!
 * \brief 获取最近设置的ERRNO。
 * \note 如果不支持，返回0。
 * \return errno，如果大于0，表示有错误；如果等于0，表示未检测到连接错误；如果小于0，表示链路层错误。
 */
extern uplus_s32 uplus_net_errno_get(void);

/*!
 * \brief 设置主机名称。
 * \param [in] hostname 以’\0’结尾的字符串。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_net_hostname_set(const uplus_s8 *hostname);

/*!
 * \typedef uplus_ping_stat_t
 * \brief ping结果。
 * sent_num 发送包数。
 * rcvd_num 接收包数。
 * max_time 最大时间，单位毫秒。
 * min_time 最小时间，单位毫秒。
 * total_time 总时间，单位毫秒。
 * time_spent ping操作花费的时间，单位毫秒。
 * ping_index ping索引。
 * \return N/A。
 */
typedef struct
{
	uplus_u32 sent_num;
	uplus_u32 rcvd_num;
	uplus_u16 max_time;
	uplus_u16 min_time;
	uplus_u32 total_time;
	uplus_u32 time_spent;

	int ping_index;
} uplus_ping_stat_t;

/*!
 * \typedef uplus_ping_stat_cb_func
 * \brief ping结果回调接口。
 * \param [in] cb_para 参数。
 * \param [in] stat ping结果。
 * \return N/A。
 */
typedef void (* uplus_ping_stat_cb_func)(void * cb_para, uplus_ping_stat_t *stat);

/*!
 * \brief 执行ping操作。
 * \param [in] target 域名或者IP地址。
 * \param [in] data_size 净荷数据长度。
 * \param [in] times 次数，0表示持续。
 * \param [in] timeout 超时时间，0表示使用系统默认超时时间。
 * \param [in] cb 结果统计的回调函数。
 * \param [in] cb_para 回调函数的参数。
 * \return 成功返回索引，用于主动停止；失败返回-1。
 */
extern uplus_s32 uplus_net_ping(const uplus_s8 * target, uplus_u16 data_size, uplus_u32 times,
	uplus_u32 timeout, uplus_ping_stat_cb_func cb, void *cb_para);
typedef struct
{
	uplus_u32 ping_max_sent_times; /*0: not care*/
	uplus_u32 ping_max_rcvd_times; /*0: not care*/
	uplus_u32 ping_max_timeout_times; /*0: not care*/
	uplus_u32 ping_max_time; /*0: not care*/ /*ms*/

	uplus_u16 ping_data_size;
	uplus_u16 ping_timeout; /*ms*/

	uplus_u8 force_ping;
} uplus_ping_para_t;

extern uplus_s32 uplus_net_ping_para(const uplus_s8 * target, uplus_ping_para_t *para, uplus_ping_stat_cb_func cb, void *cb_para);

/*!
 * \brief 停止ping操作。
 * \param [in] index 开始ping返回的索引值。
 * \return N/A。
 */
extern void uplus_net_ping_stop(uplus_s32 index);

/*!
 * \brief 在指定接口上发送免费ARP。
 *
 * \param [in] netif_type 网络接口类型，NETIF_TYPE_XXX。
 *
 * \return N/A。
 */
extern void uplus_net_tx_gratuitous_arp(uplus_u8 netif_type);

/*!
 * \brief 设置IP冲突检测回调。
 *
 * \param [in] cb 回调接口。
 *
 * \return N/A。
 */
extern void uplus_net_set_ip_conflict_callback(void (*cb)(void));

/*!
 * \brief 使能/关闭发送ICMP不可达功能。
 *
 * \param [in] enable 0-关闭，1-使能。
 *
 * \return N/A。
 */
extern void uplus_net_enable_icmp_unreach(int enable);

/*!
 * \brief 查找系统ARP表项。
 * \param [in] netif_type 网络接口类型，NETIF_TYPE_XXX。
 * \param [in] ip_addr 待查找的IPv4地址，网络字节序。
 * \param [out] eth_addr ARP地址。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_net_find_arp(uplus_u8 netif_type, uplus_u32 ip_addr, uplus_u8* eth_addr);

/*!
 * \}
 */


typedef struct
{
	uplus_u8 frame_is_ok;
	uplus_u8 wifi_strength;
	uplus_u16 rx_channel;
} promisc_frame_info_t;

typedef struct
{
	uplus_s8 ssid[32];
	uplus_u8 ap_mac[6];
	uplus_u8 ap_power;
	uplus_u32 encryption;
	uplus_u16 channel;
} ap_list_api_t;

typedef void (* wifi_scan_cb_func)(void * param);

/*!
 * \brief 获取WIFI MAC。
 * \param [out] mac 6字节MAC地址。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_wifi_mac_get(uplus_u8 *mac);
/*!
 * \brief 发送802.11帧。
 * \details 802.11帧主要包括probe request帧。
 * \param [in] frame 802.11帧。
 * \param [in] length 802.11帧长度。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_wifi_tx_frame(uplus_u8 * frame, uplus_s32 length);
/*!
 * \brief 获取BSSID。
 * \param [out] bssid 如果已经连接到AP，返回AP的BSSID；否则返回全0。
 * \param [out] bssid_len BSSID长度，当前固定返回6。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_wifi_get_bssid(uplus_u8 *bssid, uplus_u16 *bssid_len);

/*!
 * \brief 查询WIFI状态。
 * \param [out] status WIFI状态WIFI_DOWN/WIFI_UP。
 * \param [out] wifi_strength，当前所连接AP的信号强度，0-100。如果status是WIFI_DOWN，则wifi_strength等于0。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_wifi_status_get(uplus_u8 *status, uplus_u8 *wifi_strength);

/*!
 * \typedef wifi_mgmt_cb_func
 * \brief 收到802.11帧的回调通知。
 * \param [in] frame 802.11帧，包含帧头和帧体。
 * \param [in] length 802.11帧长度。
 * \param [in] param 用户参数。
 * \return N/A。
 */
typedef void (* wifi_frame_cb_func)(uplus_u8 * frame, uplus_s32 length, void * param);

/*!
 * \brief 设置802.11帧接收回调。
 * \details 802.11帧主要包括beacon帧和probe response帧。目前仅支持在STA模式下，接收所连接AP发出的帧。
 * \param [in] enable 使能或者禁止接收802.11帧回调。
 * \param [in] cb 收到帧的回调通知。设置为NULL表示不需要管理帧回调。
 * \param [in] param 回调接口的用户参数。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_wifi_set_rx_frame(uplus_s32 enable, wifi_frame_cb_func cb, void * param);

/*!
 * \brief WIFI功能关闭。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_wifi_power_off(void);

/*!
 * \brief 查询WIFI Station模式下信噪比。
 * \param [in] dev 指向设备实例的指针。
 * \param [out] snr 信噪比。
 * \return 成功返回UH_OK，失败返回UH_FAIL。
 */
extern uplus_s32 uplus_wifi_snr_get(uplus_u8 *snr);

#define WIFI_DOWN 0
#define WIFI_UP 1

#define WIFI_DOWN_UNDEF 0
#define WIFI_DOWN_TIMEOUT -1
#define WIFI_DOWN_CONNECTIVITY_FAIL -2
#define WIFI_DOWN_AP_PAPA_CHANGE -3
#define WIFI_DOWN_AP_RESTART -4
#define WIFI_DOWN_AP_DISCONNECT -10
#define WIFI_DOWN_AP_DISCONNECT_MAX -(10 + 66) /*def in 802.11, total error code 66*/
/*!
 * \brief WIFI状态变化通知。
 * \note 当WIFI状态发生变化（UP变化为DOWN或者DOWN变化为UP）时，通过此回调接口通知uplugSDK。
 * \param [in] status WIFI状态WIFI_DOWN/WIFI_UP。
 * \param [in] extra_error  WIFI_FOWN时的附加错误码 WIFI_DOWN_XXX
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_wifi_status_callback(uplus_u8 status, uplus_s32 extra_error);


#if 0
/*!
 * \defgroup WIFI PAL APIs
 * \details WIFI相关PAL接口。
 * \note
 * \{
 */

/*! \def WIFI_MODE_XXX
 *
 * \brief WIFI模式。
 * AP AP模式。
 * STA Station模式。
 * AP_STA AP+Station模式。
 *
 */
#define WIFI_MODE_AP 1
#define WIFI_MODE_STA 2
#define WIFI_MODE_AP_STA 3

/*! \def WIFI_ENCRYPT_AUTH_XXX
 *
 * \brief WIFI鉴权模式。
 * NONE 开放。
 * WPA_PSK WPA PSK。
 * WPA2_PSK WPA2 PSK。
 * MASK 鉴权模式掩码。
 *
 */
#define WIFI_ENCRYPT_AUTH_NONE 0x0
#define WIFI_ENCRYPT_AUTH_WPA_PSK 0x1
#define WIFI_ENCRYPT_AUTH_WPA2_PSK 0x2
#define WIFI_ENCRYPT_AUTH_MASK 0xFFFF

/*! \def WIFI_ENCRYPT_CIPHER_XXX
 *
 * \brief WIFI加密方式。
 * NONE 不加密。
 * TKIP TKIP。
 * CCMP CCMP。
 * WEP WEP
 * MASK 加密方式掩码。
 *
 */
#define WIFI_ENCRYPT_CIPHER_NONE 0x00000
#define WIFI_ENCRYPT_CIPHER_TKIP 0x10000
#define WIFI_ENCRYPT_CIPHER_CCMP 0x20000
#define WIFI_ENCRYPT_CIPHER_WEP 0x40000
#define WIFI_ENCRYPT_CIPHER_MASK 0xFFFF0000

/*! \def WIFI_DOWN/WIFI_UP
 *
 * \brief WIFI状态。
 * DOWN WIFI不可用。
 * UP WIFI可用。
 *
 */
#define WIFI_DOWN 0
#define WIFI_UP 1

/*!
 * \brief WIFI功能打开。WIFI必须支持AP模式和STA模式。
 * \param [in] wifi_mode WIFI工作模式，WIFI_MODE_XXX。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_wifi_power_on(uplus_u8 wifi_mode);

/*!
 * \brief WIFI功能关闭。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_wifi_power_off(void);

/*!
 * \brief 断开WIFI连接。
 * \details 如果是STA模式，取消当前正在进行的连接或者断开已经存在的连接；如果是AP模式，断开当前所有连接。断开WIFI连接后，不能自动再进行新的连接。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_wifi_disconnect(void);

/*!
 * \brief 设置WIFI区域。不同区域支持的信道不同，需要根据区域设置所支持的信道。
 * \param [in] country_code 区域码。可以设置的区域码包括：中国“CN”、北美“US”、欧洲“EU”和日本“JP”。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_wifi_country_set(uplus_s8 *country_code);

/*!
 * \typedef promisc_frame_info_t
 * \brief 混杂模式下收到的报文附加信息。
 * frame_is_ok 是否有完整信息的报文，1表示正常的报文，0表示报文不可用，仅长度可用。
 * wifi_strength 接收信号强度，0-100，0xFF表示此参数无效。
 * rx_channel 接收的信道，0表示信道不可用。
 *
 */
typedef struct
{
	uplus_u8 frame_is_ok;
	uplus_u8 wifi_strength;
	uplus_u16 rx_channel;
} promisc_frame_info_t;

#pragma pack(1)
struct ieee80211_frame
{
	uplus_u8 i_fc[2];
	uplus_u8 i_dur[2];
	uplus_u8 i_addr1[6];
	uplus_u8 i_addr2[6];
	uplus_u8 i_addr3[6];
	uplus_u8 i_seq[2];
};
#pragma pack()

/*!
 * \typedef promisc_enhance_info_t
 * \brief 锁定信道后，增加混杂模式下该目标AP收取转发报文能力。
 * ap_mac 目标AP。
 *
 */
typedef struct
{
	uplus_u8 ap_mac[6];
} promisc_enhance_info_t;

/*!
 * \brief 锁定信道后，增加混杂模式下该目标AP收取转发报文能力。
 * \details WIFI需支持Promiscuous模式（在所设定的监听信道内）。在Promiscuous模式下，会根据目标AP信息，通过RSSI增加调节底层收包增益以提高收包效率。
 * \param [in] enable 使能参数；开启设定为1；关闭设定为0。
 * \return 成功返回0，失败返回-1。
 */
uplus_s32 uplus_wifi_sniffer_enhance(uplus_s32 enable, promisc_enhance_info_t *info);

/*!
 * \typedef promisc_cb_func
 * \brief 混杂模式下报文接收接口。
 * \param [in] frame 802.11帧，包含帧头和帧体。
 * \param [in] length 802.11帧长度。
 * \param [in] info 报文附加信息。
 * \return N/A。
 */
typedef void (* promisc_cb_func)(uplus_u8 * frame, uplus_s32 length, promisc_frame_info_t * info);

/*!
 * \brief 进入WIFI Promiscuous模式。
 * \details WIFI需支持Promiscuous模式（在所设定的监听信道内）。在Promiscuous模式下，需要将所有嗅探到的正确的WIFI数据包进行回调处理。
 * \param [in] cb 混杂模式下报文接收接口。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_wifi_sniffer_start(promisc_cb_func cb);

/*!
 * \brief 退出WIFI promiscuous模式。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_wifi_sniffer_stop(void);

/*!
 * \brief 设置WIFI信道。
 * \details 当WIFI工作在AP模式时，设置AP的工作信道。当WIFI工作在Promiscuous模式时，设置sniffer的监听信道。其他模式，此设置不生效。
 * \param [in] channel WIFI工作信道（2.4GHZ），1-14，最大信道与WIFI区域相关，可能是11、13或者14。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_wifi_channel_set(uplus_u16 channel);

/*! \def WIFI_EXT_CHANNEL_XXX
 *
 * \brief 扩展信道。
 * NONE 不使用/不支持扩展信道。
 * ABOVE 当前信道与后一个信道合并成40M信道。
 * BELOW 当前信道与前一个信道合并成40M信道。
 * KEEP 保持不变。
 *
 */
#define WIFI_EXT_CHANNEL_NONE 0
#define WIFI_EXT_CHANNEL_ABOVE 1
#define WIFI_EXT_CHANNEL_BELOW 2
#define WIFI_EXT_CHANNEL_KEEP 3

/*!
 * \brief 设置WIFI扩展信道。
 * \details 用于40MHz信道的设置。当WIFI工作在Promiscuous模式时，设置sniffer的监听信道。其他模式，此设置不生效。
 * \param [in] channel WIFI工作信道（2.4GHZ），1-14，最大信道与WIFI区域相关，可能是11、13或者14。
 * \param [in] ext_channel WIFI_EXT_CHANNEL_XXX。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_wifi_sniffer_channel_set(uplus_u16 channel, uplus_s32 ext_channel);

/*!
 * \typedef wifi_scan_cb_func
 * \brief 扫描完成后的回调通知。
 * \param param 用户参数。
 * \return N/A。
 */
typedef void (* wifi_scan_cb_func)(void * param);

/*!
 * \brief 开始扫描WIFI AP。
 * \param [in] cb 扫描完成后的回调通知。可以设置为NULL。
 * \param [in] param 回调接口的用户参数。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_wifi_scan_start(wifi_scan_cb_func cb, void * param);

/*!
 * \brief 开始扫描指定SSID的WIFI AP。
 * \param [in] ssid 待扫描的SSID。
 * \param [in] ssid_len 待扫描的SSID长度。
 * \param [in] cb 扫描完成后的回调通知。可以设置为NULL。
 * \param [in] param 回调接口的用户参数。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_wifi_scan_with_ssid_start(const uplus_u8 *ssid, uplus_u16 ssid_len, wifi_scan_cb_func cb, void * param);

/*!
 * \brief 停止扫描WIFI AP。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_wifi_scan_stop(void);

typedef struct
{
	uplus_s8 ssid[32];
	uplus_u8 ap_mac[6];
	uplus_u8 ap_power;
	uplus_u32 encryption;
	uplus_u16 channel;
} ap_list_api_t;

/*!
 * \brief 查询WIFI AP列表。
 * \note 此接口是同步接口，自扫描AP开始需在10秒以内获得。扫描的AP列表需按照信号强度由高到低排序。
 * \param [inout] ap_number 作为输入，表示list能存储的最大AP数量；作为输出，表示实际扫描到的AP数量。
 * \param [out] list，扫描到的AP列表。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_wifi_ap_list_get(uplus_u8 *ap_number, ap_list_api_t *list);

/*!
 * \brief 使能WIFI省电模式。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_wifi_ps_enable(void);

/*!
 * \brief 关闭WIFI省电模式。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_wifi_ps_disable(void);

/*!
 * \brief 查询WIFI状态。
 * \param [out] status WIFI状态WIFI_DOWN/WIFI_UP。
 * \param [out] wifi_strength，当前所连接AP的信号强度，0-100。如果status是WIFI_DOWN，则wifi_strength等于0。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_wifi_status_get(uplus_u8 *status, uplus_u8 *wifi_strength);

#define WIFI_DOWN_UNDEF 0
#define WIFI_DOWN_TIMEOUT -1
#define WIFI_DOWN_CONNECTIVITY_FAIL -2
#define WIFI_DOWN_AP_PAPA_CHANGE -3
#define WIFI_DOWN_AP_RESTART -4
#define WIFI_DOWN_AP_DISCONNECT -10
#define WIFI_DOWN_AP_DISCONNECT_MAX -(10 + 66) /*def in 802.11, total error code 66*/
/*!
 * \brief WIFI状态变化通知。
 * \note 当WIFI状态发生变化（UP变化为DOWN或者DOWN变化为UP）时，通过此回调接口通知uplugSDK。
 * \param [in] status WIFI状态WIFI_DOWN/WIFI_UP。
 * \param [in] extra_error  WIFI_FOWN时的附加错误码 WIFI_DOWN_XXX
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_wifi_status_callback(uplus_u8 status, uplus_s32 extra_error);

/*!
 * \brief 获取WIFI MAC。
 * \param [out] mac 6字节MAC地址。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_wifi_mac_get(uplus_u8 *mac);

/*! \def WIFI_NET_CFG_XXX
 *
 * \brief WIFI连接错误码。
 * OK 正确。
 * ERR_GENERAL 其他错误。
 * ERR_NONE_NETWORK 找不到网络（SSID）。
 * ERR_CONNECT_FAIL 连接失败。
 * AUTH_FAIL 密码错。
 * MAYBE_AUTH_FAIL 疑似密码错。
 *
 */
#define WIFI_NET_CFG_OK 0
#define WIFI_NET_CFG_ERR_GENERAL (-1)
#define WIFI_NET_CFG_ERR_NONE_NETWORK (-2)
#define WIFI_NET_CFG_ERR_CONNECT_FAIL (-3)
#define WIFI_NET_CFG_ERR_AUTH_FAIL (-4)
#define WIFI_NET_CFG_ERR_MAYBE_AUTH_FAIL (-5)

/*!
 * \brief 启动WIFI网络连接。
 * \note 此接口是同步接口，超时时间为15秒。
 * \param [in] wifi_mode WIFI工作模式，WIFI_MODE_AP/WIFI_MODE_STA。
 * \param [in] wifi_ssid SSID，最长32字节。
 * \param [in] wifi_key passphrase，最长64字节。
 * \param [in] channel AP信道。WIFI_MODE_STA模式可以为0，表示未指定信道。
 * \param [in] encryption，加密方式。WIFI_ENCRYPT_AUTH_XXX和WIFI_ENCRYPT_CIPHER_XXX。
 * \return 成功返回0，失败返回错误码。
 */
extern uplus_s32 uplus_wifi_network_set(uplus_u8 wifi_mode, uplus_s8 *wifi_ssid, uplus_s8 *wifi_key, uplus_u16 channel, uplus_u32 c);

/*!
 * \brief 查询WIFI网络信息。
 * \param [in] wifi_mode WIFI工作模式，WIFI_MODE_STA。
 * \param [in] wifi_ssid SSID，最长32字节。
 * \param [out] wifi_key passphrase，最长64字节。
 * \param [out] channel AP信道。WIFI_MODE_STA模式可以为0，表示未指定信道。
 * \param [out] encryption，加密方式。WIFI_ENCRYPT_AUTH_XXX和WIFI_ENCRYPT_CIPHER_XXX。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_wifi_network_get(uplus_u8 wifi_mode, uplus_s8 *wifi_ssid, uplus_u16 *channel, uplus_u32 *encryption, uplus_u8 *bssid);

/*! \def WIFI_CUSTOM_IE_TRANS_XXX
 *
 * \brief WIFI管理帧类型，按比特表示。
 * BEACON Beacon帧。
 * PROBE_REQ Probe Request帧。
 * PROBE_RSP Probe Response帧。
 *
 */
#define WIFI_CUSTOM_IE_TRANS_BEACON 1
#define WIFI_CUSTOM_IE_TRANS_PROBE_REQ 2
#define WIFI_CUSTOM_IE_TRANS_PROBE_RSP 4

#define WIFI_CUSTOM_IE_INFO_MAX_LEN (255)
typedef struct wifi_custom_ie
{
	uplus_u8 id;
	uplus_u8 len;
	uplus_u8 info[WIFI_CUSTOM_IE_INFO_MAX_LEN];
} wifi_custom_ie_t;

/*!
 * \brief WIFI管理帧中增加自定义IE。
 * \param [in] ie_info IE信息，802.11定义的IE TLV格式 wifi_custom_ie_t
 * \param [in] trans_type 管理帧类型，按bit表示，WIFI_CUSTOM_IE_TRANS_XXX。
 * \return 成功返回索引，用于删除；失败返回-1。
 */
extern uplus_s32 uplus_wifi_custom_ie_add(uplus_u8 * ie_info, uplus_u16 trans_type);

/*!
 * \brief WIFI管理帧中删除自定义IE。
 * \param [in] index 增加自定义IE返回的索引。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_wifi_custom_ie_del(uplus_s32 index);

/*!
 * \brief 获取BSSID。
 * \param [out] bssid 如果已经连接到AP，返回AP的BSSID；否则返回全0。
 * \param [out] bssid_len BSSID长度，当前固定返回6。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_wifi_get_bssid(uplus_u8 *bssid, uplus_u16 *bssid_len);

/*!
 * \typedef wifi_mgmt_cb_func
 * \brief 收到802.11帧的回调通知。
 * \param [in] frame 802.11帧，包含帧头和帧体。
 * \param [in] length 802.11帧长度。
 * \param [in] param 用户参数。
 * \return N/A。
 */
typedef void (* wifi_frame_cb_func)(uplus_u8 * frame, uplus_s32 length, void * param);

/*!
 * \brief 设置802.11帧接收回调。
 * \details 802.11帧主要包括beacon帧和probe response帧。目前仅支持在STA模式下，接收所连接AP发出的帧。
 * \param [in] enable 使能或者禁止接收802.11帧回调。
 * \param [in] cb 收到帧的回调通知。设置为NULL表示不需要管理帧回调。
 * \param [in] param 回调接口的用户参数。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_wifi_set_rx_frame(uplus_s32 enable, wifi_frame_cb_func cb, void * param);

/*!
 * \brief 发送802.11帧。
 * \details 802.11帧主要包括probe request帧。
 * \param [in] frame 802.11帧。
 * \param [in] length 802.11帧长度。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_wifi_tx_frame(uplus_u8 * frame, uplus_s32 length);

#ifdef NEED_TO_OPTIMALIZE
/*
	deal ssid
*/
extern uplus_s32 uplus_wifi_get_network_and_bssid_check(uplus_s8 *wifi_ssid, uplus_u16 *channel, uplus_u32 *encryption, uplus_u8 *bssid);

/*
	double_freq for 5g
*/
extern uplus_s32 uplus_wifi_get_network_for_dual_freq(uplus_s8 *wifi_ssid, uplus_u16 *channel, uplus_u32 *encryption, uplus_u8 *bssid, uplus_u8 enable_fuzzy_matching);
#endif

/*!
 * \brief 查询WIFI Station模式下信噪比。
 * \param [in] dev 指向设备实例的指针。
 * \param [out] snr 信噪比。
 * \return 成功返回UH_OK，失败返回UH_FAIL。
 */
extern uplus_s32 uplus_wifi_snr_get(uplus_u8 *snr);
#endif


#ifdef __cplusplus
}
#endif

#endif /*__UPLUS_PAL_DEF_H__*/

