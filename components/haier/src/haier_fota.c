#include "haier_appmain.h"
#include "http_api.h"
#include "http_download.h"
#include "fupdate.h"
#include "zk_md5.h"

#define FOTA_DOWNLOAD_MAX_TIME     (5*60*1000)

extern void led_fota_change(uint8_t typ);

static TimerHandle_t fota_download_Timers;

static char content_type[] = "application/json";
//static char body_content[] = "{\"productName\":\"AIR_SE-A_CT1\",\"currentVersion\":\"2.2.3\",\"extraInfo\":{}}";
static char url[] = "http://os.uhome.haier.net/osfota/app/package/v3";

static char *fwpkg_md5;
static char *fwpkg_url;

static void fota_download_SoftTimerCallback(void *argument)
{
    led_fota_change(LED_FOTA_STOP);	
	restart(7);
}

static void zk_fota_update(uint8_t *data, uint32_t len)
{
    if((data == NULL) || (len == 0))
    {
        OSI_LOGI(0, "[zk fota] fota_data_cpy_0: param fail %d", len);
        return;
    }
    OSI_LOGI(0, "[zk fota] zk_fota_data_cpy_1: len=%d", len);
    /*if(len > 100)
        zk_debug(data, 100);
    else
        zk_debug(data, (uint16_t)len);*/
    
    fupdateInvalidate(true);
    vfs_mkdir(CONFIG_FS_FOTA_DATA_DIR, 0);
    int wsize = vfs_file_write(FUPDATE_PACK_FILE_NAME, data, len);
    if (wsize != len)
    {
        OSI_LOGE(0, "[zk] Fota Error : write file fail, errno %d", wsize);
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

        led_fota_change(LED_FOTA_STOP);	

        local.fota_flag = 0;
        local.fota_fail_ret_num = 0;

        write_local_cfg_Info();
       
        osiShutdown(OSI_SHUTDOWN_RESET);
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
        if(fwpkg_url != NULL)
        {
            free(fwpkg_url);
            fwpkg_url = NULL;
        }
        fwpkg_url = calloc(1, cnt+1);
        if(fwpkg_url != NULL)
        {
            memcpy(fwpkg_url, f_p, cnt);
            OSI_LOGXI(OSI_LOGPAR_S, 0, "[zk ota] get fwpkg_url:%s", fwpkg_url);
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
        if(fwpkg_url != NULL)
        {
            free(fwpkg_md5);
            fwpkg_md5 = NULL;
        }
        fwpkg_md5 = calloc(1, cnt+1);
        if(fwpkg_md5 != NULL)
        {
            memcpy(fwpkg_md5, md5_p, cnt);
            OSI_LOGXI(OSI_LOGPAR_S, 0, "[zk ota] get md5:%s", fwpkg_md5);
        }
    }
}

extern uint8_t* zk_http_response_print(mUpnpHttpPacket *httpPkt, uint32_t *buff_len);
static int8_t fota_get_fwpkg_url(void)
{
    int8_t result = -1;

    Http_info *zk_http_ctx = RD_Http_Init();
    if(zk_http_ctx == NULL)
    {
        OSI_LOGI(0, "[zk ota] http ctx malloc fail");
        return -1;
    }

    OSI_LOGI(0, "[zk ota] http ctx init suc");

    zk_http_ctx->cg_http_api->nCID = 1;
    strncpy(zk_http_ctx->url, url, strlen(url));
    strncpy(zk_http_ctx->content_type, content_type, strlen(content_type));

    snprintf(zk_http_ctx->body_content, 255, "{\"productName\":\"AIR_SE-A_CT1\",\"currentVersion\":\"%s\",\"extraInfo\":{}}", APP_VERSION);
    OSI_LOGXI(OSI_LOGPAR_S, 0, "%s", zk_http_ctx->body_content);
    //strncpy(zk_http_ctx->body_content, body_content, strlen(body_content));
    zk_http_ctx->contentLen = strlen(zk_http_ctx->body_content);

    mUpnpHttpResponse *response = NULL;

    if ((response = Http_post(zk_http_ctx)) == NULL)
    {
        OSI_LOGE(0, "[zk ota] http post fail");

        goto exit;
    }
    else
    {
        OSI_LOGI(0, "[zk ota] http post suc");

        OSI_LOGXI(OSI_LOGPAR_S, 0, "%s", response->content->value);

        char *value = response->content->value;

        char *p = strstr(value, "\"resultCode\":");
        if(p !=NULL)
        {
            char code = *(p+strlen("\"resultCode\":")) -'0';
            if(code == 0)
            {
                OSI_LOGI(0, "[zk ota] get a new upgrade pack ok");

                get_url(value);
                get_md5(value);
                
                result = 0;
            }
            else
            {
                OSI_LOGE(0, "[zk ota] http post recv code error");
                goto exit;
            }
            
        }
        else
        {
            OSI_LOGE(0, "[zk ota] http post recv data error");
            goto exit;
        }
    }
exit:
    if(response != NULL)
    {
        //http_response_print((mUpnpHttpPacket *)response);
        mupnp_http_response_clear(response);

        mupnp_http_response_delete(response);
    }
    if(RDHttpTerm(zk_http_ctx) == true)
    {
        OSI_LOGI(0, "[zk ota] http ctx free suc 1");
    }
    return result;
}

static int8_t fota_download(void)
{
    if(fwpkg_url != NULL)
    {
        Http_info *zk_http_ctx = RD_Http_Init();
        if(zk_http_ctx == NULL)
        {
            OSI_LOGI(0, "[zk ota] http ctx malloc fail 1");
            //free(fwpkg_url);
            return -1;
        }

        OSI_LOGI(0, "[zk ota] http ctx init suc 1");

        zk_http_ctx->cg_http_api->nCID = 1;
        strncpy(zk_http_ctx->url, fwpkg_url, strlen(fwpkg_url));

        mUpnpHttpResponse *response = NULL;
        if ((response = Http_get(zk_http_ctx)) == NULL)
        {
            OSI_LOGE(0, "[zk ota] http GET fwpkg error");
            goto exit;
        }
        else
        {
           //http_response_print((mUpnpHttpPacket *)response);

           uint32_t contentLen = 0;
           uint8_t *content = zk_http_response_print((mUpnpHttpPacket *)response, &contentLen);
           OSI_LOGI(0, "[zk ota] http GET fwpkg ok len=%d", contentLen);
           //zk_debug((uint8_t *)(content+90), (uint32_t)(contentLen - 90));

           //uint8_t content1 = (uint8_t *)response->content->value;
           uint32_t contentLen1 = (uint32_t)response->content->valueSize;

           OSI_LOGI(0, "[zk ota] http GET ok len_1=%d", contentLen1);

           ZK_MD5_CTX http_context={0};
           MD5Init(&http_context);

           MD5Update(&http_context, content, contentLen);

           uint8_t out_data[16] = {0};
           MD5Final(&http_context, out_data);

           zk_debug(out_data, 16);
           
           char md51[33] = {0};
           uint8_t i=0,cnt=0;
           for(; i<16; i++)
           {
                cnt += snprintf(md51+cnt, 33-cnt, "%02x", out_data[i]);
           }
           OSI_LOGXI(OSI_LOGPAR_S, 0, "[zk ota] http md51->%s", md51);
           if(strncmp(fwpkg_md5, md51, 32) == 0)
           {
               OSI_LOGI(0, "[zk ota] http md5 cmp ok");

               zk_fota_update(content, contentLen);
           }
           free(fwpkg_url);
           free(fwpkg_md5);
           if(RDHttpTerm(zk_http_ctx) == true)
           {
                OSI_LOGI(0, "[zk ota] http free suc 1");
           }
        }
        return 0;
        exit:
            if(response != NULL)
            {
                //http_response_print((mUpnpHttpPacket *)response);
                mupnp_http_response_clear(response);

            // mupnp_http_response_delete(response);
            }

            if(RDHttpTerm(zk_http_ctx) == true)
            {
                OSI_LOGI(0, "[zk ota] http ctx free suc 1");
            }
            return -1;
    }
    else
    {
        OSI_LOGE(0, "[zk ota] http fwpkg_url is null");
        return -1;
    }
}

void ota_task(void *pParameter)
{
    uint8_t fota_get_url_cnt = 0;
    uint8_t fota_download_cnt = 0;
    TASK_MSG *msg;
    while(1)
    {
        if(xQueueReceive(fota_event_queue, &msg, portMAX_DELAY) == pdPASS)
		{
            switch(msg->id)
            {
                case FOTA_START_MSG:
                    OSI_LOGI(0, "[zk ota] fota start");
                    
                    led_fota_change(LED_FOTA_START);
                    
                    //创建一个软件定时器，用来控制http下载，超时没下载成功就强制复位
                    fota_download_Timers = xTimerCreate("fota_download_Timer", pdMS_TO_TICKS(FOTA_DOWNLOAD_MAX_TIME), pdFALSE, NULL, fota_download_SoftTimerCallback);
                    if(fota_download_Timers == NULL)
                    {
                        OSI_LOGE(0, "[zk] fota_download_Timers Created Fail");
                    }
                    else
                    {
	                    xTimerStart(fota_download_Timers, 0);
                    }
                    
                    while (fota_get_url_cnt < 3)
                    {
                        if(fota_get_fwpkg_url() == 0)
                        {
                            while((fota_download_cnt < 3) && (fota_download() != 0))
                            {
                                fota_download_cnt++;
                                OSI_LOGE(0, "[zk ota] download fwpkg error download_cnt=%d", fota_download_cnt);
                                vTaskDelay(osiMsToOSTick(10000));
                            }
                            if(fota_download_cnt < 3)
                                break;
                        }
                        else
                        {
                            fota_get_url_cnt++;
                            OSI_LOGE(0, "[zk ota] get fwpkg url error download_cnt=%d", fota_get_url_cnt);
                            vTaskDelay(osiMsToOSTick(10000));
                        }
                    }
                    led_fota_change(LED_FOTA_STOP);
                    //restart(6);
                    break;
                case FOTA_UPDATE_MSG:
                    OSI_LOGI(0, "[zk ota] fota update");
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
