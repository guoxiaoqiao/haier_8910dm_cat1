#ifndef _HAIER_LED_H_
#define _HAIER_LED_H_

#include "haier_appmain.h"

#define LED_CONTROL(LEDx, LEVE)    drvGpioWrite(LEDx, LEVE)

//红灯
#define LED_MODE_PIN        13
#define LED_MODE_OFF        LED_CONTROL(red_led, false)
#define LED_MODE_ON         LED_CONTROL(red_led, true)
//绿灯
#define LED_NET_STATUS_PIN 	22
#define LED_NET_STATUS_OFF  LED_CONTROL(green_led, false)
#define LED_NET_STATUS_ON   LED_CONTROL(green_led, true)
//蓝灯
#define LED_STATUS_PIN      18 
#define LED_STATUS_OFF      LED_CONTROL(blue_led, false)
#define LED_STATUS_ON       LED_CONTROL(blue_led, true)

#endif