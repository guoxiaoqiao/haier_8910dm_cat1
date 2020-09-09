#include "haier_led.h"

drvGpio_t *red_led;
drvGpio_t *green_led;
drvGpio_t *blue_led;

static void led_init(void)
{
    //drvGpioInit();

    drvGpioConfig_t config = {0};
    config.mode = DRV_GPIO_OUTPUT;
    config.out_level = true;
    //红灯初始化为输出模式，默认高电平
    drvGpio_t *drv_led_mode = drvGpioOpen(LED_MODE_PIN, &config, NULL, NULL);
    if(drv_led_mode != NULL)
    {
        red_led = drv_led_mode;
        OSI_LOGI(0, "[zk led] gpio_Init_2 red led init suc");
    }
    //绿灯初始化为输出模式，默认高电平
    drvGpio_t *drv_led_net_ststus = drvGpioOpen(LED_NET_STATUS_PIN, &config, NULL, NULL);
    if(drv_led_net_ststus != NULL)
    {
        green_led = drv_led_net_ststus;
        OSI_LOGI(0, "[zk led] gpio_Init_2 green led init suc");
    }
    //蓝灯初始化为输出模式，默认高电平
    drvGpio_t *drv_led_ststus = drvGpioOpen(LED_STATUS_PIN, &config, NULL, NULL);
    if(drv_led_ststus != NULL)
    {
        blue_led = drv_led_ststus;
        OSI_LOGI(0, "[zk led] gpio_Init_2 blud led init suc");
    }
    //上电3个灯常亮2s，然后常灭。（用于工厂工人查看灯是否良好）
    set_sys_state(SYS_STATE_POWN);
	vTaskDelay(osiMsToOSTick(2000)); 
	LED_MODE_OFF;
	LED_STATUS_OFF;
    LED_STATUS_OFF;
}

//static uint8_t LEDState_ChangeFlag;
void LEDState_Change(uint8_t FlashCnt, uint16_t time)
{
	uint8_t i;
	
	for(i=0; i<FlashCnt; i++)
	{
		LED_STATUS_ON;
		vTaskDelay(osiMsToOSTick(time)); 
		LED_STATUS_OFF;
		if(FlashCnt > 1)
		{
			vTaskDelay(osiMsToOSTick(time)); 
		}
	}
}

static void led_task_main_handle(void)
{
	switch((uint32_t)get_sye_state())
	{
		case SYS_STATE_POWN:
			set_sys_state(SYS_STATE_NETWORK_CONNECT);
			OSI_LOGI(0, "[zk led] led_task_main_0:enter network connect");
			break;
		case SYS_STATE_NETWORK_CONNECT:
			LED_MODE_ON;
			vTaskDelay(osiMsToOSTick(300)); 
			LED_MODE_OFF;
			vTaskDelay(osiMsToOSTick(300)); 
            //OSI_LOGI(0, "[zk led] led_task_main_1:network connect...");
			break;
        case SYS_STATE_REG:
            LED_MODE_ON;
			vTaskDelay(osiMsToOSTick(3000)); 
			LED_MODE_OFF;
			vTaskDelay(osiMsToOSTick(3000)); 
            //OSI_LOGI(0, "[zk led] led_task_main_1:reg server...");
            break;
		case SYS_STATE_RUN:
			LED_MODE_ON;
			vTaskDelay(osiMsToOSTick(1000)); 
			LED_MODE_OFF;
			vTaskDelay(osiMsToOSTick(1000)); 
			break;
		case SYS_STATE_FOTA:
			LED_STATUS_ON;
			LED_MODE_OFF;
			vTaskDelay(osiMsToOSTick(500)); 
			LED_STATUS_OFF;
			vTaskDelay(osiMsToOSTick(500)); 
			OSI_LOGI(0, "[zk led] led_task_main_3 sys fota...");
			break;
		default:
			OSI_LOGE(0, "[zk led] led_task_main_4 sys status error");
			vTaskDelay(osiMsToOSTick(1000)); 
			break;
	}
}

void led_task_main(void *pParameter)
{
	led_init();
	while(1)
	{
		led_task_main_handle();
	}
}