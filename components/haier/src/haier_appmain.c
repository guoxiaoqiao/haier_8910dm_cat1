#include "haier_appmain.h"

void restart(uint8_t typ);
void zk_queue_msg_send(void *qhandle, TASK_MSG_ID id, void *param, uint16_t len, uint32_t timeout);
void zk_debug(uint8_t *buff, uint32_t len);
int os_get_random(unsigned char *buf, size_t len);

void set_net_state(uint8_t net_state);
uint8_t get_net_state(void);

void set_sys_state(SYS_STATE sysStata);
SYS_STATE get_sye_state(void);

void write_local_cfg_Info(void);

//task handle
TaskHandle_t air_recv_task_handle;
TaskHandle_t air_task_handle;
TaskHandle_t vat_send_task_handle;
TaskHandle_t vat_recv_task_handle;
TaskHandle_t network_task_handle;
TaskHandle_t uplus_server_handle;
TaskHandle_t led_task_handle;
TaskHandle_t fota_task_handle;
//queue handle
QueueHandle_t uart_recv_queue;
QueueHandle_t haier_app_queue;
QueueHandle_t vat_send_queue;
QueueHandle_t vat_recv_queue;
QueueHandle_t network_queue;
QueueHandle_t uplus_server_queue;
QueueHandle_t fota_event_queue;
//timer handle
TimerHandle_t Haier_Timers;
TimerHandle_t network_Timers;

Haier_AppSystem appSysTem;
LOCAL_CFG local;

void restart(uint8_t typ)
{
	switch (typ)
	{
		case 1:
			OSI_LOGI(0, "[zk] restart_0: network timeout reset");
			break;
		case 2:
			OSI_LOGI(0, "[zk] restart_1: Inquire_BigData_FailCnt reset");
			break;
		case 3:
			OSI_LOGI(0, "[zk] restart_2: u+ sdk reset");
			break;
		case 4:
			OSI_LOGI(0, "[zk] restart_3: u+ sdk DNS parsing error");
			break;
		case 5:
			OSI_LOGI(0, "[zk] restart_4: u+ sdk net work disconnect");
			break;
		case 6:
			OSI_LOGI(0, "[zk] restart_5: fota reset");
			break;
		case 7:
			OSI_LOGI(0, "[zk] restart_6: fota download fail");
			break;
		default:
			return;
	}
	vat_cmd_send("AT+TRB\r\n", strlen("AT+TRB\r\n"));
	//osiShutdown(OSI_SHUTDOWN_RESET);
}

void zk_queue_msg_send(void *qhandle, TASK_MSG_ID id, void *param, uint16_t len, uint32_t timeout)
{
    TASK_MSG *msg = NULL;

    msg = (TASK_MSG *)malloc(sizeof(TASK_MSG));
	if(msg == NULL)
	{
		OSI_LOGE(0, "[zk] task_msg_send_0 malloc fail %d", sizeof(TASK_MSG));
		return;
	}
	memset(msg, 0, sizeof(TASK_MSG));
    msg->id = id;
	if((param != NULL) && (len > 0))
	{
		msg->param = malloc(len+1);
		if(msg->param == NULL)
		{
			OSI_LOGE(0, "[zk] task_msg_send_1 malloc fail %d", len);
			free(msg);
			return ;
		}
		memset(msg->param, 0, len+1);
		memcpy(msg->param, param, len);
    	msg->len = len;
	}

	if(osiMessageQueueTryPut((osiMessageQueue_t *)qhandle, &msg, timeout) == false)
	{
		OSI_LOGE(0, "[zk] zk_queue_msg_send_2 Queue Send fail");
	}
}

int os_get_random(unsigned char *buf, size_t len)
{    
	int i, j;    
	unsigned long tmp;     

	for (i = 0; i < ((len + 3) & ~3) / 4; i++) 
	{        
		//tmp = r_rand();    
		tmp = rand();    
		for (j = 0; j < 4; j++) 
		{            
			if ((i * 4 + j) < len) 
			{                
				buf[i * 4 + j] = (uint8_t)(tmp >> (j * 8));
			} 
			else 
			{                
				break;            
			}        
		}    
	}     
	return 0;
}


void zk_debug(uint8_t *buff, uint32_t len)
{
	if(buff == NULL)
	{
		OSI_LOGE(0, "zk_debug buff is null");
		return;
	}
	OSI_LOGI(0, "*****************************start******************************");

	uint32_t i,j,m;
	j = (uint32_t)(len % 10);
	uint8_t *p = buff;

	//OSI_LOGE(0, "zk_debug len=%d j=%d", len, j);

	for(i=0; i < (len - j); i+=10)
	{
		OSI_LOGI(0, "%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",p[i],p[i+1],p[i+2],p[i+3],p[i+4],p[i+5],p[i+6],p[i+7],p[i+8],p[i+9]);
	
	}
	for(m=0; m<j; m++)
	{
		OSI_LOGI(0, "%02X", p[len - j + m]);
	}
	OSI_LOGI(0, "*****************************end******************************");
}

void set_net_state(uint8_t net_state)
{
	uint32_t critical = osiEnterCritical();
    
	appSysTem.netCurrState = net_state;

	osiExitCritical(critical);
}

uint8_t get_net_state(void)
{
	return appSysTem.netCurrState;
}

void set_sys_state(SYS_STATE sysStata)
{
	uint32_t critical = osiEnterCritical();

    appSysTem.sysCurrState = sysStata;
    
    osiExitCritical(critical);
}

SYS_STATE get_sye_state(void)
{
	return appSysTem.sysCurrState;
}

void write_local_cfg_Info(void)
{
	int fd = vfs_open(HAIER_FILE_NAME, O_RDWR | O_CREAT);
    if (fd < 0)
    {
        OSI_LOGE(0, "[zk local] write_local_cfg_Info_0: vfs_open Fail");
        return;
    }

    if (vfs_lseek(fd, 0, SEEK_SET) != 0)
    {
        OSI_LOGE(0, "[zk local] write_local_cfg_Info_1: vfs_lseek Fail");
        vfs_close(fd);
        return;
    }

    if(vfs_write(fd, &local, sizeof(LOCAL_CFG)) != sizeof(LOCAL_CFG))
    {
        OSI_LOGE(0, "[zk local] write_local_cfg_Info_2: vfs_write Fail");
        vfs_close(fd);
        return;
    }
    vfs_close(fd);

	OSI_LOGI(0, "[zk local] Write 'Haier_Local' file Succ");
}

static void default_config_info(void)
{
	//先对整个结构体清0
	memset(&local, 0, sizeof(LOCAL_CFG));
	//设置用户区标识符
	strncpy((char *)local.Identifier, USER_IDENTIFIER, sizeof(local.Identifier));
	
	OSI_LOGI(0, "[zk local] Use default configuration");

	write_local_cfg_Info();
}

static void read_local_cfg_Info(void)
{
	int fd = vfs_open(HAIER_FILE_NAME, O_RDWR | O_CREAT);
    if (fd < 0)
    {
        OSI_LOGE(0, "[zk local] read_local_cfg_Info_0: vfs_open Fail");
        return;
    }

    if (vfs_lseek(fd, 0, SEEK_SET) != 0)
    {
        OSI_LOGE(0, "[zk local] read_local_cfg_Info_1: vfs_lseek Fail");
        vfs_close(fd);
        return;
    }

	if (vfs_read(fd, &local, sizeof(LOCAL_CFG)) != sizeof(LOCAL_CFG))
    {
        OSI_LOGE(0, "[zk local] read_local_cfg_Info_2: vfs_read Fail");

        vfs_close(fd);

        default_config_info();

        return;
    }
	vfs_close(fd);
	//校验是否是有效数据
	if(strncmp((char *)local.Identifier, USER_IDENTIFIER, sizeof(local.Identifier)) != 0)
	{
		OSI_LOGI(0, "[zk local] read_local_cfg_Info_3: Data Fail");
		
		default_config_info();  
	}
}

static void local_cfg_init(void)
{
	read_local_cfg_Info();

	local.rest_num++;

	if((local.fota_flag > (uint8_t)FOTA_NULL) && (local.fota_flag < (uint8_t)FOTA_STATUS_MAX))
	{
		if(local.fota_fail_ret_num < FOTA_FAIL_RETRIES_NUM)
		{
			set_sys_state(SYS_STATE_FOTA);
			local.fota_fail_ret_num++;
		}
		else
		{
			local.fota_flag = (uint8_t)FOTA_NULL;
			local.fota_fail_ret_num = 0;
		}
	}
	OSI_LOGI(0, "[zk local] fota_flag=%d rest_num=%d,fota_num=%d", local.fota_flag, local.rest_num, local.fota_fail_ret_num);

	OSI_LOGXI(OSI_LOGPAR_S, 0, "[zk local] app_version->%s", APP_VERSION);

	write_local_cfg_Info();
}

static void haier_app_IPC_created(void)
{
	//用于接收空调串口数据
	uart_recv_queue = xQueueCreate(3, sizeof(TASK_MSG *));
	if(uart_recv_queue == NULL)
	{
		OSI_LOGE(0, "[zk] uart_recv_queue Created Fail");
	}
    //用于定时获取获取空调底板数据
	haier_app_queue = xQueueCreate(5, sizeof(TASK_MSG *));
	if(haier_app_queue == NULL)
	{
		OSI_LOGE(0, "[zk] haier_app_queue Created Fail");
	}
    //用于虚拟AT发送
	vat_send_queue = xQueueCreate(5, sizeof(TASK_MSG *));
	if(vat_send_queue == NULL)
	{
		OSI_LOGE(0, "[zk] vat_send_queue Created Fail");
	}
    //用于虚拟AT接收
	vat_recv_queue = xQueueCreate(5, sizeof(TASK_MSG *));
	if(vat_recv_queue == NULL)
	{
		OSI_LOGE(0, "[zk] vat_recv_queue Created Fail");
	}
    //用于网络任务处理
    network_queue = xQueueCreate(5, sizeof(TASK_MSG *));
	if(network_queue == NULL)
	{
		OSI_LOGE(0, "[zk] network_queue Created Fail");
	}
	//用于优家服务器数据上下行处理
    uplus_server_queue = xQueueCreate(5, sizeof(TASK_MSG *));
	if(uplus_server_queue == NULL)
	{
		OSI_LOGE(0, "[zk] uplus_server_queue Created Fail");
	}
	//用于FOTA事件通知	
	fota_event_queue = xQueueCreate(3, sizeof(TASK_MSG *));
	if(fota_event_queue == NULL)
	{
		OSI_LOGE(0, "[zk] fota_event_queue Created Fail");
	}
}

static void app_soft_timer_created(void)
{
	if(local.fota_flag == 0)
	{
		//用于定时获取空调底板状态
		extern void Haier_SoftTimerCallback(void *argument);
		Haier_Timers = xTimerCreate("Haier_Timer", pdMS_TO_TICKS(5000), pdFALSE, NULL, Haier_SoftTimerCallback);
		if(Haier_Timers == NULL)
		{
			OSI_LOGE(0, "[zk] Haier_Timers Created Fail");
		}
	}
    //用于控制搜网时间
    extern void net_timer_callback(void *argument);
    network_Timers = xTimerCreate("network_Timer", pdMS_TO_TICKS(SEARCH_NET_MAX_TIME), pdFALSE, NULL, net_timer_callback);
	if(network_Timers == NULL)
	{
		OSI_LOGE(0, "[zk] network_Timers Created Fail");
	}
}

void app_task_created(void)
{
	extern void air_task_main(void *pParameter);
	//海尔空调控制主任务 ：定时获取空调底板状态,转发服务器下发的控制数据给底板
	if(xTaskCreate((TaskFunction_t)air_task_main, "air_task", 2048, NULL, OSI_PRIORITY_NORMAL+2, &air_task_handle) != pdPASS)
	{
		OSI_LOGE(0, "[zk] air_task_main Created Fail");
	}
    //海尔空调底板数据接收任务：负责获取底板发来的数据，并对其进行合法性判断，符合相对应条件后也负责将应用数据上报到云平台
    extern void air_recv_task_main(void *param);
    if(xTaskCreate((TaskFunction_t)air_recv_task_main, "air_recv_task", 2048, NULL, OSI_PRIORITY_NORMAL+2, &air_recv_task_handle) != pdPASS)
    {
        OSI_LOGE(0, "[zk] air_recv_task Created Fail");
    }
	//网络侧数据上下行任务：负责直接和uplus SDK交互，数据上下行都由此任务负责
	extern void uplus_server_task_main(void *pParameter);
    if(xTaskCreate((TaskFunction_t)uplus_server_task_main, "uplus_server_task", 4096, NULL, OSI_PRIORITY_NORMAL+2, &uplus_server_handle) != pdPASS)
    {
        OSI_LOGE(0, "[zk] uplus_server_task Created Fail");
    }
}

static void haier_resource_created(void)
{
	local_cfg_init();

	haier_app_IPC_created();

	app_soft_timer_created();

	if(local.fota_flag == 0)
	{
		app_task_created();
	}
	
	//fota控制任务：主要负责fota过程的下载阶段，下载完成后会将数据保存到文件系统，然后复位，升级过程在boot中执行
	extern void ota_task(void *pParameter);
	if(xTaskCreate((TaskFunction_t)ota_task, "fota_task", 2048, NULL, OSI_PRIORITY_NORMAL+4, &fota_task_handle) != pdPASS)
	{
		OSI_LOGE(0, "[zk] fota_task Created Fail");
	}
	
	//搜网任务：负责控制模组网络联网和丢网的事件处理
    extern void network_task_main(void *pParameter);
    if(xTaskCreate((TaskFunction_t)network_task_main, "network_task", 2048, NULL, OSI_PRIORITY_NORMAL+3, &network_task_handle) != pdPASS)
    {
        OSI_LOGE(0, "[zk] network_task Created Fail");
    }

	//虚拟AT发送任务：为保证虚拟AT通道的互斥性，创建一个保护任务，所有向虚拟AT通道发送的数据都由此任务来进行，最大程度保证资源的原子性
    extern void vat_send_task_main(void *pParameter);
    if(xTaskCreate((TaskFunction_t)vat_send_task_main, "vat_send_task", 1024, NULL, OSI_PRIORITY_NORMAL+1, &vat_send_task_handle) != pdPASS)
    {
        OSI_LOGE(0, "[zk] vat_send_task Created Fail");
    }
    //虚拟AT接收任务：接收虚拟AT通道返回的数据，并对其进行解析处理
    extern void vat_recv_task_main(void *pParameter);
    if(xTaskCreate((TaskFunction_t)vat_recv_task_main, "vat_recv_task", 2048, NULL, OSI_PRIORITY_NORMAL+1, &vat_recv_task_handle) != pdPASS)
    {
        OSI_LOGE(0, "[zk] vat_recv_task Created Fail");
    }

	//LED控制任务：根据系统当前状态表现出各种不同的闪灯形式
	extern void led_task_main(void *pParameter);
	if(xTaskCreate((TaskFunction_t)led_task_main, "led_task", 512, NULL, OSI_PRIORITY_NORMAL, &led_task_handle) != pdPASS)
    {
        OSI_LOGE(0, "[zk] led_task Created Fail");
    }
}

void haier_appimg_enter(void)
{
    haier_resource_created();

	//extern void uplus_sdk_test(void);
	//uplus_sdk_test();
}
