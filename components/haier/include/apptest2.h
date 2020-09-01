#ifndef _APP_TEST2_H_
#define _APP_TEST2_H_

#define OSI_LOG_TAG OSI_MAKE_LOG_TAG('Z', 'K', 'D', 'B')
#define OSI_LOCAL_LOG_LEVEL OSI_LOG_LEVEL_VERBOSE

#include "osi_log.h"
#include "osi_api.h"

unsigned char DataAccumulateSumCRC1(void *DataBuff, unsigned int DataLen);

#endif