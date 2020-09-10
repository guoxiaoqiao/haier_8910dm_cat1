
/**
 * @file uplus_softap_method_extern.h
 * @brief Softap配置方法接口。
 *
 * @date 2017-01-11
 * @author fanming
 *
 */

#ifndef __UPLUS_SOFTAP_METHOD_EXTERN_H__
#define __UPLUS_SOFTAP_METHOD_EXTERN_H__

#ifdef __cplusplus
extern "C" {
#endif

#define TCP_SOFTAP_CONFIG_LISTEN_PORT 56797

/*U+ softap config API*/
/**
 * @brief Softap报文接收接口。
 * @param data 报文数据。
 * @param data_len 报文数据长度。
 * @param ack 输出，应答报文，可以为NULL，表示无应答报文。
 * @param ack_len 输出，应答报文长度。
 * @param is_delay_rcv 是否是延迟处理的报文。
 * @return SOFTAP_RCV_XXX。
 */
extern uplus_s32 uplus_softap_rcv(uplus_u8 *data, uplus_u32 data_len, uplus_u8 **ack, uplus_u32 *ack_len, uplus_u8 is_delay_rcv);

/**
 * @brief Softap开始接口。
 * @return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_softap_start(void);

/**
 * @brief Softap停止接口。
 * @return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_softap_stop(void);

/**
 * @brief 获取Smartlink配置结果接口。
 * @param ssid SSID，最长32字节。
 * @param passwd passphrase，最长64字节。
 * @param ext_data 输出，额外数据，动态申请，可以为NULL。
 * @return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_softap_result_get(uplus_u8 *ssid, uplus_u8 *passwd, uplus_u8 **ext_data);

/*
 * @brief
 * @param
 * @return
 */
extern uplus_s32 uplus_softap_get_next_read_len(uplus_u8 *data, uplus_u32 data_len);

/**
 * @brief 获取Softap配置的额外信息。
 * @param ext_data Softap配置完成后的额外数据。
 * @param ext_data_type 数据类型，EXT_DATA_TYPE_XXX。
 * @param out_val 输出的信息。
 * @return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_softap_ext_analysis(uplus_u8 * ext_data, uplus_u16 ext_data_type, void *out_val);

#ifdef __cplusplus
}
#endif

#endif /*__UPLUS_SOFTAP_METHOD_H__*/

