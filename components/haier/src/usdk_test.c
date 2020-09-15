#include "pal_common.h"
#include "dns.h"

void uplus_sdk_test(void);

uplus_mutex_id Mux_id;
uplus_sem_id sem_id;

uplus_task_id test1_id;
uplus_task_id test2_id;

static void resource(uint8_t taskid)
{
    if(Mux_id == NULL)
    {
        OSI_LOGI(0, "[zk] resource_0:Mux_id is NULL,taskid=%d", taskid);
        return;
    }
    if(uplus_os_mutex_take(Mux_id, TIME_WAIT_FOREVER)==0)
    {
        OSI_LOGI(0, "[zk] resource_1:task %d take resource", taskid);
        uplus_os_task_sleep(2000);
        OSI_LOGI(0, "[zk] resource_2:task %d give resource", taskid);
        uplus_os_mutex_give(Mux_id);
    }
}

void test1_task_main(void *pParameter)
{
    uint8_t parm = *(uint8_t *)pParameter;
    if(uplus_os_sem_create(&sem_id) == 0)
    {
        OSI_LOGI(0, "[zk test1] syn sem create suc");
    }
    //OSI_LOGI(0, "[zk test1] parm1=%d", parm);
	while(1)
	{
		OSI_LOGI(0, "[zk test1] test1_task RUN... parm=%d", parm);
        resource(1);
        if(uplus_os_sem_take(sem_id, TIME_WAIT_FOREVER) == 0)
        {
            OSI_LOGI(0, "[zk test1] sem take suc");
        }
        OSI_LOGI(0, "test1_task RUN...");
        uplus_os_sem_delete(sem_id);
        uplus_os_task_sleep(2000);
        OSI_LOGI(0, "test1_task end");
        uplus_os_task_delete(NULL);
	}
}

static void pal_os_api_test(void)
{
    if(uplus_os_mutex_create(&Mux_id) == 0)
    {
        OSI_LOGI(0, "[zk test2] mutex create suc");
    }
    OSI_LOGI(0, "[zk test2] test2_task RUN...");
    uplus_os_task_sleep(1000);
    resource(2);
    uplus_time cur_ticks = uplus_os_current_time_get();
    OSI_LOGI(0, "[zk test2] cur_ticks=%d", cur_ticks);
    uplus_os_task_sleep(2000);
    uplus_time cur_ticks1 = uplus_os_current_time_get();
    OSI_LOGI(0, "[zk test2] cur_ticks1=%d", cur_ticks1);
    uplus_time diff_ms = uplus_os_diff_time_cal(cur_ticks1, cur_ticks);
    OSI_LOGI(0, "[zk test2] diff_ms=%d", diff_ms);
    uplus_os_task_sleep(2000);

    uplus_os_sem_give(sem_id);

    uplus_os_task_sleep(500);

    uplus_os_task_delete(test1_id);
}

static void pal_tools_api_test(void)
{
    uint16_t bufflen = 200;
    char *testbuff = (char*)uplus_tool_data_malloc(bufflen);
    if(testbuff == NULL)
    {
        uplus_sys_debug_printf("uplus_debug:malloc fial len=%d", bufflen);
        uplus_sys_log("uplus_log:malloc fial len=%d", bufflen);
        return;
    }
    uplus_tool_memset(testbuff, 0, bufflen);

    uplus_tool_srand(0x12345678);
    uplus_s32 radns = uplus_tool_rand();
    uplus_sys_log("tools_api_test_0:radns=0x%x", radns);

    uplus_tool_snprintf(testbuff, bufflen, "%s", "zhang kai zhen shuai");

    uplus_sys_debug_printf("len=%d:%s", uplus_tool_strlen(testbuff), testbuff);
    uplus_sys_log("len=%d:%s", uplus_tool_strlen(testbuff), testbuff);

    uplus_tool_free(testbuff);
}

static void pal_sys_api_test(void)
{
    char *name = "zhangkai";
    uplus_s32 fd = 0;
    if(uplus_sys_file_open(&fd, FILE_TYPE_EPP_CONFIG, name, FILE_FLAGS_RDWR) != 0)
    {
        uplus_sys_log("sys_api_test_0:open %s fail", name);
    }
    if(uplus_sys_file_seek(fd, 0, FILE_SEEK_SET) != 0)
    {
        uplus_sys_log("sys_api_test_1:seek fail");
        uplus_sys_file_close(fd);
    }
    char buff[] = "zhang kai zhen shuai";
    if(uplus_sys_file_write(fd, (uplus_u8 *)buff, sizeof(buff)) != sizeof(buff))
    {
        uplus_sys_log("sys_api_test_2:write fail");
        uplus_sys_file_close(fd);
    }
    uplus_sys_file_close(fd);

    fd = 0;
    if(uplus_sys_file_open(&fd, FILE_TYPE_EPP_CONFIG, name, FILE_FLAGS_RDWR) != 0)
    {
        uplus_sys_log("sys_api_test_3:open %s fail", name);
    }

    if(uplus_sys_file_seek(fd, 0, FILE_SEEK_SET) != 0)
    {
        uplus_sys_log("sys_api_test_4:seek fail");
        uplus_sys_file_close(fd);
    }
    char buff1[25]={0};
    if(uplus_sys_file_read(fd, (uplus_u8 *)buff1, sizeof(buff)) != sizeof(buff))
    {
        uplus_sys_log("sys_api_test_5:read fail");
        uplus_sys_file_close(fd);
    }
    uplus_sys_file_close(fd);
    uplus_sys_log("sys_api_test_6:%s", buff1);


    uint16_t bufflen = 256;
    char *testbuff = (char*)uplus_tool_data_malloc(bufflen);
    if(testbuff == NULL)
    {
        uplus_sys_log("sys_api_test_7:malloc fial len=%d", bufflen);
        return;
    }
    uplus_tool_memset(testbuff, 0, bufflen);
    uplus_tool_strncpy(testbuff, "zhang kai hao shuai a !!!", sizeof("zhang kai hao shuai a !!!"));
    uplus_sys_res_area_write(0, (uplus_u8 *)testbuff, bufflen);
    uplus_tool_memset(testbuff, 0, bufflen);
    uplus_sys_res_area_read(0, (uplus_u8 *)testbuff, bufflen);
    uplus_sys_log("sys_api_test_8:%s", testbuff);
    uplus_sys_res_area_write(5, (uplus_u8 *)"jing", 1);
    uplus_tool_memset(testbuff, 0, bufflen);
    uplus_sys_res_area_read(5, (uplus_u8 *)testbuff, 1);
    uplus_sys_log("sys_api_test_9:%s", testbuff);
    uplus_sys_res_area_read(0, (uplus_u8 *)testbuff, bufflen);
    uplus_sys_log("sys_api_test_10:%s", testbuff);
    uplus_tool_free(testbuff);
    testbuff = NULL;


    bufflen = 512;
    testbuff = (char*)uplus_tool_data_malloc(bufflen);
    if(testbuff == NULL)
    {
        uplus_sys_log("sys_api_test_11:malloc fial len=%d", bufflen);
        return;
    }
    uplus_tool_memset(testbuff, 0, bufflen);
    uplus_tool_strncpy(testbuff, "zhang kai tai shuai le a !!!", sizeof("zhang kai tai shuai le a !!!"));
    uplus_sys_config_write(ZONE_1, (uplus_u8 *)testbuff, bufflen);
    uplus_sys_config_write(ZONE_2, (uplus_u8 *)testbuff, bufflen);
    uplus_tool_memset(testbuff, 0, bufflen);
    uplus_sys_config_read(ZONE_1, (uplus_u8 *)testbuff, bufflen);
    uplus_sys_log("sys_api_test_12:zone_1->%s", testbuff);
    uplus_tool_memset(testbuff, 0, bufflen);
    uplus_sys_config_read(ZONE_2, (uplus_u8 *)testbuff, bufflen);
    uplus_sys_log("sys_api_test_13:zone_2->%s", testbuff);
    uplus_tool_free(testbuff);
    testbuff = NULL;


    uplus_tool_memset(buff1, 0, sizeof(buff1));
    uplus_tool_random_generate((uplus_u8 *)buff1, 4);
}

static void pal_net_api_test(void)
{
    while((get_sye_state() != SYS_STATE_REG))
    {
        uplus_os_task_sleep(2000);
    }
    uplus_os_task_sleep(2000);
    char *str_addr = "192.168.1.1";
    uint32_t ip_addr = uplus_net_inet_addr(str_addr);
    uplus_sys_log("net_api_test_0:%s -> 0x%x", str_addr, ip_addr);
    uplus_sys_log("net_api_test_1:0x%x", uplus_net_ntohl(ip_addr));
    uplus_sys_log("net_api_test_2:0x%x", uplus_net_htonl(uplus_net_ntohl(ip_addr)));

    uplus_sys_log("net_api_test_3:0x%x", uplus_net_ntohs(0x1234));
    uplus_sys_log("net_api_test_4:0x%x", uplus_net_htons(0x3412));

    struct uplus_in_addr in= {0};
    in.s_addr = ip_addr;
    uplus_sys_log("net_api_test_5:0x%x -> %s", in.s_addr, uplus_net_inet_ntoa(in));

    uplus_net_fd_size();
    fd_set fd;
    fd.fd_bits[0] = 0x55;
    uplus_net_fd_zero(&fd);
    uplus_sys_log("net_api_test_7:fd zero=%d", fd.fd_bits[0]);
    uplus_net_fd_set(8, &fd);
    uplus_sys_log("net_api_test_8:fd set=0x%x", fd.fd_bits[0]);
    uplus_sys_log("net_api_test_9:fd iseet=%d", uplus_net_fd_isset(8, &fd));
    uplus_net_fd_clr(8, &fd);
    uplus_sys_log("net_api_test_9:fd clr=%d", fd.fd_bits[0]);

    //uplus_net_dns_config(OP_SET, "8.8.8.8");
    char str_ip_addr[20] = {0};
    uplus_net_dns_config(OP_GET, str_ip_addr);

    memset(str_ip_addr, 0, sizeof(str_ip_addr));
    uplus_net_dns_request("www.baidu.com", str_ip_addr);

    memset(str_ip_addr, 0, sizeof(str_ip_addr));
    uplus_net_dns_request("gw.haier.net", str_ip_addr);

    memset(str_ip_addr, 0, sizeof(str_ip_addr));
    uplus_net_dns_request("gw-sea.haieriot.net", str_ip_addr);
}

static void socket_api_test(void)
{	
	//struct timeval tm={0};
    //fd_set ReadSet;
	//int32_t Result;

    /*while((get_sye_state() != SYS_STATE_REG))
    {
        uplus_os_task_sleep(2000);
    }
    uplus_os_task_sleep(2000);*/
    int socketfd;
    int connErr;
    struct sockaddr_in tcp_server_addr = {0}; 

	char ipaddr[20] = {0};
	//unsigned short port = 12415;  //56626;
	//memcpy(ipaddr, SERVER_TCP_IP, sizeof(SERVER_TCP_IP));
	
	uplus_net_dns_request("gw-sea.haieriot.net", ipaddr);
	//memcpy(ipaddr, "101.200.183.79", sizeof("101.200.183.79"));
	unsigned short port = 56814;//56808;  
    // 创建tcp socket
    socketfd = uplus_net_socket_create(AF_INET, SOCK_STREAM, 0);
    /*if (socketfd < 0)
    {
        uplus_sys_log("[socket] create tcp socket error");
        return;
    }
       
    uplus_sys_log("[socket] create tcp socket success");*/

	uint8_t optval[4] = {1,0,0,0};
	uplus_net_socket_opt_set(socketfd, 0, 1, optval, 4);
	uplus_net_socket_opt_set(socketfd, 2, 1, optval, 4);
    
    // 建立TCP链接 
    tcp_server_addr.sin_family = AF_INET;  
    tcp_server_addr.sin_port = uplus_net_htons(port);  
    //inet_aton(ipaddr,&tcp_server_addr.sin_addr);
    tcp_server_addr.sin_addr.s_addr = uplus_net_inet_addr(ipaddr);

    //uplus_sys_log("[socket] tcp connect to addr %s", ipaddr);
    //struct uplus_sockaddr u_sockaddr = {0};
    //memcpy(&u_sockaddr, &tcp_server_addr, sizeof(struct uplus_sockaddr));
    uplus_sys_log("[socket] sizeof:uplus=%d in=%d sock=%d", sizeof(struct uplus_sockaddr), sizeof(struct sockaddr_in), sizeof(struct sockaddr));
    uplus_net_socket_connect(socketfd, (const struct uplus_sockaddr *)&tcp_server_addr, sizeof(struct uplus_sockaddr));
    ///connErr = lwip_connect(socketfd, (const struct sockaddr *)&tcp_server_addr, sizeof(struct sockaddr));
    //uplus_sys_log("[socket] tcp connect connErr=%d", connErr);

    //uplus_net_socket_close(socketfd);
    //uplus_sys_log("[socket] tcp close connErr=%d", connErr);


	uplus_ctx_id ssl_ctx = uplus_net_ssl_client_create(socketfd, NULL, 0);

    connErr = uplus_net_ssl_client_handshake(ssl_ctx);
    uplus_sys_log("[socket] ssl handshake connErr=%d", connErr);

    uplus_net_ssl_client_close(ssl_ctx);

    uplus_net_socket_close(socketfd);
		
	/*FD_ZERO(&ReadSet);
    FD_SET(socketfd, &ReadSet);
	uint8 Buf[100];
	uint16 RxLen=5;
	
	tm.tv_sec = 10;
   	tm.tv_usec = 0;
	
	Result = select(socketfd + 1, &ReadSet, NULL, NULL, &tm);
    if(Result > 0)
    {
    	Result = recv(socketfd, Buf, RxLen, 0);
        if(Result == 0)
        {
        	iot_debug_print("[zk test]socket close!");
            return -1;
        }
        else if(Result < 0)
        {
        	iot_debug_print("[zk test]recv error %d", socket_errno(socketfd));
            return -1;
        }
        //iot_debug_print("SSL_SocketRx:recv %d %x %x %x %x %x", Result, p[0], p[1], p[2], p[3], p[4]);
		return Result;
    }
    else
    {
    	iot_debug_print("[zk test] read timeout");
    	return -1;
    }*/
}

void test2_task_main(void *pParameter)
{
    uint8_t parm = *(uint8_t *)pParameter;
    OSI_LOGI(0, "[zk test2] parm2=%d", parm);
	while(1)
	{
        //pal_os_api_test();

        //pal_tools_api_test();

        //pal_sys_api_test();

        pal_net_api_test();

        socket_api_test();

        OSI_LOGI(0, "[zk test2] test2_task end");
        uplus_os_task_delete(NULL);
	}
}

void uplus_sdk_test(void)
{
    //static uint8_t tset1_param = 1;
    static uint8_t tset2_param = 2;

    //uplus_os_task_create("task1", 256, 5, test1_task_main, (void *)&tset1_param, &test1_id);

    uplus_os_task_create("task2", 1024, 4, test2_task_main, (void *)&tset2_param, &test2_id);
}
