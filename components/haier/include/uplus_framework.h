
/*!
 * \file uplus_framework.h
 * \brief uplugSDK API接口
 *
 * \date 2017-01-11
 * \author fanming
 *
 */

#ifndef __UPLUS_FRAMEWORK_H__
#define __UPLUS_FRAMEWORK_H__

#ifdef USE_MODULE_BIN
#include "module/uplus_module_redefine.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif


/************************************************/
/*!
 * \defgroup Data info APIs
 * \brief uplugSDK和设备之间交互的数据描述。
 * \{
 */

/*! \def DATA_TYPE_XXX
 *
 * \brief 数据格式类型。
 * NONE 内部使用。
 * EPP E++帧。
 * RAW 透明处理帧。
 * EPP_RAW 开放E++帧。
 *
 */
enum data_type
{
	DATA_TYPE_NONE = 0, /*internal, should not be used*/
	DATA_TYPE_EPP = 0x1,
	DATA_TYPE_JSON = 0x2, /*not supported*/
	DATA_TYPE_RAW = 0x4,
	DATA_TYPE_EPP_RAW = 0x8,
	DATA_TYPE_EPP_WITH_PARA = 0x10
};

#if 0
/*for DATA_TYPE_JSON*/
enum
{
	OP_METHOD_NULL, /*must be 0*/

	/*up & down*/
	OP_METHOD_DATA,
	OP_METHOD_SERVICE,

	/*down*/
	OP_METHOD_READ,
	OP_METHOD_WRITE,
	OP_METHOD_OP,

	/*up*/
	OP_METHOD_RSP_READ,
	OP_METHOD_RSP_WRITE,
	OP_METHOD_RSP_OP,
	OP_METHOD_RPT_STATUS,
	OP_METHOD_RPT_ALARM
};
#endif

/*! \def EPP_RAW_DATA_XXX
 *
 * \brief 开放E++帧的子类型。
 * NULL 双向，无数据。
 * CTRL 网络-->设备，读写控制帧。
 * GROUP 网络-->设备，组命令帧。
 * ALARM_GET 网络-->设备，报警查询。
 * ALARM_STOP 网络-->设备，停止报警。
 * STATUS 设备-->网络，状态帧。
 * ALARM 设备-->网络，报警帧。
 * BIGDATA 设备-->网络，大数据帧。
 * ACK_OK 设备-->网络，应答帧。
 * ACK_NG 设备-->网络，错误应答帧。
 *
 */
/*for DATA_TYPE_EPP_RAW*/
enum
{
	EPP_RAW_DATA_NULL = 0, /*must be 0*/

	EPP_RAW_DATA_CTRL, /*read/write*/
	EPP_RAW_DATA_GROUP, /*op*/
	EPP_RAW_DATA_ALARM_GET,
	EPP_RAW_DATA_ALARM_STOP,

	EPP_RAW_DATA_STATUS,
	EPP_RAW_DATA_ALARM,
	EPP_RAW_DATA_BIGDATA,
	EPP_RAW_DATA_ACK_OK,
	EPP_RAW_DATA_ACK_NG
};

/*! \def EPP_DATA_XXX
 *
 * \brief E++帧的子类型。
 * NULL 双向，无数据。
 * DATA 双向，普通E++帧。
 * BIG_DATA 设备-->网络，大数据帧。
 * INFRARED_DATA 双向，红外E++帧。
 *
 */
/*for DATA_TYPE_EPP*/
enum
{
	EPP_DATA_NULL = 0, /*must be 0*/

	EPP_DATA,
	EPP_BIG_DATA,
	EPP_INFRARED_DATA
};

#define DATA_TLV_TYPE_REQ 1
#define DATA_TLV_TYPE_RSP_TIME_SPENT 2

#define DATA_PARA_TLV_HEADER_SIZE 2
typedef struct
{
	uplus_u8 type;
	uplus_u8 length; /*length of value*/
	uplus_u8 value[2];
} data_para_tlv_t;

typedef struct
{
	uplus_u16 length;
	uplus_u16 tlv_num;
	/*data_para_tlv_t*/
} data_para_t;

/*!
 * \typedef data_free
 * \brief 释放数据资源接口。
 * \param data 数据资源。
 * \return N/A。
 */
struct data_info;
typedef void (* data_free)(struct data_info * data);

/*!
 * \typedef data_info_t
 * \brief 数据资源结构体。设备和uplugSDK交互的数据载体。
 * data_type 数据格式类型。
 * data_sub_type 数据格式子类型。
 * free 数据资源释放接口。
 * data.raw_data.data 数据存储指针。
 * data.raw_data.data 数据长度。
 *
 */
/*data info*/
typedef struct data_info
{
	uplus_u16 data_type; /*DATA_TYPE_XXX*/
	uplus_u16 data_sub_type; /*see diff def*/
	data_free free; /*free data*/

	union
	{
#if 0
		/*following defs for DATA_TYPE_JSON*/
		/*OP_METHOD_READ*/
		struct
		{
			uplus_s8 * name;
		} read;
		/*OP_METHOD_WRITE*/
		struct
		{
			uplus_s8 * name;
			uplus_s8 * value;
		} write;
		/*OP_METHOD_OP*/
		struct
		{
			uplus_s8 * op;
			void * op_param; /*cae_strpair_list_t**/
		} op;
		/*OP_METHOD_RSP_READ*/
		struct
		{
			uplus_s32 result;
			uplus_s8 * value;
		} read_rsp;
		/*OP_METHOD_RSP_WRITE*/
		struct
		{
			uplus_s32 result;
		} write_rsp;
		/*OP_METHOD_RSP_OP*/
		struct
		{
			uplus_s32 result;
			void * info; /*cae_strpair_list_t**/
		} op_rsp;
		/*OP_METHOD_RPT_STATUS OP_METHOD_RPT_ALARM*/
		struct
		{
			void * info; /*cae_strpair_list_t**/
		} rpt; /*status alarm*/
		/*OP_METHOD_DATA OP_METHOD_SERVICE*/
		struct
		{
			uplus_s8 * data_type;
			uplus_u8 * data;
			uplus_u32 data_len;
		} extra_data;
#endif
		/*following defs for
			DATA_TYPE_EPP
			DATA_TYPE_EPP_RAW
			DATA_TYPE_RAW
			DATA_TYPE_NONE*/
		struct
		{
			uplus_u8 * data;
			uplus_u32 data_len;
		} raw_data;
	} data;
} data_info_t;
/*!
 * \}
 */
/************************************************/

/************************************************/
/*!
 * \defgroup Packet buf APIs
 * \brief 任务间通信通用数据包缓存。
 * \{
 */

/*! \def DATA_FROM_XXX
 *
 * \brief 数据来源。
 * UNKNOWN 未知来源。
 * LOCAL 本地连接。
 * CLOUD 远程连接。
 * SOFTAP SOFTAP连接。
 * BLE 蓝牙连接。
 *
 */
#define DATA_FROM_UNKNOWN 0
#define DATA_FROM_LOCAL 1
#define DATA_FROM_CLOUD 2
#define DATA_FROM_SOFTAP 3
#define DATA_FROM_BLE 4

/*!
 * \typedef session_desc_t
 * \brief 数据会话标识结构体。发给设备的每条数据都会关联一个会话，以便数据应答可以关联到会话上。
 * protocol 应用协议，对于设备是透明的。
 * from_where 数据来源。
 * session 会话标识，同一协议，且同一来源下是唯一的。
 *
 */
typedef struct session_desc
{
	uplus_u8 protocol;
	uplus_u8 from_where; /*DATA_FROM_XXX*/
	uplus_u8 session[6];
} session_desc_t;

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

struct pkt_buf;
struct pkt_buf_list;
/*!
 * \typedef pkt_buf_free
 * \brief 释放数据包buf接口。
 * \param pkt_buf packet buf。
 * \return N/A。
 */
typedef void (* pkt_buf_free)(struct pkt_buf *pkt_buf);

/*!
 * \typedef pkt_buf_t
 * \brief 数据包缓存结构体。
 * next 指向下一个数据包缓存，用于支持数据包缓存列表。
 * pkt_buf_list 数据包缓存资源池。
 * free 数据包缓存释放接口。
 * info_res 数据包缓存关联的信息。
 * info_res.dir 数据方向类型。
 * info_res.device_protocol 设备定位信息。
 * info_res.instance 设备定位信息。
 * info_res.session_desc 关联的会话。主动上报不需要填写。请求需要应答时，需要复制出此字段。
 * info_res.sn 序列号。主动上报不需要填写。请求需要应答时，需要复制出此字段。
 * data_info 数据资源。
 *
 */
typedef struct pkt_buf
{
	struct pkt_buf * next;
	struct pkt_buf_list *pkt_buf_list;
	pkt_buf_free free; /*free pkt_buf*/

	/*extra info. used for dev access.
		data(from uplugSDK to dev, no need response): none
		request data(from uplugSDK to dev, need response): transparent request handle
		response data(from dev to uplugSDK): transparent request handle
		report data(from dev to uplugSDK): none
	*/
	struct
	{
		uplus_u8 dir; /*PKT_BUF_DIR_XXX, must be filled*/
		uplus_u8 device_protocol;
		uplus_u8 instance;
		uplus_u8 reserved;
		session_desc_t session_desc; /*for PKT_BUF_DIR_RSP, copy from REQ*/
		uplus_u32 sn; /*for PKT_BUF_DIR_RSP, copy from REQ*/
		uplus_u8 free_flag;
		uplus_u8 reserved1[3];
	} info_res;

	data_info_t data_info;
} pkt_buf_t;

/*!
 * \typedef pkt_buf_list_t
 * \brief 数据包缓存资源池结构体。
 * mutex_id 资源池互斥保护。
 * free 可用数据包缓存列表。当资源池中有可用数据包缓存时，直接从资源池中申请，否则自动申请系统内存。
 * free_size 可用数据包缓存数量。
 * max_size 最大可用数据包缓存数量。当可用数据包缓存数量超过最大可用数据包缓存数量时，自动释放可用数据包缓存列表。
 *
 */
typedef struct pkt_buf_list
{
	uplus_mutex_id mutex_id;
	pkt_buf_t * free;
	uplus_u16 free_size;
	uplus_u16 max_size;
} pkt_buf_list_t;

/*!
 * \brief 从数据包缓存资源池中申请数据包缓存。
 * \param pkt_buf_list 数据包缓存资源池。
 * \return 成功返回数据包缓存指针，失败返回NULL。
 */
extern pkt_buf_t * uplus_pkt_buf_malloc(pkt_buf_list_t *pkt_buf_list);

/*!
 * \brief 释放数据包缓存。
 * \param pkt_buf 数据包缓存
 * \return N/A。
 */
extern void uplus_pkt_buf_free(pkt_buf_t *pkt_buf);

/*!
 * \brief 释放数据包缓存资源池。
 * \param pkt_buf_list 数据包缓存资源池。
 * \return N/A。
 */
extern void uplus_pkt_buf_list_free(pkt_buf_list_t *pkt_buf_list);

/*!
 * \brief 初始化数据包缓存资源池。
 * \param pkt_buf_list 数据包缓存资源池。
 * \param max_size 最大可用数据包缓存数量。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_pkt_buf_list_init(pkt_buf_list_t *pkt_buf_list, uplus_u16 max_size);

/*!
 * \brief 获取uplugSDK系统数据包缓存资源池。
 * \return uplugSDK系统数据包缓存资源池。
 */
extern pkt_buf_list_t * uplus_get_pkt_buf_list(void);

/*!
 * \}
 */
/************************************************/

/************************************************/
/*!
 * \defgroup Device APIs
 * \{
 */

/*! \def DEVICE_PROTO_FAMILY_XXX
 *
 * \brief 设备协议类型。
 * RESERVED 保留。
 * ALL 所有。
 * EPP E++协议。
 */
/*device protocol*/
#define DEVICE_PROTO_FAMILY_RESERVED (0)
#define DEVICE_PROTO_FAMILY_ALL (0xFF)
#define DEVICE_PROTO_FAMILY_EPP ('E')

/*dev type*/
#define DEVICE_TYPE_UNKNOWN 0x00 /*unknown*/
#define DEVICE_TYPE_RFR 0x01 /*refrigerator*/
#define DEVICE_TYPE_SAC 0x02 /*split air conditioner*/
#define DEVICE_TYPE_AC 0x03 /*vertical air conditioner*/
#define DEVICE_TYPE_PWM 0x04 /*pulsator washing machine*/
#define DEVICE_TYPE_WM 0x05 /*cylinder washing machine*/
#define DEVICE_TYPE_EWH 0x06 /*electric water heater*/
#define DEVICE_TYPE_MWO 0x07 /*microwave oven*/
#define DEVICE_TYPE_CELL 0x08 /*wine refrigerator*/
#define DEVICE_TYPE_RH 0x09 /*range hood*/
#define DEVICE_TYPE_DW 0x0A /*dish-washing machine*/
#define DEVICE_TYPE_DTC 0x0B /*disinfection cabinet*/
#define DEVICE_TYPE_BAC 0x0D /*commercial air conditioner*/
#define DEVICE_TYPE_TV	0x0E /*deprecated*/
#define DEVICE_TYPE_ETM 0x0F /*entertainment appliances*/
#define DEVICE_TYPE_LIGHT 0x10 /*lighting equipment*/
#define DEVICE_TYPE_SECU 0x11 /*intelligent security*/
#define DEVICE_TYPE_VM 0x12 /*video monitoring*/
#define DEVICE_TYPE_SEN	0x13 /*deprecated*/
#define DEVICE_TYPE_SH 0x14 /*smart home devices*/
#define DEVICE_TYPE_HC 0x15 /*health care*/
#define DEVICE_TYPE_IC 0x16 /*freezer*/
#define DEVICE_TYPE_MED 0x17 /*medical freezer*/
#define DEVICE_TYPE_GWH 0x18 /*gas water heater*/
#define DEVICE_TYPE_BWH 0x19 /*heating boiler*/
#define DEVICE_TYPE_SO 0x1A /*steam box*/
#define DEVICE_TYPE_CM	0x1B /*deprecated*/
#define DEVICE_TYPE_WD	0x1C /*deprecated*/
#define DEVICE_TYPE_CU 0x1D /*stove*/
#define DEVICE_TYPE_OV 0x1E /*oven*/
#define DEVICE_TYPE_SWH 0x1F /*solar water heater*/
#define DEVICE_TYPE_HWH 0x20 /*heat pump*/
#define DEVICE_TYPE_ACD 0x21 /*air conditioning device*/
#define DEVICE_TYPE_WTD 0x22 /*water treatment  device*/
#define DEVICE_TYPE_KT	0x23 /*deprecated*/
#define DEVICE_TYPE_FAS 0x24 /*fresh air system*/
#define DEVICE_TYPE_UHS 0x25 /*underfloor heating system*/
#define DEVICE_TYPE_PUB 0x26 /*public service*/
#define DEVICE_TYPE_VCL 0x27 /*vacuum cleaner*/
#define DEVICE_TYPE_SHA 0x28 /*small household appliances*/
#define DEVICE_TYPE_HEM 0x29 /*household environment monitors*/
#define DEVICE_TYPE_GTW 0x30 /*gateway*/
#define DEVICE_TYPE_DRY 0x31 /*dryer*/
#define DEVICE_TYPE_WEAR 0x32 /*wearable devices*/
#define DEVICE_TYPE_ACB 0x33 /*air cube*/
#define DEVICE_TYPE_EHD 0x34 /*electric heating device*/
#define DEVICE_TYPE_OTHER 0x35 /*others*/
#define DEVICE_TYPE_PC 0x36 /*personal care*/
#define DEVICE_TYPE_FAN 0x37 /*electric fan*/
#define DEVICE_TYPE_FIT 0x38 /*fitness equipment*/
#define DEVICE_TYPE_WAC 0x39 /*window air conditioner*/
#define DEVICE_TYPE_RBT 0xA1 /*domestic robot*/
#define DEVICE_TYPE_RU 0xE1 /*route module*/
#define DEVICE_TYPE_RD 0xE2 /*router*/
#define DEVICE_TYPE_CT 0xF1 /*intelligence terminal*/

/*!
 * \typedef dev_info_t
 * \brief 设备描述信息。
 * \details 包括以下信息
 * type 设备类型，如果未知，使用DEVICE_TYPE_UNKNOWN。
 * dev_name 当使用SoftAP时，SSID中显示中的设备类型。如果不填写，则根据设备类型显示。
 * dev_id 海极网注册设备返回的TypeID。
 * proto_ver 海极网注册设备返回的协议版本，例如2.16。
 * sw_ver 底板软件版本。
 * hw_ver 底板硬件版本。
 *
 */
typedef struct dev_info
{
	uplus_u32 type; /*device type DEVICE_TYPE_XXX*/
	uplus_u32 dev_minor;
	uplus_s8 dev_name[8];
	uplus_s8 dev_id[32]; /*device id*/
	uplus_s8 proto_ver[16]; /*protocol ver*/
	uplus_s8 sw_ver[16]; /*device sw version*/
	uplus_s8 hw_ver[16]; /*device hw version*/
} dev_info_t;

/*!
 * \brief 数据格式转换。
 * \brief uplugSDK当前只支持DATA_TYPE_EPP和DATA_TYPE_EPP_RAW两种格式，
 * 如果设备数据不是这两种格式需要提供格式转换接口。
 * \param dev_handle 设备注册时返回的句柄。
 * \param param 设备注册时的自定义数据。
 * \param pkt_buf 输入表示待转换的数据，输出表示转换后的数据。无论转换成功，原始数据占用的资源均被释放。
 * \param dst_data_type 需要转换成的数据格式。
 * \return
 */
typedef uplus_s32 (* dev_data_convert)(
	void *dev_handle,
	void *param,
	pkt_buf_t *pkt_buf,
	uplus_u8 dst_data_type);
/*!
 * \brief 向设备发送数据。
 * \param dev_handle 设备注册时返回的句柄。
 * \param param 设备注册时的自定义数据。
 * \param pkt_buf 待发送的数据。无论发送是否成功，pkt_buf占用的资源都被释放。
 * \return 功返回0，失败返回-1。
 */
typedef uplus_s32 (* dev_data_tx)(
	void *dev_handle,
	void *param,
	pkt_buf_t *pkt_buf);

enum device_protocol_ctrl_cmd
{
	/*0 based*/
	DEVICE_CTRL_CONFIG_STATUS = 0, /*val : uplus_u8 **/
	MAX_DEVICE_CTRL_NUM
};
/*!
 * \brief 向设备通知状态。
 * \param dev_handle 设备注册时返回的句柄。
 * \param param 设备注册时的自定义数据。
 * \param ctrl_type 控制命令/状态。
 * \param val 控制命令/状态的参数。
 * \return 成功返回0，失败返回-1。
 */
typedef uplus_s32 (*dev_ctrl)(
	void *dev_handle,
	void *param,
	uplus_s32 ctrl_type,
	void *val);

/*!
 * \typedef device_control_info_t
 * \brief 设备注册时的控制信息。
 * \details 包括以下信息
 * data_type 设备数据格式。
 * param 自定义数据。
 * convert 数据格式转换接口。
 * tx 数据发送接口，不能为NULL。
 * ctrl 控制状态接口，不能为NULL。
 *
 */
/*device control info*/
typedef struct device_control_info
{
	dev_info_t dev_info;
	uplus_u8 data_type; /*DATA_TYPE_XXX*/

	/*func*/
	void *param; /*can be passed to func*/
	dev_data_convert convert;
	dev_data_tx tx; /*tx to service, pkt_buf.data is consumed*/
	dev_ctrl ctrl;
} device_control_info_t;

/*!
 * \brief 设备向uplugSDK发送数据。
 * \param dev_handle 设备注册时返回的句柄。
 * \param pkt_buf 待发送的数据。无论是否成功，pkt_buf占用的资源都被释放。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_dev_tx(void *dev_handle, pkt_buf_t *pkt_buf);

/*!
 * \brief 设置默认设备。
 * \param default_dev_info 设备注册时的控制信息。
 * \return 成功返回默认设备的句柄，失败返回NULL。
 */
extern void * uplus_dev_set_default(dev_info_t *default_dev_info);

/*!
 * \brief 注册设备
 * \param device_protocol 设备协议标识，自定义字符，不能为DEVICE_PROTO_FAMILY_RESERVED或者DEVICE_PROTO_FAMILY_ALL。
 * \param instance 设备实例，同一设备协议标识下唯一。
 * \param dev_ctl_info 设备注册时的控制信息。
 * \return 成功返回设备句柄，失败返回NULL。
 */
extern void * uplus_dev_register(uplus_u8 device_protocol, uplus_u8 instance, device_control_info_t *dev_ctl_info);

/*!
 * \brief 设备状态通知
 * \note 当设备发生故障时，可以通过此接口通知uplugSDK，uplugSDK将关闭本地发现和本地连接，并将故障通知给云平台。
 * 当设备故障恢复后，可以通过此接口通知uplugSDK，uplugSDK将再次开启本地发现和本地连接。
 * \param dev_handle 设备注册时返回的句柄。
 * \param status 1-正常 0-故障。
 * \return N/A。
 */
extern void uplus_dev_status(void *dev_handle, uplus_u8 status);

/*!
 * \}
 */
/************************************************/


/************************************************/
/*!
 * \defgroup WIFI connnection APIs
 * \details WIFI连接支持四种模式
 * Station模式，STA连接到AP，并且通过DHCP获取到IP地址，这是正常工作的模式。
 * AP模式，用于SoftAP配置。
 * Mon模式，用于Smartlink配置。
 * Net模式，用于uplugSDK不管理WIFI连接的情况。
 * \{
 */

#define CONN_TIME_FOREVER (-1)
/*!
 * \typedef wifi_conn_config_para_t
 * \brief WIFI连接配置参数结构体。
 * \details WIFI连接配置参数包括以下信息
 * vendor_id 厂商标识，默认为haier。
 * fast_wifi_conn_interval WIFI快速连接的间隔。
 * fast_wifi_conn_time 快速连接的时长。
 * slow_wifi_conn_interval WIFI慢速连接的间隔。
 * slow_wifi_conn_time 慢速连接的时长。
 * ap_channel 信道，用于AP模式。
 * support_mon_softap，是否支持promisc+softap。
 *
 */
typedef struct wifi_conn_config_para
{
	uplus_u8 vendor_id[16]; /*vendor id, "haier"*/

	/*for sta*/
	uplus_s32 fast_wifi_conn_interval;
	uplus_s32 fast_wifi_conn_time;
	uplus_s32 slow_wifi_conn_interval;
	uplus_s32 slow_wifi_conn_time;

	/*for ap*/
	uplus_u16 ap_channel;

	/*for mon*/
	uplus_u8 support_mon_softap; /*support promisc+softap*/
} wifi_conn_config_para_t;

/*!
 * \brief WIFI连接Station模式初始化。
 * \param config_para WIFI连接配置参数
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_wifi_conn_sta_init(wifi_conn_config_para_t *config_para);

/*!
 * \brief WIFI连接AP模式初始化。
 * \param config_para WIFI连接配置参数
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_wifi_conn_ap_init(wifi_conn_config_para_t *config_para);

/*!
 * \brief 结束WIFI连接AP模式。
 * \return N/A。
 */
extern void uplus_wifi_conn_ap_fini(void);

/*!
 * \brief WIFI连接Mon模式初始化。
 * \param config_para WIFI连接配置参数
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_wifi_conn_mon_init(wifi_conn_config_para_t *config_para);

/*!
 * \brief 结束WIFI连接Mon模式。
 * \return N/A。
 */
extern void uplus_wifi_conn_mon_fini(void);

/*!
 * \brief WIFI连接Net模式初始化。
 * \param config_para WIFI连接配置参数
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_wifi_conn_net_init(wifi_conn_config_para_t *config_para);

/*!
 * \}
 */
/************************************************/


/************************************************/
/*!
 * \defgroup Init config APIs
 * \{
 */

typedef void (* conf_data_convert_func)(uplus_u8 *);
typedef void (* conf_extend_clr_func)(uplus_u8 *);

/*init config para*/
typedef struct init_config_para
{
	uplus_u8 hw_ver[16]; /*hardware version*/
	uplus_u8 sw_ver[16]; /*software version*/
	uplus_u8 platform[32]; /*platform*/

	uplus_u8 sdk_ver[16]; /*SDK version*/
	uplus_u8 sdk_platform[32]; /*SDK platform*/

	uplus_u16 max_client_num; /*max client can be accepted, > 1*/
	uplus_u8 conf_extend_block_num; /*extend config block, 0~3*/
	uplus_u8 max_wifi_scan_ap_num; /*max scan ap number, > 0*/

	uplus_u32 wifi_config_smartlink_time; /*config time, 300-3600s*/
	uplus_u32 wifi_config_softap_time; /*config time, 300-3600s*/
	uplus_u32 wifi_config_smartap_time; /*config time, 300-3600s*/
	uplus_u32 wifi_config_third_time; /*config time, 300-3600s*/
	uplus_u32 wifi_config_smartlink_to_softap_time; /*smart to softap config time, 30-600s*/
	uplus_u32 led_to_power_save_time; /*led power save time, 300-3600s*/
	uplus_u32 product_mode_wifi_conn_time; /*wifi conn time for product mode, 300-7200s*/
	uplus_u32 watchdog_time; /*watchdog time*/

	uplus_u8 support_product_mode; /*support product mode*/
	uplus_u8 enter_product_mode_power_on_without_wifi_conf;
	uplus_u8 smartlink_method_num; /*smartlink method number*/
	uplus_u8 softap_method_num; /*softap method number*/

	conf_data_convert_func old_to_new;
	conf_data_convert_func new_to_old;

	conf_extend_clr_func conf_extend_clr[3];

	wifi_conn_config_para_t wifi_conn_config;
} init_config_para_t;

/*!
 * \}
 */
/************************************************/


/************************************************/
/*!
 * \defgroup User data config APIs
 * \{
 */

/*!
 * \brief 扩展数据区存储数据。
 * \note 存储数据是异步操作，接口返回0不意味着数据已经存储，通常会有不超过1秒的延迟。
 * \param ext_block_index 扩展数据区索引，0-2。
 * \param offset 偏移地址。
 * \param conf 待存储的数据。
 * \param conf_len 待存储的数据长度。
 * \return 成功且数据没有变化返回0，成功且数据发生变化返回1，失败返回-1。
 */
extern uplus_s32 uplus_conf_extend_data_set(uplus_u8 ext_block_index, uplus_u16 offset, uplus_u8 *conf, uplus_u16 conf_len);

/*!
 * \brief 从扩展数据区读取数据。
 * \param ext_block_index 扩展数据区索引，0-2。
 * \param offset 偏移地址。
 * \param conf 读取数据的存储指针。
 * \param conf_len 待读取的数据长度。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_conf_extend_data_get(uplus_u8 ext_block_index, uplus_u16 offset, uplus_u8 *conf, uplus_u16 conf_len);

/*!
 * \}
 */
/************************************************/


/************************************************/
/*for wifi config*/
/*!
 * \defgroup WIFI config APIs
 * \{
 */

/*!
 * \typedef config_mode_t
 * \brief WIFI配置模式。
 * \details
 * CONFIG_MODE_NONE 非WIFI配置模式。
 * CONFIG_MODE_SMARTLINK Smartlink配置模式。
 * CONFIG_MODE_SOFTAP Softap配置模式。
 * CONFIG_MODE_BLE BLE配置模式。
 * CONFIG_MODE_SMARTAP SmartAP配置模式（先smartlink再softap；smartlink与softap共存）。
 * CONFIG_MODE_PICASSO 毕加索配置模式
 * CONFIG_MODE_OTHERS 其他配置模式。
 *
 */
typedef enum config_mode
{
	CONFIG_MODE_NONE = 0, /*only used for none config*/
	CONFIG_MODE_SMARTLINK = 1, /*bit 0*/
	CONFIG_MODE_SOFTAP = 2, /*bit 1*/
	CONFIG_MODE_BLE = 4, /*bit 2*/
	CONFIG_MODE_SMARTAP = CONFIG_MODE_SMARTLINK + CONFIG_MODE_SOFTAP,
	CONFIG_MODE_PICASSO = CONFIG_MODE_SOFTAP + CONFIG_MODE_BLE,
	CONFIG_MODE_SMARTAP40 = CONFIG_MODE_SMARTAP + CONFIG_MODE_BLE,
	CONFIG_MODE_OTHERS
} config_mode_t;

/*! \def SNIFFER_RCV_XXX
 *
 * \brief Smartlink配置模式下，需要对接收到的每一个数据帧进行处理。
 * 处理结果包括：
 * NG 非配置帧或者信道未锁定。
 * LOST 信道锁定后又丢失。
 * CHANNEL_LOCKED 信道锁定。
 * OK 成功收到完整配置信息。
 */
#define SNIFFER_RCV_NG -1
#define SNIFFER_RCV_CHANNEL_LOST -2
#define SNIFFER_RCV_CHANNEL_LOCKED 0
#define SNIFFER_RCV_OK 1

/*! \def SOFTAP_RCV_XXX
 *
 * \brief Softap配置模式下，需要对接收到的每一个命令进行处理。
 * 处理结果包括：
 * NG 不是配置信息。
 * OK 成功收到完整配置信息。
 * DELAY 收到的命令需要延迟处理。
 */

#define SOFTAP_RCV_NG -1
#define SOFTAP_RCV_OK 1
#define SOFTAP_RCV_DELAY 2

/*! \def EXT_DATA_TYPE_XXX
 *
 * \brief Smartlink/SoftAP配置模式下，配置完成后，有可能获取到一些额外的信息。
 * \note 额外信息是可选的。
 * CHANNEL AP信道。
 * ENCRYPT 加密方式。
 * SEED 随机种子。
 * BSSID BSSID。
 * TRACEID TRACEID。
 */
/*result_analysis*/
#define EXT_DATA_TYPE_CHANNEL 0 /*uplus_u16*/
#define EXT_DATA_TYPE_ENCRYPT 1 /*uplus_u32, WIFI_ENCRYPT_XXX*/
#define EXT_DATA_TYPE_SEED 2 /*uplus_u32*/
#define EXT_DATA_TYPE_BSSID 3 /*uplus_u8 [6]*/
#define EXT_DATA_TYPE_CONFIG_TYPE 4 /*uplus_u8, 0-multicast, 1-broadcast, 2-softap tcp, 3-softap udp, 4-BLE*/
#define EXT_DATA_TYPE_TRACEID 5 /*uplus_u8 [32]*/
#define EXT_DATA_TYPE_TOKEN 6 /*uplus_u8 [32]*/
#define EXT_DATA_TYPE_DOMAIN 7 /*uplus_u8 [32]*/
#define EXT_DATA_TYPE_COUNTRY 8 /*uplus_u8 [4]*/
/*!
 * \typedef wifi_config_ack_t
 * \brief 配置应答信息结构体。Smartlink配置完成后，需要发送ACK信息。
 * \details
 * mac 设备MAC地址。
 * ip_addr 设备IP地址。
 * ip_broadcast_addr 设备广播地址。
 * dev_info_t 设备信息。
 * ext_data 额外数据。
 * ssid SSID，最长32字节。
 *
 */
typedef struct
{
	uplus_s8 *mac;
	uplus_s8 *ip_addr;
	uplus_s8 *ip_broadcast_addr;
	dev_info_t *dev_info;
	uplus_u8 * ext_data;
	uplus_u8 *ssid;
} wifi_config_ack_t;

typedef uplus_s32 (*wifi_config)(void);
typedef uplus_s32 (*wifi_config_result_get)(uplus_u8 *ssid, uplus_u8 *passwd, uplus_u8 **ext_data);
typedef uplus_s32 (*wifi_config_result_analysis)(uplus_u8 * ext_data, uplus_u16 ext_data_type, void *out_val);
typedef uplus_s32 (*wifi_config_rcv)(uplus_u8 * frame, uplus_u32 length, promisc_frame_info_t * info);
typedef uplus_s32 (*wifi_sniffer_locked_channel_get)(uplus_u16 *cur_channel, uplus_u8 *ext_channel);
typedef uplus_s32 (*wifi_config_ack_build)(wifi_config_ack_t *info, uplus_u8 **ack, uplus_u16 *ack_len, struct uplus_sockaddr *addr, uplus_u32 *addrlen);
typedef uplus_s32 (*wifi_config_ack_interval)(uplus_u32 cur_times); /*0 based, interval of next smartlink ack, unit : ms, -1 is the end*/

/*! \def CONFIG_METHOD_XXX
 *
 * \brief WIFI配置方法。
 * ALL 所有或者未指定。
 * UPLUS 海尔U+方法。
 * AIRKISS 微信方法。
 * JOYLINK 京东方法。
 * ALINK 阿里方法。
 * HILINK 华为方法。
 */
#define CONFIG_METHOD_ALL (0)
#define CONFIG_METHOD_UPLUS ('U')
#define CONFIG_METHOD_AIRKISS ('W')
#define CONFIG_METHOD_JOYLINK ('J')
#define CONFIG_METHOD_ALINK ('A')
#define CONFIG_METHOD_HILINK ('H')

typedef struct wifi_config_smartlink_info
{
	uplus_u8 method; /*any char, but diff for diffent protocol, 'U' is reserved for U+, CONFIG_METHOD_UPLUS*/
	uplus_u32 channel_locked_time; /*max channel locked time, unit: second*/

	wifi_config start; /*start config, option*/
	wifi_config stop; /*stop config, option*/
	wifi_config_result_get result_get; /*get config result, mandatory*/
	wifi_config ch_sw; /*channel switched, option*/
	wifi_config_rcv rcv; /*rcv, mandatory*/
	wifi_sniffer_locked_channel_get locked_ch_get; /*option*/
} wifi_config_smartlink_info_t;

typedef struct wifi_config_smartlink_ack_info
{
	uplus_u8 method; /*any char, but diff for diffent protocol, 'U' is reserved for U+, CONFIG_METHOD_UPLUS*/

	wifi_config_result_analysis ext_analysis; /*analysis ext data, option*/
	wifi_config_ack_build ack_build; /*smartlink ack info build, option*/
	wifi_config_ack_interval ack_interval; /*smartlink ack interval, valid when ack_build isnot NULL*/
} wifi_config_smartlink_ack_info_t;

typedef uplus_s32 (*wifi_config_softap_get_next_read_len_func)(uplus_u8 *data, uplus_u32 data_len);
/*!
 * \typedef wifi_config_softap_rcv
 * \brief Softap报文接收接口。
 * \param data 报文数据。
 * \param data_len 报文数据长度。
 * \param ack 输出，应答报文，可以为NULL，表示无应答报文。
 * \param ack_len 输出，应答报文长度。
 * \param is_delay_rcv 是否是延迟处理的报文。
 * \return SOFTAP_RCV_XXX。
 */
typedef uplus_s32 (*wifi_config_softap_rcv)(uplus_u8 *data, uplus_u32 data_len, uplus_u8 **ack, uplus_u32 *ack_len,
	uplus_u8 is_delay_rcv);

typedef struct wifi_config_softap_info
{
	uplus_u8 method; /*any char, but diff for diffent protocol, 'U' is reserved for U+, CONFIG_METHOD_UPLUS*/

	uplus_u16 bind_port; /*udp*/
	uplus_u16 listen_port; /*tcp*/
	wifi_config_softap_get_next_read_len_func get_next_read_len; /*tcp*/
	wifi_config start; /*start config, option*/
	wifi_config stop; /*stop config, option*/
	wifi_config_result_get result_get; /*get config result, mandatory*/
	wifi_config_result_analysis ext_analysis; /*analysis ext data, option*/
	wifi_config_softap_rcv rcv; /*rcv, mandatory*/
} wifi_config_softap_info_t;

/*!
 * \brief 注册一个Smartlink配置方法。
 * \param info Smartlink配置方法。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_wifi_conf_smartlink_register(wifi_config_smartlink_info_t *info);

/*!
 * \brief 注册一个Smartlink配置ACK方法。
 * \param info Smartlink配置方法。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_wifi_conf_smartlink_ack_register(wifi_config_smartlink_ack_info_t *info);

/*!
 * \brief Smartlink配置初始化。
 *
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_wifi_conf_smartlink_init(void);

/*!
 * \brief Smartlink配置卸载。
 *
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_wifi_conf_smartlink_fini(void);

/*!
 * \brief Smartlink配置ACK初始化。
 *
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_wifi_conf_smartlink_ack_init(void);

/*!
 * \brief Smartlink配置ACK启动状态查询。
 *
 * \return 运行中返回1，否则返回0。
 */
extern uplus_u8 uplus_wifi_conf_smartlink_ack_is_started(void);

/*!
 * \brief 注册一个SoftAP配置方法。
 * \param info SoftAP配置方法。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_wifi_conf_softap_register(wifi_config_softap_info_t *info);

/*!
 * \brief Softap配置初始化。
 *
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_wifi_conf_softap_init(void);

/*!
 * \brief Softap配置卸载。
 *
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_wifi_conf_softap_fini(void);

/*!
 * \brief 进入第三方WIFI配置模式。
 * 第三方WIFI配置模式是指不使用uplugSDK提供的Smartlink和SoftAP配置方式，又需要占用WIFI资源的配置模式。
 * 通过此接口，可以确保在第三方WIFI配置期间，uplugSDK不使用WIFI资源。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_wifi_conf_other_enter(void);

/*!
 * \brief 退出第三方WIFI配置模式。
 * \param is_ok 配置是否成功，1-成功，0-失败。
 * \param ssid 配置的SSID，is_ok为1是有效。
 * \param passwd 配置的密码，is_ok为1是有效。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_wifi_conf_other_exit(int is_ok, const uplus_u8 ssid[32], const uplus_u8 passwd[64]);

/*!
 * \brief 上电进配置。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_wifi_conf_poweron_enter(void);

/*!
 * \brief APP操作进配置。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_wifi_conf_app_enter(void);

/*!
 * \brief 进入Smartlink配置模式。
 * \param method WIFI配置方法。
 * \param smartap 是否在smartlink失败或者超时后转SoftAP。
 * \param clr_wifi_config 是否清除当前配置的SSID和密码，1-清除，0-保持。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_wifi_conf_smartlink_enter(uplus_u8 method, uplus_u8 smartap, uplus_u8 clr_wifi_config);

/*!
 * \brief 进入SoftAP配置模式。
 * \param method WIFI配置方法。
 * \param clr_wifi_config 是否清除当前配置的SSID和密码，1-清除，0-保持。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_wifi_conf_softap_enter(uplus_u8 method, uplus_u8 clr_wifi_config);

/*!
 * \brief 退出Smartlink或者SoftAP配置模式。
 *
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_wifi_conf_exit(void);

/*!
 * \brief 设置WIFI SSID和密码。
 * \note 当通过其他方式（例如蓝牙，PLC等）获取到WIFI信息后，可以通过此接口设置。
 * \param ssid 配置的SSID。
 * \param passwd 配置的密码。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_wifi_conf_notify(const uplus_u8 ssid[32], const uplus_u8 passwd[64]);

/*!
 * \}
 */
/************************************************/


/************************************************/
/*!
 * \defgroup Misc APIs
 * \{
 */

/*!
 * \brief 清除配置区数据。
 * \note 包括扩展配置区的数据，所有配置区的数据恢复成默认数据。
 *
 * \param type 类型 0-清除配置区数据后进入无网状态；1-仅清除配置区数据。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_clr_user_config(uplus_u8 type);

/*!
 * \brief 模块复位。
 *
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_module_reset(void);

/*!
 * \brief 模块复位。
 *
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_module_user_reset(void);

/*!
 * \brief 获取模块重启类型。
 *
 * \return SYSTEM_RESET_TYPE_XXX exclude SYSTEM_RESET_TYPE_UNDEF，see uplus_sys_reset_type_get
 */
extern uplus_s32 uplus_reset_type_get(void);

/*!
 * \brief 通知系统时间变化（仅应用于有系统时间的系统）。
 * \note 当系统时间变化后，有可能会导致某些系统接口（如信号量的等待超时）出现不能返回的情况。
 * 通过此接口可以防止uplugSDK陷入长等状态。
 * \return N/A。
 */
extern void uplus_time_change_notify(void);

/*!
 * \brief 设置系统时间。
 * \param tv 输入，UTC时间，1970年1月1日零时起至今的时间，不含时区信息。
 * \return N/A。
 */
extern void uplus_time_set(struct uplus_timeval *tv);

/*!
 * \brief 获取系统时间。
 * \param tv 输出，UTC时间，1970年1月1日零时起至今的时间，不含时区信息。
 * \return N/A。
 */
extern void uplus_time_get(struct uplus_timeval *tv);

/*!
 * \brief WIFI开关控制。
 * \note WIFI开关控制会存储到配置区，断电重启后继续有效。
 * \param admin_enable 1-打开WIFI，0-关闭WIFI。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_state_wifi_control(uplus_u8 admin_enable);

#define LOW_POWER_NORMAL 0
#define LOW_POWER_WAIT1 1
#define LOW_POWER_WAIT3_1 3
#define LOW_POWER_WAIT3_2 4
#define LOW_POWER_SLEEP_1 5
#define LOW_POWER_SLEEP_2 6

/*!
 * \brief 设置低功耗模式。
 * \param low_power_mode 低功耗模式，参见LOW_POWER_XXX。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_low_power_mode_set(uplus_u8 low_power_mode);

/*!
 * \brief 获取低功耗模式。
 * \return 低功耗模式，参见LOW_POWER_XXX。
 */
extern uplus_u8 uplus_low_power_mode_get(void);

/*! \def UPLUS_DEBUG_TYPE_XXX
 *
 * \brief 可开关日志信息所属模块。
 * SDK uplugSDK。
 * CAE_USS CAE和/或USS。
 *
 */
#define UPLUS_DEBUG_TYPE_SDK 1
#define UPLUS_DEBUG_TYPE_CAE_USS 2

/*! \def UPLUS_DEBUG_LEVEL_XXX
 *
 * \brief 可开关日志信息打印级别。
 * NONE 关闭。
 * BUF 打印收发数据，并开启DEBUG级别。
 * DEBUG 调试级别。
 * INFO 信息级别，并开启DEBUG级别。
 * WARN 警告级别，并开启INFO级别。
 * ERROR 错误级别，并开启WARN级别。
 *
 */
#define UPLUS_DEBUG_LEVEL_NONE 0
#define UPLUS_DEBUG_LEVEL_BUF 1
#define UPLUS_DEBUG_LEVEL_DEBUG 2
#define UPLUS_DEBUG_LEVEL_INFO 3
#define UPLUS_DEBUG_LEVEL_WARN 4
#define UPLUS_DEBUG_LEVEL_ERROR 5

/*!
 * \brief 设置uplugsdk及其库的日志打印级别
 * \param debug_type 可开关日志信息所属模块。
 * \param debug_level 可开关日志信息打印级别。
 * \return N/A。
 */
extern void uplus_debug_level_set(uplus_u32 debug_type, uplus_u32 debug_level);

/*!
 * \}
 */
/************************************************/


/************************************************/
/*for info get*/
/*!
 * \defgroup Info get APIs
 * \brief 获取uplugSDK信息。
 * \{
 */

/*!
 * \typedef module_base_info_t
 * \brief 基本信息结构体。
 * hw_ver 硬件版本。
 * sw_ver 软件版本。
 * platform 平台信息。
 * server_domain 服务器域名。
 * server_port 服务器端口。
 * ip_addr 记录的IP地址，点分十进制。
 */
/*basic info*/
typedef struct module_base_info
{
	uplus_s8 hw_ver[16];
	uplus_s8 sw_ver[16];
	uplus_s8 platform[32];
	uplus_s8 server_domain[64];
	uplus_u16 server_port;
	uplus_s8 ip_addr[16];
} module_base_info_t;

/*!
 * \typedef module_wifi_info_t
 * \brief WIFI配置信息结构体。
 * ssid_valid SSID是否有效。
 * ssid WIFI SSID。
 * password WIFI密码。
 */
/*wifi info*/
typedef struct module_wifi_info
{
	uplus_u8 ssid_valid;
	uplus_s8 ssid[32];
	uplus_s8 password[64];
} module_wifi_info_t;

/*! \def CONAP_XXX
 *
 * \brief WIFI连接错误码。
 * OK 连接成功。
 * MAYBE_PASSWORD_ERR 疑似密码错误。
 * OTHER_ERR 其他错误。
 * PASSWORD_ERR 密码错误。
 * NONE_NETWORK_ERR 找不到网络（SSID）。
 * NOSIGNAL_ERR 信号弱，连接不上。
 * DHCP_ERR 未获取到IP地址。
 * TIMEOUT_ERR 规定的时间未配置或者连接成功。
 *
 */
enum ap_connect_err_e
{
	CONAP_OK = 0,
	/*CONAP_MAYBE_PASSWORD_ERR is not a macro defined by wifi base protocol*/
	CONAP_MAYBE_PASSWORD_ERR = 1,
	CONAP_OTHER_ERR = 60001,
	CONAP_PASSWORD_ERR,
	CONAP_NONE_NETWORK_ERR,
	CONAP_NOSIGNAL_ERR,
	CONAP_DHCP_ERR,
	CONAP_TIMEOUT_ERR
};

/*!
 * \typedef module_status_info_t
 * \brief 状态信息结构体。
 * led_control led状态，详见LED pal定义。
 * wifi_rssi 当前信号强度。
 * cfg_mode WIFI配置模式（运行）。
 * cfg_set_mode WIFI配置模式（设置）。
 * last_conn_ap_err 上一次WIFI连接的错误码。
 */
/*status info*/
typedef struct module_status_info
{
	uplus_u16 led_control;
	uplus_u8 wifi_rssi;
	uplus_u8 cfg_mode; /*CONFIG_MODE_XXX*/
	uplus_u8 cfg_set_mode; /*CONFIG_MODE_XXX*/
	uplus_u32 last_conn_ap_err; /*CONAP_XXX*/
} module_status_info_t;

/*!
 * \typedef module_dev_info_t
 * \brief 设备信息结构体。
 * dev_is_ok 设备是否注册。
 * dev_info 如果设备已注册，返回注册设备信息；如果设备没注册，设置了默认设备，返回默认设备信息。
 * unqiue_id 设备的唯一安全标识。如果是非安全设备，等同于设备唯一标识。
 * mac 设备唯一标识，通常是设备MAC。
 */
/*dev info*/
typedef struct module_dev_info
{
	uplus_u8 dev_is_ok;
	dev_info_t dev_info;
	uplus_s8 unqiue_id[32];
	uplus_s8 mac[32];
} module_dev_info_t;

/*! \def MODULE_INFO_TYPE_XXX
 *
 * \brief 获取信息类型。
 * BASE 基本信息。
 * WIFI WIFI配资信息。
 * STATUS 状态信息。
 * DEV 设备信息。
 *
 */
typedef enum module_info_type
{
	MODULE_INFO_TYPE_BASE, /*module_base_info_t*/
	MODULE_INFO_TYPE_WIFI, /*module_wifi_info_t*/
	MODULE_INFO_TYPE_STATUS, /*module_status_info_t*/
	MODULE_INFO_TYPE_DEV /*module_dev_info_t*/
} module_info_type_t;

/*!
 * \brief 获取uplugSDK版本信息。
 *
 * \return uplugSDK版本信息字符串。
 */
extern const uplus_s8 *uplus_sdk_ver_get(void);

/*!
 * \brief 获取uplugSDK信息。
 * \param info_type 获取信息类型
 * \param info 不同类型返回不同的结构体。
 * \return N/A。
 */
extern void uplus_sdk_info_get(module_info_type_t info_type, void *info);

/*!
 * \}
 */
/************************************************/


/************************************************/
/*!
 * \defgroup WIFI scan APIs
 * \brief 支持WIFI扫描功能。
 * \{
 */

/*!
 * \typedef wifi_scan_result_t
 * \brief WIFI扫描结果结构体。
 * ap_num 扫描到的AP数量。
 * ref_cnt 内部使用，不关心。
 * ap_list 扫描到的AP列表，详见ap_list_api_t。
 */
typedef struct
{
	uplus_u8 ap_num;
	uplus_u32 ref_cnt;
	ap_list_api_t *ap_list;
} wifi_scan_result_t;

/*!
 * \brief 获取WIFI扫描结果。
 * \note 获取非空扫描结果后，必须调用uplus_wifi_scan_unlock以释放资源。
 *
 * \return 最近一次的扫描结果。系统错误或者从未扫描过，则返回NULL。
 */
extern wifi_scan_result_t * uplus_wifi_scan_get_lock(void);

/*!
 * \brief 释放WIFI扫描结果资源
 * \param result 通过uplus_wifi_scan_get_lock获取的非空扫描结果。
 * \return N/A。
 */
extern void uplus_wifi_scan_unlock(wifi_scan_result_t *result);

/*!
 * \brief WIFI扫描功能初始化。
 *
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_wifi_scan_init(void);

/*!
 * \brief 清除WIFI扫描缓存。
 *
 * \return 成功返回0，失败返回-1。
 */
extern void uplus_wifi_scan_flush(void);

/*!
 * \brief 触发一次WIFI扫描动作。
 * \note 两次触发动作间隔不小于5秒，否则忽略第2次的触发。
 * 触发后，有可能不能马上获取到扫描结果，根据WIFI的PAL实现，有不同的延迟。
 * \param cb 扫描完成后的回调通知。可以设置为NULL。
 * \param param 回调接口的用户参数。
 *
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_wifi_scan_trigger(wifi_scan_cb_func cb, void * param);

/*!
 * \}
 */
/************************************************/


/************************************************/
/*!
 * \defgroup OTA APIs
 * \{
 */

/*!
 * \typedef ota_process_func
 * \brief 自定义OTA数据的处理接口。
 * \param image_zone OTA数据存储的区域。
 * \param total_len OTA数据长度。
 * \param param 用户自定义数据。
 * \return N/A。
 */
typedef void (* ota_process_func)(uplus_u8 image_zone, uplus_u32 total_len, void * param);

/*!
 * \brief 设置自定义OTA数据的处理接口。
 * \param process 自定义OTA数据的处理接口。
 * \param param 用户自定义数据。
 * \return N/A。
 */
extern void uplus_ota_process_func_set(
	ota_process_func process,
	void *param);

/*!
 * \}
 */
/************************************************/


/************************************************/
/*!
 * \defgroup DNS Async APIs
 * \{
 */

/*!
 * \typedef dns_cb_func
 * \brief DNS解析结果通知接口。
 * \param param 用户自定义数据。
 * \param unique_id 本次解析的唯一标识。
 * \param dns_ret 0表示成功，-1表示失败。
 * \param ip 解析的IP地址，当dns_ret=0时有效。
 * \return N/A。
 */
typedef void (*dns_cb_func)(void *param, uplus_u32 unique_id, uplus_s32 dns_ret, uplus_s8 *ip);

/*!
 * \brief DNS解析，异步回调方式。
 * \param domain 待解析的域名。
 * \param cb DNS解析结果通知接口。
 * \param param 用户自定义数据。
 * \param unique_id 本次解析的唯一标识。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_dns_request(uplus_s8 domain[64], dns_cb_func cb, void *param,
	uplus_u8 use_httpdns, uplus_u32 unique_id);

/*!
 * \}
 */
/************************************************/


/************************************************/
/*!
 * \defgroup Network(local/cloud) status APIs
 * \details U+本地连接或者远程连接发生变化时，可以提供接口通知到应用程序，进行后续特定处理。
 * \{
 */

/*!
 * \typedef cloud_status_func
 * \brief 远程连接状态变化通知接口。
 * \param status 1-连接成功，0-连接失败或者连接中断。
 * \param param 用户自定义数据。
 * \return N/A。
 */
typedef void (*cloud_status_func)(uplus_u8 status, void *param);

/*!
 * \brief 设置远程连接状态变化通知回调接口。
 * \param cloud_status 远程连接状态变化通知回调接口。
 * \param param 用户自定义数据。
 * \return 成功返回0，失败返回-1。
 */
extern void uplus_cloud_status_func_set(
	cloud_status_func cloud_status,
	void *param);

/*!
 * \typedef local_client_info_t
 * \brief 本地客户端信息结构体。
 * \details AP信息定义包括以下信息
 * addr_v4 本地客户端地址和端口。
 * status 0-客户端未发生变化，1-新建一个客户端连接, -1-删除一个客户端连接。
 */
typedef struct local_client_info
{
	union
	{
		struct uplus_sockaddr addr;
		struct uplus_sockaddr_in addr_v4;
	};
	uplus_s32 status; /*0-keep, 1-add, -1-delete*/
} local_client_info_t;

/*!
 * \typedef local_status_func
 * \brief 本地连接状态变化通知接口，全量通知，包括已有连接，新建连接和待删除的连接。
 * \param info 本地连接信息列表，详见local_client_info_t。
 * \param number 本地连接数量。
 * \param param 用户自定义数据。
 * \return N/A。
 */
typedef void (*local_status_func)(local_client_info_t *info, uplus_u8 number, void *param);

/*!
 * \brief 设置本地连接状态变化通知回调接口。
 * \param local_status 本地连接状态变化通知回调接口。
 * \param param 用户自定义数据。
 * \return 成功返回0，失败返回-1。
 */
extern void uplus_local_status_func_set(
	local_status_func local_status,
	void *param);

/*!
 * \}
 */
/************************************************/


/************************************************/
/*!
 * \defgroup System APIs
 * \{
 */

/*!
 * \brief uplugSDK初始化接口。
 * \note 必须在调用uplugSDK任何API接口之前调用，否则将产生不可预知的错误。
 * \param init_config 初始化配置参数。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_init(init_config_para_t * init_config);

/*!
 * \brief uplugSDK初始化完成后，启动uplugSDK。
 * \details 在uplus_init和uplus_start之间可以执行其他初始化的动作，用以实现uplugSDK的延迟启动。

 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_start(void);

/*!
 * \}
 */
/************************************************/


/************************************************/
/*!
 * \defgroup Plugin APIs
 * \details 当第三方应用希望控制或者获取底板设备信息时，可以通过插件模块实现控制底板设备和底板设备信息同步功能。
 * \note 插件模块交互的数据均是底板设备可识别的数据。
 * \{
 */

/*!
 * \typedef plugin_response_rx_func
 * \brief 应答数据回调接口。
 * \param param 用户自定义数据。
 * \param data_info 应答数据。
 * \param session 自定义会话标识。与sn可以唯一标识一条数据。
 * \param sn 自定义的序列号。
 */
typedef void (* plugin_response_rx_func)(void *param, data_info_t *data_info, uplus_u8 session[6], uplus_u32 sn);
/*!
 * \typedef plugin_report_rx_func
 * \brief 上报数据回调接口。
 * \param param 用户自定义数据。
 * \param data_info 上报数据。
 */
typedef void (* plugin_report_rx_func)(void *param, data_info_t *data_info);

/*!
 * \brief 初始化插件模块
 *
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_plugin_init(void);

/*!
 * \brief 第三方应用向底板设备发送数据
 *
 * \param data_info 待发送的数据。不论成功失败，数据资源都被释放。
 * \param session 自定义会话标识。与sn可以唯一标识一条数据。
 * 如果数据有响应，会通过注册的应答回调接口将数据应答及session+sn传递出去。
 * \param sn 自定义的序列号。
 *
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_plugin_tx(data_info_t *data_info, uplus_u8 session[6], uplus_u32 sn);

/*!
 * \brief 第三方应用注册应答数据回调接口和上报数据回调接口。
 *
 * \param param 用户自定义数据。
 * \param response_rx 应答数据回调接口。
 * \param report_rx 上报数据回调接口。
 *
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_plugin_register(void *param, plugin_response_rx_func response_rx, plugin_report_rx_func report_rx);

/*!
 * \}
 */
/************************************************/

/************************************************/
/*!
 * \defgroup Sys event APIs
 * \details
 * \note
 * \{
 */

enum
{
	/*mode*/
	SYS_EVENT_WORK_MODE_ENTER,
	SYS_EVENT_WORK_MODE_EXIT,
	SYS_EVENT_PRODUCT_MODE_ENTER,
	SYS_EVENT_PRODUCT_MODE_EXIT,
	SYS_EVENT_WIFI_CONFIG_MODE_ENTER,
	SYS_EVENT_WIFI_CONFIG_MODE_EXIT,

	/*wifi connect to ap, only for work & product mode*/
	SYS_EVENT_CONNECT_AP_START,
	SYS_EVENT_CONNECT_AP_STATUS_OK,
	SYS_EVENT_CONNECT_AP_STATUS_FAIL,

	/*cloud conn*/
	SYS_EVENT_CLOUD_START,
	SYS_EVENT_CLOUD_OK,
	SYS_EVENT_CLOUD_FAIL,

	/*local*/
	SYS_EVENT_LOCAL_EMPTY, /*no local client*/
	SYS_EVENT_LOCAL_NORMAL,
	SYS_EVENT_LOCAL_FULL, /*local clients are full*/

	SYS_EVENT_MAX
};

/*!
 * \typedef sys_event_cb_func
 * \brief sys event callback
 * \param sys_event sys event see SYS_EVENT_XXX
 * \param param user param
 */
typedef void (* sys_event_cb_func)(uplus_u16 sys_event, void *param);

/*!
 * \brief set sys event callback
 * \param cb callback function
 * \param param 用户自定义数据。
 * \return 成功返回0，失败返回-1。
 */
extern uplus_s32 uplus_sys_event_cb_set(sys_event_cb_func cb, void * param);


typedef void (* app_bindkey_cb_func)(uplus_u8 *buf, uplus_u32 len, void *param);
extern uplus_s32 uplus_bindkey_cb_set(app_bindkey_cb_func cb, void *param);

/*!
 * \}
 */
/************************************************/


#ifdef __cplusplus
}
#endif

#endif /*__UPLUS_FRAMEWORK_H__*/

