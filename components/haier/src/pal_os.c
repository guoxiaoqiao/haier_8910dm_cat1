
/**
 * @file pal_os.c
 * @brief
 *
 * @date
 * @author
 *
 */

#include "pal_common.h"

/*!
 * \brief 创建任务。
 * \param [in] name 任务名称，以‘\0’结尾的字符串。
 * \param [in] stack_size 任务栈大小，单位字节。仅包含uplugSDK占用的栈空间，PAL占用的栈空间需要额外计算。
 * \param [in] priority 任务优先级，取值0-7，共8个优先级，0最高。
 * \param [in] func 任务处理函数。
 * \param [in] para 函数参数。
 * \param [out] id 如果创建成功，返回创建的任务ID，任务ID不能为NULL；否则不填写。
 * \return 成功返回0，失败返回-1。
 */
uplus_s32 uplus_os_task_create(uplus_s8 * name
										, uplus_u32 stack_size
										, uplus_u8 priority
										, task_func func
										, void *para
										, uplus_task_id *id)
{
	TaskHandle_t task_handle = NULL;
	uplus_sys_log("[zk u+] task_creat name=%s prio=%d stack_size=%d", name, priority, stack_size);
	if(xTaskCreate((TaskFunction_t)func, (char*)name, stack_size, para, OSI_PRIORITY_NORMAL-priority, &task_handle) != pdPASS)
    {
        uplus_sys_log("[zk] %s Created Fail", name);
		return -1;
    }
	*id = task_handle;
	return 0;
}					

/*!
 * \brief 删除任务。
 * \param [in] id 任务ID（成功创建任务返回的任务ID）。
 * \return 成功返回0，失败返回-1。
 */
uplus_s32 uplus_os_task_delete(uplus_task_id id)
{
	vTaskDelete(id);

	return 0;
}

/*!
 * \brief 任务睡眠（当前任务）。
 * \param [in] delay 任务睡眠时间，单位毫秒。
 * \return 成功返回0，失败返回-1。
 */
uplus_s32 uplus_os_task_sleep(uplus_u32 delay)
{
	//uplus_sys_log("[zk u+] os_task_sleep time=%d", delay);
	vTaskDelay(osiMsToOSTick(delay));
	
	return 0;
}

/*!
 * \brief 创建互斥信号量。
 * \param [out] id 如果创建成功，返回创建的信号量ID；否则不填写。信号量ID不能为NULL。
 * \return 成功返回0，失败返回-1。
 */
uplus_s32 uplus_os_mutex_create(uplus_mutex_id *id)
{
	xSemaphoreHandle xMutex = xSemaphoreCreateMutex();
	if(xMutex == NULL)
	{
		uplus_sys_log("[u+] Mutex Created Fail");
		return -1;
	}
	*id = xMutex;
	return 0;
}

/*!
 * \brief 获取互斥信号量。
 * \param [in] id 信号量ID。
 * \param [in] time_wait 等待操作，TIME_NO_WAIT、TIME_WAIT_FOREVER或者等待时间（单位毫秒）。
 * \return 成功返回0，失败返回-1。
 */

uplus_s32 uplus_os_mutex_take(uplus_mutex_id id, uplus_s32 time_wait)
{ 
	uint32_t wait_tick = 0;
	if(time_wait == 0)
		wait_tick = 0;
	else if (time_wait == -1)
	{
		wait_tick = portMAX_DELAY;
	}
	else
	{
		wait_tick = osiMsToOSTick(time_wait);
	}
	if(xSemaphoreTake(id, wait_tick) == pdPASS)
		return 0;
	else
		return -1;
}

/*!
 * \brief 释放互斥信号量。
 * \param [in] id 信号量ID。
 * \return 成功返回0，失败返回-1。
 */
uplus_s32 uplus_os_mutex_give(uplus_mutex_id id)
{
	if(xSemaphoreGive(id) == pdPASS)
		return 0;
	else
		return -1;
}

/*!
 * \brief 删除互斥信号量。
 * \param [in] id 信号量ID。
 * \return 成功返回0，失败返回-1。
 */
uplus_s32 uplus_os_mutex_delete(uplus_mutex_id id)
{
	vSemaphoreDelete(id);

	return 0;
}

/*!
 * \brief 创建同步信号量。
 * \param [out] id 如果创建成功，返回创建的信号量ID；否则不填写。信号量ID不能为NULL。
 * \return 成功返回0，失败返回-1。
 */
uplus_s32 uplus_os_sem_create(uplus_sem_id *id)
{
	xSemaphoreHandle xBinarySemaphore = NULL;

	vSemaphoreCreateBinary(xBinarySemaphore);
	if(xBinarySemaphore != NULL)
	{
		*id = xBinarySemaphore;
		return 0;
	}
	return -1;
}

/*!
 * \brief 获取同步信号量。
 * \param id 信号量ID。
 * \param [in] time_wait 等待操作，TIME_NO_WAIT、TIME_WAIT_FOREVER或者等待时间（单位毫秒）。
 * \return 成功返回0，失败返回-1。
 */
uplus_s32 uplus_os_sem_take(uplus_sem_id id, uplus_s32 time_wait)
{
	//uplus_sys_log("[zk u+] os_sem_take time=%d", time_wait);
	return uplus_os_mutex_take(id, time_wait);	
}

/*!
 * \brief 释放同步信号量。
 * \param [in] id 信号量ID。
 * \return 成功返回0，失败返回-1。
 */
uplus_s32 uplus_os_sem_give(uplus_sem_id id)
{
	return uplus_os_mutex_give(id);
}

/*!
 * \brief 删除互斥信号量。
 * \param [in] id 信号量ID。
 * \return 成功返回0，失败返回-1。
 */
uplus_s32 uplus_os_sem_delete(uplus_sem_id id)
{
	return uplus_os_mutex_delete(id);
}

/*!
 * \brief 获取系统运行时间
 * \note 获取系统运行时间。uplugSDK不关心系统时间单位，但时间单位精度要小于100毫秒。
 * \return 系统运行时间。
 */
uplus_time uplus_os_current_time_get(void)
{
	uplus_time time = (uplus_time)xTaskGetTickCount();
	//uplus_sys_log("[zk u+] os_current_time_get time=%d", time);
	return time;
}

/*!
 * \brief 计算时间差。
 * \param [in] new_time 新时间（uplus_os_current_time_get返回的时间）。
 * \param [in] old_time 旧时间（uplus_os_current_time_get返回的时间）。
 * \return 时间差，单位毫秒。
 */
uplus_time uplus_os_diff_time_cal(uplus_time new_time, uplus_time old_time)
{
	uplus_time time=0;
	//uplus_sys_log("[zk u+] os_diff_time_cal new=%d, old=%d", new_time, old_time);
	if(new_time >= old_time)
		time = (new_time - old_time)*20;
	else
		time = ((0xFFFFFFFF - old_time) + new_time)*20;

	return time;
}


