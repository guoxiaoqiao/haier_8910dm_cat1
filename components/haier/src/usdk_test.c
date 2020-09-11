#include "pal_common.h"

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

void test2_task_main(void *pParameter)
{
    uint8_t parm = *(uint8_t *)pParameter;
    OSI_LOGI(0, "[zk test2] parm2=%d", parm);
    if(uplus_os_mutex_create(&Mux_id) == 0)
    {
        OSI_LOGI(0, "[zk test2] mutex create suc");
    }
	while(1)
	{
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
        OSI_LOGI(0, "[zk test2] test2_task end");
        uplus_os_task_delete(NULL);
	}
}

void uplus_sdk_test(void)
{
    static uint8_t tset1_param = 1;
    static uint8_t tset2_param = 2;

    uplus_os_task_create("task1", 256, 5, test1_task_main, (void *)&tset1_param, &test1_id);

    uplus_os_task_create("task2", 256, 4, test2_task_main, (void *)&tset2_param, &test2_id);
}
