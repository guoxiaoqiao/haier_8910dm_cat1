#ifndef    _UPLUS_SERVER_H_
#define    _UPLUS_SERVER_H_

#include "haier_appmain.h"
#include "pal_common.h"
#include "uplus_framework.h"
#include "uplus_config_platform.h"

/*! \def PKT_BUF_DIR_XXX
 *
 * \brief 数据方向类型。
 * REQ 网络-->设备，请求。
 * RSP 设备-->网络，应答。
 * RPT 设备-->网络，主动上报。
 *
 */
#define PKT_BUF_DIR_REQ 1
#define PKT_BUF_DIR_RSP 2
#define PKT_BUF_DIR_RPT 3

#define EPP_DATA		  1
#define	EPP_BIG_DATA	  2

typedef enum
{
    UPLUS_STATE_UNINIT = 0,
 	UPLUS_STATE_INIT_FAIL,
	UPLUS_STATE_RUN,
		
}UPLUS_STATE;

extern dev_info_t dev;

extern void Haier_EventNotifyServer(uint8_t dir, uint8_t data_sub_type, uint8_t *appData, uint32_t appDataLen);
extern void set_uplus_sdk_state(UPLUS_STATE state);
extern uint8_t get_uplus_sdk_state();

#endif