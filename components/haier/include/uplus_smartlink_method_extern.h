
/**
 * @file uplus_smartlink_method_extern.h
 * @brief Smartlink配置方法接口。
 *
 * @date 2017-01-11
 * @author fanming
 *
 */

#ifndef __UPLUS_SMARTLINK_METHOD_EXTERN_H__
#define __UPLUS_SMARTLINK_METHOD_EXTERN_H__

#ifdef __cplusplus
extern "C" {
#endif

/*U+ smartlink config API*/
/**
 * @brief Smartlink信道切换处理接口。
 * @return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_smartlink_ch_sw(void);

/**
 * @brief Smartlink报文接收接口。
 * @param frame 802.11帧，包含帧头和帧体。
 * @param length 802.11帧长度。
 * @param info 报文附加信息。
 * @return SNIFFER_RCV_XXX。
 */
extern uplus_s32 uplus_smartlink_rcv(uplus_u8 * frame, uplus_u32 length, promisc_frame_info_t * info);

/**
 * @brief Smartlink开始接口。
 * @return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_smartlink_start(void);

/**
 * @brief Smartlink停止接口。
 * @return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_smartlink_stop(void);

/**
 * @brief 获取Smartlink配置结果接口。
 * @param ssid SSID，最长32字节。
 * @param passwd passphrase，最长64字节。
 * @param ext_data 输出，额外数据，动态申请，可以为NULL。
 * @return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_smartlink_result_get(uplus_u8 *ssid, uplus_u8 *passwd, uplus_u8 ** ext_data);

/**
 * @brief 获取Smartlink锁定的信道。
 * @param cur_channel 锁定的信道。
 * @param ext_channel
 * @return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_smartlink_locked_ch_get(uplus_u16 *cur_channel, uplus_u8 *ext_channel);

/**
 * @brief 构造Smartlink配置完成后的ACK报文接口。
 * @param info 可以提供的信息。
 * @param ack 构造出的报文。接口内使用uplus_tool_malloc申请的动态内存。
 * @param ack_len 构造出的报文长度。
 * @param addr ack报文的UDP目的地址。
 * @param addrlen ack报文的目的地址长度。
 * @return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_smartlink_ack_build(wifi_config_ack_t *info, uplus_u8 **ack, uplus_u16 *ack_len, struct uplus_sockaddr *addr, uplus_u32 *addrlen);

/**
 * @brief 发送下一个Smartlink ACK报文的间隔。
 * @param cur_times 已经发送ACK报文的次数。
 * @return -1表示停止发送ACK报文，其他表示发送下一个ACK报文的间隔，单位毫秒。
 */
extern uplus_s32 uplus_smartlink_ack_interval(uplus_u32 cur_times);

/**
 * @brief 获取Smartlink配置的额外信息。
 * @param ext_data Smartlink配置完成后的额外数据。
 * @param ext_data_type 数据类型，EXT_DATA_TYPE_XXX。
 * @param out_val 输出的信息。
 * @return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_smartlink_ext_analysis(uplus_u8 * ext_data, uplus_u16 ext_data_type, void *out_val);

#ifdef __cplusplus
}
#endif

#endif /*__UPLUS_SMARTLINK_METHOD_EXTERN_H__*/

