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

#define OSI_LOCAL_LOG_TAG OSI_MAKE_LOG_TAG('N', 'D', 'E', 'V')
#define OSI_LOCAL_LOG_LEVEL OSI_LOG_LEVEL_INFO

#include "cfw.h"
#include "osi_api.h"
#include "osi_log.h"
#include "lwip/netif.h"
#include "lwip/dns.h"
#include "lwip/tcpip.h"
#include "lwip/ip.h"

#include "drv_ps_path.h"
#include "netdev_interface_imp.h"
#include "netdev_interface_nat_lan.h"
#include "drv_ether.h"
#include "hal_chip.h"
#include "lwip/prot/ip.h"
#include "lwip/prot/icmp6.h"

#if IP_NAT
#include "lwip/ip4_nat.h"
#include "netutils.h"

static ip4_nat_entry_t ndev_nat_entry;

extern osiThread_t *netGetTaskID();
extern bool isDhcpPackage(drvEthPacket_t *pkt, uint32_t size);
extern void dhcps_reply(drvEther_t *ether, drvEthPacket_t *pkt, uint32_t size, struct netif *netif);
extern bool isARPPackage(drvEthPacket_t *pkt, uint32_t size);
extern void ARP_reply(drvEther_t *ether, drvEthPacket_t *pkt, uint32_t size, struct netif *netif);
extern bool isRouterSolicitationPackage(drvEthPacket_t *pkt, uint32_t size);
extern void routerAdvertisement_reply(drvEther_t *ether, drvEthPacket_t *pkt, uint32_t size, struct netif *netif);
extern bool isRouterAdvertisementPackage(drvEthPacket_t *pkt, uint32_t size);
extern bool isNeighborSolicitationPackage(drvEthPacket_t *pkt, uint32_t size);
extern void neighborAdvertisement_reply(drvEther_t *ether, drvEthPacket_t *pkt, uint32_t size, struct netif *netif);
extern u8_t ip4_wan_forward(struct pbuf *p, struct netif *inp);

static bool prvNdevLanDataToPs(netSession_t *session, const void *data, size_t size);
static netSession_t *prvNdevLanSessionCreate(uint8_t sim, uint8_t cid);
static bool prvNdevLanSessionDelete(netSession_t *session);

extern netdevIntf_t gNetIntf;
extern u8_t netif_num;
static void prvEthLanUploadDataCB(drvEthPacket_t *pkt, uint32_t size, void *ctx)
{
    netdevIntf_t *nc = (netdevIntf_t *)ctx;
    netSession_t *session = nc->session;

    if (session == NULL)
    {
        OSI_LOGE(0x1000756c, "upload data no session");
        return;
    }

    if (size > ETH_HLEN)
    {
        //OSI_LOGE(0x1000756d, "prvEthUploadDataCB data size: %d", size - ETH_HLEN);
        uint8_t *ipData = pkt->data;
        sys_arch_dump(ipData, size - ETH_HLEN);
#ifdef CONFIG_NET_TRACE_IP_PACKET
        uint8_t *ipdata = ipData;
        uint16_t identify = (ipdata[4] << 8) + ipdata[5];
        OSI_LOGI(0x0, "prvEthUploadDataCB data size: %d identify %04x", size - ETH_HLEN, identify);
#endif
    }

    if (session->dev_netif != NULL && isARPPackage(pkt, size)) //drop ARP package
    {
        ARP_reply(nc->ether, pkt, size, session->dev_netif);
        return;
    }

    if (session->dev_netif != NULL && isNeighborSolicitationPackage(pkt, size))
    {
        neighborAdvertisement_reply(nc->ether, pkt, size, session->dev_netif);
        return;
    }

    bool isDHCPData = isDhcpPackage(pkt, size);
    if (session->dev_netif != NULL && isDHCPData)
    {
        dhcps_reply(nc->ether, pkt, size, session->dev_netif);
    }
    else
    {
        prvNdevLanDataToPs(session, pkt->data, size - ETH_HLEN);
    }
}

static err_t
wan_to_netdev_lan_datainput(struct pbuf *p, struct netif *nif)
{
    uint16_t offset = 0;
    struct pbuf *q = NULL;
    netdevIntf_t *nc = &gNetIntf;
    drvEthReq_t *tx_req;
    uint32_t size = 0;
    while ((tx_req = drvEtherTxReqAlloc(nc->ether)) == NULL)
    {
        osiThreadSleep(1);
        if (nc->ether == NULL)
            return -1;
    }
    uint8_t *pData = tx_req->payload->data;
    size = p->tot_len;

    for (q = p; q != NULL; q = q->next)
    {
        memcpy(&pData[offset], q->payload, q->len);
        offset += q->len;
    }

    sys_arch_dump(pData, p->tot_len);

    nif->u32RndisDLSize += p->tot_len;
#ifdef CONFIG_NET_TRACE_IP_PACKET
    uint8_t *ipdata = pData;
    uint16_t identify = (ipdata[4] << 8) + ipdata[5];
    OSI_LOGI(0x0, "Wan to NC identify %04x", identify);
#endif
    OSI_LOGE(0x0, "Wan to NC %d", p->tot_len);
    pbuf_free(p);
    if (!drvEtherTxReqSubmit(nc->ether, tx_req, size))
    {
        drvEtherTxReqFree(nc->ether, tx_req);
        OSI_LOGW(0x0, "Wan to NC error %d", size);
        return -1;
    }
    return 0;
}

static bool prvNdevLanDataToPs(netSession_t *session, const void *data, size_t size)
{
    struct netif *inp_netif = session->dev_netif;
    if (inp_netif == NULL)
        return false;

    if (size <= 0)
        return false;

    struct pbuf *p, *q;
    int offset = 0;
    uint8_t *pData = (uint8_t *)data;
    p = (struct pbuf *)pbuf_alloc(PBUF_RAW, size, PBUF_POOL);
    if (p != NULL)
    {
        if (size > p->len)
        {
            for (q = p; size > q->len; q = q->next)
            {
                memcpy(q->payload, &(pData[offset]), q->len);
                size -= q->len;
                offset += q->len;
            }
            if (size != 0)
            {
                memcpy(q->payload, &(pData[offset]), size);
            }
        }
        else
        {
            memcpy(p->payload, &(pData[offset]), size);
        }
#if LWIP_IPV6
        if (IP_HDR_GET_VERSION(p->payload) == 6)
        {
            struct ip6_hdr *ip6hdr;
            ip6hdr = (struct ip6_hdr *)p->payload;
            u8_t nexth = IP6H_NEXTH(ip6hdr);
            if (nexth == IP6_NEXTH_ICMP6)
            {
                struct icmp6_hdr *icmp6hdr;
                uint8_t *ip6Data = p->payload;
                icmp6hdr = (struct icmp6_hdr *)(ip6Data + 40);
                OSI_LOGI(0x0, "prvNdevLanDataToPs get IPV6 ICMP6");
                if (icmp6hdr->type == ICMP6_TYPE_RS)
                {
                    //save IPV6 local addr to lan netif
                    ip6_addr_t src_ip6 = {0};
                    ip6_addr_copy_from_packed(src_ip6, ip6hdr->src);
                    netif_ip6_addr_set(inp_netif, 0, &src_ip6);
                    OSI_LOGI(0x0, "prvNdevLanDataToPs get IPV6 local addr");
                }
            }
            //find Wan netif with same SimCid to send IPV6 packeage out
            struct netif *Wannetif = netif_get_by_cid_type(inp_netif->sim_cid, NETIF_LINK_MODE_NAT_WAN);
            if (Wannetif)
            {
                OSI_LOGI(0x0, "prvNdevLanDataToPs IPV6 to Wan netif");
#if LWIP_TCPIP_CORE_LOCKING
                LOCK_TCPIP_CORE();
#endif
                Wannetif->output_ip6(Wannetif, p, NULL);
#if LWIP_TCPIP_CORE_LOCKING
                UNLOCK_TCPIP_CORE();
#endif
            }
        }
        else
#endif
        {
#if LWIP_TCPIP_CORE_LOCKING
            LOCK_TCPIP_CORE();
#endif

            u8_t taken = ip4_wan_forward(p, inp_netif);
            if (taken == 0)
                taken = ip4_nat_out(p);
#if LWIP_TCPIP_CORE_LOCKING
            UNLOCK_TCPIP_CORE();
#endif
            OSI_LOGI(0x0, "prvNdevLanDataToPs %d", taken);
        }
        inp_netif->u32RndisULSize += p->tot_len;
        pbuf_free(p);
    }
    return true;
}

static void prvProcessNdevLanConnect(void *par)
{
    netdevIntf_t *nc = (netdevIntf_t *)par;
    OSI_LOGI(0x0, "prvProcessNdevLanConnect (ether %p, session %p)", nc->ether, nc->session);
    if (nc->ether == NULL)
        return;

    if (nc->session == NULL && netif_default != NULL)
    {
        uint8_t nCid = netif_default->sim_cid & 0x0f;
        uint8_t nSimId = netif_default->sim_cid >> 4;
        nc->session = prvNdevLanSessionCreate(nSimId, nCid);
    }

    if (nc->session == NULL)
        return;

    drvEtherSetULDataCB(nc->ether, prvEthLanUploadDataCB, nc);
    if (!drvEtherOpen(nc->ether))
    {
        OSI_LOGI(0x10007575, "NC connect ether open fail");
        prvNdevLanSessionDelete(nc->session);
        nc->session = NULL;
    }
}

void netdevLanConnect()
{
    OSI_LOGI(0x0, "netdevLanConnect timer start.");
    netdevIntf_t *nc = &gNetIntf;
    if (nc->connect_timer != NULL)
        osiTimerDelete(nc->connect_timer);
    nc->connect_timer = osiTimerCreate(netGetTaskID(), prvProcessNdevLanConnect, nc);
    osiTimerStart(nc->connect_timer, 500);
}

void netdevLanDisconnect()
{
    netdevIntf_t *nc = &gNetIntf;
    drvEtherClose(nc->ether);
    prvNdevLanSessionDelete(nc->session);
    nc->session = NULL;
    if (nc->connect_timer != NULL)
    {
        osiTimerDelete(nc->connect_timer);
        nc->connect_timer = NULL;
    }
}

void netdevLanNetUp(uint8_t nSimID, uint8_t nCID)
{
    OSI_LOGI(0x10007577, "netdevNetUp");
    netdevIntf_t *nc = &gNetIntf;
    if (nc->ether == NULL)
        return;

    if (nc->session != NULL)
    {
        OSI_LOGI(0, "netdevLanNetUp Net session create already created");
        return;
    }
    nc->session = prvNdevLanSessionCreate(nSimID, nCID);

    if (nc->session == NULL)
        return;

    drvEtherSetULDataCB(nc->ether, prvEthLanUploadDataCB, nc);
    if (!drvEtherOpen(nc->ether))
    {
        OSI_LOGI(0x10007578, "NC net up ether open fail.");
        prvNdevLanSessionDelete(nc->session);
        nc->session = NULL;
        return;
    }
}

void netdevLanNetDown(uint8_t nSimID, uint8_t nCID)
{
    netdevIntf_t *nc = &gNetIntf;
    OSI_LOGI(0x10007579, "pre netdevNetDown");
    if ((nc->netdevIsConnected == true) && (nc->session != NULL) && (nc->session->is_created == true))
    {
        struct netif *inp_netif = nc->session->dev_netif;
        if ((inp_netif != NULL) && ((nSimID << 4 | nCID) == inp_netif->sim_cid))
        {
            OSI_LOGI(0x1000757a, "netdevNetDown");
            drvEtherClose(nc->ether);
            prvNdevLanSessionDelete(nc->session);
            nc->session = NULL;
        }
    }
}

static err_t nat_lan_netdev_data_output(struct netif *netif, struct pbuf *p,
                                        ip_addr_t *ipaddr)
{
    return 0;
}

static err_t netif_gprs_nat_lan_ndev_init(struct netif *netif)
{
    /* initialize the snmp variables and counters inside the struct netif
   * ifSpeed: no assumption can be made!
   */
    netif->name[0] = 'N';
    netif->name[1] = 'C';
    netif->mtu = GPRS_MTU;
#if LWIP_IPV4
    netif->output = (netif_output_fn)nat_lan_netdev_data_output;
#endif
    return ERR_OK;
}

netSession_t *prvNdevLanSessionCreate(uint8_t sim, uint8_t cid)
{
    static netSession_t s_net_nat_session;
    OSI_LOGI(0x1000757d, "Net session create");
    uint8_t T_cid = sim << 4 | cid;
    struct netif *wan_netif = netif_get_by_cid_type(T_cid, NETIF_LINK_MODE_NAT_WAN);
    if (wan_netif == NULL)
    {
        return NULL;
    }
    ip4_addr_t *wan_ip4_addr = (ip4_addr_t *)netif_ip4_addr(wan_netif);
    ip4_addr_t ip4;
    ip4_addr_t ip4_gw;
    OSI_LOGI(0x0, "prvNdevLanSessionCreate wan_ip4_addr 0x%x\n", wan_ip4_addr->addr);
    if ((wan_ip4_addr->addr & 0xff) != 0xc0) //192.xxx.xxx.xxx
    {
        IP4_ADDR(&ip4, 192, 168, cid, 2 + netif_num);
        IP4_ADDR(&ip4_gw, 192, 168, cid, 1);
    }
    else
    {
        IP4_ADDR(&ip4, 10, 0, cid, 2 + netif_num);
        IP4_ADDR(&ip4_gw, 10, 0, cid, 1);
    }
    ip4_addr_t ip4_netMast;
    IP4_ADDR(&ip4_netMast, 255, 255, 255, 0);

    uint32_t critical = osiEnterCritical();
    struct netif *netif = (struct netif *)calloc(1, sizeof(struct netif));
    CFW_GPRS_PDPCONT_INFO_V2 pdp_context;
    CFW_GprsGetPdpCxtV2(cid, &pdp_context, sim);
    netif->sim_cid = T_cid;
    netif->is_ppp_mode = 0;
    netif->link_mode = NETIF_LINK_MODE_NAT_NETDEV_LAN;
    netif->pdnType = pdp_context.PdnType;
    netif->is_used = 1;

    netif_add(netif, &ip4, &ip4_netMast, &ip4_gw, NULL, netif_gprs_nat_lan_ndev_init, wan_to_netdev_lan_datainput);
    netif_set_up(netif);
    netif_set_link_up(netif);

    ndev_nat_entry.in_if = netif;
    ndev_nat_entry.out_if = wan_netif;

    ip4_addr_copy(ndev_nat_entry.dest_net, *wan_ip4_addr);
    IP4_ADDR(&ndev_nat_entry.dest_netmask, 255, 255, 255, 255);

    ip4_addr_copy(ndev_nat_entry.source_net, ip4);
    IP4_ADDR(&ndev_nat_entry.source_netmask, 255, 255, 255, 255);
    ip4_nat_add(&ndev_nat_entry);

    netSession_t *session = &s_net_nat_session;
    if (session->is_created == true)
    {
        osiExitCritical(critical);
        OSI_LOGI(0x1000757f, "Net session create already created");
        return session;
    }
    memset(session, 0, sizeof(netSession_t));
    session->dev_netif = netif;
    OSI_LOGI(0x10007580, "Net end session %x\n", session);
    session->is_created = true;
    osiExitCritical(critical);
    return session;
}

bool prvNdevLanSessionDelete(netSession_t *session)
{
    uint32_t critical = osiEnterCritical();
    if (session == NULL)
    {
        osiExitCritical(critical);
        return false;
    }
    ip4_nat_remove(&ndev_nat_entry);
    if (session->dev_netif != NULL)
    {
        netif_set_link_down(session->dev_netif);
        netif_remove(session->dev_netif);
        free(session->dev_netif);
    }
    session->is_created = false;
    osiExitCritical(critical);
    return true;
}
#endif
