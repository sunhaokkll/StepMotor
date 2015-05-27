#include "includes.h"

void vRTCConfig(void)
{
//    NVIC_InitTypeDef NVIC_InitStructure;
    
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_BKP | RCC_APB1Periph_PWR, ENABLE); /*打开BKP PWR时钟*/

//	NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//	NVIC_Init(&NVIC_InitStructure);

	PWR_BackupAccessCmd(ENABLE);     /*使能RTC和后备寄存器访问*/
	BKP_DeInit();                     /*复位备份寄存器设置*/
	RCC_LSEConfig(RCC_LSE_ON);       /*开启LSE*/
	while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET);          /*等待LSE起振*/
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);                       /*选择LSE为RTC时钟源*/
	RCC_RTCCLKCmd(ENABLE);                                        /*使能RTC时钟*/
    RTC_WaitForSynchro();                                         /*等待RTC寄存器同步完成*/
    RTC_WaitForLastTask();										/*等待最近 一次对RTC寄存器的写操作完成*/
//	RTC_ITConfig(RTC_IT_SEC, ENABLE);                             /*使能RTC秒中断*/
    RTC_WaitForLastTask();										/*等待最近 一次对RTC寄存器的写操作完成*/
    RTC_SetPrescaler(32767);                                    /*设置RTC时钟分频 32767 计数频率为 32.768kHz / (32767+1) = 1Hz*/
    RTC_WaitForLastTask();										/*等待最近 一次对RTC寄存器的写操作完成*/

}
/*将给定的公元格式时间转换成UNIX时间格式*/
void      vRTCTime_SetCalendarTime(struct tm t)    
{
    vRTCTime_SetUnixTime(vRTCTime_ConvCalendarToUnix(t));
	//return;
}
/*写入RTC时钟当前时间*/
u32     vRTCTime_ConvCalendarToUnix(struct tm t)        
{
    t.tm_year -= 1900;
	return mktime(&t);
}
/*转换UNIX时间戳为日历时间*/
struct tm xRTCTimeConvUnixToCalendar(time_t t)      
{
    struct tm *t_tm;
	t_tm = localtime(&t);
	t_tm -> tm_year += 1900;
	return *t_tm;
}
 /*从RTC获取当前UNIX时间格式的时间值*/
u32       u32RTCTime_GetUnixTime(void)            
{
    return (u32)RTC_GetCounter();
}
void      vRTCTime_SetUnixTime(time_t t)           /*将给定的Unix时间戳写入RTC*/
{
    RTC_WaitForLastTask();										/*等待最近 一次对RTC寄存器的写操作完成*/
    RTC_SetCounter((u32)t);
    RTC_WaitForLastTask();										/*等待最近 一次对RTC寄存器的写操作完成*/
    //return;
}
