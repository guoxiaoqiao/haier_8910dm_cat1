/*
 * mempool.h
 *
 *  Created on: 2019��11��28��
 *      Author: Zeecy.Zhao
 */

#ifndef MIDDLEWARE_UNISOC_BLUETOOTH_BLUEU_HAL_FREERTOS_OSI_INCLUDE_OSI_MEMPOOL_H_
#define MIDDLEWARE_UNISOC_BLUETOOTH_BLUEU_HAL_FREERTOS_OSI_INCLUDE_OSI_MEMPOOL_H_

#include <stdlib.h>
void osi_mempool_free( void *pv );
void osi_mempool_init(size_t size);
void *osi_mempool_malloc(size_t size);
void osi_mempool_deinit();
#endif /* MIDDLEWARE_UNISOC_BLUETOOTH_BLUEU_HAL_FREERTOS_OSI_INCLUDE_OSI_MEMPOOL_H_ */
