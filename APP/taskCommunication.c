#include "includes.h"

/********引用全局变量**********/
extern vu8 fSR04Distance;
extern float Current_Temp; 
extern struct tm time_now;
extern OS_EVENT *RTCMutex;            /*time_now 变量保护 互斥信号*/
extern OS_FLAG_GRP *LiftStatus;       /*Lift操作 状态标志组*/
extern u16 PreSet_Position_No;

/******************************/

FrameS xFrame;

/********数据传输数组**********/
u8 u8SendBuf[20];
/*******函数指针映射数组*******/
void  (*pf[8])(void) =
{   0,             /*0:未定义*/
    prvDisReport,   /*Distance(1):距离上报*/
    0, 
    prvTempReport,   /*Temp(3):芯片温度查询*/
    prvTimeGet,       /*Timeget(4):时间查询*/
    prvTimeCalibration,  /*TimeCal(5):时间校准*/
    prvLift,
    prvMotor
};
/******************************/

void vTaskComm(void *p_arg)
{
	static u8 State = 0;
	static u8 i = 0;
    u8 data;

	for(;;)
    {
        if(FrameBufRead(&data))
        {
            switch(State)
            {
                case 0:
                    if(data == HEAD1)      /*判断帧头HEAD1*/
                        State = 1;
                    else
                        State = 0;                 
                    break;
                case 1:
                    if(data == HEAD2)      /*判断帧头HEAD2*/
                    {	
                        i = 0;
                        xFrame.CHECK = 0;
                        State = 2;
                    }
                    else
                        State = 0;
                    break;
                case 2:                   /*判断指令*/
                    if((data == Distance) || (data == VERSION) || (data == Temp) || (data == TimeGet) || (data == TimeCalibration) || (data == Lift) || (data == StepMotor))
                    {
                        xFrame.CMD = data;
                        xFrame.CHECK ^= data;
                        State = 3;
                    }
                    else
                        State = 0;
                    break;
                case 3:                                /*数据长度 在（0~10）之间 进行数据接收*/
                    if((data > 0) && (data < 10))
                    {
                        xFrame.RDataLen = data;
                        xFrame.CHECK ^= data;
                        State = 4;
                    }
                    else if(data == 0)                 /*数据长度为0时 不接收数据*/
                     {
                        xFrame.RDataLen = data;
                        xFrame.CHECK ^= data;
                        State = 5;
                    }                       
                    else 
                        State = 0;
                    break;
                case 4:
                    if( i < xFrame.RDataLen )           /*数据接收*/
                    {
                        xFrame.FrameRData[i] = data;
                        xFrame.CHECK ^= data;
                        i++;
                    }
                    if( i == xFrame.RDataLen )
                    {
                        State = 5;
                        i = 0;
                    }
                    break;
                case 5:                                /*校验比较*/
                    if(data == xFrame.CHECK)
                        State = 6;
                    else
                        State = 0;
                    break;
                case 6:
                    if(data == END1)
                        State = 7;
                    else
                        State = 0;				
                    break;
                case 7:
                    if(data == END2)
                    {
                        pf[xFrame.CMD]();             /*调用处理函数*/
                        State = 0;
                    }
                    else
                        State = 0;
                    break;

                default:
                    break;
            }
        }
        else 
            OSTimeDlyHMSM(0, 0, 0, 60);      /**/ 
//		lwip_pkt_handle();
//		OSTimeDlyHMSM(0, 0, 0, 60);      /**/
              
    }
}
/**********距离上报*************/
void prvDisReport(void)
{
    u8 i = 0;
    prvSendBuFlush();
    u8SendBuf[0] = HEAD1;
    u8SendBuf[1] = HEAD2;
    u8SendBuf[2] = xFrame.CMD;
    
    u8SendBuf[3] = 1;
    u8SendBuf[4] = (u8)fSR04Distance;
    for(i= 2; i < 5; i++)
        u8SendBuf[5] ^= u8SendBuf[i];
    u8SendBuf[6] = END1;
    u8SendBuf[7] = END2;
    UART1_Sendarray(u8SendBuf, 8);
    
}
/**********温度上报*************/
void prvTempReport(void)
{
    u8 i = 0;
    prvSendBuFlush();
    u8SendBuf[0] = HEAD1;
    u8SendBuf[1] = HEAD2;
    u8SendBuf[2] = xFrame.CMD;
    
    u8SendBuf[3] = 2;
    u8SendBuf[4] = (u8)(Current_Temp);
    u8SendBuf[5] = (u8)((u16)(Current_Temp * 100) % 100);
    for(i= 2; i < 6; i++)
        u8SendBuf[6] ^= u8SendBuf[i];
    u8SendBuf[7] = END1;
    u8SendBuf[8] = END2;
    UART1_Sendarray(u8SendBuf, 9);    
}
/*********时间上报*************/

void prvTimeGet()
{
    u8 i = 0;
    u8 err;
    
	prvSendBuFlush();
    u8SendBuf[0] = HEAD1;
    u8SendBuf[1] = HEAD2;
    u8SendBuf[2] = xFrame.CMD;
    
    u8SendBuf[3]  = 7;
    OSMutexPend(RTCMutex, 0, &err);   /*释放互斥锁*/ 
	u8SendBuf[4]  = time_now.tm_year / 100;
	u8SendBuf[5]  = time_now.tm_year % 100;

	u8SendBuf[6]  = time_now.tm_mon;
	u8SendBuf[7]  = time_now.tm_mday;
	u8SendBuf[8]  = time_now.tm_hour;
	u8SendBuf[9]  = time_now.tm_min;
	u8SendBuf[10] = time_now.tm_sec;
    OSMutexPost(RTCMutex);            /*释放互斥锁*/ 
	for(i = 2; i < 11; i++)
		u8SendBuf[11] ^= u8SendBuf[i];
    
    u8SendBuf[12] = END1;
    u8SendBuf[13] = END2;
    
	UART1_Sendarray(u8SendBuf, 14); 	
}
/*********时间校准*************/
void prvTimeCalibration()
{
    u8 err;
    
    OSMutexPend(RTCMutex, 0, &err);    /*获取互斥锁*/  
    time_now.tm_year = (xFrame.FrameRData[0] * 100) + xFrame.FrameRData[1];
	time_now.tm_mon  = xFrame.FrameRData[2];
	time_now.tm_mday  = xFrame.FrameRData[3];
	time_now.tm_hour  = xFrame.FrameRData[4];
	time_now.tm_min  = xFrame.FrameRData[5];
	time_now.tm_sec  = xFrame.FrameRData[6];

	vRTCTime_SetCalendarTime(time_now);
    OSMutexPost(RTCMutex);            /*释放互斥锁*/ 
    
    prvSendBuFlush();
    u8SendBuf[0] = HEAD1;
    u8SendBuf[1] = HEAD2;
    u8SendBuf[2] = xFrame.CMD;
    u8SendBuf[3] = 0;
    u8SendBuf[4] = u8SendBuf[2];
    u8SendBuf[5] = END1;
    u8SendBuf[6] = END2;
    
	UART1_Sendarray(u8SendBuf, 7); 
}
/*********升降平台处理*************/
void prvLift(void)
{
    u8 err;
    if(xFrame.RDataLen == 3)
        PreSet_Position_No = (u16)((u16)(xFrame.FrameRData[1] << 8) | xFrame.FrameRData[2]); 
    OSFlagPost(LiftStatus, xFrame.FrameRData[0], OS_FLAG_SET, &err);
    
}

/*************电机动作处理***************/
void prvMotor()
{
//    unsigned short Pulse_Num, Freq;
//    if(xFrame.RDataLen == 5)
//    {
//        Freq  = (xFrame.FrameRData[1] << 8) | (xFrame.FrameRData[2]);

//        Pulse_Num = (xFrame.FrameRData[3] << 8) | (xFrame.FrameRData[4]);
//    }
//    switch(xFrame.FrameRData[0])
//    {
//        case MotorFor:
//            MotorF(Freq, Pulse_Num);
//            break;
//        case MotorBack:
//            MotorB(Freq, Pulse_Num);
//            break;
//        case MotorStop:
//            MotorS();
//            break;
//        default:
//            break;
//    }        
}
void prvSendBuFlush()
{
    u8 i= 0;
    for(i = 0;i < 20; i++)
    {
        u8SendBuf[i] = 0;    
    }
}
//time_now.tm_year, time_now.tm_mon, time_now.tm_mday, time_now.tm_hour, time_now.tm_min, time_now.tm_sec



