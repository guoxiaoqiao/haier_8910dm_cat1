#include "haier_uplus_server.h"

#define UPLUS_INSTANCE	1 			//instance  数值可以自定义

#define PKT_BUF_MAX	5			//APP允许最大的PKT_BUF数量

void Haier_EventNotifyServer(uint8_t dir, uint8_t data_sub_type, uint8_t *appData, uint32_t appDataLen);
void set_uplus_sdk_state(UPLUS_STATE state);
uint8_t get_uplus_sdk_state();

//向uplusSDK注册设备返回的句柄
static void *device_handle;

//uplusSDK内部pkt_bufde的list指针
static pkt_buf_list_t *pkt_buf_list;

//uplusSDK注册的设备信息
static device_control_info_t device_ctr_info;

//空调的一些基本信息
dev_info_t dev;

//服务器下发的session，需要保存，回复时需要带上
static session_desc_t session;
//服务器下发的数据携带的SN，需要保存，回复时需要带上
static uplus_u32 sn;

static UPLUS_STATE uplus_state;

uint8_t z_ssid[32] = "zhangkai";
uint8_t z_passwd[64] = "12345678";

void set_uplus_sdk_state(UPLUS_STATE state)
{
    uint32_t critical = osiEnterCritical();
    
	uplus_state = state;

	osiExitCritical(critical);
}

uint8_t get_uplus_sdk_state(void)
{
	return uplus_state;
}

static void uplus_config_init(void)
{
	char* file_name[]={FILE_NAME_U_CONFIG, FILE_NAME_U_CONFIG_BAT};
	uint8_t i = 0;
	int fd = 0;
	
	for(i=0; i<sizeof(file_name)/sizeof(file_name[0]); i++)
	{
        fd = vfs_open(file_name[i], O_RDWR);
		if(fd < 0)
		{
			fd = vfs_open(file_name[i], O_RDWR | O_CREAT);
			if(fd  < 0)
			{
				uplus_sys_log("[zk u+] uplus_config_init_0 :error name=%s", file_name[i]);
				continue;
			}
			vfs_lseek(fd, 0, SEEK_SET);

			uint16_t infoLen = 4096;
			uint8_t *info = malloc(infoLen);
			if(info == NULL)
			{
				vfs_close(fd);
				continue;
			}
		
			if(vfs_write(fd, info, infoLen) != infoLen)
			{
				free(info);
				vfs_close(fd);
				uplus_sys_log("[zk u+] uplus_config_init_1 error:name=%s len=%d", file_name[i], infoLen);
				continue;
			}
			uplus_sys_log("[zk u+] uplus_config_init_2 suc:%s", file_name[i]);
			vfs_close(fd);
			free(info);
		}
		else
		{
			vfs_close(fd);
			uplus_sys_log("[zk u+] uplus_config_init_3:%s file existed", file_name[i]);
		}
		fd = 0;
	}
}

#define EPP_HEAD_LEN	46
#define EPP_DATA_LEN_MAX (300 +EPP_HEAD_LEN)
//当服务器侧有数据下来时,此callback会被调用，从pkt_buf中把数据取出透传给底板
uplus_s32 recv_server_data_callback(void *dev_handle, void *param, pkt_buf_t *pkt_buf) 
{
	uplus_s32 result = -1;
	/**** 把数据拿出来透传给底板 ****/

	if((dev_handle == NULL) || (pkt_buf == NULL))
    {
        goto exit;
    }

    uplus_sys_log("[zk u+] recv_server_data_callback_0 len=%d", pkt_buf->data_info.data.raw_data.data_len);
    
    //这两个属性需要保存起来，后面应答时需要
	memset(&session, 0, sizeof(session_desc_t));
	sn = 0;
	memcpy(&session, &pkt_buf->info_res.session_desc, sizeof(session_desc_t));

	sn = pkt_buf->info_res.sn;

	//拷贝数据长度
	uint32_t len = pkt_buf->data_info.data.raw_data.data_len;
	if((len > EPP_HEAD_LEN) && (len < EPP_DATA_LEN_MAX))
	{
		//拷贝数据
		uint8_t *data_buff = pkt_buf->data_info.data.raw_data.data+EPP_HEAD_LEN;
        if(data_buff == NULL)
        {
            uplus_sys_log("[zk u+] recv_server_data_callback_1 data_buff is NULL");
            goto exit;
        }
		//uplus_sys_log("[zk u+] recv_server_data_callback_2 len=%d", len);
        zk_queue_msg_send(uplus_server_queue, UPLUS_SDK_RECV_MSG, data_buff, len-EPP_HEAD_LEN, 0);

		result = 0;
	}
    else
    {
        uplus_sys_log("[zk u+] recv_server_data_callback_3 datalen error len=%d", len);
        goto exit;
    } 

exit:
    if(pkt_buf->data_info.free != NULL)
    {
        uplus_sys_log("[zk u+] recv_server_data_callback_4:pkt_buf->data_info.free");
	    pkt_buf->data_info.free(&(pkt_buf->data_info));
    }
    else
    {
        uplus_sys_log("[zk u+] recv_server_data_callback_5 pkt_buf->data_info.free is NULL");
        result = -1;
    }
    
    if(pkt_buf->free != NULL)
    {
        uplus_sys_log("[zk u+] recv_server_data_callback_6 pkt_buf->free");
	    pkt_buf->free(pkt_buf);
    }
    else
    {
        uplus_sys_log("[zk u+] recv_server_data_callback_7 pkt_buf->free is NULL");
        result = -1;
    }
    uplus_sys_log("[zk u+] recv_server_data_callback_8 recv_callback end reslut=%d", result);
    return result;
}

//当服务器侧有控制数据下来时，此callback会被调用，从val指针中把数据取出透传给底板
uplus_s32 recv_server_CtrData_callback(void *dev_handle, void *param, uplus_s32 ctrl_type, void *val)
{
	/**** 把数据拿出来透传给底板 ****/

	//task_msg_send(u_server_task_handle, UPLUS_SDK_RECV_MSG, val, 1);

	uint8_t *p = (uint8_t*)val;
	uplus_sys_log("[zk] recv_server_callback_0 ctrType=%d val=%d", ctrl_type, *p);

    return 0;
}

static void pkt_data_free(struct data_info * DataInfo)
{
	if(DataInfo != NULL)
	{
		if(DataInfo->data.raw_data.data != NULL)
		{
			free(DataInfo->data.raw_data.data);
			DataInfo->data.raw_data.data = NULL;
			DataInfo->data.raw_data.data_len = 0;
		}
	}
}

//事件上报到海尔优家服务器
void Haier_EventNotifyServer(uint8_t dir, uint8_t data_sub_type, uint8_t *appData, uint32_t appDataLen)
{
	uint16_t len = appDataLen+2;
	uint8_t *databuff = NULL;

	if((appData == NULL) || (appDataLen == 0))
		return;

	if(get_sye_state() == SYS_STATE_RUN)
	{
		//if(get_net_state() == NETWORK_LINKED)
		{
			databuff = (uint8_t *)malloc(len);
			if(databuff == NULL)
			{
				uplus_sys_log("[zk] Haier_EventNotifyServer_0 malloc fail len=%d", len);
				return ;
			}
			memset(databuff, 0, len);
			
			databuff[0] = dir;
			databuff[1] = data_sub_type;
			
			memcpy(databuff+2, appData, appDataLen);

            zk_queue_msg_send(uplus_server_queue, UPLUS_SDK_SEND_MSG, databuff, len, 0);
		
        	free(databuff);
		}
	}
}


//当有数据要发送到服务器时，调用此接口
void send_server_data(uint8_t dir, uint8_t data_sub_type, uint8_t *buff, uint32_t len)
{	
	if((buff == NULL) || (len == 0))
		return;
	
	pkt_buf_t *pkt_buf = NULL;
	
	//申请一个pkt_buf用于发送数据
	if(pkt_buf_list != NULL)
	{
		pkt_buf = uplus_pkt_buf_malloc(pkt_buf_list);
		if(pkt_buf == NULL)
		{
			uplus_sys_log("[zk u+] send_server_data_1:malloc pkt_buf fail");
			return;
		}
	}
	else
	{
		uplus_sys_log("[zk u+] send_server_data_2:pkt_buf_list is NULL");
		return;
	}

	//pkt_buf->data_info.data_type = DATA_TYPE_NONE;
	pkt_buf->data_info.free = pkt_data_free;

	uint8_t *data = malloc(len);
	if(data == NULL)
	{
		pkt_buf->free(pkt_buf);
		uplus_sys_log("[zk u+] send_server_data:malloc fail");
		return;
	}
	memset(data, 0, len);
	pkt_buf->data_info.data.raw_data.data = data;
	pkt_buf->data_info.data.raw_data.data_len = len;

	pkt_buf->info_res.dir = dir;
	//如果当前数据包是应答类型，那么需要带上这些信息（服务器下发的），主动上报不需要
	if(PKT_BUF_DIR_RSP == dir)
	{
		memcpy(&pkt_buf->info_res.session_desc, &session, sizeof(session_desc_t));
		pkt_buf->info_res.sn = sn;
	}
	pkt_buf->data_info.data_type = DATA_TYPE_EPP;
	pkt_buf->data_info.data_sub_type = data_sub_type;

	memcpy(data, buff, len);
	
	/* ****    发送pkt_buf     **** */
	
	if(uplus_dev_tx(device_handle, pkt_buf) == 0)
	{
		uplus_sys_log("[zk u+] send_server_data_:ok");
	}
	else
	{
		uplus_sys_log("[zk u+] send_server_data_:fail");
	}
}

static void u_sys_event_cb_func(uplus_u16 sys_event, void *param)
{
	switch(sys_event)
	{
		case SYS_EVENT_WIFI_CONFIG_MODE_ENTER:
			uplus_sys_log("[zk u+] u_sys_event_cb_4 exit wifi config start");
			break;
		case SYS_EVENT_CLOUD_START:
			uplus_sys_log("[zk u+] u_sys_event_cb_0 cloud Start...");
			break;
		case SYS_EVENT_CLOUD_OK:
			set_sys_state(SYS_STATE_RUN);
			set_uplus_sdk_state(UPLUS_STATE_RUN);
			uplus_sys_log("[zk u+] u_sys_event_cb_1 conn cloud OK");
			break;
		case SYS_EVENT_CLOUD_FAIL:
			set_sys_state(SYS_STATE_NETWORK_CONNECT);
			set_uplus_sdk_state(UPLUS_STATE_INIT_FAIL);
			uplus_sys_log("[zk u+] u_sys_event_cb_2 conn cloud fail");
			break;
		default:
			uplus_sys_log("[zk u+] u_sys_event_cb_3 sys_event=%d", sys_event);
			break;
	}
}

static uplus_s32 normal_start(init_config_para_t * init_config)
{
	uplus_s32 result = uplus_wifi_conn_net_init(&init_config->wifi_conn_config);

	uplus_sys_log("[zk u+] normal_start init resylt=%d", result);
	
	return result;
}

int32_t uplus_usr_init(void)
{
	init_config_para_t * init_config;
	wifi_conn_config_para_t * wifi_conn_config;
	int32_t ret;

	//call pal init
	ret = uplus_sys_init();
	if (ret)
	{
		uplus_sys_log("[zk u+] uplus_usr_init_1:sys_init fail");
		return ret;
	}

	init_config = (init_config_para_t *)uplus_tool_malloc(sizeof(init_config_para_t));
	if (init_config == NULL)
	{
		uplus_sys_log("[zk u+] uplus_usr_init_2:malloc fail");
		return -1;
	}
	uplus_tool_memset(init_config, 0, sizeof(init_config_para_t));

	
	strncpy((char *)init_config->hw_ver, "U_2.0.01", strlen("U_2.0.01"));   
	strncpy((char *)init_config->sw_ver, "e_2.7.02", strlen("e_2.7.02"));
	strncpy((char *)init_config->platform, "UDISCOVERY_UWT", strlen("UDISCOVERY_UWT"));
	strncpy((char *)init_config->sdk_ver, "2.0.0", strlen("2.0.0"));
	strncpy((char *)init_config->sdk_platform, "UDISCOVERY_UWT", strlen("UDISCOVERY_UWT"));
	
	init_config->watchdog_time = 250;

	init_config->max_client_num = UPLUS_CONFIG_MAX_CLIENT_NUM;
	init_config->conf_extend_block_num = UPLUS_CONFIG_CONF_EXTEND_BLOCK_NUM;
	init_config->max_wifi_scan_ap_num = UPLUS_CONFIG_MAX_WIFI_SCAN_AP_NUM;
	init_config->wifi_config_smartlink_time = UPLUS_CONFIG_WIFI_CONFIG_SMARTLINK_TIME;
	init_config->wifi_config_softap_time = UPLUS_CONFIG_WIFI_CONFIG_SOFTAP_TIME;
	init_config->wifi_config_smartap_time = UPLUS_CONFIG_WIFI_CONFIG_SMARTAP_TIME;
	init_config->wifi_config_third_time = UPLUS_CONFIG_WIFI_CONFIG_THIRD_TIME;
	init_config->wifi_config_smartlink_to_softap_time = UPLUS_CONFIG_WIFI_CONFIG_SMARTLINK_TO_SOFTAP_TIME;
	init_config->led_to_power_save_time = UPLUS_CONFIG_LED_TO_POWER_SAVE_TIME;
	init_config->product_mode_wifi_conn_time = UPLUS_CONFIG_PRODUCT_MODE_WIFI_CONN_TIME;
	init_config->watchdog_time = UPLUS_CONFIG_WATCHDOG_TIME;

	init_config->support_product_mode = UPLUS_CONFIG_SUPPORT_PRODUCT_MODE;
	init_config->enter_product_mode_power_on_without_wifi_conf = UPLUS_CONFIG_ENTER_PRODUCT_MODE_POWER_ON_WITHOUT_WIFI_CONF;
	init_config->smartlink_method_num = UPLUS_CONFIG_SMARTLINK_METHOD_NUM;
	init_config->softap_method_num = UPLUS_CONFIG_SOFTAP_METHOD_NUM;

	wifi_conn_config = &init_config->wifi_conn_config;
	uplus_tool_strncpy((uplus_s8 *)wifi_conn_config->vendor_id, UPLUS_CONFIG_VERDOR_ID, sizeof(wifi_conn_config->vendor_id));
	wifi_conn_config->fast_wifi_conn_interval = UPLUS_CONFIG_FAST_WIFI_CONN_INTERVAL;
	wifi_conn_config->fast_wifi_conn_time = UPLUS_CONFIG_FAST_WIFI_CONN_TIME;
	wifi_conn_config->slow_wifi_conn_interval = UPLUS_CONFIG_SLOW_WIFI_CONN_INTERVAL;
	wifi_conn_config->slow_wifi_conn_time = UPLUS_CONFIG_SLOW_WIFI_CONN_TIME;
	wifi_conn_config->ap_channel = UPLUS_CONFIG_AP_CHANNEL;
	wifi_conn_config->support_mon_softap = UPLUS_CONFIG_SUPPORT_MON_SOFTAP;

	//uplus_sys_log("[zk u+] uplus_usr_init_00:uplus_init START");
	//framework init
	ret = uplus_init(init_config);
	if(ret)
	{
		uplus_sys_log("[zk u+] uplus_usr_init_3:uplus_init fail");
		return ret;
	}
	
	/* do other init*/
	ret = normal_start(init_config);
	if (ret)
	{
		uplus_sys_log("[zk u+] uplus_usr_init_4:normal_start fail");
		return ret;
	}

	ret = uplus_start();
	
	uplus_sys_log("[zk u+] uplus_usr_init_8 uplus_start=%d", ret);
	
	/*ret = uplus_wifi_conf_other_enter();
	if(ret)
	{
		uplus_sys_log("[zk u+] uplus_usr_init_5:wifi_conf_other_enter fail");
		return ret;
	}*/
	
	uplus_os_task_sleep(3000);
	ret = uplus_wifi_conf_notify(z_ssid, z_passwd);
	if(ret)
	{
		uplus_sys_log("[zk u+] uplus_usr_init_6:wifi_conf_notify fail");
		return ret;
	}
	/*uplus_sys_log("[zk u+] uplus_usr_init_9:exir config start");
	ret = uplus_wifi_conf_other_exit(1, z_ssid, z_passwd);
	if(ret)
	{
		uplus_sys_log("[zk u+] uplus_usr_init_7:wifi_conf_other_exit fail");
		return ret;
	}
	uplus_sys_log("[zk u+] uplus_usr_init_10:exir config end");*/
	return ret;
}

extern uint8_t TypeId[32];
int uplus_server_init(void)
{
	/****其它的初始化过程****/
	
	//uplusSDK初始化和启动
	if(uplus_usr_init() == -1)
	{
		uplus_sys_log("[zk u+] uplus_server_init_0 init fail");

		set_uplus_sdk_state(UPLUS_STATE_INIT_FAIL);
		
		return -1;
	}
	
	//获取uplusSDK内部的PKT_BUF指针                      
	pkt_buf_list = uplus_get_pkt_buf_list();
	if(pkt_buf_list != NULL)
	{
		//初始化APP所需要的PKT_BUF最大数量
		uplus_pkt_buf_list_init(pkt_buf_list, PKT_BUF_MAX);
	}

	#if 0
    dev.type = DEVICE_TYPE_AC;
    memcpy(dev.dev_name, local.dev_name, 8);
    memcpy(dev.dev_id, TypeId, 32);
	memcpy(dev.proto_ver, local.proto_ver, 5);
	memcpy(dev.sw_ver, local.sw_ver, 8);
    memcpy(dev.hw_ver, local.hw_ver, 8);
	#endif
	//拷贝空调底板信息
	memcpy(&device_ctr_info.dev_info, &dev, sizeof(dev_info_t)); 
	
	uplus_sys_log("[zk u+] type=%d", device_ctr_info.dev_info.type);
	uplus_sys_log("[zk u+] id=0x%x", device_ctr_info.dev_info.dev_id[31]);
	uplus_sys_log("[zk u+] sv=%s", device_ctr_info.dev_info.sw_ver);
	uplus_sys_log("[zk u+] hv=%s", device_ctr_info.dev_info.hw_ver);
	uplus_sys_log("[zk u+] pv=%s", device_ctr_info.dev_info.proto_ver);
	uplus_sys_log("[zk u+] name=%s", device_ctr_info.dev_info.dev_name);
	
	//数据类型为E++           
	device_ctr_info.data_type = DATA_TYPE_EPP | DATA_TYPE_EPP_WITH_PARA;
	
	//需要传递给callback的参数，目前不需要，填NULL
	device_ctr_info.param = NULL;
	
	//不需要数据转换，填空
	device_ctr_info.convert = NULL;
	
	//添加tx的callback（有对应事件发生时会被调用）
	device_ctr_info.tx = recv_server_data_callback;
	
	//添加ctrl的callback（有对应事件发生时会被调用）
	device_ctr_info.ctrl = recv_server_CtrData_callback;
	
	//向uplusSDK注册一个设备
	device_handle = uplus_dev_register(DEVICE_PROTO_FAMILY_EPP, UPLUS_INSTANCE, &device_ctr_info);
	if(device_handle == NULL)
	{
		uplus_sys_log("[zk u+] uplus_server_init_1 init fal");
		set_uplus_sdk_state(UPLUS_STATE_INIT_FAIL);
		return -1;
	}
	uplus_sys_log("[zk u+] uplus_server_init_1 init suc");
		
	set_uplus_sdk_state(UPLUS_STATE_RUN);

	//通知SDK，终端正常
	uplus_dev_status(device_handle, 1);

	uplus_sys_event_cb_set(u_sys_event_cb_func, NULL);
	
	return 0;
}

extern void LEDState_Change(uint8_t FlashCnt, uint16_t time);
static void uplus_server_send_handle(uint8_t *buff, uint16_t len)
{
	if((buff == NULL) || (len == 0))
		return;
	
	uint8_t dir = buff[0];
	uint8_t data_sub_type = buff[1];

	uplus_sys_log("[zk u+] uplus_server_send_0 notify airdatalen=%d", len-2);
	
	zk_debug(buff+2, len-2);

	LEDState_Change(2, 100);
	send_server_data(dir, data_sub_type, buff+2, len-2);
}

static void uplus_server_recv_handle(uint8_t *buff, uint16_t len)
{
	zk_debug(buff, len);

	LEDState_Change(1, 200);
	
    zk_queue_msg_send(haier_app_queue, SERVER_CTR_AIR_MSG, buff, len, 0);
	
	uplus_sys_log("[zk u+] uplus_server_recv_1 recv");
}

void uplus_server_task_main(void *pParameter)
{
	TASK_MSG *msg = NULL;

	while(1)
	{
		if(xQueueReceive(uplus_server_queue, &msg, portMAX_DELAY) == pdPASS)
        {
            switch(msg->id)
            {
                case UPLUS_SDK_INIT_MSG:
                    uplus_config_init();

                    uplus_server_init();
                    break;
                case UPLUS_SDK_SEND_MSG:
                    uplus_server_send_handle(msg->param, msg->len);
                    break;
                case UPLUS_SDK_RECV_MSG:
                    uplus_server_recv_handle(msg->param, msg->len);
                    break;
                default:
                    break;
            }
        }
		
	    if(msg)
	    {
	        if(msg->param)
	        {
	            free(msg->param);
	            msg->param = NULL;
	        }
	        free(msg);
	        msg = NULL;
	    }
	}
}
