#ifndef _HAIER_APPMAIN_H_
#define _HAIER_APPMAIN_H_

#define OSI_LOG_TAG OSI_MAKE_LOG_TAG('Z', 'K', 'D', 'B')
#define OSI_LOCAL_LOG_LEVEL OSI_LOG_LEVEL_VERBOSE

#include "osi_log.h"
#include "osi_api.h"
#include "osi_pipe.h"
#include "at_engine.h"

#include "drv_uart.h"
#include "drv_gpio.h"
#include "vfs.h"

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "semphr.h"

#include "stdlib.h"
#include "string.h"
#include "stdio.h"

#include "haier_wtd.h"

#define HAIER_FILE_NAME     "/Haier_Local.cfg"

#define USER_IDENTIFIER		"USER:"

#define IMEI_LEN			15
#define IMSI_LEN			15
#define ICCID_LEN			20

#define SEARCH_NET_MAX_TIME	(5*60*1000)

typedef enum
{
	NULL_CTR = 0,
    BT_CONTROL_AIR,
	SERVER_CONTROL_AIR,
		
}REMOTE_CTR_TYPE;

//往这个枚举添加值时，请特别注意要往末尾添加，不要在中间添加，否则会影响某个地方的计算
typedef enum
{
    BT_UART_RECV_MSG = 1,
	BT_EVENT_NOTIFY_MSG = 2,	
	
	AIR_UART_RECV_MSG = 3,
	GET_AIR_DATA_MSG = 4,
	
	BT_CTR_AIR_MSG = 5,
	SERVER_CTR_AIR_MSG = 6,

	UPLUS_SDK_INIT_MSG = 7,
	UPLUS_SDK_SEND_MSG = 8,
	UPLUS_SDK_RECV_MSG = 9,

	FOTA_START_MSG = 10,
	FOTA_UPDATE_MSG = 11,

	NETWORK_ATTACHING = 12,
	NETWORK_ATTACHED = 13,
	NETWORK_DISCONNECT = 14,
	NETWORK_READY = 15,
	NETWORK_LINKING = 16,
	NETWORK_LINKED = 17,

	VAT_SEND_MSG = 18,
	VAT_RECV_MSG = 19,
		
}TASK_MSG_ID;

typedef enum
{
    SYS_STATE_POWN = 1,
	SYS_STATE_NETWORK_CONNECT,
	SYS_STATE_REG,
    SYS_STATE_RUN,
    SYS_STATE_FOTA,
		
}SYS_STATE;

typedef struct
{
    TASK_MSG_ID id;
    uint32_t len;
    void *param;
}TASK_MSG;

//此结构体为系统用到的一些全局变量，为了统一管理放到一个结构体内，后续有其他变量建议都放到这里
typedef struct Haier_AppSystem1{

	SYS_STATE sysCurrState;   //系统当前状态

	uint8_t netCurrState;		//网络当前状态   
	
	//上电初始化就读出来的模组基本信息IMEI, IMSI, ICCID
	char Module_IMEI  [IMEI_LEN+1];
	char Module_IMSI  [IMSI_LEN+1];
	char Module_ICCID [ICCID_LEN+1];
	//保存蓝牙MAC地址，12字节+字符串结束符
	char BT_MAC_BUF[15]; 
	//核心网分配给模组的IP
	char Module_ipaddr	[20];

	uint16_t get_air_data_cnt;

	uint8_t GetDeviceVer_OK_Flag; 	//获取海尔设备软件版本成功标志位
	uint8_t GetDeviceVer_Fail_Flag;	//获取海尔设备软件版本失败标志位

	REMOTE_CTR_TYPE remote_ctr_air_flag; //远程控制空调标志位

	uint8_t Inquire_BigData_FailCnt; //连续10次查询大数据没有返回，则模组整机复位

	uint8_t reconnect_flag;

	uint8_t vat_init_finsh;
	
	char uplus_hostname[50];

}Haier_AppSystem;

typedef struct {
	
	//用户区标识符	
	uint8_t Identifier[5];	
	//fota标志位
	uint8_t fota_flag;
	//模组复位次数
	uint32_t rest_num;
	
}LOCAL_CFG;

extern Haier_AppSystem appSysTem;
extern LOCAL_CFG local;

//task handle
extern TaskHandle_t air_recv_task_handle;
extern TaskHandle_t air_task_handle;
extern TaskHandle_t vat_send_task_handle;
extern TaskHandle_t vat_recv_task_handle;
extern TaskHandle_t network_task_handle;
extern TaskHandle_t led_task_handle;
//queue handle
extern QueueHandle_t uart_recv_queue;
extern QueueHandle_t haier_app_queue;
extern QueueHandle_t vat_send_queue;
extern QueueHandle_t vat_recv_queue;
extern QueueHandle_t network_queue;
//timer handle
extern TimerHandle_t Haier_Timers;
extern TimerHandle_t network_Timers;

extern uint32_t osiMsToOSTick(uint32_t ms);

extern void restart(uint8_t typ);
extern void zk_queue_msg_send(void *qhandle, TASK_MSG_ID id, void *param, uint16_t len, uint32_t timeout);
extern void zk_debug(uint8_t *buff, uint16_t len);
extern int os_get_random(unsigned char *buf, size_t len);

extern void set_net_state(uint8_t net_state);
extern uint8_t get_net_state(void);

extern void set_sys_state(SYS_STATE sysStata);
extern SYS_STATE get_sye_state(void);

#endif