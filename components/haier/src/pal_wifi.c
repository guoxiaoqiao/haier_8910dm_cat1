
/**
 * @file pal_wifi.c
 * @brief
 *
 * @date
 * @author
 *
 */

#include "pal_common.h"

/*!
 * \brief 获取WIFI MAC。
 * \param [out] mac 6字节MAC地址。
 * \return 成功返回0，失败返回-1。
 */
uplus_s32 uplus_wifi_mac_get(uplus_u8 *mac)
{
	memcpy(mac, appSysTem.Module_IMEI, sizeof(appSysTem.Module_IMEI));
	uplus_sys_log("[zk u+] wifi_mac_get %s %s", mac, appSysTem.Module_IMEI);
	
	return 0;
}
/*!
 * \brief 发送802.11帧。
 * \details 802.11帧主要包括probe request帧。
 * \param [in] frame 802.11帧。
 * \param [in] length 802.11帧长度。
 * \return 成功返回0，失败返回-1。
 */
uplus_s32 uplus_wifi_tx_frame(uplus_u8 * frame, uplus_s32 length)
{
	uplus_sys_log("[zk u+] wwifi_tx_frame");
	return -1;
}
/*!
 * \brief 获取BSSID。
 * \param [out] bssid 如果已经连接到AP，返回AP的BSSID；否则返回全0。
 * \param [out] bssid_len BSSID长度，当前固定返回6。
 * \return 成功返回0，失败返回-1。
 */
uplus_s32 uplus_wifi_get_bssid(uplus_u8 *bssid, uplus_u16 *bssid_len)
{
	uplus_sys_log("[zk u+] wifi_get_bssid");
	return -1;
}

/*!
 * \brief 查询WIFI状态。
 * \param [out] status WIFI状态WIFI_DOWN/WIFI_UP。
 * \param [out] wifi_strength，当前所连接AP的信号强度，0-100。如果status是WIFI_DOWN，则wifi_strength等于0。
 * \return 成功返回0，失败返回-1。
 */
uplus_s32 uplus_wifi_status_get(uplus_u8 *status, uplus_u8 *wifi_strength)
{
	uplus_sys_log("[zk u+] wifi_status_get");
	return -1;
}

/*!
 * \brief 设置802.11帧接收回调。
 * \details 802.11帧主要包括beacon帧和probe response帧。目前仅支持在STA模式下，接收所连接AP发出的帧。
 * \param [in] enable 使能或者禁止接收802.11帧回调。
 * \param [in] cb 收到帧的回调通知。设置为NULL表示不需要管理帧回调。
 * \param [in] param 回调接口的用户参数。
 * \return 成功返回0，失败返回-1。
 */
uplus_s32 uplus_wifi_set_rx_frame(uplus_s32 enable, wifi_frame_cb_func cb, void * param)
{
	uplus_sys_log("[zk u+] wifi_set_rx_frame");
	return -1;
}

/*!
 * \brief WIFI功能关闭。
 * \return 成功返回0，失败返回-1。
 */
uplus_s32 uplus_wifi_power_off(void)
{
	uplus_sys_log("[zk u+] wifi_power_off");
	return -1;
}

/*!
 * \brief 查询WIFI Station模式下信噪比。
 * \param [in] dev 指向设备实例的指针。
 * \param [out] snr 信噪比。
 * \return 成功返回UH_OK，失败返回UH_FAIL。
 */
uplus_s32 uplus_wifi_snr_get(uplus_u8 *snr)
{
	uplus_sys_log("[zk u+] wifi_snr_get");
	return -1;
}


