#include "haier_appmain.h"
#include "http_api.h"
#include "http_download.h"
#include "fupdate.h"
#include "zk_md5.h"

#define FOTA_DOWNLOAD_MAX_TIME     (5*60*1000)

#define SUB_PACK_MAX_SIZE           1000
#define FOTA_GET_FWPKG_CNT_MAX      3

extern void led_fota_change(uint8_t typ);

static TimerHandle_t fota_download_Timers;

static char uhome_content_type[] = "application/json";
//static char body_content[] = "{\"productName\":\"AIR_SE-A_CT1\",\"currentVersion\":\"2.2.2\",\"extraInfo\":{}}";
static char body_content[256];
static char uhome_url[] = "http://os.uhome.haier.net/osfota/app/package/v3";

static void fota_download_SoftTimerCallback(void *argument)
{
    led_fota_change(LED_FOTA_STOP);	
	restart(7);
}

static void zk_fota_update(uint8_t *data, uint32_t len)
{
    if((data != NULL) && (len != 0))
    {
        OSI_LOGI(0, "[zk fota] zk_fota_data_cpy_1: len=%d", len);

        fupdateInvalidate(true);
        vfs_mkdir(CONFIG_FS_FOTA_DATA_DIR, 0);

        int wsize = vfs_file_write(FUPDATE_PACK_FILE_NAME, data, len);
        if (wsize != len)
        {
            OSI_LOGE(0, "[zk] Fota Error : write file fail, errno %d", wsize);
        }
    }
    
    // To check current version, pass the current version string
    // as parameter.
    if (!fupdateSetReady(NULL))
    {
        OSI_LOGE(0, "[zk] Fota Error: not ready");
    }
    else
    {
        OSI_LOGE(0, "[zk] Fota download ok");

        //led_fota_change(LED_FOTA_STOP);	

        /*local.fota_flag = 0;
        local.fota_fail_ret_num = 0;

        write_local_cfg_Info();*/
       
        //restart(6);
        //osiShutdown(OSI_SHUTDOWN_RESET);
    }
}

static void get_url(char *content)
{
    char *p = NULL;
    p = strstr(content, "\"url\":\"");
    if(p != NULL)
    {
        uint16_t cnt = 0;
        p += strlen("\"url\":\"");
        char *f_p = p;
        while(((*p) != '\"')&&((*p) != '\r')&&((*p) != '\n')&&((*p) != 0))
        {
            cnt++;
            p++;
        }
        if(cnt < 255)
        {
            memset(local.fw_pack_url, 0, sizeof(local.fw_pack_url));
            memcpy(local.fw_pack_url, f_p, cnt);
            OSI_LOGXI(OSI_LOGPAR_S, 0, "[zk ota] get fwpkg_url:%s", local.fw_pack_url);
        }
    }
}

static void get_md5(char *content)
{
    //md5值获取(这里获取完之后，需要保存，后面要对下载的包进行MD5计算，与这里保存的值进行比较，以此来判断包是否正确下载)
    char *p = NULL;
    p = strstr(content, "\"md5\":\"");
    if(p != NULL)
    {
        uint16_t cnt = 0;
        p += strlen("\"md5\":\"");
        char *md5_p = p;
        while(((*p) != '\"')&&((*p) != '\r')&&((*p) != '\n')&&((*p) != 0))
        {
            cnt++;
            p++;
        }
        if(cnt < 33)
        {
            memset(local.fw_pack_md5, 0, sizeof(local.fw_pack_md5));
            memcpy(local.fw_pack_md5, md5_p, cnt);
            OSI_LOGXI(OSI_LOGPAR_S, 0, "[zk ota] get fwpkg_md5:%s", local.fw_pack_md5);
        }
    }
}

static void fota_get_fwpkg_url_1(void)
{
    Http_info *zk_http_ctx = RD_Http_Init();
    if(zk_http_ctx == NULL)
    {
        OSI_LOGE(0, "[zk http] http ctx malloc fail");
        return;
    }

    zk_http_ctx->cg_http_api->nCID = 1;
    strncpy(zk_http_ctx->url, uhome_url, strlen(uhome_url));
    strncpy(zk_http_ctx->content_type, uhome_content_type, strlen(uhome_content_type));
    strncpy(zk_http_ctx->body_content, body_content, strlen(body_content));
    zk_http_ctx->contentLen = strlen(body_content);

    mUpnpHttpResponse *response = NULL;

    if ((response = Http_post(zk_http_ctx)) == NULL)
    {
        OSI_LOGE(0, "[zk http] http post fail");
    }
    else
    {
        OSI_LOGXI(OSI_LOGPAR_S, 0, "%s", response->content->value);
    }
    RDHttpTerm(zk_http_ctx);
}

extern bool gContentTypeFlag;
static int8_t fota_get_fwpkg_url(void)
{
    int8_t result = -1;

    nHttp_info *zk_http_info = Init_Http();
    if(zk_http_info == NULL)
    {
        OSI_LOGE(0, "[zk ota] get_fwpkg_url_0:http ctx malloc fail");
        return -1;
    }

    zk_http_info->CID = 1;
    zk_http_info->cg_http_api->nCID = 1;
    strncpy(zk_http_info->url, uhome_url, sizeof(uhome_url));
    strncpy(zk_http_info->CONTENT_TYPE, uhome_content_type, sizeof(uhome_content_type));

    snprintf(body_content, 255, "{\"productName\":\"AIR_SE-A_CT1\",\"currentVersion\":\"%s\",\"extraInfo\":{}}", APP_VERSION);
    if(! http_setUserdata(zk_http_info, body_content, strlen(body_content)))
    {
        OSI_LOGE(0, "[zk ota] get_fwpkg_url_1:http set User data fail");

        goto exit;
    }
    gContentTypeFlag = true;
    if (Http_postn(zk_http_info))
    {
        char *value = zk_http_info->user_data;

        OSI_LOGXI(OSI_LOGPAR_IS, 0, "[zk ota] get_fwpkg_url_2:len=%d %s", zk_http_info->content_length,value);

        char *p = strstr(value, "\"resultCode\":");
        if(p !=NULL)
        {
            char code = *(p+strlen("\"resultCode\":")) -'0';
            if(code == 0)
            {
                //OSI_LOGI(0, "[zk ota] get_fwpkg_url_4:get a new upgrade pack ok");

                get_url(value);
                get_md5(value);
                
                result = 0;
                goto exit;
            }
            else
            {
                OSI_LOGE(0, "[zk ota] get_fwpkg_url_5:http post recv code error");
                goto exit;
            }
            
        }
        else
        {
            OSI_LOGE(0, "[zk ota] get_fwpkg_url_6:http post recv data error");
            goto exit;
        }
    }
    else
    {
        OSI_LOGE(0, "[zk ota] get_fwpkg_url_7:http postn fail");
        goto exit;
    }
exit:
    Term_Http(zk_http_info);
    return result;
}

static ZK_MD5_CTX md5_context={0};
bool zk_Http_getn_break(nHttp_info *http_info1)
{
    uint32_t readLen = 50 * 1000;
    uint32_t remain = 0;
    uint8_t count, i;
    char tmpString[30] = {0};

    uint32_t contentLen = (uint32_t)Http_headn_contentlength(http_info1);
    OSI_LOGI(0, "[ZK OTA] zk_Http_getn_break_0:%d", contentLen);
    if (contentLen > readLen)
    {
        count = contentLen / readLen;
        remain = contentLen % readLen;
        for (i = 0; i <= count; i++)
        {
            http_info1->BREAK = i * readLen;
            http_info1->BREAKEND = (i + 1) * readLen - 1;
            if (Http_getn(http_info1))
            {
                //sprintf(tmpString, "%d %d %d", i,count,(http_info1->BREAKEND-http_info1->BREAK));
                // Http_WriteUart(tmpString, strlen(tmpString));
                //Http_WriteUart(http_info1->user_data, http_info1->content_length);
                //OSI_LOGI(0, "[zk ota] zk_Http_getn_break_1:%d %d", (uint32_t)http_info1->data_length, (http_info1->BREAKEND-http_info1->BREAK));

                http_info1->breakFlag = true;
            }
            else
            {
                http_info1->breakFlag = false;
                return false;
            }
        }
        OSI_LOGI(0, "[zk ota] zk_Http_getn_break_1:%d", http_info1->content_length);
            
        MD5Update(&md5_context, http_info1->user_data, http_info1->content_length);

        int wsize = zk_fota_vfs_file_write(FUPDATE_PACK_FILE_NAME, http_info1->user_data, http_info1->content_length);
        if (wsize != http_info1->content_length)
        {
            OSI_LOGE(0, "[zk] zk_Http_getn_break_2:write file fail, errno %d", wsize);
            return false;
        }
        return true;
    }
    else if (contentLen < 0)
    {
        http_info1->content_length = 0;
        http_info1->status_code = 400;
        //sprintf(tmpString, "%d %d %ld", 0, http_info1->status_code, http_info1->content_length);
        //Http_WriteUart(tmpString, strlen(tmpString));
        //OSI_LOGXI(OSI_LOGPAR_S, 0, "[zk ota] zk_Http_getn_break:%s", tmpString);
        return false;
    }
    else
    {
        if(Http_getn(http_info1))
        {
            MD5Update(&md5_context, http_info1->user_data, http_info1->content_length);

            int wsize = zk_fota_vfs_file_write(FUPDATE_PACK_FILE_NAME, http_info1->user_data, http_info1->content_length);
            if (wsize != http_info1->content_length)
            {
                OSI_LOGE(0, "[zk] zk_Http_getn_break_3:write file fail, errno %d", wsize);
                return false;
            }
            return true;
        }
        else
        {
            return false;
        }
    }
}

//static char fwpkg_url_1[] = "https://haier-os.oss-cn-beijing.aliyuncs.com/16007426242898910_0922_a_b.pack";
static int8_t fota_download_1(void)
{
    int8_t result = -1;

    nHttp_info *zk_http_info = Init_Http();
    if(zk_http_info == NULL)
    {
        OSI_LOGE(0, "[zk ota] fota_download_1_0:http ctx malloc fail");
        return -1;
    }

    MD5Init(&md5_context);

    fupdateInvalidate(true);
    vfs_mkdir(CONFIG_FS_FOTA_DATA_DIR, 0);

    zk_http_info->CID = 1;
    zk_http_info->cg_http_api->nCID = 1;
    //strncpy(zk_http_info->url, fwpkg_url_1, strlen(fwpkg_url_1));
    strncpy(zk_http_info->url, local.fw_pack_url, strlen(local.fw_pack_url)+1);

    if(zk_Http_getn_break(zk_http_info))
    {
        OSI_LOGI(0, "[zk ota] fota_download_1_1: get fwpkg ok %d", zk_http_info->content_length);

        uint8_t out_data[16] = {0};
        MD5Final(&md5_context, out_data);
        //zk_debug(out_data, 16);
        
        char md51[33] = {0};
        uint8_t i=0,cnt=0;
        for(; i<16; i++)
        {
            cnt += snprintf(md51+cnt, 33-cnt, "%02x", out_data[i]);
        }
        OSI_LOGXI(OSI_LOGPAR_S, 0, "[zk http] http md51->%s", md51);
        if(strncmp(local.fw_pack_md5, md51, 32) == 0)
        {
            OSI_LOGI(0, "[zk http] http md5 cmp ok");

            zk_fota_update(NULL, 0);

            result = 0;
        }
        else
        {
            OSI_LOGE(0, "[zk http] http md5 cmp error");
        }
    }
    else
    {
        OSI_LOGE(0, "[zk ota] fota_download_1_2: get fwpkg error %d", zk_http_info->content_length);

        fupdateInvalidate(true);
    }
    Term_Http(zk_http_info);

    return result;
}

void ota_task(void *pParameter)
{
    TASK_MSG *msg;
    uint8_t get_fwpkg_url_cnt = 0;
    while(1)
    {
        if(xQueueReceive(fota_event_queue, &msg, portMAX_DELAY) == pdPASS)
		{
            switch(msg->id)
            {
                case FOTA_GET_FWPKG_URL_MSG:
                    while((fota_get_fwpkg_url() != 0))
                    {
                        if(get_fwpkg_url_cnt < FOTA_GET_FWPKG_CNT_MAX)
                            get_fwpkg_url_cnt++;
                        else
                            break;
                        vTaskDelay(osiMsToOSTick(get_fwpkg_url_cnt*(30*1000)));
                    }
                    if(get_fwpkg_url_cnt < FOTA_GET_FWPKG_CNT_MAX)
                    {
                        local.fota_flag = (uint8_t)FOTA_DOWNLOAD;
		                local.fota_fail_ret_num = 0;
		                write_local_cfg_Info();
                    }
                    else
                    {
                        OSI_LOGE(0, "[zk ota] get fwpfg url Fail fail_cnt=%d", local.fota_fail_ret_num);
                        if(local.fota_flag != (uint8_t)FOTA_GET_FWPKG_URL)
                        {
                            local.fota_flag = (uint8_t)FOTA_GET_FWPKG_URL;
                            local.fota_fail_ret_num = 0;
                            write_local_cfg_Info();
                        }
                    }
                    restart(6);
                    break;
                case FOTA_DOWNLOAD_MSG:
                    led_fota_change(LED_FOTA_START);
                    //创建一个软件定时器，用来控制http下载，超时没下载成功就强制复位
                    fota_download_Timers = xTimerCreate("fota_download_Timer", pdMS_TO_TICKS(FOTA_DOWNLOAD_MAX_TIME), pdFALSE, NULL, fota_download_SoftTimerCallback);
                    if(fota_download_Timers == NULL)
                        OSI_LOGE(0, "[zk] fota_download_Timers Created Fail");
                    else
	                    xTimerStart(fota_download_Timers, 0);

                    if(fota_download_1() == 0)
                    {
                        local.fota_flag = FOTA_NULL;
                        local.fota_fail_ret_num = 0;
                        write_local_cfg_Info();
                    }
                    led_fota_change(LED_FOTA_STOP);
                    restart(6);
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
