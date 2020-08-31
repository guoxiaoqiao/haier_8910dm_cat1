/* Copyright (C) 2018 RDA Technologies Limited and/or its affiliates("RDA").
 * All rights reserved.
 *
 * This software is supplied "AS IS" without any warranties.
 * RDA assumes no responsibility or liability for the use of the software,
 * conveys no license or title under any patent, copyright, or mask work
 * right to the product. RDA reserves the right to make changes in the
 * software without notification.  RDA also make no representation or
 * warranty that such application will be suitable for the specified use
 * without further testing or modification.
 */

#define OSI_LOCAL_LOG_LEVEL OSI_LOG_LEVEL_DEBUG

#include "osi_log.h"
#include "osi_api.h"
#include "osi_internal.h"
#include "cmsis_core.h"
#include "hwregs.h"
#include <stdlib.h>

void osiIdleHook(void)
{
}

void osiIdleSleep(uint32_t idle_tick)
{
    osiPmSleep(idle_tick);
}
