#include "includes.h"

struct tm time_now = {0, 02, 17, 27, 3, 2015};
//DWORD ttime;
OS_EVENT *RTCMutex; /*time_now 变量保护 互斥信号*/

void vTaskRTC(void *p_arg)
{   
    u8 err;
    RTCMutex = OSMutexCreate(TASK_PIP_PRIO, &err);  /*创建互斥信号量*/
    if(err != OS_ERR_NONE)
    {
        printf("RTCMutex create failure  err = %d\n", err);
    }
    
	vRTCTime_SetCalendarTime(time_now);   /*设定初始时间*/
	for(;;)
	{
        OSMutexPend(RTCMutex, 0, &err);   /*获取互斥锁*/   
        time_now = xRTCTimeConvUnixToCalendar(u32RTCTime_GetUnixTime());
        OSMutexPost(RTCMutex);            /*释放互斥锁*/  
		//printf("\r\nTime:%d-%d-%d,%d:%d:%d\r\n", time_now.tm_year, time_now.tm_mon, time_now.tm_mday, time_now.tm_hour, time_now.tm_min, time_now.tm_sec);       
	}	
}
