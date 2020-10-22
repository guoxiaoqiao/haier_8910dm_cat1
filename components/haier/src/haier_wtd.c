#include "haier_wtd.h"

void wdg_disable(void);
void wdg_reset(void);
void wdg_feed(void);
void wdg_init(void);

drvGpio_t *wdt_wdi;
drvGpio_t *wdt_rest;

void wdg_disable(void)
{
    drvGpioWrite(wdt_rest, true);
}

void wdg_reset(void)
{
	drvGpioWrite(wdt_rest, true);
	vTaskDelay(osiMsToOSTick(100)); 
	drvGpioWrite(wdt_rest, false);
}

void wdg_feed(void)
{
	drvGpioWrite(wdt_wdi, false);
	vTaskDelay(osiMsToOSTick(1000)); 
	drvGpioWrite(wdt_wdi, true);  //鍠傜嫍涔嬪悗锛屽氨鎶婄數骞虫媺鍥炴潵
}

void wdg_init(void)
{
	drvGpioConfig_t config = {0};
    config.mode = DRV_GPIO_OUTPUT;
    config.out_level = true;
    //wdt_wdi初始化为输出模式，默认高电平
    drvGpio_t *drv_wdi = drvGpioOpen(WTD_WDI_PIN, &config, NULL, NULL);
    if(drv_wdi != NULL)
    {
        wdt_wdi = drv_wdi;
        //OSI_LOGI(0, "[zk wdt] wdg_init_0 wdt_wdi init suc");
    }
    else
    {
        OSI_LOGE(0, "[zk wdt] wdg_init_0 wdt_wdi init error");
    }
    
    //wdt_rest初始化为输出模式，默认低电平
    config.out_level = false;
    drvGpio_t *drv_rest = drvGpioOpen(WTD_REST_IN_PIN, &config, NULL, NULL);
    if(drv_rest != NULL)
    {
        wdt_rest = drv_rest;
        //OSI_LOGI(0, "[zk wdt] wdg_init_1 wdt_rest init suc");
    }
    else
    {
        OSI_LOGE(0, "[zk wdt] wdg_init_1 wdt_rest init error");
    }
    
}
