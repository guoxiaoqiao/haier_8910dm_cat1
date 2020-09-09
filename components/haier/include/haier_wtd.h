#ifndef _HAIER_WTD_H_
#define _HAIER_WTD_H_

#include "haier_appmain.h"

#define WTD_REST_IN_PIN		17
#define	WTD_WDI_PIN			16

extern void wdg_reset(void);
extern void wdg_feed(void);
extern void wdg_init(void);

#endif