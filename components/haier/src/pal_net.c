
/**
 * @file pal_net.c
 * @brief
 *
 * @date
 * @author
 *
 */

#include "pal_common.h"

/*!
 * \brief Socket库socket create。
 */
uplus_s32 uplus_net_socket_create(uplus_s32 domain, uplus_s32 type, uplus_s32 protocol)
{
	int socketid = socket(domain, type, protocol);
	
	uplus_sys_log("[zk u+] socket_create socketid=%d domain=%d type=%d protocol=%d", socketid, domain, type, protocol);
	
	return socketid;
}

/*!
 * \brief
 * \param
 * \return 成功返回0，失败返回-1。
 */
uplus_s32 uplus_net_socket_close(uplus_s32 fd)
{	
	int result = close(fd);
	if(result < 0)
	{
		uplus_sys_log("[zk u+] net_socket_close:error=%d", uplus_net_last_error_get(fd, NULL));
	}
	
	uplus_sys_log("[zk u+] socket_close:socket=%d result=%d", fd, result);
	
	return result;
}

/*!
 * \brief Socket库bind。
 */
uplus_s32 uplus_net_socket_bind(uplus_s32 sockfd, const struct uplus_sockaddr *addr, uplus_socklen_t addrlen)
{
	struct sockaddr z_addr = {0};
	
	z_addr.sa_family = addr->sa_family;
	memcpy(z_addr.sa_data, addr->sa_data, sizeof(addr->sa_data));
	
	int result = bind(sockfd, &z_addr, sizeof(struct sockaddr));

	if(result < 0)
	{
		uplus_sys_log("[zk u+] net_socket_bind:error=%d", uplus_net_last_error_get(sockfd, NULL));
	}
	
	uplus_sys_log("[zk u+] socket_bind:socket=%d result=%d", sockfd, result);
	
	return result;

	/*int result = bind(sockfd, addr, addrlen);
	
	uplus_sys_log("[zk u+] socket_bind:socket=%d result=%d", sockfd, result);

	return result;*/
}

/*!
 * \brief Socket库listen。
 */
uplus_s32 uplus_net_socket_listen(uplus_s32 sockfd, uplus_s32 backlog)
{
	int result = listen(sockfd, backlog);
	if(result < 0)
	{
		uplus_sys_log("[zk u+]net_socket_listen:error=%d", uplus_net_last_error_get(sockfd, NULL));
	}
	
	uplus_sys_log("[zk u+] socket_listen:socket=%d backlog=%d result=%d", sockfd, backlog, result);
	
	return result;
}

/*!
 * \brief Socket库accept。
 */
uplus_s32 uplus_net_socket_accept(uplus_s32 sockfd, struct uplus_sockaddr *addr, uplus_socklen_t *addrlen)
{
	struct sockaddr z_addr = {0};
	
	z_addr.sa_family = addr->sa_family;
	memcpy(z_addr.sa_data, addr->sa_data, sizeof(addr->sa_data));

	int result = accept(sockfd, &z_addr, addrlen);
	if(result < 0)
	{
		uplus_sys_log("[zk u+] net_socket_accept:error=%d", uplus_net_last_error_get(sockfd, NULL));
	}
	
	uplus_sys_log("[zk u+] socket_accept:sockfd=%d result=%d", sockfd, result);
	
	return result;

	/*int result = accept(sockfd, addr, addrlen);
	
	uplus_sys_log("[zk u+] socket_accept:sockfd=%d result=%d", sockfd, result);
	
	return result;*/
}

/*!
 * \brief Socket库connect。
 */
uplus_s32 uplus_net_socket_connect(uplus_s32 sockfd, const struct uplus_sockaddr *addr, uplus_socklen_t addrlen)
{
	struct sockaddr z_addr = {0};
	
	z_addr.sa_family = addr->sa_family;
	memcpy(z_addr.sa_data, addr->sa_data, sizeof(addr->sa_data));
	
	int result = connect(sockfd, &z_addr, sizeof(struct sockaddr));
	if(result < 0)
	{
		uplus_sys_log("[zk u+] net_socket_connect:error=%d", uplus_net_last_error_get(sockfd, NULL));
	}
	
	uplus_sys_log("[zk u+] socket_connect:socket=%d result=%d", sockfd, result);
	
	return result;

	/*int result = connect(sockfd, addr, addrlen);
	
	uplus_sys_log("[zk u+] socket_connect:socket=%d result=%d", sockfd, result);
	
	return result;*/
}

/*!
 * \brief Socket库send。
 */
uplus_s32 uplus_net_socket_send(uplus_s32 sockfd, const void *buf, uplus_size_t len, uplus_s32 flags)
{
	uplus_sys_log("[zk u+] socket_send");
	
	return send(sockfd, buf, len, flags);
}

/*!
 * \brief Socket库sendto。
 */
uplus_s32 uplus_net_socket_sendto(uplus_s32 sockfd, const void *buf, uplus_size_t len, uplus_s32 flags, const struct uplus_sockaddr *addr, uplus_socklen_t addrlen)
{
	struct sockaddr z_addr = {0};
	
	z_addr.sa_family = addr->sa_family;
	memcpy(z_addr.sa_data, addr->sa_data, sizeof(addr->sa_data));
	
	uplus_sys_log("[zk u+] socket_sendto");
	
	return sendto(sockfd, buf, len, flags, &z_addr, sizeof(struct sockaddr));

	/*uplus_sys_log("[zk u+] socket_sendto");
	
	return sendto(sockfd, buf, len, flags, addr, addrlen);*/
}

/*!
 * \brief Socket库recv。
 */
uplus_s32 uplus_net_socket_recv(uplus_s32 sockfd, void *buf, uplus_size_t len, uplus_s32 flags)
{
	uplus_sys_log("[zk u+] socket_recv");
	
	return recv(sockfd, buf, len, flags);
}

/*!
 * \brief Socket库recvfrom。
 */
uplus_s32 uplus_net_socket_recvfrom(uplus_s32 sockfd, void *buf, int len, int flags, struct uplus_sockaddr *addr, uplus_socklen_t *addrlen)
{
	struct sockaddr z_addr = {0};
	
	z_addr.sa_family = addr->sa_family;
	memcpy(z_addr.sa_data, addr->sa_data, sizeof(addr->sa_data));
	
	uplus_sys_log("[zk u+] socket_recvfrom");
	
	return recvfrom(sockfd, buf, len, flags, &z_addr, addrlen);

	/*uplus_sys_log("[zk u+] socket_recvfrom");
	
	return recvfrom(sockfd, buf, len, flags, addr, addrlen);*/
}

/*!
 * \brief 设置socket选项。
 * \param [in] sockfd socket描述符。
 * \param [in] level SO_LEVEL_XXX。
 * \param [in] optname SO_OPT_XXX。
 * \param [in] optval 指向设置值的指针。
 * \param [in] optlen 数据长度。
 * \return 成功返回0，不支持返回0，其他错误返回-1。
 */
uplus_s32 uplus_net_socket_opt_set (uplus_s32 sockfd, uplus_s32 level, uplus_s32 optname, const void *optval, uplus_s32 optlen)
{
	uplus_s32 level_1;
	uplus_s32 optname_1;

	if (level == SO_LEVEL_SOCKET)
	{
		level_1 = SOL_SOCKET;
		switch (optname)
		{
			case SO_OPT_KEEPALIVE:
				optname_1 = SO_KEEPALIVE;
				break;
			case SO_OPT_SNDTIMEO:
				optname_1 = SO_SNDTIMEO;
				break;
			case SO_OPT_RCVTIMEO:
				optname_1 = SO_RCVTIMEO;
				break;
			case SO_OPT_BROADCAST:
				optname_1 = SO_BROADCAST;
				break;
			case SO_OPT_REUSEADDR:
				optname_1 = SO_REUSEADDR;
				break;
			default:
				return (0);
		}
	}
	else if (level == SO_LEVEL_TCP)
	{
		level_1 = IPPROTO_TCP;
		switch (optname)
		{
			case SO_TCP_NODELAY:
				optname_1 = TCP_NODELAY;
				break;
			default:
				return (0);
		}
	}
	else
	{
		return (0);
	}
	
	int result = setsockopt(sockfd, level_1, optname_1, optval, optlen);
	if(result < 0)
	{
		uplus_sys_log("[zk u+] net_socket_opt_set:error=%d", uplus_net_last_error_get(sockfd, NULL));
	}
	uint8_t *p = (uint8_t *)optval;
	uplus_sys_log("[zk u+] socket_opt_set:0x%x 0x%x 0x%x 0x%x", p[0], p[1], p[2], p[3]);
	uplus_sys_log("[zk u+] socket_opt_set:socket=%d level=0x%x optname=0x%x optlen=%d result=%d", sockfd, level_1, optname_1, optlen,result);
	
	return result;
}

 
/*!
 * \brief 字节序转换htonl
 * \param [in] hostlong 主机序4字节整数。
 * \return 网络序4字节整数。
 */
uplus_u32 uplus_net_htonl(uplus_u32 hostlong)
{
	uplus_u32 hostlong_1 = htonl(hostlong);

	//uplus_sys_log("[zk u+]net_ntohl:hostlong=%x hostlong_1=%x", hostlong, hostlong_1);
		
	return hostlong_1;
}

/*!
 * \brief 字节序转换ntohl
 * \param [in] netlong 网络序4字节整数。
 * \return 主机序4字节整数。
 */
uplus_u32 uplus_net_ntohl(uplus_u32 netlong)
{
	uplus_u32 netlong_1 = ntohl(netlong);

	//uplus_sys_log("[zk u+]net_ntohl:netlong=%x netlong_1=%x", netlong, netlong_1);
	
	return netlong_1;
}

/*!
 * \brief 字节序转换htons
 * \param [in] hostshort 主机序2字节整数。
 * \return 网络序2字节整数。
 */
uplus_u16 uplus_net_htons(uplus_u16 hostshort)
{
	uplus_u16 hostshort_1 = htons(hostshort);

	//uplus_sys_log("[zk u+]net_htons:hostshort=%x hostshort_1=%x", hostshort, hostshort_1);
	
	return hostshort_1;
}

/*!
 * \brief 字节序转换ntohs
 * \param [in] netshort 网络序2字节整数。
 * \return 主机序2字节整数。
 */
uplus_u16 uplus_net_ntohs(uplus_u16 netshort)
{
	uplus_u16 netshort_1 =  ntohs(netshort);

	//uplus_sys_log("[zk u+]net_ntohs:netshort=%x netshort_1=%x", netshort, netshort_1);
	
	return netshort_1;
}

/*!
 * \brief IP地址转换为字符串。
 * \param [in] in 网络字节序的IP地址。
 * \return 点分十进制表示的IP地址字符串。
 */
uplus_s8 *uplus_net_inet_ntoa(struct uplus_in_addr in)
{
	char *p = inet_ntoa(in);
	
	//uplus_sys_log("[zk u+] net_inet_ntoa:p=%s addr=%x", p, in.s_addr);
	
	return p;
}

/*!
 * \brief 字符串转换为IP地址。
 * \param [in] cp 点分十进制表示的IP地址字符串。
 * \return 网络序4字节整数表示的IP地址。
 */
uplus_u32 uplus_net_inet_addr(const uplus_s8 *cp)
{
	uplus_u32 addr = 0;
	
	addr = inet_addr((char *)cp);
	//inet_aton(cp, &addr);

	//uplus_sys_log("[zk u+] net_inet_addr:cp=%s addr=%x", cp, addr);

	if(addr == 0)
		addr = 0xFFFFFFFF;

	return addr;
}

/*!
 * \brief IO复用。
 * \param [in] nfds 描述集合中所有文件描述符，最大值加1。
 * \param [inout] readfds 指向fd set的指针。监视文件描述是否可读。NULL表示不关心。
 * \param [inout] writefds 指向fd set的指针。监视文件描述是否可写。NULL表示不关心。
 * \param [inout] exceptfds 指向fd set的指针。监视文件描述是否异常。NULL表示不关心。
 * \param [in] timeout select超时时间。NULL表示select处于阻塞状态。
 * \return >0，一些文件描述符可读、可写或者异常。<0，错误。=0，超时。
 */
uplus_s32 uplus_net_select(uplus_s32 nfds, void *readfds, void *writefds, void *exceptfds, struct uplus_timeval *timeout)
{
	//uplus_sys_log("[zk u+] net_select nfds=%d", nfds);
	struct timeval tmout = {0};
	tmout.tv_sec = timeout->tv_sec;
	tmout.tv_usec = timeout->tv_usec;
	
	return select(nfds, (fd_set*)readfds, (fd_set*)writefds, (fd_set*)exceptfds, &tmout);
}

/*!
 * \brief 清空FD集合。
 * \param [in] set 指向fd set的指针。
 * \return N/A。
 */
void uplus_net_fd_zero(void *set)
{
	fd_set *p = (fd_set*)set;

	memset(p, 0, sizeof(fd_set));
	
	//uplus_sys_log("[zk u+] net_fd_zero p[0]=%d p[1]=%d p[2]=%d p[3]=%d", p->fd_bits[0], p->fd_bits[1], p->fd_bits[2], p->fd_bits[3]);
}

/*!
 * \brief 清除FD集合中指定的文件描述符。
 * \param [in] fd 文件描述符。
 * \param [in] set 指向fd set的指针。
 * \return N/A。
 */
void uplus_net_fd_clr(uplus_s32 fd, void *set)
{
	fd_set *p = (fd_set*)set;
	
	FD_CLR(fd, p);

	//uplus_sys_log("[zk u+] net_fd_clr fd=%d p[0]=%x p[1]=%x p[2]=%x p[3]=%x", fd, p->fd_bits[0], p->fd_bits[1], p->fd_bits[2], p->fd_bits[3]);
}

/*!
 * \brief 设置FD集合中指定的文件描述符。
 * \param [in] fd 文件描述符。
 * \param [in] set 指向fd set的指针。
 * \return N/A。
 */
void uplus_net_fd_set(uplus_s32 fd, void *set)
{
	fd_set *p = (fd_set*)set;

	FD_SET(fd, p);
	
	//uplus_sys_log("[zk u+] net_fd_set fd=%d p[0]=%x p[1]=%x p[2]=%x p[3]=%x", fd, p->fd_bits[0], p->fd_bits[1], p->fd_bits[2], p->fd_bits[3]);
}

/*!
 * \brief 判断指定的文件描述符是否就绪。
 * \param [in] fd 文件描述符。
 * \param [in] set 指向fd set的指针。
 * \return 1表示fd已就绪，0表示fd未就绪。
 */
uplus_s32 uplus_net_fd_isset(uplus_s32 fd, void *set)
{
	fd_set *p = (fd_set*)set;
	
	//uplus_sys_log("[zk u+] net_fd_isset fd=%d p[0]=%x p[1]=%x p[2]=%x p[3]=%x", fd, p->fd_bits[0], p->fd_bits[1], p->fd_bits[2], p->fd_bits[3]);
	if(FD_ISSET(fd, p) != 0)
		return 1;
	else
		return 0;
}

/*!
 * \brief 获取fd set的大小。
 * \return Fd set的大小。
 */
uplus_u32 uplus_net_fd_size(void)
{
	uplus_sys_log("[zk u+] net_fd_size %d", sizeof(fd_set));
	return (sizeof(fd_set));
}

/*!
 * \brief 申请可用于select的文件描述符。
 * \return 成功返回FD，失败返回-1。
 */
uplus_s32 uplus_net_alloc_fd(void)
{
	uplus_sys_log("[zk u+] net_alloc_fd");
	
	return -1;
}

/*!
 * \brief 释放文件描述符。
 * \param [in] fd 文件描述符。
 * \return N/A。
 */
void uplus_net_free_fd(uplus_s32 fd)
{
	uplus_sys_log("[zk u+] net_free_fd");
}

/*!
 * \brief 通知select有数据可读。
 * \param [in] fd 文件描述符。
 * \return N/A。
 */
void uplus_net_fd_rcv_plus(uplus_s32 fd)
{
	uplus_sys_log("[zk u+] net_fd_rcv_plus");
}

/*!
 * \brief 数据读取后清除可读标记。
 * \param [in] fd 文件描述符。
 * \return N/A。
 */
void uplus_net_fd_rcv_minus(uplus_s32 fd)
{
	uplus_sys_log("[zk u+] net_fd_rcv_minus");
}

/*!
 * \brief IPv4地址设置与查询
 * \param [in] netif_type 网络接口类型，NETIF_TYPE_XXX。
 * \param [in] op 操作方式，OP_SET/OP_GET。
 * \param [inout] local_ip_addr 点分十进制表示的IP地址。
 * \param [inout] net_mask 点分十进制表示的IP掩码。
 * \param [inout] gateway_ip_addr 点分十进制表示的IP网关。
 * \return 成功返回0，未获取到IP地址也返回0，其他错误返回-1。
 */
uplus_s32 uplus_net_ip_config(uplus_u8 netif_type, uplus_u8 op, uplus_s8 *local_ip_addr, uplus_s8 *net_mask, uplus_s8 *gateway_ip_addr)
{
	if(op == OP_GET)
	{
		if(local_ip_addr != NULL)
		{
			strcpy(local_ip_addr, appSysTem.Module_ipaddr);
			
			uplus_sys_log("[zk u+] net_ip_config get %s", local_ip_addr);

			return 0;
		}
		else
		{
			uplus_sys_log("[zk u+] net_ip_config get error");
		}
	}
	return -1;
}

/*!
 * \brief DNS服务器设置与查询。
 * \param [in] op 操作方式，OP_SET/OP_GET。
 * \param [in] dns_server，域名服务器，点分十进制的IP地址。当操作方式是查询时，如果成功填写域名服务器；否则不填写。
 * \return 成功返回0，失败返回-1。
 */
uplus_s32 uplus_net_dns_config(uplus_u8 op, uplus_s8 *dns_server)
{
	if(dns_server == NULL)
	{
		uplus_sys_log("[zk u+] net_dns_config parmer error op=%d ", op);
		return -1;
	}
	if(op == OP_SET)
	{	
		uplus_u32 addr = uplus_net_inet_addr(dns_server);
		uplus_sys_log("[zk u+] net_dns_config OP_SET dns_server=%s addr=0x%x", dns_server, addr);
		dns_setserver(0, &addr);
		
		return 0;
	}
	else if(op == OP_GET)
	{
		struct uplus_in_addr *addres = (struct uplus_in_addr *)dns_getserver(0);
		struct uplus_in_addr addres1;
		addres1.s_addr = addres->s_addr;
		char *str_ip = uplus_net_inet_ntoa(addres1);
		if(str_ip == NULL)
		{
			addres = (struct uplus_in_addr *)dns_getserver(1);
			addres1.s_addr = addres->s_addr;
			str_ip = uplus_net_inet_ntoa(addres1);
			if(str_ip == NULL)
			{
				uplus_sys_log("[zk u+] net_dns_config OP_GET error");
				return -1;
			}
			else
				strcpy(dns_server, str_ip);
		}
		else
			strcpy(dns_server, str_ip);

		//这段代码只是把备用DNS显示出来， 用于调试
		addres = (struct uplus_in_addr *)dns_getserver(1);
		addres1.s_addr = addres->s_addr;
		
		uplus_sys_log("[zk u+] net_dns_config OP_GET dns_server1=%s dns_server2=%s", dns_server, uplus_net_inet_ntoa(addres1));

		return 0;
	}
}

#define DNS_PARSING_WAIT_TIME_MAX	(30000/5)
static uint8_t dns_parsing_falg;
void zk_found_callback(const char *name, const ip_addr_t *ipaddr, void *callback_arg)
{
	struct uplus_in_addr addr = {0};
	char *ip_addr = (char *)callback_arg;

	addr.s_addr = ipaddr->u_addr.ip4.addr;
	char *str_ip = uplus_net_inet_ntoa(addr);
	if(str_ip == NULL)
	{
		dns_parsing_falg = 1;
		return;
	}
	dns_parsing_falg = 2;
	strcpy(ip_addr, str_ip);
	//uplus_sys_log("[zk u+] zk_found_callback: %s -> %s", name, uplus_net_inet_ntoa(addr));
}

static int gethostbyname(char *hostname, char *ip_addr)
{
	struct uplus_in_addr addres = {0};
	uint16_t cnt = 0;

	dns_parsing_falg = 0;
	dns_gethostbyname(hostname, &addres, zk_found_callback, ip_addr);

	while((dns_parsing_falg==0) && (cnt < DNS_PARSING_WAIT_TIME_MAX))
	{
		uplus_os_task_sleep(5);
		cnt++;
	}

	switch (dns_parsing_falg)
	{
		case 0:
			uplus_sys_log("[zk u+] gethostbyname_0: %s-> parsing time out", hostname);
			return -1;
		case 1:
			uplus_sys_log("[zk u+] gethostbyname_1: %s-> parsing fail", hostname);
			return -1;
		case 2:
			uplus_sys_log("[zk u+] gethostbyname_2: %s -> %s", hostname, ip_addr);
			return 0;
		default:
			uplus_sys_log("[zk u+] gethostbyname_3: not error");
			return -1;
	}
}

/*!
 * \brief DNS域名解析
 * \note 阻塞方式，超时时间不超过5秒。
 * \param [in] hostname 待解析的域名，以’\0’结尾的字符串。
 * \param [in] ip_addr 域名的IP地址。成功放回解析域名获得的IP地址；失败不填写。
 * \return 成功返回0，失败返回-1。
 */
uplus_s32 uplus_net_dns_request(const uplus_s8 *hostname, uplus_s8 *ip_addr)
{
	//uplus_sys_log("[zk u+] net_dns_request hostname=%s", hostname);

	int result = gethostbyname((char *)hostname, (char *)ip_addr);

	//uplus_sys_log("[zk u+] net_dns_request ip:%s", ip_addr);
	
	return result;
}

void uplus_net_dhcp_offer_ip(uplus_s8 * offered_ip)
{
	uplus_sys_log("[zk u+] net_dhcp_offer_ip");
}


uplus_s32 uplus_net_dhcp_config(uplus_u8 op, uplus_u8 mode)
{
	uplus_sys_log("[zk u+] net_dhcp_config");
	return -1;
}


uplus_s32 uplus_net_dhcp_pool_set(uplus_u8 op, uplus_s8 *address_pool_start, uplus_s8 *address_pool_end)
{
	uplus_sys_log("[zk u+] net_dhcp_pool_set");
	return -1;
}

/*!
 * \brief 打开SSL客户端。
 * \param fd 已连接的socket描述符。
 * \param [in] root_ca 根证书，如果为空，表示不验证服务器证书。PEM证书格式。
 * \param [in] root_ca_num 根证书的数量。
 * \return 成功返回SSL会话标识，失败返回NULL。
 */
uplus_ctx_id uplus_net_ssl_client_create(uplus_s32 fd, struct uplus_ca_chain *root_ca, uplus_u8 root_ca_num)
{
	/*uplus_sys_log("[zk u+] net_ssl_client_create fd=%d", fd);
	return net_ssl_client_create(fd, root_ca, root_ca_num);*/

	return NULL;
}

/*!
 * \brief SSL握手。
 * \param [in] id SSL会话标识。
 * \return 成功返回0，失败返回-1，继续握手返回1。
 */
uplus_s32 uplus_net_ssl_client_handshake(uplus_ctx_id id)
{
	/*uplus_sys_log("[zk u+] net_ssl_client_handshake");
	return net_ssl_client_handshake(id);*/

	return 0;
}

/*!
 * \brief 关闭SSL客户端。
 * \note 此接口不能关闭已绑定的socket fd。
 * \param [in] id SSL会话标识。
 * \return 成功返回0，失败返回-1。
 */
uplus_s32 uplus_net_ssl_client_close(uplus_ctx_id id)
{
	/*uplus_s32 result = net_ssl_client_close(id);

	uplus_sys_log("[zk u+] net_ssl_client_close result=%d", result);
	
	return result;*/

	return 0;
}

/*!
 * \brief 获取SSL可读数据的长度。
 * \param [in] id SSL会话标识。
 * \return 返回SSL会话可读数据的长度，如果无可读数据，返回0。
 */
uplus_s32 uplus_net_ssl_pending(uplus_ctx_id id)
{
	//if(get_sye_state() != SYS_STATE_FOTA)
	/*{
		uplus_sys_log("[zk u+] net_ssl_pending start");
		
		uplus_s32 read_num = net_ssl_pending(id);
		
		uplus_sys_log("[zk u+] net_ssl_pending end:num=%d", read_num);
		
		return read_num;
	}*/
	//else
	//{
		//uplus_sys_log("[zk u+] net_ssl_pending fota task run....");
		//return 0;
	//}
	return 0;
}

/*!
 * \brief 读取SSL数据
 * \param [in] id SSL会话标识。
 * \param [out] buf 数据缓存指针。
 * \param [in] len 数据长度。
 * \return 成功返回实际写入的数据长度，失败返回-1。
 */
uplus_s32 uplus_net_ssl_read(uplus_ctx_id id, uplus_u8 *buf, uplus_size_t len)
{
	//if(get_sye_state() != SYS_STATE_FOTA)
	/*{
		uplus_sys_log("[zk u+] net_ssl_read_1 start:len=%d", len);
		
		uplus_s32 read_num = net_ssl_read(id, buf, len);

		uplus_sys_log("[zk u+] net_ssl_read_2 end:num=%d ", read_num);
		
		return read_num;
	}*/
	//else
	//{
		//uplus_sys_log("[zk u+] net_ssl_read fota task run....");
		//return 0;
	//}
	return 0;
}

/*!
 * \brief 写入SSL数据。
 * \param [in] id SSL会话标识。
 * \param [in] buf 数据缓存指针。
 * \param [in] len 数据长度。
 * \return 成功返回实际读取的数据长度，失败返回-1。
 */
uplus_s32 uplus_net_ssl_write(uplus_ctx_id id, uplus_u8 *buf, uplus_size_t len)
{
	//if(get_sye_state() != SYS_STATE_FOTA)
	/*{
		uplus_sys_log("[zk u+] net_ssl_write start:len=%d", len);
		
		uplus_s32 write_num = net_ssl_write(id, buf, len);
		
		uplus_sys_log("[zk u+] net_ssl_write end:write_num=%d", write_num);
		
		return write_num;
	}*/
	//else
	//{
		//uplus_sys_log("[zk u+] net_ssl_write fota task run....");
		//return 0;
	//}
	return 0;
}

/*!
 * \brief 获取socket接口或者ssl接口调用返回的错误码。
 * \param [in] fd socket描述符，SSL会话标识为NULL时有效。
 * \param [in] ssl_id SSL会话标识。
 * \return 成功返回0，失败返回-1。
 */
uplus_s32 uplus_net_last_error_get(uplus_s32 fd, uplus_ctx_id ssl_id)
{
	/*uplus_sys_log("[zk u+] net_last_error_get");
	if(fd != NULL)
		return socket_errno(fd);
	else*/
		return -1;
}

/*!
 * \brief 获取最近设置的ERRNO。
 * \note 如果不支持，返回0。
 * \return errno，如果大于0，表示有错误；如果等于0，表示未检测到连接错误；如果小于0，表示链路层错误。
 */
uplus_s32 uplus_net_errno_get(void)
{
	uplus_sys_log("[zk u+] net_errno_get");
	return 0;
}

/*!
 * \brief 设置主机名称。
 * \param [in] hostname 以’\0’结尾的字符串。
 * \return 成功返回0，失败返回-1。
 */
uplus_s32 uplus_net_hostname_set(const uplus_s8 *hostname)
{
	uplus_sys_log("[zk u+] net_hostname_set=%s", hostname);

	strcpy(appSysTem.uplus_hostname, hostname);
	
	return -1;
}

/*!
 * \brief 执行ping操作。
 * \param [in] target 域名或者IP地址。
 * \param [in] data_size 净荷数据长度。
 * \param [in] times 次数，0表示持续。
 * \param [in] timeout 超时时间，0表示使用系统默认超时时间。
 * \param [in] cb 结果统计的回调函数。
 * \param [in] cb_para 回调函数的参数。
 * \return 成功返回索引，用于主动停止；失败返回-1。
 */
uplus_s32 uplus_net_ping(const uplus_s8 * target, uplus_u16 data_size, uplus_u32 times,
	uplus_u32 timeout, uplus_ping_stat_cb_func cb, void *cb_para)
{
	uplus_sys_log("[zk u+] net_ping");
	return -1;
}
uplus_s32 uplus_net_ping_para(const uplus_s8 * target, uplus_ping_para_t *para, uplus_ping_stat_cb_func cb, void *cb_para)
{
	uplus_sys_log("[zk u+] net_ping_para");
	return -1;
}

/*!
 * \brief 停止ping操作。
 * \param [in] index 开始ping返回的索引值。
 * \return N/A。
 */
void uplus_net_ping_stop(uplus_s32 index)
{
	uplus_sys_log("[zk u+] net_ping_stop");
}

/*!
 * \brief 在指定接口上发送免费ARP。
 *
 * \param [in] netif_type 网络接口类型，NETIF_TYPE_XXX。
 *
 * \return N/A。
 */
void uplus_net_tx_gratuitous_arp(uplus_u8 netif_type)
{
	uplus_sys_log("[zk u+] net_tx_gratuitous_arp");
}

/*!
 * \brief 设置IP冲突检测回调。
 *
 * \param [in] cb 回调接口。
 *
 * \return N/A。
 */
void uplus_net_set_ip_conflict_callback(void (*cb)(void))
{
	uplus_sys_log("[zk u+] net_set_ip_conflict_callback");
}

/*!
 * \brief 使能/关闭发送ICMP不可达功能。
 *
 * \param [in] enable 0-关闭，1-使能。
 *
 * \return N/A。
 */
void uplus_net_enable_icmp_unreach(int enable)
{
	uplus_sys_log("[zk u+] net_enable_icmp_unreach");
}

/*!
 * \brief 查找系统ARP表项。
 * \param [in] netif_type 网络接口类型，NETIF_TYPE_XXX。
 * \param [in] ip_addr 待查找的IPv4地址，网络字节序。
 * \param [out] eth_addr ARP地址。
 * \return 成功返回0，失败返回-1。
 */
uplus_s32 uplus_net_find_arp(uplus_u8 netif_type, uplus_u32 ip_addr, uplus_u8* eth_addr)
{
	uplus_sys_log("[zk u+] net_find_arp");
	return -1;
}


