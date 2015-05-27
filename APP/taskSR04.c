#include "includes.h"

/*全局变量*/
vu8 fSR04Distance = 0;  /*SR04测得距离存放变量*/ 
/*END*/

/*局部变量*/
vu8 EXTI_LINE7_FLAG = 0; /*SR04测量标志位：0：测量完毕或无测量任务，2：测量开始，上升沿，1：测量过程，下降沿*/
u16 *pp;                /*测量数据中间值*/        
OS_EVENT *SR04CommMbox; /*测量数据传递邮箱*/      
/*END*/

void vTaskSR04(void *p_arg)
{ 
	u8 err;             /*邮箱等待错误指示*/ 
    u16 Pulse, pa, pb;  /*测量数据计算中间值、指针赋值专用*/
	void *msg;          /*邮箱数据指针定义*/
    
    pp = &pa;           /*指针初始化*/
    msg = &pb;          /*指针初始化*/
    
    (void)p_arg;    /*'p_arg' 并没有用到，防止编译器提示警告*/
    SR04CommMbox = OSMboxCreate((void *)0); /*创建邮箱*/
	
    
    for(;;)
    {
        /*启动测量*/
        OSTaskSuspend(TASK_AM2305_PRIO);
        
        GPIO_SetBits(GPIOE, GPIO_Pin_8);       /*拉高Trig管脚*/
        EXTI_LINE7_FLAG = 2;                   /*测量标志位赋值*/
        delay_10us();						   /*持续10us以上*/                        
        GPIO_ResetBits(GPIOE, GPIO_Pin_8);     /*拉低Trig管脚，激活SR04测量*/
        /*end*/
        
        /*等待邮箱信号*/
        msg = OSMboxPend(SR04CommMbox, 5, &err);
        OSTaskResume(TASK_AM2305_PRIO);
        if(err == OS_ERR_NONE)
        {
            Pulse = *((u16*)msg);
            fSR04Distance = ((Pulse) * 340) / 20000; /*距离 = (脉冲平均值 * 声速) / (1000 * 2) mm */
        }        
        OSTimeDlyHMSM(0, 0, 0, 50);                   
    }
}
/*软件延时 10us*/
void delay_10us(void)
{
    u8 n;
    for(n = 13; n > 0; n--);
}

void delay_ms(unsigned int i)
{
    u8 n;
    while(i--)
    {
        for(n = 0;n < 100;n++)
            delay_10us();
    }
}

void vtaskSR04Handle(void)
{
	if(EXTI_LINE7_FLAG == 2)
	{
		TIM_SetCounter(TIM2, 0);  /*上升沿中断 定时器打开 清零计数*/
		TIM_Cmd(TIM2, ENABLE);
		EXTI_LINE7_FLAG --;
	}
	else if(EXTI_LINE7_FLAG == 1)
	{
		*pp = TIM_GetCounter(TIM2);/*读取计数*/
		TIM_Cmd(TIM2, DISABLE);    /*下降沿中断 定时器关闭 清零计数*/
        TIM_SetCounter(TIM2, 0);
		EXTI_LINE7_FLAG = 0;       /*标志位置0*/
		OSMboxPost(SR04CommMbox, (void *)(pp));/*发送邮箱数据*/
	}
}

void EXTI9_5_IRQHandler(void)
{
    OSIntEnter();
	
	if(EXTI_GetITStatus(EXTI_Line7) == SET)
	{
		vtaskSR04Handle();   /*调用处理函数*/
		EXTI_ClearITPendingBit(EXTI_Line7);
	}
    
	OSIntExit();
}
