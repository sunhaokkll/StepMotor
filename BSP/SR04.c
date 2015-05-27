#include "includes.h"


void vSR04Config(void)
{
    /*SR04      管脚配置
	*TRIG(PE8)  推挽输出
	*ECHO(PE7)  浮空输入  外部中断
    */
	prvSR04GPIOInit();
	
	prvSR04NVICInit();
	
	prvSR04EXTIInit();
	
	prvSR04TIMInit();

}
static void prvSR04GPIOInit(void)
{
	/*GPIO初始化结构体*/
	GPIO_InitTypeDef GPIO_InitStructure; 

	 /*开启时钟GPIOC, GPIOE, TIM8,AFIO*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE | RCC_APB2Periph_GPIOD  | RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
		
	 /*GPIO定义*/	
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_8;	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;       
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_7;	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;       
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource7);


}
static void prvSR04NVICInit(void)
{
	 /*NVIC初始化结构体*/
	NVIC_InitTypeDef NVIC_InitStructure;
	#ifdef VECT_TAB_RAM 	   
	 /* Set the Vector Table base location at 0x20000000 */ 	  
	 NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0); 
	#else /* VECT_TAB_FLASH */		  
	 /* Set the Vector Table base location at 0x08000000 */ 	  
	 NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);	
	#endif	 
	
	 /* 选择NVIC优先分组4 所有4位用于抢占式优先级  */
	 NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4); 
	
	 /*使能EXTI9_5_IRQn 12级占先式优先级，因在程序中需要使用信号量*/
	 NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
	 NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = SR04_EXTI_PRIO;
	 NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	 NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	 NVIC_Init(&NVIC_InitStructure);


}
static void prvSR04EXTIInit(void)
{
	/*EXTI初始化结构体*/
	EXTI_InitTypeDef EXTI_InitStructure;

	/*设置外部中断通道7在跳变沿触发中断*/
	EXTI_InitStructure.EXTI_Line = EXTI_Line7;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
}   
static void prvSR04TIMInit(void)
{

	/*TIM_TimeBse初始化结构体*/
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_DeInit(TIM2);                              //TIM2复位

	TIM_TimeBaseStructure.TIM_Period = 0xFFFF;
	TIM_TimeBaseStructure.TIM_Prescaler = 71;/*71 + 1 分频 72 000 000 / 72 = 1 000 000*/
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
    TIM_Cmd(TIM2, DISABLE);
}

