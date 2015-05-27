#ifndef __RTC_H__
#define __RTC_H__
#include "includes.h"

void vRTCConfig(void); /*初始化函数 在系统启动时调用*/

/*UNIX时间相关函数*/
void      vRTCTime_SetCalendarTime(struct tm t);     /*将给定的公元格式时间转换成UNIX时间格式*/
struct tm xRTCTimeConvUnixToCalendar(time_t t);      /*转换UNIX时间戳为日历时间*/


static u32      vRTCTime_ConvCalendarToUnix(struct tm t);  /*写入RTC时钟当前时间*/
u32       u32RTCTime_GetUnixTime(void);              /*从RTC获取当前UNIX时间格式的时间值*/
static void      vRTCTime_SetUnixTime(time_t t);            /*将给定的Unix时间戳写入RTC*/
#endif
