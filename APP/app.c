
#include "lwip/sockets.h"
#include "netif/etharp.h"
#include "lwip/dhcp.h"
#include "ethernetif.h" 
#include "lwip/timers.h"
#include "lwip/tcp_impl.h"
#include "lwip/ip_frag.h"
#include "lwip/tcpip.h" 

#include "includes.h"
OS_STK task_AM2305_stk[TASK_AM2305_STK_SIZE];    //定义栈
OS_STK task_SR04_stk[TASK_SR04_STK_SIZE];        //定义栈
OS_STK task_COMM_stk[TASK_COMM_STK_SIZE];        //定义栈
OS_STK task_RTC_stk[TASK_RTC_STK_SIZE];          //定义栈
OS_STK task_MSD_stk[TASK_MSD_STK_SIZE];          //定义栈
OS_STK task_LIFT_stk[TASK_LIFT_STK_SIZE];        //定义栈
OS_STK task_STEP_stk[TASK_STEP_STK_SIZE];        //定义栈



#if uDEBUG > 0u
OS_STK task_STKCHK_stk[TASK_STKCHK_STK_SIZE];
static void vTaskStkCHK(void *p_arg);
#endif


///********************/
//主任务
void Task_Start(void *p_arg)
{
   
  (void)p_arg;    //'p_arg' 并没有用到，防止编译器提示警告
  OSStatInit();
     
//  OSTaskCreateExt(vTask2305,(void *)0,           //创建AM2305任务
//    (OS_STK *)&task_AM2305_stk[TASK_AM2305_STK_SIZE - 1], TASK_AM2305_PRIO,
//	TASK_AM2305_PRIO, (OS_STK *)&task_AM2305_stk[0], TASK_AM2305_STK_SIZE, 
//	(void *)0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR
//  ); 
  OSTaskCreateExt(vTaskStepMotor,(void *)0,           //创建StepMotor自动任务
    (OS_STK *)&task_STEP_stk[TASK_STEP_STK_SIZE - 1], TASK_STEP_PRIO,
	TASK_STEP_PRIO, (OS_STK *)&task_STEP_stk[0], TASK_STEP_STK_SIZE, 
	(void *)0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR
  );    
  OSTaskCreateExt(vTaskLIFT,(void *)0,           //创建LIFT任务
    (OS_STK *)&task_LIFT_stk[TASK_LIFT_STK_SIZE - 1], TASK_LIFT_PRIO,
	TASK_LIFT_PRIO, (OS_STK *)&task_LIFT_stk[0], TASK_LIFT_STK_SIZE, 
	(void *)0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR
  );
////////    
////////  OSTaskCreateExt(vTaskSR04,(void *)0,           //创建SR04任务
////////    (OS_STK *)&task_SR04_stk[TASK_SR04_STK_SIZE - 1], TASK_SR04_PRIO,
////////	TASK_SR04_PRIO, (OS_STK *)&task_SR04_stk[0], TASK_SR04_STK_SIZE, 
////////	(void *)0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR
////////  );
////////  
  OSTaskCreateExt(vTaskComm,(void *)0,           //创建通信任务
    (OS_STK *)&task_COMM_stk[TASK_COMM_STK_SIZE - 1], TASK_COMM_PRIO,
    TASK_COMM_PRIO, (OS_STK *)&task_COMM_stk[0], TASK_COMM_STK_SIZE, 
	(void *)0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR
  );
//  
//  OSTaskCreateExt(vTaskRTC,(void *)0,           //创建rtc任务
//    (OS_STK *)&task_RTC_stk[TASK_RTC_STK_SIZE - 1], TASK_RTC_PRIO7,
//    TASK_RTC_PRIO7, (OS_STK *)&task_RTC_stk[0], TASK_RTC_STK_SIZE, 
//	(void *)0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR
//  );

////  OSTaskCreateExt(vTaskMSD,(void *)0,           //创建SD存储任务
////    (OS_STK *)&task_MSD_stk[TASK_MSD_STK_SIZE - 1], TASK_MSD_PRIO,
////    TASK_MSD_PRIO, (OS_STK *)&task_MSD_stk[0], TASK_MSD_STK_SIZE, 
////	(void *)0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR
////  );
//#if uDEBUG > 0u
//  OSTaskCreateExt(vTaskStkCHK,(void *)0,           //创建堆栈检查任务
//    (OS_STK *)&task_STKCHK_stk[TASK_STKCHK_STK_SIZE - 1], TASK_STKCHK_PRIO,
//    TASK_STKCHK_PRIO, (OS_STK *)&task_STKCHK_stk[0], TASK_STKCHK_STK_SIZE, 
//	(void *)0, OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR
//  );
//#endif	 

  while(1)
  {
     OSTaskDel(OS_PRIO_SELF);
  }
}

void vTaskStkCHK(void *p_arg)
{
    
	OS_STK_DATA stk_data;

	(void)p_arg;  //'p_arg' 并没有用到，防止编译器提示警告
    
	for(;;)
	{

        OSTaskStkChk(TASK_SR04_PRIO, &stk_data);
		printf("TASK_SR04:       USED /FREE:%d/%d   USAGE: %%%d\r\n",
                stk_data.OSUsed, stk_data.OSFree, 
                (stk_data.OSUsed * 100) / (stk_data.OSFree + stk_data.OSUsed));

		OSTaskStkChk(TASK_COMM_PRIO, &stk_data);
		printf("TASK_COMM:       USED /FREE:%d/%d  USAGE: %%%d\r\n",
                stk_data.OSUsed, stk_data.OSFree, 
                (stk_data.OSUsed * 100) / (stk_data.OSFree + stk_data.OSUsed));
        
		OSTaskStkChk(TASK_RTC_PRIO7, &stk_data);
		printf("TASK_RTC:     USED /FREE:%d/%d  USAGE: %%%d\r\n",
                stk_data.OSUsed, stk_data.OSFree, 
                (stk_data.OSUsed * 100) / (stk_data.OSFree + stk_data.OSUsed));
        
		OSTaskStkChk(OS_PRIO_SELF, &stk_data);
		printf("TASK_STKCHK:     USED /FREE:%d/%d  USAGE: %%%d\r\n",
                stk_data.OSUsed, stk_data.OSFree, 
                (stk_data.OSUsed * 100) / (stk_data.OSFree + stk_data.OSUsed));
        
        OSTaskStkChk(TASK_AM2305_PRIO, &stk_data);
		printf("TASK_AM2305:     USED /FREE:%d/%d  USAGE: %%%d\r\n",
                stk_data.OSUsed, stk_data.OSFree, 
                (stk_data.OSUsed * 100) / (stk_data.OSFree + stk_data.OSUsed));
                
        OSTaskStkChk(TASK_MSD_PRIO, &stk_data);
		printf("TASK_MSD:     USED /FREE:%d/%d  USAGE: %%%d\r\n\r\r",
                stk_data.OSUsed, stk_data.OSFree, 
                (stk_data.OSUsed * 100) / (stk_data.OSFree + stk_data.OSUsed));



                

        OSTimeDlyHMSM(0,0,5,0);
	}
}
