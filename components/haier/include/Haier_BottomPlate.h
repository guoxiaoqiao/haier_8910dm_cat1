#ifndef _HAIER_BOTTOMPLATE_H_
#define _HAIER_BOTTOMPLATE_H_

#include "haier_appmain.h"

#define AIR_UART_NAME	DRV_NAME_UART1
#define AIR_BAUD		(9600)

#define AIR_UART_RECVBUFF_MAX		200

#define AIR_UART_RX_BUF_SIZE (256)
#define AIR_UART_TX_BUF_SIZE (256)

#define UART_WAIT_TX_DONE_TIMEOUT (500)

//串口重发最大次数
#define UART_RESEND_NUM    		3
//连续x次获取大数据失败，模组整机复位
#define CHECK_BIGDATA_MAX			10

#define GET_BIG_DATA_NUM			360

#define ALARM_MAX					51

#define SERVER_INQURE_BIGDATA_TIMEOUT 35
#define SERVER_INQURE_BIGDATA_CNT_MAX 3

typedef struct
{
	uint16_t recvLen;
	uint8_t recvbuff[AIR_UART_RECVBUFF_MAX];
	
}air_uart_recv1;

//设备信息数据结构体
struct DevicVersion1{

			char 		DevicProtocolVersion[8]; //设备协议版本
			char 		SoftVersion[8];			 //软件版本
			uint8_t 	UartEncryptionMethod;	 //串口加密方式
			uint16_t 	EncryptionParamter;		 //加密参数
			char		HardVersion[8];			 //硬件版本
			uint8_t		CommTheWay;				 //通信方式
			char		DeviceName[8];			 //设备名称
			uint16_t 	DeviceFeaturesInfo;		 //设备功能信息

}__attribute__((packed));

struct StateData1{

			uint8_t 		SetTemperature; 		 //设定温度    				byte1
			uint8_t 		SetUpDown;				 //设定上下摆风				byte2
			uint8_t 		SetMdWd;				 //设定模式、设定风速		byte3
			uint8_t 		Reserved;				 //服务器下发累计电量存储周期	byte4
			uint16_t		WordA;			 		 //字A 不知道什么意思		byte5,6
			uint8_t		    SetHumidity;			 //设定湿度				byte7
			uint8_t		    SetPeradlr;			 //设定感人模式、左右摆风		byte8
			uint16_t 		WordB;		 			 //字B 也不知道什么意思		byte9,10
			uint8_t 		SetrentTime;		     //共享时长				byte11
			uint8_t 		NOP;					 //预留					byte12
			uint8_t 		RoomTemperature;		 //室内温度				byte13
			uint8_t 		RoomHumidity; 			 //室内湿度				byte14
			uint8_t		    OutdoorTemperature;	 //室外温度				byte15
			uint8_t		    AirPm;					 //机型、空气质量等级		byte16
			uint8_t		    ErrorCode;				 //错误码					byte17
			uint8_t 		CtrCmdSource;			 //控制命令来源			     byte18
			uint16_t 		CLEAN;					 //净化功能底板累计时间		byte19,20
			uint16_t 		RoomPM;					 //室内PM2.5实际值			byte21,22
			uint16_t 		OutdoorPM;				 //室外PM2.5实际值			byte23,24
			uint16_t		Formaldehyde;			 //甲醛实际值				byte25,26
			uint16_t		AirQuality;			 //空气质量实际值			byte27,28
			uint16_t		CO2;					 //二氧化碳实际值			byte29,30
			uint32_t 		Batery;					 //累计电量				byte31,34

}__attribute__((packed));


//大数据全量结构体
struct BigData1{

			uint8_t 		SetTemperature; 		 //设定温度    				byte1
			uint8_t 		SetUpDown;				 //设定上下摆风			byte2
			uint8_t 		SetMdWd;				 //设定模式、设定风速		byte3
			uint8_t 		Reserved;				 //预留					byte4
			uint16_t		WordA;			 		 //字A 不知道什么意思		byte5,6
			uint8_t		    SetHumidity;			 //设定湿度				byte7
			uint8_t		    SetPeradlr;			 	 //设定感人模式、左右摆风		byte8
			uint16_t 		WordB;		 			 //字B 也不知道什么意思		byte9,10
			uint8_t 		SetrentTime;		     //共享时长				byte11
			uint8_t 		NOP;					 //预留					byte12
			uint8_t 		RoomTemperature;		 //室内温度				byte13
			uint8_t 		RoomHumidity; 			 //室内湿度				byte14
			uint8_t		    OutdoorTemperature;		 //室外温度				byte15
			uint8_t		    AirPm;					 //机型、空气质量等级		byte16
			uint8_t		    ErrorCode;				 //错误码					byte17
			uint8_t 		CtrCmdSource;			 //控制命令来源			byte18
			uint16_t 		CLEAN;					 //净化功能底板累计时间		byte19,20
			uint16_t 		RoomPM;					 //室内PM2.5实际值			byte21,22
			uint16_t 		OutdoorPM;				 //室外PM2.5实际值			byte23,24
			uint16_t		Formaldehyde;			 //甲醛实际值				byte25,26
			uint16_t		AirQuality;				 //空气质量实际值			byte27,28
			uint16_t		CO2;					 //二氧化碳实际值			byte29,30
			uint32_t 		Batery;					 //累计电量				byte31,34
			uint16_t		Power;					 //功率					byte35,36
			uint8_t		    RoomCoilTemperature;	 //室内盘管温度			byte37
			uint8_t		    OutdoorExhaleTemperature;//室外吐气温度			byte38
			uint8_t		    OutdoorCoilTemperature;	 //室外盘管温度			byte39
			uint8_t  		OutdoorInhaleTemperature;//室外吸气温度			byte40
			uint8_t         OutdoorDefrostTemperature;//室外除霜温度			byte41
			uint8_t		    PressFrequency;			 //压机频率				byte42
			uint16_t 		PressCurrent;			 //压机电流				byte43,44
			uint16_t		WordD;					 //字D 同样不知道什么意思	byte45,46
			uint16_t        ElectronicSwellSwitch;	 //电子膨胀开关			byte47,48

}__attribute__((packed));

struct NET_STATS1{
	
	int16_t 	rsrp; 	        
	int16_t 	rssi;	        
	int16_t 	snr;	       
	char	    Cell_ID[10];	
	uint8_t 	Coverage_level;
	
}__attribute__((packed));

struct ModuleData1{

	char Module_ICCID [ICCID_LEN+1];	
	char Module_IMSI  [IMSI_LEN+1];
	
	struct NET_STATS1  net_state_info;
	
}__attribute__((packed));

extern struct NET_STATS1 net_info;

#endif