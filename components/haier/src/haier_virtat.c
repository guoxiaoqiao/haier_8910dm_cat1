#include "haier_virtat.h"

uint8_t get_vat_init_status(void);
void set_vat_init_status(uint8_t status);
void vat_cmd_send(char *cmdbuf, uint32_t len);

static osiPipe_t *at_rx_pipe;
static osiPipe_t *at_tx_pipe;

uint8_t get_vat_init_status(void)
{
    return appSysTem.vat_init_finsh;
}

void set_vat_init_status(uint8_t status)
{
    appSysTem.vat_init_finsh = status;
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

        zk_queue_msg_send(vat_recv_queue, VAT_RECV_MSG, buf, (uint16_t)bytes, 0);
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
    //指示虚拟AT通道已经初始化完成，后续其它功能模块可以使用虚拟AT功能了
    set_vat_init_status(1); 
    //OSI_LOGI(0, "[zk vat] virt_at_init_3 Virt at init succ");
}

static void vat_send_handle(char* cmd, uint32_t cmdlen)
{
    if((cmd == NULL) || (cmdlen == 0))
    {
        OSI_LOGE(0, "[zk vat] vat_send_handle_0 param error len=%d", cmdlen);
        return;
    }

    OSI_LOGXI(OSI_LOGPAR_SII, 0, "[zk vat] vat_send_handle_1:VAT1 -->:%s len=%d", cmd, cmdlen);

    osiPipeWriteAll(at_rx_pipe, cmd, strlen(cmd), 5000);
    //int writelen = osiPipeWriteAll(at_rx_pipe, cmd, strlen(cmd), 5000);

    //OSI_LOGE(0, "[zk vat] vat_send_handle_2:writelen=%d", writelen);
    //OSI_LOGXI(OSI_LOGPAR_SII, 0, "[zk vat] vat_send_handle_1:VAT1 -->:%s,len=%d,writelen=%d", cmd, cmdlen, writelen);
}

void vat_send_task_main(void *pParameter)
{
    TASK_MSG *msg = NULL;

    virt_at_init();
    while (1)
    {
        if(xQueueReceive(vat_send_queue, &msg, portMAX_DELAY) == pdPASS)
        {
            switch (msg->id)
            {
                case VAT_SEND_MSG:
                    vat_send_handle(msg->param, msg->len);
                    break;
                default:
                    OSI_LOGE(0, "[zk vat] vat_send_task_main_1 not cmd %d", msg->id);
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
		//加这个350ms的延时是防止连续发送AT命令，导致串口数据冲突
		vTaskDelay(osiMsToOSTick(200));
    }
}

static void vat_CINIT_rsp_handle(char *rsp_buff, uint32_t len)
{
   // OSI_LOGXI(OSI_LOGPAR_IS, 0, "[zk vat] cinit_rsp_(%d):%s", len, rsp_buff);
}

static void vat_CREG_rsp_handle(char *rsp_buff, uint32_t len)
{
    //OSI_LOGXI(OSI_LOGPAR_IS, 0, "[zk vat] creg_rsp_(%d):%s", len, rsp_buff);
}

static void vat_CEREG_rsp_handle(char *rsp_buff, uint32_t len)
{
    //OSI_LOGXI(OSI_LOGPAR_IS, 0, "[zk vat] cereg_rsp_(%d):%s", len, rsp_buff);

    TASK_MSG_ID msg_id = 0;

    uint8_t netstatus = (uint8_t)(rsp_buff[0] - '0');
    if(netstatus != get_net_state())
    {
        //OSI_LOGI(0, "[zk vat] net status change old=%d cur=%d", get_net_state(), netstatus);
        set_net_state(netstatus);
        switch (netstatus)
        {
            case 1:
            case 5:
                msg_id = NETWORK_ATTACHED;
                break;
            case 2:
                msg_id = NETWORK_ATTACHING;
                break;
            case 0:
            case 3:
            default:
                msg_id = NETWORK_DISCONNECT;
                break;
        }
        zk_queue_msg_send(network_queue, msg_id, NULL, 0, 0);
    }
}

static void vat_CGREG_rsp_handle(char *rsp_buff, uint32_t len)
{
    //OSI_LOGXI(OSI_LOGPAR_IS, 0, "[zk vat] cgreg_rsp_(%d):%s", len, rsp_buff);
}

static void vat_CSCON_rsp_handle(char *rsp_buff, uint32_t len)
{
    //OSI_LOGXI(OSI_LOGPAR_IS, 0, "[zk vat] cscon_rsp_(%d):%s", len, rsp_buff);
}

static void vat_CGDCONT_rsp_handle(char *rsp_buff, uint32_t len)
{
    //OSI_LOGXI(OSI_LOGPAR_IS, 0, "[zk vat] cgdcont_rsp_(%d):%s", len, rsp_buff);
    if(strstr(rsp_buff, "1,\"IP\""))
    {
        zk_queue_msg_send(network_queue, NETWORK_LINKING, NULL, 0, 0);
    }
}

static void vat_CGPADDR_rsp_handle(char *rsp_buff, uint32_t len)
{
    //OSI_LOGXI(OSI_LOGPAR_IS, 0, "[zk vat] cgPADDR_rsp_(%d):%s", len, rsp_buff);

    uint8_t cnt = 0;
    char *p = strstr(rsp_buff, "\"");
    if(p != NULL)
    {
        char *ip_p = ++p;
        while(((*p) != '\"')&&((*p) != '\r')&&((*p) != '\n'))
        {
            //OSI_LOGI(0, "[zk vat] get ipaddr:0x%x", (*p));
            cnt++;
            p++;
        }
        memcpy(appSysTem.Module_ipaddr, ip_p, cnt);
        
        //OSI_LOGXI(OSI_LOGPAR_S, 0, "[zk vat] get ipaddr:%s", appSysTem.Module_ipaddr);
    }
    else
    {
        OSI_LOGI(0, "[zk vat] get ipaddr:error");
    }
}

static void vat_CGACT_rsp_handle(char *rsp_buff, uint32_t len)
{
    //OSI_LOGXI(OSI_LOGPAR_IS, 0, "[zk vat] cgact_rsp_(%d):%s", len, rsp_buff);
    if(strstr(rsp_buff, "1, 1"))
    {
        zk_queue_msg_send(network_queue, NETWORK_LINKED, NULL, 0, 0);
    }
}

static void vat_CTZV_rsp_handle(char *rsp_buff, uint32_t len)
{
    //OSI_LOGXI(OSI_LOGPAR_IS, 0, "[zk vat] cscon_rsp_(%d):%s", len, rsp_buff);
}

static void vat_CGSN_rsp_handle(char *rsp_buff, uint32_t len)
{
    memcpy(appSysTem.Module_IMEI, rsp_buff+1, IMEI_LEN);

    OSI_LOGXI(OSI_LOGPAR_IS, 0, "[zk vat] cgsn_rsp_(%d):%s", len, appSysTem.Module_IMEI);
}

static void vat_CCID_rsp_handle(char *rsp_buff, uint32_t len)
{
    memcpy(appSysTem.Module_ICCID, rsp_buff, ICCID_LEN);

    OSI_LOGXI(OSI_LOGPAR_IS, 0, "[zk vat] ccid_rsp_(%d):%s", len, appSysTem.Module_ICCID);
}

static void vat_CIMI_rsp_handle(char *rsp_buff, uint32_t len)
{
    memcpy(appSysTem.Module_IMSI, rsp_buff+2, IMSI_LEN);

    OSI_LOGXI(OSI_LOGPAR_IS, 0, "[zk vat] cimi_rsp_(%d):%s", len, appSysTem.Module_IMSI);
}

static vat_cmd_cb_s vat_cmd_table[] = {

    {"^CINIT:", vat_CINIT_rsp_handle},
    {"+CREG: ", vat_CREG_rsp_handle},
    {"+CEREG: ", vat_CEREG_rsp_handle},
    {"+CGREG: ", vat_CGREG_rsp_handle},
    {"+CSCON: ", vat_CSCON_rsp_handle},
    {"+CGDCONT: ", vat_CGDCONT_rsp_handle},
    {"+CGPADDR: ", vat_CGPADDR_rsp_handle},
    {"+CGACT: ", vat_CGACT_rsp_handle},
    {"+CTZV: ", vat_CTZV_rsp_handle},
    {"+CGSN:", vat_CGSN_rsp_handle},
    {"+CCID: ", vat_CCID_rsp_handle},
    //{"+CIMI", vat_CIMI_rsp_handle},
};

static uint8_t vat_get_cmd_table_size(void)
{
    return (uint8_t)(sizeof(vat_cmd_table)/sizeof(vat_cmd_cb_s));
}

vat_cmd_cb_s * vat_get_atcmd_cb_from_atstring(char *p_atstring, uint16_t *len)
{
    vat_cmd_cb_s     *cmd_cb;
    vat_cmd_cb_s     *p_at_cmd_table = vat_cmd_table;
    uint16_t         i;
    uint8_t          count;

    count = vat_get_cmd_table_size();

    //这一段要不要无所谓， 因为回过来的数据肯定是大写，节省时间，注释掉
    /*for(i = 0; i < at_cmd_length; i++)
    {
        *(p_atstring + i) = (char) toupper( *(p_atstring + i));
    }*/

    for( i = 0; i < count; i++)
    {
        cmd_cb = &(p_at_cmd_table[i]);

        char * p = strstr(p_atstring, cmd_cb->cmd_str);
        if(p != NULL)
        {
            *len = (uint16_t)((p - p_atstring) + strlen(cmd_cb->cmd_str));
            return cmd_cb;
        }
    }
    return NULL;
}

static void vat_perform_atcommand(char *p_str, uint32_t len)
{
    uint16_t vat_cmd_length = 0;
    char *str = p_str;

    vat_cmd_cb_s *cmd_cb = vat_get_atcmd_cb_from_atstring(str, &vat_cmd_length);
    if(cmd_cb)
    {
        str += vat_cmd_length; 
        if(cmd_cb->vat_rsp_data_process_handler)
        {
           (cmd_cb->vat_rsp_data_process_handler)(str, len-vat_cmd_length);
        }
        return;
    }

    if((p_str[2] >= '0') && (p_str[2] <= '9'))  //为什么是p_str[2]呢？  因为前面有\r\n.
    {
        vat_CIMI_rsp_handle(p_str, len); //CIMI的RSP没有标志前缀，用数字作为判断标准，难搞哟。
        return;
    }
	//OSI_LOGXI(OSI_LOGPAR_S, 0, "[zk vat]vat_cmd no mismatch :%s", p_str);
}

static void vat_recv_handle(char *recvbuff, uint32_t recvlen)
{
    if((recvbuff == NULL) || (recvlen == 0))
    {
        OSI_LOGE(0, "[zk vat] vat_recv_handle_0 param error len=%d", recvlen);
        return;
    }

    if((strstr(recvbuff, "OK")) || (strstr(recvbuff, "ERROR")))
    {
       //OSI_LOGXI(OSI_LOGPAR_S, 0, "%s", recvbuff);
       return;
    }
    else if(strstr(recvbuff, "AT"))
    {
        return;
    }
    //OSI_LOGXI(OSI_LOGPAR_IS, 0, "[zk vat] vat_recv_handle_2:VAT1 <--(%d):%s", recvlen, recvbuff);
    vat_perform_atcommand(recvbuff, recvlen);
}

void vat_recv_task_main(void *pParameter)
{
    TASK_MSG *msg = NULL;

    while (1)
    {
        if(xQueueReceive(vat_recv_queue, &msg, portMAX_DELAY) == pdPASS)
        {
            switch (msg->id)
            {
                case VAT_RECV_MSG:
                    vat_recv_handle(msg->param, msg->len);
                    break;
                default:
                    OSI_LOGE(0, "[zk vat] vat_recv_task_main_1 not cmd %d", msg->id);
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

void vat_cmd_send(char *cmdbuf, uint32_t len)
{
    if((cmdbuf == NULL) || (len == 0))
    {
        return;
    }
    zk_queue_msg_send(vat_send_queue, VAT_SEND_MSG, (void *)cmdbuf, len, 0);
}

void net_timer_callback(void *argument)
{	
	restart(1);
}

static void module_init(void)
{
   // vat_cmd_send("AT+CGMR\r\n", strlen("AT+CGMR\r\n"));
    //vat_cmd_send("AT+CEREG=1\r\n", strlen("AT+CEREG=1\r\n"));
    vat_cmd_send("AT+CFUN=1\r\n", strlen("AT+CFUN=1\r\n"));
    vat_cmd_send("AT+CGSN\r\n", strlen("AT+CGSN\r\n"));
    //vTaskDelay(osiMsToOSTick(1000));
    //vat_cmd_send("AT+CGATT=1\r\n", strlen("AT+CGATT=1\r\n"));

}

void network_task_main(void *pParameter)
{
    TASK_MSG *msg = NULL;

    while (!(get_vat_init_status()))
    {
        vTaskDelay(osiMsToOSTick(30)); 
    }
        //上电先关射频
    vat_cmd_send("AT+CFUN=0\r\n", strlen("AT+CFUN=0\r\n"));

    if(local.fota_flag == 0)
    {
        //等获取到底板版本信息以后才开始进行联网
        while (1)
        {
            vTaskDelay(osiMsToOSTick(1000)); 
            if((appSysTem.GetDeviceVer_OK_Flag == 1)||(appSysTem.GetDeviceVer_Fail_Flag == 1))
            {
                module_init();
                break;
            }
        }
    }
    else
    {
        vTaskDelay(osiMsToOSTick(3000)); 
        module_init();
    }
    if(network_Timers != NULL)
    {
        //开启定时器，用来控制搜网，x min没搜网成功就整机复位
        xTimerStart(network_Timers, 0);
    }
    OSI_LOGI(0, "[zk net] network_task_main_0: init finish");
    while (1)
    {
        if(xQueueReceive(network_queue, &msg, portMAX_DELAY) == pdPASS)
        {
            if(local.fota_flag)
            {
                switch (msg->id)
                {
                    case NETWORK_ATTACHED:
                        OSI_LOGI(0, "[zk net] network_task_main_1: fota net work attached");
                        //set_sys_state(SYS_STATE_NETWORK_CONNECT);
                        //定义本地PDP上下文
                        vat_cmd_send("AT+CGDCONT=1,\"IP\",\"\"\r\n", strlen("AT+CGDCONT=1,\"IP\",\"\"\r\n"));
                        vat_cmd_send("AT+CGDCONT?\r\n", strlen("AT+CGDCONT?\r\n"));
                        break;
                    case NETWORK_LINKING:
                        OSI_LOGI(0, "[zk net] network_task_main_3: fota net work Linking...");
                        //set_sys_state(SYS_STATE_NETWORK_CONNECT);
                        //附着上核心网以后，需要手动激活PDP承载通道
                        vat_cmd_send("AT+CGACT=1,1\r\n", strlen("AT+CGACT=1,1\r\n"));
                        break;
                    case NETWORK_LINKED:
                        OSI_LOGI(0, "[zk net] network_task_main_5: fota net work linked");
                        //搜网成功以后，关闭这个定时器，否则会整机复位
                        if(xTimerIsTimerActive(network_Timers) == pdPASS)
                        {
                            xTimerStop(network_Timers, 0);
                            //OSI_LOGI(0, "[zk net] network_task_main_6: stop network time");
                        }
                        else
                        {
                            OSI_LOGE(0, "[zk net] network_task_main_6: fota stop network time error");
                        }
                        //PDP激活以后,延时一段时间在启动FOTA
                        vTaskDelay(osiMsToOSTick(1000));
                        if(local.fota_flag == (uint8_t)FOTA_DOWNLOAD)
                            zk_queue_msg_send(fota_event_queue, FOTA_DOWNLOAD_MSG, NULL, 0, 0);
                        else if(local.fota_flag == (uint8_t)FOTA_GET_FWPKG_URL)
                            zk_queue_msg_send(fota_event_queue, FOTA_GET_FWPKG_URL_MSG, NULL, 0, 0);
                        else
                        {
                            OSI_LOGE(0, "[zk net] network_task_main_7: fota status error fota_flag=%d", local.fota_flag);
                            local.fota_flag = (uint8_t)FOTA_NULL;
                            local.fota_fail_ret_num = 0;
                            write_local_cfg_Info();
                        }  
                        break;
                    default:
                        OSI_LOGE(0, "[zk net] network_task_main_8: fota not cmd %d", msg->id);
                        break;
                }
            }
            else
            {
                switch (msg->id)
                {
                    case NETWORK_ATTACHING:
                        OSI_LOGI(0, "[zk net] network_task_main_0: net work attaching...");
                        set_sys_state(SYS_STATE_NETWORK_CONNECT);
                        break;
                    case NETWORK_ATTACHED:
                        OSI_LOGI(0, "[zk net] network_task_main_1: net work attached");
                        set_sys_state(SYS_STATE_NETWORK_CONNECT);
                        //定义本地PDP上下文
                        vat_cmd_send("AT+CGDCONT=1,\"IP\",\"\"\r\n", strlen("AT+CGDCONT=1,\"IP\",\"\"\r\n"));
                        vat_cmd_send("AT+CGDCONT?\r\n", strlen("AT+CGDCONT?\r\n"));
                        break;
                    case NETWORK_DISCONNECT:
                        OSI_LOGI(0, "[zk net] network_task_main_2: net work disconnect");
                        set_sys_state(SYS_STATE_NETWORK_CONNECT);
                        //断网后需要手动停用之前激活的PDP承载通道
                        vat_cmd_send("AT+CGACT=0,1\r\n", strlen("AT+CGACT=0,1\r\n"));
                        restart(5);
                        break;
                    case NETWORK_LINKING:
                        OSI_LOGI(0, "[zk net] network_task_main_3: net work Linking...");
                        set_sys_state(SYS_STATE_NETWORK_CONNECT);
                        //附着上核心网以后，需要手动激活PDP承载通道
                        vat_cmd_send("AT+CGACT=1,1\r\n", strlen("AT+CGACT=1,1\r\n"));
                        break;
                    case NETWORK_LINKED:
                        OSI_LOGI(0, "[zk net] network_task_main_5: net work linked");
                        //搜网成功以后，关闭这个定时器，否则会整机复位
                        if(xTimerIsTimerActive(network_Timers) == pdPASS)
                        {
                            xTimerStop(network_Timers, 0);
                            //OSI_LOGI(0, "[zk net] network_task_main_6: stop network time");
                        }
                        else
                        {
                            OSI_LOGE(0, "[zk net] network_task_main_6: stop network time error");
                        }
                        
                        set_sys_state(SYS_STATE_REG);
                        //成功激活PDP承载通道后，获取IMSI,ICCID，为后续连接u+云做准备
                        vat_cmd_send("AT+CCID\r\n", strlen("AT+CCID\r\n"));
                        vat_cmd_send("AT+CIMI\r\n", strlen("AT+CIMI\r\n"));
                        vat_cmd_send("AT+CGPADDR\r\n", strlen("AT+CGPADDR\r\n"));

                        //PDP激活以后,延时一段时间在启动优家SDK
                        vTaskDelay(osiMsToOSTick(5000));
                        zk_queue_msg_send(uplus_server_queue, UPLUS_SDK_INIT_MSG, NULL, 0, 0);
                        break;
                    default:
                        OSI_LOGE(0, "[zk net] network_task_main_7: not cmd %d", msg->id);
                        break;
                }
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
