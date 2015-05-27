#ifndef __TASKCOMMUNICATION_H__
#define __TASKCOMMUNICATION_H__

/*HEAD定义*/
#define HEAD1       02
#define HEAD2       0xAA

#define END1 	    0x55
#define END2 		0x03

/*指令定义*/
#define Distance    1          //获取距离
#define VERSION     2          //获取版本号
#define Temp        3          //温度
#define TimeGet     4          //时间
#define TimeCalibration   5    //时间校准
#define Lift              6    //升降平台指令簇
#define StepMotor         7    //电机指令簇

/*升降平台指令簇*/
#define  LiftUp                       0x01      //平台上升                                        
#define  LiftDown					  0x02 		//平台下降
#define  LiftStop                     0x04      //平台停止
#define  LiftPreSetPosition_Set       0x08      //预置位设置
#define  LiftPreSetPosition_Invoke    0x10      //预置位置调用
#define  LiftPreSetPosition_Get       0x20      //预置位获取
#define  LiftPositionTostore          0x40      //预置位存储

/*电机指令簇*/
#define MotorFor                      0x01 //前进
#define MotorBack                     0x02 //后退
#define MotorStop                     0x04 //停止

/*指令长度定义*/
#define LEN         3

#define TimeLen     7

typedef struct 
{
    unsigned char CMD;             /*指令内容*/ 
    unsigned char RDataLen;        /*数据长度*/    
    unsigned char FrameLen;        /*数据长度*/
    unsigned char CHECK;           /*异或校验值*/
    unsigned char FrameRData[10];  /*接收数据数组*/
}FrameS;/*协议解析内容 结构体*/


void vTaskComm(void *p_arg);
static void prvDisReport(void);
static void prvTempReport(void);
static void prvTimeGet(void);
static void prvTimeCalibration(void);
static void prvSendBuFlush(void);
static void prvLift(void);
static void prvMotor(void);
#endif
