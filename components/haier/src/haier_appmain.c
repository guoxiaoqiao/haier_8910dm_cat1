#include "haier_appmain.h"
#include "vfs.h"

void zk_queue_msg_send(void *qhandle, TASK_MSG_ID id, void *param, uint16_t len, uint32_t timeout);
void zk_debug(uint8_t *buff, uint16_t len);

void set_sys_state(SYS_STATE sysStata);
SYS_STATE get_sye_state(void);

//task handle
TaskHandle_t air_recv_task_handle;
TaskHandle_t air_task_handle;
//queue handle
QueueHandle_t uart_recv_queue;
QueueHandle_t haier_app_queue;
//timer handle
TimerHandle_t Haier_Timers;

Haier_AppSystem appSysTem;
LOCAL_CFG local;

osiPipe_t *at_rx_pipe;
osiPipe_t *at_tx_pipe;

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

void zk_debug(uint8_t *buff, uint16_t len)
{
	OSI_LOGI(0, "*****************************start******************************");

	uint16_t i,j,m;
	j = len % 10;
	uint8_t *p = buff;

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

	OSI_LOGI(0, "[zk local] fota_flag=%d rest_num=%d", local.fota_flag, local.rest_num);

	write_local_cfg_Info();
}

static void prvVirtAtRespCallback(void *param, unsigned event)
{
    osiPipe_t *pipe = (osiPipe_t *)param;
    char buf[256];
    for (;;)
    {
        int bytes = osiPipeRead(pipe, buf, 255);
        if (bytes <= 0)
            break;

        buf[bytes] = '\0';
        OSI_LOGXI(OSI_LOGPAR_IS, 0, "[zk vat] VAT1 <--(%d): %s", bytes, buf);
    }
}

static void virt_at_init(void)
{
    osiPipe_t *at_rx_pipe1 = osiPipeCreate(512);
    osiPipe_t *at_tx_pipe1 = osiPipeCreate(512);

    if((at_rx_pipe1 == NULL) || (at_tx_pipe1 == NULL))
    {
        OSI_LOGE(0, "[zk vat] virt_at_init_0 Pipe Create Fail");
        return;
    }

    at_rx_pipe = at_rx_pipe1;
    at_tx_pipe = at_tx_pipe1;

    osiPipeSetReaderCallback(at_tx_pipe, OSI_PIPE_EVENT_RX_ARRIVED, prvVirtAtRespCallback, at_tx_pipe);

    atDeviceVirtConfig_t cfg = {
        .name = OSI_MAKE_TAG('V', 'A', 'T', '1'),
        .rx_pipe = at_rx_pipe,
        .tx_pipe = at_tx_pipe,
    };

    atDevice_t *device = atDeviceVirtCreate(&cfg);
    if(device == NULL)
    {
        OSI_LOGE(0, "[zk vat] virt_at_init_1 at Device VirtCreate Fail");
        return;
    }
    atDispatch_t *dispatch = atDispatchCreate(device);
    atDeviceSetDispatch(device, dispatch);
    if(atDeviceOpen(device) == false)
    {
        OSI_LOGE(0, "[zk vat] virt_at_init_2 at Device open Fail");
        return;
    }
    OSI_LOGI(0, "[zk vat] virt_at_init_3 Virt at init succ");
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
		OSI_LOGE(0, "haier_app_queue Created Fail");
	}
}

static void app_soft_timer_created(void)
{
	//用于定时获取空调底板状态
    extern void Haier_SoftTimerCallback(void *argument);
	Haier_Timers = xTimerCreate("Haier_Timer", pdMS_TO_TICKS(5000), pdFALSE, NULL, Haier_SoftTimerCallback);
	if(Haier_Timers == NULL)
	{
		OSI_LOGE(0, "[zk] Haier_Timers Created Fail");
	}
}

void app_task_created(void)
{
	extern void air_task_main(void *pParameter);
	//海尔空调控制主任务 ：定时获取空调底板状态、获取环境曲线数据，并发送给海尔服务器
	if(xTaskCreate((TaskFunction_t)air_task_main, "air_task", 512, NULL, OSI_PRIORITY_NORMAL+1, &air_task_handle) != pdPASS)
	{
		OSI_LOGE(0, "[zk] air_task_main Created Fail");
	}

    extern void air_recv_task_main(void *param);
    if(xTaskCreate((TaskFunction_t)air_recv_task_main, "air_recv_task", 1024, NULL, OSI_PRIORITY_NORMAL, &air_recv_task_handle) != pdPASS)
    {
        OSI_LOGE(0, "[zk] air_recv_task Created Fail");
    }
}

static void haier_resource_created(void)
{
	local_cfg_init();

    virt_at_init();

    haier_app_IPC_created();

	app_soft_timer_created();
		
	app_task_created();
}

int haier_appimg_enter(void)
{
    haier_resource_created();

    const char *cmd = "AT+CGMR\r\n";
    OSI_LOGXI(OSI_LOGPAR_S, 0, "VAT1 -->: %s", cmd);
    osiPipeWriteAll(at_rx_pipe, cmd, strlen(cmd), 1000);

    return 0;
}
