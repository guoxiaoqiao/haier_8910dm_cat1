
/**
 * @file uplus_epp_extern.h
 * @brief
 *
 * @date 2017-01-11
 * @author fanming
 *
 */

#ifndef __UPLUS_EPP_EXTERN_H__
#define __UPLUS_EPP_EXTERN_H__

#ifdef __cplusplus
extern "C" {
#endif

/*for E++ access*/
enum epp_access_method
{
	EPP_ACCESS_METHOD_SERIAL = 0, /*using serial fd + select*/
	EPP_ACCESS_METHOD_MSG, /*using queue + polling*/
	EPP_ACCESS_METHOD_TCP_CLIENT, /*using tcp client + select*/
	EPP_ACCESS_METHOD_MAX
};

typedef int (*epp_tx)(void *ctl_para, uplus_u8 *buf, uplus_u32 len);

typedef enum device_class_type
{
	DEVICE_CLASS_GENERAL = 0,
	DEVICE_CLASS_EPP_1XX
} device_class_type_t;

/*E++ access info*/
typedef struct epp_access_control_info
{
	uplus_u8 instance;
	uplus_u8 access_method; /*EPP_ACCESS_METHOD_XXX*/

	char dev_name[32]; /*for tcp client, this is server address*/
	uplus_u32 info; /*for tcp client, this is server port*/

	void *ctl_para;
	epp_tx tx; /*for msg*/
} epp_access_control_info_t;

typedef int (* epp_tx_sounding_func)(uplus_u8 * session, uplus_u16 session_len, uplus_u8 * data, uplus_u16 data_len);
#define FROM_BASE_BOARD 0
#define FROM_MODULE 1
typedef void (* epp_rx_rsp_sounding_func)(uplus_u8 * session, uplus_u16 session_len, uplus_u8 * req_data, uplus_u16 req_data_len,
	uplus_u8 * rsp_data, uplus_u16 rsp_data_len, uplus_u8 from);
typedef void (* epp_rx_rpt_sounding_func)(uplus_u8 * rpt_data, uplus_u16 rpt_data_len, uplus_u8 from);

/*E++*/
extern int uplus_epp_init(device_class_type_t device_class);
extern void uplus_epp_register_sounding_cb(epp_tx_sounding_func tx_sounding, epp_rx_rsp_sounding_func rx_rsp_sounding, epp_rx_rpt_sounding_func rx_rpt_sounding);
extern int uplus_epp_tx_rsp(uplus_u8 * session, uplus_u16 session_len, uplus_u8 * req_data, uplus_u16 req_data_len,
	uplus_u8 * rsp_data, uplus_u16 rsp_data_len);
extern int uplus_epp_tx_rpt(uplus_u8 * rpt_data, uplus_u16 rpt_data_len);

/*E++ access*/
extern int uplus_epp_access_register(epp_access_control_info_t *epp_access_ctl_info);
extern int uplus_epp_access_tx_data(uplus_u8 instance, uplus_u8 *buf, uplus_u32 len); /*for EPP_ACCESS_METHOD_MSG*/

#ifdef __cplusplus
}
#endif

#endif /*__UPLUS_EPP_EXTERN_H__*/
