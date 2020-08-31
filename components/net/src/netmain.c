/* Copyright (C) 2016 RDA Technologies Limited and/or its affiliates("RDA").
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
#include "osi_api.h"
#include "osi_log.h"
#include "cfw_dispatch.h"
#include "cfw.h"
#include "cfw_event.h"
#include "tcpip.h"
#include "lwipopts.h"
#include "netmain.h"
#include "ppp_interface.h"
#include "netdev_interface.h"
#include "sockets.h"
#if IP_NAT
#include "lwip/ip4_nat.h"
#endif

osiThread_t *netThreadID = NULL;

extern void tcpip_thread(void *arg);

#ifdef CONFIG_SOC_8910
extern int vnet4gSelfRegister(uint8_t nCid, uint8_t nSimId);
#endif

#ifdef CONFIG_SOC_8910
#define NET_STACK_SIZE 8192 * 4
#else
#define NET_STACK_SIZE 8192 * 2
#endif

extern struct netif *TCPIP_nat_lan_lwip_netif_create(uint8_t nCid, uint8_t nSimId);
extern void TCPIP_nat_lan_lwip_netif_destory(uint8_t nCid, uint8_t nSimId);
extern struct netif *TCPIP_nat_wan_netif_create(uint8_t nCid, uint8_t nSimId);
extern void TCPIP_nat_wan_netif_destory(uint8_t nCid, uint8_t nSimId);

static void net_thread(void *arg)
{

    for (;;)
    {
        osiEvent_t event = {};
        osiEventWait(osiThreadCurrent(), &event);
        if (event.id == 0)
            continue;
        OSI_LOGI(0x1000752b, "Netthread get a event: 0x%08x/0x%08x/0x%08x/0x%08x", (unsigned int)event.id, (unsigned int)event.param1, (unsigned int)event.param2, (unsigned int)event.param3);

        OSI_LOGI(0x1000752c, "Netthread switch");
        if ((!cfwIsCfwIndicate(event.id)) && (cfwInvokeUtiCallback(&event)))
        {
            ; // handled by UTI
        }
        else
        {
            CFW_EVENT *cfw_event = (CFW_EVENT *)&event;
            switch (event.id)
            {
            case EV_INTER_APS_TCPIP_REQ:
            {
                struct tcpip_msg *msg;
                msg = (struct tcpip_msg *)event.param1;
                tcpip_thread(msg);
                break;
            }
            case EV_TCPIP_CFW_GPRS_ACT:
            {
                uint8_t nCid, nSimId;
                nCid = event.param1;
                nSimId = cfw_event->nFlag;
#if IP_NAT
                if (get_nat_enabled(nSimId, nCid))
                {
                    TCPIP_nat_wan_netif_create(nCid, nSimId);
                    TCPIP_nat_lan_lwip_netif_create(nCid, nSimId);
                }
                else
                {
#endif
                    TCPIP_netif_create(nCid, nSimId);
#if IP_NAT
                }
#endif
#ifdef CONFIG_SOC_8910
                if (netdevIsConnected())
                {
                    netdevNetUp();
                }
                vnet4gSelfRegister(nCid, nSimId); //dianxin4G��ע��
#endif
                break;
            }
            case EV_TCPIP_CFW_GPRS_DEACT:
            {
                uint8_t nCid, nSimId;
                nCid = event.param1;
                nSimId = cfw_event->nFlag;
#ifdef CONFIG_SOC_8910
                if (netdevIsConnected())
                {
                    netdevNetDown(nSimId, nCid);
                }
#endif

#if IP_NAT
                if (get_nat_enabled(nSimId, nCid))
                {
                    TCPIP_nat_lan_lwip_netif_destory(nCid, nSimId);
                    TCPIP_nat_wan_netif_destory(nCid, nSimId);
                }
                else
                {
#endif
                    TCPIP_netif_destory(nCid, nSimId);
#if IP_NAT
                }
#endif

                break;
            }
#ifdef CONFIG_SOC_8910
            case EV_TCPIP_ETHCARD_CONNECT:
            {
                netdevConnect();
                break;
            }
            case EV_TCPIP_ETHCARD_DISCONNECT:
            {
                netdevDisconnect();
                break;
            }
#endif
            default:
                break;
            }
        }
    }
}

osiThread_t *netGetTaskID()
{
    return netThreadID;
}

void net_init()
{

    netThreadID = osiThreadCreate("net", net_thread, NULL, OSI_PRIORITY_NORMAL, NET_STACK_SIZE, 64);

    tcpip_init(NULL, NULL);

#if IP_NAT
    ip4_nat_init();
#endif
}
