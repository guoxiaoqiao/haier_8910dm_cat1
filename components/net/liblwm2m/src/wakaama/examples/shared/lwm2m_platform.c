/*******************************************************************************
 *
 * Copyright (c) 2013, 2014, 2015 Intel Corporation and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * The Eclipse Distribution License is available at
 *    http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    David Navarro, Intel Corporation - initial API and implementation
 *******************************************************************************/

#include <liblwm2m.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/time.h>
#include "osi_api.h"
#include "internals.h"
#include "osi_log.h"

#ifndef LWM2M_MEMORY_TRACE

void * lwm2m_malloc(size_t s)
{
    char * ret;
    
    //zhangyi del for porting 20180710
    //UINT32 nCallerAdd;
    //COS_GET_RA(&nCallerAdd);

    ret = malloc(s);
    if(ret == NULL){
        LOG("malloc failed");
        return NULL;}
    memset(ret,0,s);
    
    //zhangyi del for porting 20180710
    //COS_UpdateMemCallAdd(ret,nCallerAdd);
    
    return ret;
}

void lwm2m_free(void * p)
{
    free(p);
}

char * lwm2m_strdup(const char * str)
{
    char *cpStrBuf;
    cpStrBuf = (char *)lwm2m_malloc(strlen(str)+1);
    if ( NULL != cpStrBuf ) {
        strcpy(cpStrBuf, str);
    }
    return cpStrBuf;
}

#endif

int lwm2m_strncmp(const char * s1,
                     const char * s2,
                     size_t n)
{
    return strncmp(s1, s2, n);
}

time_t lwm2m_gettime(void)
{
    struct timeval tv;

    if (0 != gettimeofday(&tv, NULL))
    {
        return -1;
    }

    return tv.tv_sec;
}

void lwm2m_printf(const char * format, ...)
{
    char uart_buf[256];
    va_list ap;

    va_start (ap, format);
    vsnprintf(uart_buf, sizeof(uart_buf), format, ap);
    va_end (ap);

    OSI_LOGI(0x0, uart_buf);
}
