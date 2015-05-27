#include "includes.h"
#include <stdlib.h>

#define LiftTime_calibration 0 //和串口数据发送时间有关 7个数据 耗时7个时间常量


OS_FLAG_GRP *LiftStatus;/*升降平台 事件标志组*/

u8 LiftU[7] = {0xCA, 0x20, 0xF0, 0x21, 0x01, 0x01, 0xAC}; /*上升报文*/
u8 LiftD[7] = {0xCA, 0x20, 0xF0, 0x21, 0x01, 0x02, 0xAC}; /*下降报文*/
u8 LiftS[7] = {0xCA, 0x20, 0xF0, 0x21, 0x01, 0x00, 0xAC}; /*停止报文*/

s32 PreSet_Position[512];   /*预置位 数组*/
u16 PreSet_Position_No;     /*预置位 号*/
s32 Time_LiftTable;         /*升降平台 全局位置变量*/
const u32 FLASH_PAGE_ADDR = 0x8060800; /*FLASH起始地址 存储 预置位 数组 2K 一页*/
void vTaskLIFT(void *p_arg)
{   
    u8  err;
    u16 PreSet_Position_NoBuf;  /*预置位号 缓存*/
    u32 Time_LiftTable_buf1, Time_LiftTable_buf2, Time_LiftTable_buf; /*位置（时间）变量 缓存*/

    s32 PreSetbuf;              /*预置位 调用 中间变量*/
    OS_FLAGS LiftValues, LiftValuesTemp; /*升降平台 事件标志变量 变量缓存*/
    LiftValuesTemp = 0;                   
    LiftStatus = OSFlagCreate(0x00, &err); /*创建事件标志组*/
    if(err != OS_ERR_NONE)
    {
        printf("LiftStatus create failure  err = %d\n", err);
    }
    
	for(;;)
	{
        LiftValues = OSFlagPend(LiftStatus, LiftUp + LiftDown + LiftStop + LiftPreSetPosition_Get + LiftPreSetPosition_Invoke + LiftPreSetPosition_Set + LiftPositionTostore,
                                OS_FLAG_WAIT_SET_ANY + OS_FLAG_CONSUME, 0, &err);       /*等待事件标志组*/ 
        if(err == OS_ERR_NONE)
        {
            switch(LiftValues)/*根据事件标志组进行操作判断*/
            {
                case LiftUp:
                    if(LiftValuesTemp == LiftDown) /*如果上次操作为向下动作  发送停止报文 记录当前位置*/
                    {
                        UART4_Sendarray(LiftS, 7);
                        Time_LiftTable_buf2 = OSTimeGet();
                        if(Time_LiftTable_buf2 < Time_LiftTable_buf1)   //判断UCOS系统时间是否超过0xffffffff 超过则在计算差值时  加上此值
                            Time_LiftTable_buf = Time_LiftTable_buf2 - Time_LiftTable_buf1 + 0xFFFFFFFF;
                        else 
                            Time_LiftTable_buf = Time_LiftTable_buf2 - Time_LiftTable_buf1;
                        Time_LiftTable +=  Time_LiftTable_buf;
                    }
                    else if(LiftValuesTemp == LiftUp)
                    {
                        break;
                    }
                    UART4_Sendarray(LiftU, 7);          /*发送动作报文*/
                    Time_LiftTable_buf1 = OSTimeGet();  /*获取开始动作的时间*/
                    break;
                case LiftDown:
                    if(LiftValuesTemp == LiftUp) /*如果上次操作为向上动作  发送停止报文 记录当前位置*/
                    {
                        UART4_Sendarray(LiftS, 7);
                        Time_LiftTable_buf2 = OSTimeGet();
                        if(Time_LiftTable_buf2 < Time_LiftTable_buf1) //判断UCOS系统时间是否超过0xffffffff 超过则在计算差值时  加上此值
                            Time_LiftTable_buf = Time_LiftTable_buf2 - Time_LiftTable_buf1 + 0xFFFFFFFF;
                        else 
                            Time_LiftTable_buf = Time_LiftTable_buf2 - Time_LiftTable_buf1;                        
                        Time_LiftTable -=  Time_LiftTable_buf;
                    }
                    else if(LiftValuesTemp == LiftDown)
                    {
                        break;
                    }
                    UART4_Sendarray(LiftD, 7);       /*发送动作报文*/
                    Time_LiftTable_buf1 = OSTimeGet();/*获取开始动作的时间*/
                    break;
                case LiftStop:
                    UART4_Sendarray(LiftS, 7);      /*发送停止报文*/
                    Time_LiftTable_buf2 = OSTimeGet();/*获取动作停止的时间*/
                    /*计算动作时间*/
                    if(Time_LiftTable_buf2 < Time_LiftTable_buf1)   //判断UCOS系统时间是否超过0xffffffff 超过则在计算差值时  加上此值
                        Time_LiftTable_buf = Time_LiftTable_buf2 - Time_LiftTable_buf1 + 0xFFFFFFFF;
                    else 
                        Time_LiftTable_buf = Time_LiftTable_buf2 - Time_LiftTable_buf1;
                    /*根据动作方向 计算 全局位置变量*/
                    if(LiftValuesTemp == LiftDown)
                    {                        
                        Time_LiftTable +=  Time_LiftTable_buf;
                    }
                    if(LiftValuesTemp == LiftUp)
                    {                        
                        if(Time_LiftTable <= Time_LiftTable_buf)  /*判断是否已到上极限 默认上极限状态 位置变量为0*/
                            Time_LiftTable = 0;
                        else
                            Time_LiftTable -=  Time_LiftTable_buf;
                    }
                    printf("当前位置%d\n", Time_LiftTable);
                    break;
                case LiftPreSetPosition_Get:
                    break;
                case LiftPreSetPosition_Invoke:
                    printf("预置位%d 调用:%d 当前位置%d\n",PreSet_Position_No, PreSet_Position[PreSet_Position_No], Time_LiftTable);
                    PreSetbuf = PreSet_Position[PreSet_Position_No] - Time_LiftTable;  /*计算需要动作的距离（时间）*/
                    /*根据时间的正负 计算动作方向和距离（时间）*/
                    if(PreSetbuf > 0)
                    {                
                        UART4_Sendarray(LiftD, 7);
                        Time_LiftTable_buf1 = OSTimeGet(); 
                        OSTimeDly(PreSetbuf - LiftTime_calibration);
                    }
                    else if(PreSetbuf < 0)
                    {                        
                        UART4_Sendarray(LiftU, 7);  
                        Time_LiftTable_buf1 = OSTimeGet(); 
                        OSTimeDly(abs(PreSetbuf) - LiftTime_calibration);                    
                    }
                    /*时间到后 发送停止报文*/
                    UART4_Sendarray(LiftS, 7);                    
                    Time_LiftTable_buf2 = OSTimeGet();
                    printf("实际动作时间= %d   ", Time_LiftTable_buf2 - Time_LiftTable_buf1);
                    printf("理论需动作时间= %d\n", abs(Time_LiftTable - PreSet_Position[PreSet_Position_No]));
                    /*变更当前位置变量*/
                    Time_LiftTable = PreSet_Position[PreSet_Position_No];
                    break;
                case LiftPreSetPosition_Set:
                    if(LiftValuesTemp == LiftStop) /*上次接收的指令必须为停止 才能进行预置位设置*/
                    {
                        PreSet_Position[PreSet_Position_No] = Time_LiftTable;
                        printf("预置位%d 设置成功:%d\n",PreSet_Position_No, PreSet_Position[PreSet_Position_No]);
                    }
                    break;
                case LiftPositionTostore:
                    /*将预置位数据存入FLASH 地址为 FLASH_PAGE_ADDR */
                    PreSet_Position_NoBuf = 0;
                    RCC_HSICmd(ENABLE);    /*开启HSI*/
                    FLASH_Unlock();        /*FLASH控制块 解锁*/
                    /*清除一些标志位*/
                    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
                    /*擦除 FLASH_PAGE_ADDR开始的FLASH 页*/
                    FLASH_ErasePage(FLASH_PAGE_ADDR);
                    do/*写入数据 2K*/
                    {
                        FLASH_ProgramWord((FLASH_PAGE_ADDR + PreSet_Position_NoBuf * 4), PreSet_Position[PreSet_Position_NoBuf]);
                        PreSet_Position_NoBuf++;
                    }
                    while(PreSet_Position_NoBuf != 256);
                    FLASH_Lock();/*锁定FLASH控制块*/
                    break;
                default:
                    break;
            }
            if(Time_LiftTable < 0)
                Time_LiftTable = 0;
            LiftValuesTemp = LiftValues;
        }
	}	
}
