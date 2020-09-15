#include "Haier_BottomPlate.h"
#include "haier_uplus_server.h"

static drvUart_t *air_drv;
static air_uart_recv1 air_uart_recv;

//空调底板的一些基础信息
struct DevicVersion1 DevicVersion;
//前一次空调状态 （与当前空调状态比较用的，判断空调的状态是否改变）
static struct StateData1 Old_StateData;
//当前获取的空调状态
static struct StateData1 Curr_StateData;
//模组网络侧状态(每次查询大数据时，会查询一次网络状态数据，收到底板大数据应答时，会携带网络状态数据一起上报给服务器)
struct NET_STATS1 net_info;

//泰国底板type id
static uint8_t TypeId1[32] = {0x20,0x08,0x61,0x08,00,0x82,0x03,0x24,0x02,0x11,00,0x11,0x80,0x11,0x35,0x41,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,0x40};
//印度底板type id
uint8_t TypeId[32] = {0x20,0x08,0x61,0x08,00,0x82,0x03,0x24,0x02,0x12,00,0x11,0x80,0x11,0x35,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,0x40};

void air_uart_write(uint8_t *buff, int32_t len)
{
	if((buff == NULL) || (len <=0))
		return;
	
    drvUartSend(air_drv, (void *)buff, (size_t)len);
}

static void air_recv_Callback(void *param, uint32_t evt)
{	
   int recvlen = 0;

   switch (evt)
   {
   case DRV_UART_EVENT_RX_ARRIVED:
		recvlen = drvUartReadAvail(air_drv);
		if(recvlen > 0)
		{
			OSI_LOGI(0, "[air] recv new data len1=%d", recvlen);
	
			air_uart_recv.recvLen = 0;
			memset(air_uart_recv.recvbuff, 0, AIR_UART_RECVBUFF_MAX);
			recvlen = drvUartReceive(air_drv, (void *)air_uart_recv.recvbuff, recvlen);

			//OSI_LOGI(0, "[air] recv new data len2=%d", recvlen);
			if(recvlen > 0)
				air_uart_recv.recvLen = (uint16_t)recvlen;

	    	zk_queue_msg_send(uart_recv_queue, AIR_UART_RECV_MSG, (void *)air_uart_recv.recvbuff, air_uart_recv.recvLen, 0);
		}
		break;
   case DRV_UART_EVENT_RX_OVERFLOW:
		OSI_LOGI(0, "[air] RX FIFO overflowed");
		break;
   case DRV_UART_EVENT_TX_COMPLETE:
		//OSI_LOGI(0, "[air] All data had been sent");
		break;
   default:
		OSI_LOGI(0, "[air] not event event=%d", evt);
		break;
   }
}

static void Haier_GetBigDataHandle(void)
{
	zk_queue_msg_send(haier_app_queue, GET_AIR_DATA_MSG, NULL, 0, 0);
}

void Haier_SoftTimerCallback(void *argument)
{
	Haier_GetBigDataHandle();
}

static void air_uart_init(void)
{   
    drvUartCfg_t air_drvcfg = {0};

    air_drvcfg.baud = AIR_BAUD;
	air_drvcfg.data_bits = DRV_UART_DATA_BITS_8;
	air_drvcfg.stop_bits = DRV_UART_STOP_BITS_1;
	air_drvcfg.parity = DRV_UART_NO_PARITY;
    air_drvcfg.rx_buf_size = AIR_UART_RX_BUF_SIZE;
    air_drvcfg.tx_buf_size = AIR_UART_TX_BUF_SIZE;
    air_drvcfg.event_mask = DRV_UART_EVENT_RX_ARRIVED | DRV_UART_EVENT_RX_OVERFLOW | DRV_UART_EVENT_TX_COMPLETE;
    air_drvcfg.event_cb = air_recv_Callback;
    air_drvcfg.event_cb_ctx = NULL;

    drvUart_t *air_drv1 = drvUartCreate(AIR_UART_NAME, &air_drvcfg);
	if(air_drv1 == NULL)
	{
		OSI_LOGE(0, "[air] air_drv create fail");
	}
	else
	{
		air_drv = air_drv1;
		OSI_LOGI(0, "[air] air_drv create suc");
	}

	if(drvUartOpen(air_drv) == true)
	{
		OSI_LOGI(0, "[air] air_drv open suc");
	}
}

static void remote_control_air_handle(REMOTE_CTR_TYPE ctrType, uint8_t *databuff, uint16_t datalen)
{
	if((ctrType > SERVER_CONTROL_AIR) || (databuff == NULL) || (databuff == 0))
		return;

	if(appSysTem.remote_ctr_air_flag == NULL_CTR)
	{
		air_uart_write(databuff, datalen);

		appSysTem.remote_ctr_air_flag = ctrType;

		OSI_LOGI(0, "[zk air] remote_control_1 ctrtyp=%d", ctrType);
	}
	else if(appSysTem.remote_ctr_air_flag == BT_CONTROL_AIR)
	{
		OSI_LOGI(0, "[zk air] remote_control_1 BT Controlling...");
	}
	else if(appSysTem.remote_ctr_air_flag == SERVER_CONTROL_AIR)
	{
		OSI_LOGI(0, "[zk air] remote_control_2 Server Controlling...");
	}
}

//获取海尔设备版本号的数据是固定的（直接拿的120项目上的数据）
static uint8_t GetDeviceVer[13]={0xFF,0xFF,0x0A, 00, 00, 00, 00, 00, 0x00, 0x61, 0x00, 0x02, 0x6d};
//获取海尔空调设备版本号
static void Get_DeviceVer(void)
{
	//zk_debug(GetDeviceVer, 13);
	air_uart_write(GetDeviceVer, sizeof(GetDeviceVer));
}

static uint8_t GetDeviceBigData[13]={0xFF, 0xFF, 0x0A, 00, 00, 00, 00, 00, 00, 0x01, 0x4D, 0xFE, 0x56};
//获取空调底板大数据
static void Get_BigData(void)
{
	OSI_LOGI(0, "[zk air] get big data");
	air_uart_write(GetDeviceBigData, sizeof(GetDeviceBigData));
}


static uint8_t GetDeviceStateData[13]={0xFF, 0xFF, 0x0A, 00, 00, 00, 00, 00, 00, 0x01, 0x4D, 0x01, 0x59};
//获取空调底板状态数据
static void Get_StateData(void)
{
	OSI_LOGI(0, "[zk air] get state data get_air_data_cnt=%d rest_num=%d", appSysTem.get_air_data_cnt, local.rest_num);
	air_uart_write(GetDeviceStateData, sizeof(GetDeviceStateData));
}

static void GetHaierBottomBasicInfo(void)
{
	uint8_t Cnt = 0;
	//获取设备版本信息
	while(!appSysTem.GetDeviceVer_OK_Flag)
	{
		Get_DeviceVer();
		Cnt++;
		if(Cnt > UART_RESEND_NUM)
		{
			Cnt = 0;
			appSysTem.GetDeviceVer_Fail_Flag = 1;
			OSI_LOGI(0, "[zk air] Get DeviceVer Fail");
			break;
		}
		vTaskDelay(osiMsToOSTick(500));
	}
}

static void Get_AirData_Handle(void)
{
	//如果定时器处于活动状态，先停掉
	if(xTimerIsTimerActive(Haier_Timers) == pdPASS)
    {
		xTimerStop(Haier_Timers, 0);
	}

	appSysTem.Inquire_BigData_FailCnt++;
	if(appSysTem.Inquire_BigData_FailCnt > CHECK_BIGDATA_MAX)
	{	
		appSysTem.Inquire_BigData_FailCnt = 0; 
		restart(2);	//重启模组
	}
	
	if(appSysTem.get_air_data_cnt < GET_BIG_DATA_NUM)
	{
		Get_StateData();
		appSysTem.get_air_data_cnt++;
	}
	else
	{
		//获取大数据之前先获取一下当前网络侧参数
		/*if(get_net_state() == NETWORK_LINKED)
		{
			get_module_net_info();
		}*/
		Get_BigData();
		appSysTem.get_air_data_cnt = 0;
	}
	xTimerReset(Haier_Timers, 0);
}

void air_task_main(void *pParameter)
{
	TASK_MSG *msg = NULL;
	
	air_uart_init();
	vTaskDelay(osiMsToOSTick(3000));
	//获取海尔空调底板基本信息（设备版本、设备识别码）
	GetHaierBottomBasicInfo();
	//开启主任务定时器，定时获取空调底板数据
	xTimerStart(Haier_Timers, 0);
	while(1)
	{
		if(xQueueReceive(haier_app_queue, &msg, portMAX_DELAY) == pdPASS)
		{
			switch(msg->id)
			{
				case GET_AIR_DATA_MSG:
					Get_AirData_Handle();
					break;
				case BT_CTR_AIR_MSG:
				case SERVER_CTR_AIR_MSG:
					OSI_LOGI(0, "[zk air] air_task_main_0 cmd=%d", msg->id);
					remote_control_air_handle((REMOTE_CTR_TYPE)(msg->id - 4), msg->param, msg->len);
					break;
				default:
					OSI_LOGE(0, "[zk air] air_task_main_1 not cmd %d", msg->id);
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
		//加这个350ms的延时是防止连续控制空调，导致串口数据冲突，所有向空调发控制数据都是此任务来执行
		vTaskDelay(osiMsToOSTick(350));
	}
}

/**********  空调底板数据接收处理   **********/

static uint16_t const crc16_table[256] = {
	0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
	0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
	0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
	0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
	0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
	0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
	0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
	0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
	0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
	0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
	0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
	0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
	0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
	0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
	0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
	0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
	0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
	0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
	0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
	0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
	0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
	0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
	0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
	0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
	0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
	0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
	0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
	0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
	0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
	0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
	0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
	0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
};

static uint16_t uplus_util_crc16_byte(uint16_t crc, const uint8_t data)
{
	return (crc >> 8) ^ crc16_table[(crc ^ data) & 0xff];
}
//CRC16校验算法
static uint16_t uplus_util_crc16(uint16_t crc, uint8_t const *buffer, uint32_t len)
{
	while (len--)
		crc = uplus_util_crc16_byte(crc, *buffer++);
	return crc;
}
//累加和校验算法
uint8_t DataAccumulateSumCRC(void *DataBuff, uint16_t DataLen)
{
	uint16_t crc=0;
	uint16_t i;
	uint8_t *Ptr = (uint8_t*)DataBuff;

	if((Ptr == NULL) || (DataLen==0))
		return crc;
    for(i=0; i < DataLen; i++)
    {
    	crc += *Ptr++;
    }
	return (uint8_t)(crc&0xff);  // 取低位字节
}

static uint16_t set_55H_data(uint8_t *data, uint8_t *Buff, uint16_t bufLen)
{
	uint16_t i;
	uint16_t datalen = 0;

	if((data==NULL) || (Buff==NULL) || (bufLen==0))
	{
		OSI_LOGE(0, "[zk air] set_55H_data:parameter Error");
		return 0;
	}
	//协议帧前3个字节直接作为有效数据保存到有效缓存区
	memcpy(data, Buff, 3);
	datalen += 3;
	for(i=3; i<bufLen; i++)
	{
		data[datalen] = Buff[i];
		if(Buff[i] == 0xff)
		{
			data[datalen+1] = 0x55;
		}
		datalen++;
	}
	//OSI_LOGI(0, "[zk air] set_55H_data:datalen=%d buflen=%d", datalen, bufLen);
	return datalen;
}

static uint16_t GetValidData(uint8_t *srcbuf, uint8_t *Validbuff, uint16_t srclen, uint16_t *Validlen)
{
	uint16_t FillingDataCount = 0;
	uint8_t DataCount = 0;
	uint8_t DataCountOK = 0;
	uint16_t i;

	if((srcbuf==NULL) || (Validbuff==NULL) || (srclen==0))
	{
		OSI_LOGE(0, "[zk air] GetValidData:parameter Error");
		return 0;
	}
	//协议帧前3个字节直接作为有效数据保存到有效缓存区
	for(i=0; i<3; i++)
		Validbuff[i] = srcbuf[i];
	*Validlen = 3;
	for(; i<srclen; i++)
	{
		//判断是否为填充字节55H
		if((srcbuf[i-1] == 0xFF) && (srcbuf[i] == 0x55))
		{
			//判断是否为参与校验和计算的填充55H
			if(DataCountOK == 0)
				FillingDataCount++;
		}
		else//不为填充数据则认为是有效数据直接保存到有效数据缓存区
		{
			//除去校验和以外的有效数据是否已经查找完
			if(DataCountOK == 0)
			{
				DataCount++;
				//已经查找完不包括校验和数据以外的所有有效数据，则后续的填充55H是不参与校验和计算的（srcbuf[2]数据长度是包括校验和在内的 所以要-1）
				if(DataCount > srcbuf[2]-1)
					DataCountOK = 1;
			}
			//所有的有效数据（包括校验和数据）放到有效数据缓存区
			Validbuff[*Validlen] = srcbuf[i];
			(*Validlen) += 1;
		}
	}
	//HaierLog("DataCountOK=%d, DataCount=%d, srclen=%d, Validlen=%d, FillCount=%d\r\n", DataCountOK, DataCount, srclen, *Validlen, FillingDataCount);

	//返回参与校验的55H的个数
	return FillingDataCount;
}

//大数据处理函数
static void air_BigData_handle(uint8_t *RecvBuff, uint16_t RecvLen, uint8_t *Databuff, uint16_t Datalen)
{
	if((RecvBuff == NULL) || (RecvLen == 0)||(Databuff == NULL) || (Datalen == 0))
		return;

	if(get_sye_state() == SYS_STATE_RUN)
	{
		struct ModuleData1 ModuleData = {0};
		
		//(Databuff,Datalen);
		//获取ICCID
		memcpy(ModuleData.Module_ICCID, appSysTem.Module_ICCID, ICCID_LEN);
		//获取IMSI
		memcpy(ModuleData.Module_IMSI, appSysTem.Module_IMSI, IMSI_LEN);
		//获取模组网络侧状态信息(get大数据之前已经获取过)
		memcpy(&ModuleData.net_state_info, &net_info, sizeof(struct NET_STATS1));

		//新获取一块缓存，用于存原始大数据和新添加的模组数据，然后重新算CRC
		uint16_t air_datalen = RecvLen+sizeof(struct ModuleData1);
		uint8_t *air_data = (uint8_t *)malloc(air_datalen);
		if(air_data == NULL)
		{
			OSI_LOGE(0, "[zk bt] air_BigData_handle_0 malloc fail %d", air_datalen);
			return;
		}
		memset(air_data, 0, air_datalen);

		//判断原始数据是否带CRC校验，如果带就需要去掉(SDK接收的数据需要去掉CRC)
		uint8_t offset = 0;
		if(Databuff[3] == 0)  
		{
			offset = 1;
		}
		else if(Databuff[3] == 0x40)
		{
			offset = 3;
			Databuff[3] = 0;
		}
		else
		{
			OSI_LOGE(0, "[zk bt] air_BigData_handle_1 CRC16 Flag Error 0x%x", Databuff[3]);
			free(air_data);
			return;
		}
		//将原始数据包去掉了累加校验和CRC16校验字节,然后进行了55的添加
		uint16_t datalen = set_55H_data(air_data, Databuff, Datalen-offset);
		//把模组数据放在原始数据包后面(注意:是指去掉了校验字节的原始数据)
		memcpy(air_data+datalen, &ModuleData, sizeof(struct ModuleData1));
		//添加了模组数据后，帧长度也得相应的增加
		air_data[2] += sizeof(struct ModuleData1);
		//重新计算帧长度
		air_datalen = datalen+sizeof(struct ModuleData1);
		//重新算累加校验和
		air_data[air_datalen] = DataAccumulateSumCRC(air_data+2, air_datalen-2);
		//整包数据长度++
		air_datalen += 1;

		/*if(appSysTem.remote_ctr_air_flag == SERVER_CONTROL_AIR)
			Haier_EventNotifyServer(PKT_BUF_DIR_RSP, EPP_BIG_DATA, air_data, air_datalen);
		else
			Haier_EventNotifyServer(PKT_BUF_DIR_RPT, EPP_BIG_DATA, air_data, air_datalen);*/

		free(air_data);
		
		OSI_LOGI(0, "[zk air] air_BigData_handle_0");
	}
	else
	{
		OSI_LOGI(0, "[zk air] air_BigData_handle_1:uplus sdk not initiated");
	}
}

static void HaierBottomAlarmNotify(uint8_t *StateData, uint16_t Datalen)
{
	if((StateData==NULL) || (Datalen == 0))
		return;

	//上报告警
	//Haier_EventNotifyServer(PKT_BUF_DIR_RPT, EPP_DATA, StateData, Datalen);
}

static void air_StateData_Handle(uint8_t *RecvBuff, uint16_t RecvLen, uint8_t *Databuff, uint16_t Datalen)
{
	if((Databuff == NULL) || (Datalen == 0))
		return;
	
	if(get_sye_state() == SYS_STATE_RUN)
	{
		//拷贝接收到的数据到大数据结构体
		memcpy(&Curr_StateData, Databuff+12, sizeof(struct StateData1));
		//判断原始数据是否带CRC校验，如果带就需要去掉(SDK接收的数据需要去掉CRC)
		uint8_t offset = 0;
		if(Databuff[3] == 0)  
		{
			offset = 1;
			//这个数据类型要从2改成6，优家sdk的限制，否则上报不成功(坑爹)
			Databuff[9] = 6;
		}
		else if(Databuff[3] == 0x40)
		{
			offset = 3;
			Databuff[3] = 0;
			//这个数据类型要从2改成6，优家sdk的限制，否则上报不成功(坑爹)
			Databuff[9] = 6;
		}
		else
		{
			OSI_LOGE(0, "[zk air] air_StateData_Handle_1 CRC16 Flag Error 0x%x", Databuff[3]);
			return;
		}
		//将原始数据包去掉了累加校验和CRC16校验字节,然后进行了55的添加
		uint16_t datalen = set_55H_data(RecvBuff, Databuff, Datalen-offset);
		//重新计算累加校验和
		RecvBuff[datalen] = DataAccumulateSumCRC(RecvBuff+2, datalen-2);
		datalen += 1;
		//当前没有告警 只是空调状态发生改变上报到服务器 (目前只有51种告警 1~51)
		if((Curr_StateData.ErrorCode == 0) || (Curr_StateData.ErrorCode > ALARM_MAX))
		{
			if(appSysTem.remote_ctr_air_flag == SERVER_CONTROL_AIR)
			{
				OSI_LOGI(0, "[zk air] server ctr air ack");
				//zk_debug(Databuff,Datalen);
				//服务器下发查询指令，不管是否有变化，都上报
				//Haier_EventNotifyServer(PKT_BUF_DIR_RSP, EPP_DATA, RecvBuff, datalen);
				//更新一次大数据全量 为下次做比较
				memcpy(&Old_StateData, &Curr_StateData, sizeof(struct StateData1));
			}
			else
			{
				//新大数据全量和前一次保存的大数据全量做比较  如果不相等则认为空调状态发生了改变 需要上报到海尔服务器 （相等则不需要做处理,只判断大数据中前10个字节）
				if(memcmp(&Old_StateData, &Curr_StateData, sizeof(struct StateData1)))
				{
					OSI_LOGI(0, "[zk air] Curr_StateData != Old_StateData");
					//zk_debug(Databuff,Datalen);
					//底板状态发生变化需要上报到海尔服务器
					//Haier_EventNotifyServer(PKT_BUF_DIR_RPT, EPP_DATA, RecvBuff, datalen);
					//更新一次大数据全量 为下次做比较
					memcpy(&Old_StateData, &Curr_StateData, sizeof(struct StateData1));
				}
				else
				{
					OSI_LOGI(0, "[zk air] State Not Change");
				}
			}
		}
		else //当前有告警 需要立即上报
		{
			OSI_LOGI(0, "[zk air] Alarm Notify");
			HaierBottomAlarmNotify(RecvBuff, datalen);
			//更新一次大数据全量 为下次做比较
			memcpy(&Old_StateData, &Curr_StateData, sizeof(struct StateData1));
		}
	}
	else
	{
		OSI_LOGI(0, "[zk air] air_StateData_Handle:uplus sdk not initiated...");
	}
}

//空调底板状态返回数据处理函数
static void StateReturnData_handle(uint8_t *RecvBuff, uint16_t RecvLen, uint8_t *Databuff, uint16_t Datalen)
{
	if((Databuff==NULL) || (Datalen==0))
		return;
	
	appSysTem.Inquire_BigData_FailCnt = 0;

	switch(appSysTem.remote_ctr_air_flag)
	{
		case BT_CONTROL_AIR:
			OSI_LOGI(0, "[zk air] bt ctr air ok ctrTyp=%d", appSysTem.remote_ctr_air_flag);
			//task_msg_send(bt_task_handle, BT_EVENT_NOTIFY_MSG, RecvBuff, RecvLen);
			appSysTem.remote_ctr_air_flag = NULL_CTR;
			break;
		case SERVER_CONTROL_AIR:
		case NULL_CTR:
		default:
			if((Databuff[10] == 0x6d)&&(Databuff[11] == 0x01))//当前数据包为空调状态数据
			{
				OSI_LOGI(0, "[zk air] get Air state data ok ctrTyp=%d", appSysTem.remote_ctr_air_flag);
				air_StateData_Handle(RecvBuff, RecvLen, Databuff, Datalen);
			}
			else if((Databuff[10] == 0x7d)&&(Databuff[11] == 0x01))//当前数据包为大数据
			{
				OSI_LOGI(0, "[zk air] get Air big data ok ctrTyp=%d", appSysTem.remote_ctr_air_flag);
				air_BigData_handle(RecvBuff, RecvLen, Databuff, Datalen);
			}
			appSysTem.remote_ctr_air_flag = NULL_CTR;
			break;
	}
}

static void InvalidFrame_handle(uint8_t *RecvBuff, uint16_t RecvLen, uint8_t *Databuff, uint16_t Datalen)
{
	if((RecvBuff == NULL) || (RecvLen == 0) || (Databuff ==NULL) || (Datalen==0))
		return;
	
	if((appSysTem.remote_ctr_air_flag != NULL_CTR) && (appSysTem.remote_ctr_air_flag <= SERVER_CONTROL_AIR))
	{
		if(BT_CONTROL_AIR == appSysTem.remote_ctr_air_flag)
		{
			OSI_LOGI(0, "[zk air] InvalidFrame_handle_0 : bt handle");
			//task_msg_send(bt_task_handle, BT_EVENT_NOTIFY_MSG, RecvBuff, RecvLen);
		}
		else if(SERVER_CONTROL_AIR == appSysTem.remote_ctr_air_flag)
		{
			//判断原始数据是否带CRC校验，如果带就需要去掉(SDK接收的数据需要去掉CRC)
			uint8_t offset = 0;
			if(Databuff[3] == 0)  
			{
				offset = 1;
				//这个数据类型要从2改成6，优家sdk的限制，否则上报不成功(坑爹)
				Databuff[9] = 6;
			}
			else if(Databuff[3] == 0x40)
			{
				offset = 3;
				Databuff[3] = 0;
				//这个数据类型要从2改成6，优家sdk的限制，否则上报不成功(坑爹)
				Databuff[9] = 6;
			}
			else
			{
				OSI_LOGE(0, "[zk bt] air_StateData_Handle_1 CRC16 Flag Error 0x%x", Databuff[3]);
				return;
			}
			//将原始数据包去掉了累加校验和CRC16校验字节,然后进行了55的添加
			uint16_t datalen = set_55H_data(RecvBuff, Databuff, Datalen-offset);
			//重新计算累加校验和
			RecvBuff[datalen] = DataAccumulateSumCRC(RecvBuff+2, datalen-2);
			datalen += 1;
			//Haier_EventNotifyServer(PKT_BUF_DIR_RSP, EPP_DATA, RecvBuff, datalen);
		}
		
		OSI_LOGI(0, "[zk air] remote control air fail ctrType=%d", appSysTem.remote_ctr_air_flag);
		
		//清组控制命令标志位(此标志位会在友远程控制空调事件时 置位，会在成功应答或者失败应答处清除，这里为失败应答，成功应答在空调底板数据解析处)
		appSysTem.remote_ctr_air_flag = NULL_CTR;
	}
	else
	{
		OSI_LOGI(0, "[zk air] Haier Uart Recvied Invalid frame");
	}
}

static void Haier_BottomProtocolResolution(uint8_t *RecvBuff, uint16_t RecvLen, uint8_t *DataBuff, uint16_t DataLen)
{
	if((DataBuff==NULL) || (DataLen==0))
		return;
	switch(DataBuff[9])
	{
		case 0x02: //状态返回数据
			StateReturnData_handle(RecvBuff, RecvLen, DataBuff, DataLen);
			break;
		case 0x03: //无效命令
			InvalidFrame_handle(RecvBuff, RecvLen, DataBuff, DataLen);
			break;
		case 0x62: //设备版本应答
			//正确收到设备版本信息应答，标志位置一，初始化任务可以开始执行离散加网流程
			appSysTem.GetDeviceVer_OK_Flag = 1;
			//拷贝设备信息数据到结构体
			memcpy(&DevicVersion, DataBuff+10, sizeof(struct DevicVersion1));

			dev.type = DEVICE_TYPE_AC;
			memcpy(dev.dev_name, DevicVersion.DeviceName, 8);
			memcpy(dev.dev_id, TypeId, 32);
			memcpy(dev.proto_ver, DevicVersion.DevicProtocolVersion+3, sizeof(DevicVersion.DevicProtocolVersion)-3);
			memcpy(dev.sw_ver, DevicVersion.SoftVersion, 8);
			memcpy(dev.hw_ver, DevicVersion.HardVersion, 8);

			memcpy(local.dev_name, DevicVersion.DeviceName, 8);
			memcpy(local.proto_ver, DevicVersion.DevicProtocolVersion+3, sizeof(DevicVersion.DevicProtocolVersion)-3);
			memcpy(local.sw_ver, DevicVersion.SoftVersion, 8);
			memcpy(local.hw_ver, DevicVersion.HardVersion, 8);
			write_local_cfg_Info();
		
			OSI_LOGI(0, "[zk air] Get Device Version OK");
			break;
		case 0x71: //设备识别码应答
			break;
		default:
			OSI_LOGE(0, "[zk air] Haier Uart Recvied Data Unknown type");
			break;
	}
}

static void Haier_UartRecevied(uint8_t *RecvBuff, uint16_t RecvLen)
{
	/*uint16_t CRC16 = 0;
	uint16_t CRC161 = 0;*/
	uint8_t SumCrc = 0;
	uint8_t SumCrc1 = 0;
	//有效数据缓存区
	uint8_t ValidDataBuff[100] = {0};
	//有效数据长度
	uint16_t ValidDtaLen = 0;
	//填充数据个数 （数据帧中可能会随机出现多个55H，属于填充数据，不计入数据长度但是会参与校验，需要先清除然后才能进行帧解析）
	uint16_t FillingDataCount = 0;

	if((RecvBuff ==NULL) || (RecvLen==0))
		return;
	if (RecvLen < 8)
	{
		OSI_LOGE(0, "[zk air] Uart Recvied data length error RecvLen=%d", RecvLen);
		return;
	}
	if((RecvBuff[0] != 0xFF) || (RecvBuff[1] != 0xFF))
	{
		OSI_LOGE(0, "[zk air] Uart Recvied data PackHead error RecvLen=%d", RecvLen);
		return;
	}
	
	//zk_debug(RecvBuff, RecvLen);

	//获取有效数据（清除数据帧中的55H，返回的FillingDataCount是只参与校验的55H个数，如果是校验和后面的55H则不包含）
	FillingDataCount = GetValidData(RecvBuff, ValidDataBuff, RecvLen, &ValidDtaLen);
	//比较校验和
	switch(ValidDataBuff[3])
	{
		case 0:		//数据帧不加密，不带CRC16 校验字段
			SumCrc = DataAccumulateSumCRC(RecvBuff+2, ValidDataBuff[2]+FillingDataCount);
			SumCrc1 = ValidDataBuff[ValidDtaLen - 1];
			if(SumCrc1 != SumCrc)
			{
				OSI_LOGE(0, "[zk air] SumCrc=0x%x SumCrc1=0x%x", SumCrc, SumCrc1);
				return;
			}
			break;
		case 0x40:	//数据帧不加密， 带CRC16校验字段
			/*CRC16 = uplus_util_crc16(CRC16, ValidDataBuff+2, ValidDataBuff[2]);
			CRC161 = ((uint16)ValidDataBuff[ValidDtaLen -2]) << 8;
			CRC161 |= ValidDataBuff[ValidDtaLen-1];
			if(CRC161 != CRC16)
			{
				uplus_sys_log("CRC16=0x%x CRC161=0x%x", CRC16, CRC161);
				return;
			}*/
			SumCrc = DataAccumulateSumCRC(RecvBuff+2, ValidDataBuff[2]+FillingDataCount);
			SumCrc1 = ValidDataBuff[ValidDtaLen - 3];
			if(SumCrc1 != SumCrc)
			{
				OSI_LOGE(0, "[zk air] 1 SumCrc=0x%x SumCrc1=0x%x", SumCrc, SumCrc1);
				return;
			}
			break;
		case 0x80:	//数据帧加密， 不带CRC16校验字段（目前不支持）
			break;
		default:
			OSI_LOGE(0, "[zk air] Uart Recvied data Addr Identifier Error");
			return;
	}
	//海尔空调底板通信协议解析
	Haier_BottomProtocolResolution(RecvBuff, RecvLen, ValidDataBuff, ValidDtaLen);
}

void air_recv_task_main(void *param)
{
	TASK_MSG *msg = NULL;
	while(1)
	{
		if(xQueueReceive(uart_recv_queue, &msg, portMAX_DELAY) == pdPASS)
		{
			//osiMessageQueueGet(uart_recv_queue, &msg);
			switch(msg->id)
			{
				case AIR_UART_RECV_MSG:
					OSI_LOGI(0, "air recv len=%d", msg->len);
					/*OSI_LOGXI(OSI_LOGPAR_S, 0, "air recv:%s", (char*)msg->param);
					char *sendbuf = calloc(1, msg->len+5);
					if(sendbuf != NULL)
					{
						sprintf(sendbuf, "%s:%s", "ACK", (char*)msg->param);
					}
					OSI_LOGI(0, "air send:%d", drvUartSend(air_drv, (void *)sendbuf, (size_t)msg->len+5));
					free(sendbuf);*/
					//zk_debug(msg->param, msg->len);
					Haier_UartRecevied(msg->param, msg->len);
					break;
				default:
					OSI_LOGE(0, "[zk air] air_recv_task_main_1 cmd error %d", msg->id);
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

