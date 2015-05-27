#include "includes.h"

vu8   MotorActionState;//带加减速脉冲输出过程状态
                       //0:无动作   1：需要进入加速状态   2：加速过程中  加速完毕 进入恒速或减速  3：恒速完毕  进入减速
                       //4:需要进入减速状态               5：减速过程中  减速完毕 停止 返回状态 0
vu8   MotorDir; //1：正转 2：反转 0:不动作
vu8   HomeFlag,CWFlag,CCWFlag; // 原点、正反限位状态 CWFlag 为1时 无法进行正向动作   CCWFlag 同理
vu8   ACC_Status_Flag, DEC_Status_Flag, ConstantVelocity_Status_Flag;    //加速 减速 匀速状态标志   
u16   PulseNum_Buf[50], PulseNum_Sum_Buf[50];//脉冲个数变量
u16   MotorCycle_Buf[50]; //周期变量  单位 10us  加减速过程 根据

vu32   MotorFreqTarget, MotorCycleTarget;//目标频率
vu32   MotorFreqReal, MotorCycleReal;  //实时频率  Cycle 单位 10us
vu32   PulseNum_Global;//脉冲全局变量
vu32   PulseNum_TEMP;
vu32   PulseNum_ACC, PulseNum_DEC, PulseNum_ConstantVel;//中间变量 动作过程中 指示加减速各阶段所需动作脉冲数  暂定与加减速中间频率相同
vu32   PulseNum_Exti;

void Motor_Init(u16 TIM4per, u16 TIM5per, u16 TIM5Compare1);

void vPTOConfig(void)
{
    u8 i;
    
    /*根据宏定义进行加减速中间量计算*/
    for(i = 0; i < 50; i++)
    {
        MotorCycle_Buf[i] = 100000 / (ACC_HZ_MIN * (i + 1));
    }
    for(i = 0; i < 50; i++)
    {
        PulseNum_Buf[i] = (ACC_PERIOD *  100)/ MotorCycle_Buf[i];
    }
    PulseNum_Sum_Buf[0] = PulseNum_Buf[0];
    for(i = 1; i < 50; i++)
    {
        PulseNum_Sum_Buf[i] = PulseNum_Sum_Buf[i-1] + PulseNum_Buf[i];
    }
    
    MotorCycleReal = 0;
    PulseNum_Global = 0;
    Motor_Init(100, 100, 50);  
    
    TIM_SetCounter(TIM1, 0);
}

/****************************
**TIM5的通道1使用脉冲模式  
**
**
****************************/
//TIM5per:重装值
//Compare1:比较捕获1的预装载值
void Motor_Init(u16 TIM5per, u16 TIM1per, u16 TIM1Compare1)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    TIM_OCInitTypeDef TIM_OCInitStruct;
    GPIO_InitTypeDef GPIO_InitStruct;
		NVIC_InitTypeDef NVIC_InitStructure;
		/*EXTI初始化结构体*/
		EXTI_InitTypeDef EXTI_InitStructure;
    
    /*所需时钟 打开 定时器 1 5，PE9  复用时钟 如需软件 仿真 不能使用管脚映射 直接 使用PA8 注意更改管脚时钟和 初始化相关*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,  ENABLE);//使能定时器1的时钟
//    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5,  ENABLE);//使能定时器5的时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);//使能GPIOE时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,  ENABLE);//使能复用IO时钟
    
    /*传感器信号 管脚配置*/
		/*开启时钟GPIOD*/
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
		/*外部中断GPIOD1*/
		GPIO_InitStruct.GPIO_Pin =  GPIO_Pin_1| GPIO_Pin_2 | GPIO_Pin_4;	
		GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;       
		GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOD, &GPIO_InitStruct);
	
//		GPIO_InitStruct.GPIO_Pin =  GPIO_Pin_2;	
//		GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;       
//		GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
//		GPIO_Init(GPIOD, &GPIO_InitStruct);	
	
		GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource1);
		GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource2);
        GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource4);
	/*OVER*/
    
    
    GPIO_PinRemapConfig(GPIO_FullRemap_TIM1, ENABLE); ////Timer1完全映射  TIM1_CH1->PE9  TIM2_CH2->PE11  
    
    /*设置定时器1  TIM1_CH1 PE9 为 复用推挽*/
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;//复用推挽输出
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_11;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOE, &GPIO_InitStruct);
    
    
    
    
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;//复用推挽输出
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOE, &GPIO_InitStruct);
    
    
    /*设置方向IO PE0  为 推挽*/
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;//复用推挽输出
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOE,&GPIO_InitStruct);
    
////    /*TIM5 计时定时器 配置  */
////    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;//没有时钟分割
////    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;//向上计数模式
////    TIM_TimeBaseInitStruct.TIM_Prescaler = 7199;//预分频值,每100us计数一次
////    TIM_TimeBaseInitStruct.TIM_Period = TIM5per;//重装值
////    TIM_TimeBaseInit(TIM5, &TIM_TimeBaseInitStruct);  
////    
////    TIM_ITConfig(TIM5, TIM_IT_Update|TIM_IT_Trigger,ENABLE);//使能定时器5更新触发中断
////    TIM_ARRPreloadConfig(TIM5, ENABLE);//使能重装载
////    TIM_Cmd(TIM5, DISABLE);  //暂时不使能TIMx外设
    
		
		/*NPN开关设置外部中断通道1在上升沿触发中断*/
		EXTI_InitStructure.EXTI_Line = EXTI_Line1;
		EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
		EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
		EXTI_InitStructure.EXTI_LineCmd = ENABLE;
		EXTI_Init(&EXTI_InitStructure);
		
		/*NPN开关设置外部中断通道2在上升沿触发中断*/
		EXTI_InitStructure.EXTI_Line = EXTI_Line2;
		EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
		EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
		EXTI_InitStructure.EXTI_LineCmd = ENABLE;
		EXTI_Init(&EXTI_InitStructure);
        
        /*原点触发中断*/
		EXTI_InitStructure.EXTI_Line = EXTI_Line4;
		EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
		EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
		EXTI_InitStructure.EXTI_LineCmd = ENABLE;
		EXTI_Init(&EXTI_InitStructure);
		
       
	#ifdef VECT_TAB_RAM 	   
	 /* Set the Vector Table base location at 0x20000000 */ 	  
	 NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0); 
	#else /* VECT_TAB_FLASH */		  
	 /* Set the Vector Table base location at 0x08000000 */ 	  
	 NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);	
	#endif	 
	
	/* 选择NVIC优先分组4 所有4位用于抢占式优先级  */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4); 
////    /*定时器5更新中断配置*/ 
////    NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;  //TIM5中断
////    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = PWM_TIM_PRIO;  //先占优先级
////    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  //从优先级
////    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
////    NVIC_Init(&NVIC_InitStructure);  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器 	
   
   /*使能EXTI1_IRQn 优先级*/
	 NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;
	 NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = LS_EXTI_PRIO;
	 NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	 NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	 NVIC_Init(&NVIC_InitStructure);
	 
	    /*使能EXTI2_IRQn 优先级*/
	 NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;
	 NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = LS_EXTI_PRIO;
	 NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	 NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	 NVIC_Init(&NVIC_InitStructure);
	 
     	    /*使能原点 优先级*/
	 NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;
	 NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = LS_EXTI_PRIO;
	 NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	 NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	 NVIC_Init(&NVIC_InitStructure);
	 
    /*TIM1 脉冲输出定时器配置*/
    //TIM1工作 预分频值
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;//没有时钟分割
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;//向上计数模式
    TIM_TimeBaseInitStruct.TIM_Prescaler = 719;//预分频值,每10us计数一次
    TIM_TimeBaseInitStruct.TIM_Period = TIM1per;//重装值
    TIM_TimeBaseInitStruct.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM1,&TIM_TimeBaseInitStruct);
    
    TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM2;//在向上计数时，一旦TIMx_CNT<TIMx_CCR1时通道1为无效电平，否则为有效电平
    TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;//OC1输出使能
    TIM_OCInitStruct.TIM_OutputNState = TIM_OutputNState_Disable;
    TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;//有效电平为高
    TIM_OCInitStruct.TIM_Pulse = TIM1Compare1;//  比较捕获5的预装载值
    TIM_OC2Init(TIM1,&TIM_OCInitStruct);

    TIM_CtrlPWMOutputs(TIM1, ENABLE);   //高级定时器需要使用此函数
    TIM_OC1PreloadConfig(TIM1,TIM_OCPreload_Enable);//使能定时器1的通道1预装载寄存器
    TIM_ARRPreloadConfig(TIM1, ENABLE);
    TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);//使能定时器1更新触发中断

    TIM_Cmd(TIM1,DISABLE);//先不使能能TIM1
    
    /*定时器1更新中断配置*/ 
    NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;  //TIM5中断
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = PWM_TIM1_PRIO;  //先占优先级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  //从优先级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
    NVIC_Init(&NVIC_InitStructure);  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器 	
     
  /*TIM1 END*/
}

/*******************************************************************/
/***********************位置模式************************************/
/*******************************************************************/
/*******************************************************************/
/*  Freq:       频率
*   Pulse_Num:  绝对 脉冲数  位置
*
*
*/
void PTO_HZ_NUM(unsigned short Freq, unsigned int Pulse_Num)
{
    u8 i;
    s32 PulseNum_Rel;        //相对位置变量
    MotorFreqTarget = Freq;  
    MotorCycleTarget = 100000 / Freq;  //单位  10us   （其为脉冲输出定时器 最小计数单位
    
    ACC_Status_Flag = (u8)(MotorFreqTarget / ACC_HZ_MIN);  //计算加速阶段  即 以当前加速度 需要进行几次加速
    DEC_Status_Flag = ACC_Status_Flag;
    
    PulseNum_Rel = Pulse_Num - PulseNum_Global;//计算位移脉冲数 带方向
    
    /*1:动作方向判断*/
    if(PulseNum_Rel == 0)
    {
        MotorDir = 0;
        GPIO_ResetBits(GPIOE, GPIO_Pin_0);
    }
    else if(PulseNum_Rel > 0) //设定脉冲数大于 当前脉冲位置
    {
        MotorDir = 1;
        GPIO_ResetBits(GPIOE, GPIO_Pin_0);//正转
    }
    else if(PulseNum_Rel < 0) //设定脉冲数小于 当前脉冲位置
    {
        PulseNum_Rel = -PulseNum_Rel;
        MotorDir = 2;
        GPIO_SetBits(GPIOE, GPIO_Pin_0);//反转
    }
    /*end 1*/
    
    /*2:设定脉冲频率大于起始频率*/
    if((ACC_Status_Flag != 0) && ( PulseNum_Rel != 0))
    {   /*位移脉冲数大于 2倍 加速到指定频率的脉冲数*/
        if(PulseNum_Rel > (2*PulseNum_Sum_Buf[ACC_Status_Flag - 1] - 1))  //暂定 加速和减速度一样 （加速和减速 过程输出脉冲数相同）
        {                                                           //如果指定脉冲数大于2倍的值 则进行完全加速过程
            PTO_HZ_NUM_Start(ACC_Status_Flag, PulseNum_Rel);
        }
        /*位移脉冲数小于2倍 */
        else if(PulseNum_Rel < 2*PulseNum_Sum_Buf[ACC_Status_Flag - 1])
        {
            for(i = ACC_Status_Flag; i > 0; i--)
            {
                if(PulseNum_Rel > 2*PulseNum_Sum_Buf[i - 1])
                {
                    ACC_Status_Flag = i;
                    DEC_Status_Flag = i;
                    PTO_HZ_NUM_Start(ACC_Status_Flag, PulseNum_Rel);
                    break;
                }
            }
        } 
    }    
}
static void PTO_Cycle(unsigned short Cycle)
{
    TIM_SetAutoreload(TIM1, Cycle - 1);//设置TIM1的重装值
    TIM_SetCompare2(TIM1, Cycle / 2);//设置占空比为50%;
    MotorCycleReal = Cycle;    
}
static void PTO_HZ_NUM_Start(unsigned char Flag, signed int pPulseNum_Rel)
{
    PulseNum_ACC = PulseNum_Sum_Buf[Flag - 1]; //
    PulseNum_DEC = PulseNum_ACC;
    PulseNum_ConstantVel = pPulseNum_Rel - (PulseNum_ACC + PulseNum_DEC);
    //MotorFreqU = 1; //指示需要加速
    MotorActionState = 1;
    PTO_Cycle(MotorCycle_Buf[0]);
    TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
    TIM_Cmd(TIM1,ENABLE);//使能TIM1 
}

void PTO_Home(u16 Freq)/*输出指定频率PWM*/
{
    u16 TIM1per = 0;
    u16 Cycle = 1000000 / Freq;
    TIM1per = Cycle / 10;       //预分频为720,10us计数一次
    MotorCycleReal = TIM1per;
    MotorDir = 2;
    GPIO_SetBits(GPIOE, GPIO_Pin_0);//反转
    PTO_Cycle(TIM1per);
    TIM_Cmd(TIM1,ENABLE);//使能TIM1
}

void PTO_Stop(void)
{
    TIM_Cmd(TIM1, DISABLE);
    TIM_SetCounter(TIM1, 0);  
    MotorCycleReal = 0;                        
    MotorDir = 0;  
    MotorActionState = 0;     //转移状态 0  
}
/*******************************************************************/
/*******************************************************************/
/*******************************************************************/
/*******************************************************************/
////////////设置PWM的输出
////////////Freqq:为频率,单位(hz)
////////////Pulse_Num:为脉冲个数
//////////void PTO_HZ(u16 Freqq, u16 Pulse_Num)
//////////{
//////////  u16 TIM1per = 0;
//////////  u32 Time = 0;
//////////  u16 Cycle;
//////////  
//////////  Cycle = 1000000 / Freqq;
//////////  //改变TIM1的重装值改变脉冲频率这里设置脉冲占空比为50%
//////////  //改变TIM5的预分频器和重装值改变脉冲个数
//////////    
//////////  Time = Cycle * Pulse_Num;
//////////  Time /= 100;              //预分频为7200,100us计数一次
//////////  TIM1per = Cycle/10;       //预分频为720,10us计数一次

//////////  TIM_SetAutoreload(TIM5, Time);//设置TIM5的重装值
//////////  TIM_SetAutoreload(TIM1, TIM1per-1);//设置TIM1的重装值
//////////  TIM_SetCompare2(TIM1,TIM1per/2);//设置占空比为50%
//////////  TIM_Cmd(TIM5,ENABLE);//使能TIM5 
//////////  TIM_Cmd(TIM1,ENABLE);//使能TIM1
//////////}


/*******************************************************************/
/***********************速度模式************************************/
/*******************************************************************/
/*******************************************************************/

////void MotorF(unsigned short Freq, unsigned short Pulse_Num)
////{
////    TIM_Cmd(TIM1,DISABLE);
////    GPIO_SetBits(GPIOE, GPIO_Pin_0);

////    PTOWithAcceleration(Freq);
////}
////void MotorB(unsigned short Freq, unsigned short Pulse_Num)
////{
////    TIM_Cmd(TIM1,DISABLE);
////    GPIO_ResetBits(GPIOE, GPIO_Pin_0);

////    PTOWithAcceleration(Freq);
////}
////void MotorS()
////{
////	PTOStopWithDeceleration();
////}

//////设置PWM的输出
//////Freq:HZ PWM输出 带加速  500hz  /  20ms
////void PTOWithAcceleration(u16 Freq)
////{
////	MotorFreqU = 1;	/*频率加速启动标志*/
////    MotorFreqFlag = Freq / ACC_HZ_MIN;	/*计算频率 ACC_HZ_MIN 倍数*/
////    MotorFreqBuf  = Freq % ACC_HZ_MIN;	/*计算频率 ACC_HZ_MIN 余数*/
////    if(MotorFreqFlag != 0)/*如果频率值大于等于ACC_HZ_MIN*/
////    {
////		PTO_HZ_Period(ACC_HZ_MIN, ACC_PERIOD); /*首先输出ACC_HZ_MINHZ 脉冲 起始频率为ACC_HZ_MIN HZ*/ 	          
////    }
////    else
////	{ 
////        PTO_HZ_NONum(MotorFreqBuf);	/*如果频率小于 500HZ 直接输出指定频率*/
////	}
////}

/////*PWM输出 当着频率 以DEC_HZ_MIN HZ/ period MS 减小至0*/
////void PTOStopWithDeceleration(void)
////{
////	MotorFreqD = 1;	/*启动减速过程标志*/
////	MotorFreqFlag = MotorFreqNow / DEC_HZ_MIN;

////	if(MotorFreqFlag != 0) /*如果当前频率大于500*/
////	{
////	    PTO_HZ_Period(MotorFreqNow - DEC_HZ_MIN, DEC_PERIOD);/*输出 (MotorFreqNow - 500) 20MS*/
////	}
////	else
////	{
////		PTO_Stop();
////	}
////}



/////*pwm输出 period ms定时中断 指定频率PWM输出*/
////void PTO_HZ_Period(unsigned short Freq,unsigned short period)
////{
////	u16 TIM1per = 0;
////	u16 Cycle;
////	MotorFreqNow = Freq;	  /*当前频率变量 赋值*/

////	Cycle = 1000000 / Freq;
////	//改变TIM1的重装值改变脉冲频率这里设置脉冲占空比为50%

////	TIM1per = Cycle / 10;       //预分频为720,10us计数一次
////	
////	TIM_SetAutoreload(TIM5, period * 10);//设置TIM5的重装值		1000 * 100us = 100ms
////	TIM_SetAutoreload(TIM1, TIM1per-1);//设置TIM1的重装值
////	TIM_SetCompare2(TIM1,TIM1per/2);//设置占空比为50%
////    TIM_Cmd(TIM1,ENABLE);//使能TIM1
////    TIM_Cmd(TIM5,ENABLE);//使能TIM5 
////}


////////定时器5中断服务程序	 
////void TIM5_IRQHandler(void)
////{ 	
////   
////    static u8 FreqFlag;
////	OSIntEnter();
////    if (TIM_GetITStatus(TIM5, TIM_IT_Update) != RESET) //检查指定的TIM中断发生与否:TIM 中断源 
////	{
////		/*加速处理过程 ACCC_HZ_MIN倍数增加 达到 ACC_HZ_MIN * (Freq / ACC_HZ_MIN) 后，增加MotorFreqBuf 即达到设定频率值*/
////        /*判定是否初次启动加速 MotorFreqU 在PTOWithAcceleration中 赋值为1（如果设定频率值大于加速度ACC_HZ_MIN HZ） 表示启动频率加速过程 */
////		if(MotorFreqU == 1)
////        {
////            MotorFreqU = 2; /*赋值为2 表示进入加速过*/
////            FreqFlag = 0;	/*频率计算中间变量 设定初值 0*/
////        }
////        if(MotorFreqU == 2)	/*进行频率加速处理*/
////        {
////			TIM_Cmd(TIM1,DISABLE);           
////			if(FreqFlag < MotorFreqFlag)/*频率计算中间变量小于 MotorFreqFlag */
////            {

////				if(FreqFlag != (MotorFreqFlag -1))/*位于[0, MotorFreqFlag-1 ) 之间 不包括MotorFreqFlag-1*/
////				{
////		            PTO_HZ_Period(ACC_HZ_MIN * (FreqFlag + 2), ACC_PERIOD); /*输出频率为 500 * (FreqFlag + 2) PWM  period ms */

////				} 
////			    else if(FreqFlag == (MotorFreqFlag - 1))/*等于 MotorFreqFlag - 1 时 到达临界状态 单独处理*/
////		        {
////		            
////		            PTO_HZ_NONum(ACC_HZ_MIN * (FreqFlag + 1) +  MotorFreqBuf);/*输出最终频率PWM 取消定时*/
////					MotorFreqBuf = 0; /*恢复初值*/
////					MotorFreqFlag = 0;/*恢复初值*/
////		            MotorFreqU = 0;/*加速标志设定为0 表示加速完成*/
////		            FreqFlag = 0; /*恢复中间变量初值*/
////                    
////                    OSSemPost(StepSem);
////		        }
////				FreqFlag ++; 
////            }

////        }
////		/*加速处理过程结束*/
////        
////		/*减速处理过程*/
////		 if(MotorFreqD == 1)
////		 {
////		     MotorFreqD = 2;
////			 FreqFlag = 0;
////		 }
////		 if(MotorFreqD == 2)
////		 {
////		     TIM_Cmd(TIM1,DISABLE);
////			 if(FreqFlag < MotorFreqFlag)
////			 {  
////		     	if(FreqFlag != (MotorFreqFlag - 1))
////			 	{
////			 		PTO_HZ_Period(MotorFreqNow - DEC_HZ_MIN, DEC_PERIOD);
////			 	}
////			 	else if(FreqFlag == (MotorFreqFlag - 1))
////				{
////					PTO_Stop();
////				    TIM_Cmd(TIM5,DISABLE);//关闭定时
////					
////					MotorFreqD = 0;
////					MotorFreqFlag = 0;/*恢复初值*/
////		            FreqFlag = 0; /*恢复中间变量初值*/
////				}
////				FreqFlag ++; 
////		     }
////		 }
////	} 
////	TIM_ClearITPendingBit(TIM5, TIM_IT_Update  );  //清除TIMx的中断待处理位:TIM 中断源 
////    OSIntExit();    
////}
////定时器1更新中断服务程序	 
void TIM1_UP_IRQHandler(void)
{ 	
    static u8 tempflag = 0;

    OSIntEnter();

    if(MotorDir == 1)
        PulseNum_Global++;
    else if(MotorDir == 2)
        PulseNum_Global--;
    if(TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET) //检查指定的TIM中断发生与否:TIM 中断源 
	{        
        switch(MotorActionState)
        {
            case 1://启动加速状态 
                tempflag = 0;	/*计算中间变量 设定初值 0*/
                PulseNum_TEMP =(MotorDir > 1)? (PulseNum_Global +1):(PulseNum_Global - 1);/*记录初始加速脉冲初值*/
                MotorActionState = 2;//转移状态 2 加速过程中
            break;
            case 2://加速过程 处理
                if((u32)abs(PulseNum_Global - PulseNum_TEMP) > (PulseNum_Buf[tempflag] - 1)) //输出达到指定脉冲数
                {
                    PulseNum_TEMP = PulseNum_Global;/*记录初始加速脉冲初值*/
                    tempflag++;  
                    if(tempflag == ACC_Status_Flag) // 
                    {                        
                        if(PulseNum_ConstantVel == 0) /*如果设定脉冲数等于2倍的加减速所需脉冲个数 即匀速动作脉冲数为 0 加速完毕直接进入减速过程*/
                        {
                            tempflag = 0;
                            MotorActionState = 4;//跳至减速状态 
                            break;                            
                        }
                        else
                        {
                            MotorActionState = 3;//进入恒速动作过程
                            PTO_Cycle(MotorCycleTarget);
                            break;                            
                        }              
                    }
                    if(tempflag < ACC_Status_Flag) //未达到目标频率 继续加速
                    {
                        PTO_Cycle(MotorCycle_Buf[tempflag]);
                    }
                }
                break;
            case 3://恒速动作 
                if((u32)abs(PulseNum_Global - PulseNum_TEMP) > (PulseNum_ConstantVel - 1))//输出达到指定脉冲数
                {
                    tempflag = 0;
                    MotorActionState = 4;  //转移状态 4 需要进入减速
                }
                break;
            case 4://启动减速
                tempflag = DEC_Status_Flag;	/*计算中间变量 设定初值 0*/
                PulseNum_TEMP =(MotorDir > 1)? (PulseNum_Global +1):(PulseNum_Global - 1);               
                PTO_Cycle(MotorCycle_Buf[tempflag - 1]); 
                MotorActionState = 5; //转移状态5 减速过程中           
                break;
            case 5://减速过程中
                if((u32)abs(PulseNum_Global - PulseNum_TEMP) > ((PulseNum_Buf[tempflag -1] - 1)))//达到指定脉冲数
                {
                    PulseNum_TEMP = PulseNum_Global;/*记录脉冲值*/
                    tempflag--;
                    if(tempflag == 0) //达到0
                    {
                        PTO_Stop();                   
                    }
                    if(tempflag > 0) 
                    {
                        PTO_Cycle(MotorCycle_Buf[tempflag - 1]);
                    }            
                } 
                break;
            default:
                break;
        }
    }
    TIM_ClearITPendingBit(TIM1, TIM_IT_Update);  //清除TIMx的中断待处理位:TIM 中断源 
    OSIntExit();
}

////////////定时器1更新中断服务程序	 
////////void TIM1_UP_IRQHandler(void)
////////{ 	
////////    static u8 tempflag = 0;

////////    OSIntEnter();

////////    if(MotorDir == 1)
////////        PulseNum_Global++;
////////    else if(MotorDir == 2)
////////        PulseNum_Global--;
////////    if(TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET) //检查指定的TIM中断发生与否:TIM 中断源 
////////	{
////////        /*加速处理过程*/
////////        if(MotorFreqU == 1)
////////        {
////////            MotorFreqU = 2; /*赋值为2 表示进入加速过*/
////////            tempflag = 0;	/*计算中间变量 设定初值 0*/
////////            PulseNum_TEMP =(MotorDir > 1)? (PulseNum_Global +1):(PulseNum_Global - 1);/*记录初始加速脉冲初值*/
////////        }  
////////        if((MotorFreqU == 2) && ((u32)abs(PulseNum_Global - PulseNum_TEMP) > (PulseNum_Buf[tempflag] - 1)))
////////        {
////////            PulseNum_TEMP = PulseNum_Global;/*记录初始加速脉冲初值*/
////////            tempflag++;
////////            if(tempflag == ACC_Status_Flag) //目标频率 
////////            {
////////                PTO_Cycle(MotorCycleTarget);
////////                if(PulseNum_ConstantVel == 0) /*如果设定脉冲数等于2倍的加减速所需脉冲个数 即匀速动作脉冲数为 0 加速完毕直接进入减速过程*/
////////                {
////////                    tempflag = 0;
////////                    MotorConstantVel = 2;//跳过恒速动作  AA
////////                    MotorFreqD = 1;
////////                }
////////                else
////////                {
////////                    MotorConstantVel = 1;//进入恒速动作过程              
////////                }
////////                MotorFreqU = 0;      //清零加速标志位 标志加速过程完毕                
////////            }
////////            if(tempflag < ACC_Status_Flag)
////////            {
////////                PTO_Cycle(MotorCycle_Buf[tempflag]);
////////            }
////////        }       
////////        /*加速结束*/
////////        
////////        /*匀速动作过程*/
////////        if((MotorConstantVel == 1) && ((u32)abs(PulseNum_Global - PulseNum_TEMP) > (PulseNum_ConstantVel - 1)))
////////        {
////////            tempflag = 0;
////////           // MotorConstantVel = 0;  //AB
////////            MotorFreqD = 1;
////////        }  
////////        
////////        /*减速处理过程*/
////////        if(MotorFreqD == 1)
////////        {
////////            if((MotorConstantVel != 1) && (MotorConstantVel != 2))/*从AA或AB处顺序执行到此处时 需要下一次更新中断进行减速处理*/
////////            {
////////                MotorFreqD = 2; /*赋值为2 表示进入减速过程*/
////////                tempflag = DEC_Status_Flag;	/*计算中间变量 设定初值 0*/
////////                PulseNum_TEMP =(MotorDir > 1)? (PulseNum_Global +1):(PulseNum_Global - 1);               
////////                PTO_Cycle(MotorCycle_Buf[tempflag - 1]);               
////////            }
////////            else 
////////            {
////////                MotorConstantVel = 0;
////////            }
////////        } 
////////        if((MotorFreqD == 2) && ((u32)abs(PulseNum_Global - PulseNum_TEMP) > ((PulseNum_Buf[tempflag -1] - 1))))
////////        {
////////            PulseNum_TEMP = PulseNum_Global;/*记录初始加速脉冲初值*/
////////            tempflag--;
////////            if(tempflag == 0) //目标频率 
////////            {
////////                TIM_Cmd(TIM1, DISABLE);
////////                TIM_SetCounter(TIM1, 0);
////////                MotorFreqD = 0;      //清零加速标志位 标志减速过程完毕     
////////                MotorDir = 0;               
////////            }
////////            if(tempflag > 0)
////////            {
////////                PTO_Cycle(MotorCycle_Buf[tempflag - 1]);
////////            }            
////////        }            
////////        
////////        /**/        
////////    }
////////    TIM_ClearITPendingBit(TIM1, TIM_IT_Update);  //清除TIMx的中断待处理位:TIM 中断源 
////////    OSIntExit();
////////}
//外部中断1处理函数
void EXTI1_IRQHandler(void)
{
    OSIntEnter();
    if(EXTI_GetITStatus(EXTI_Line1) != RESET)
    {
        PulseNum_Global = 0; 
        if(HomeFlag == 0)
        {
            PTO_Stop();
            HomeFlag = 1;
        }  
  
        EXTI_ClearITPendingBit(EXTI_Line1);
    }
	OSIntExit();
}


void EXTI2_IRQHandler(void)
{
    OSIntEnter();
    if(EXTI_GetITStatus(EXTI_Line2) != RESET)
    {

        //EXTI2_OFF;
        
        EXTI_ClearITPendingBit(EXTI_Line2);
    }
	OSIntExit();
}
//外部中断4处理函数
void EXTI4_IRQHandler(void)
{
    OSIntEnter();
    if(EXTI_GetITStatus(EXTI_Line4) != RESET)
    {
        PTO_Stop();
        PulseNum_Global = 0; 
        if(HomeFlag == 0)
        {           
            HomeFlag = 1;
        }    
        EXTI_ClearITPendingBit(EXTI_Line4);
    }
	OSIntExit();
}
