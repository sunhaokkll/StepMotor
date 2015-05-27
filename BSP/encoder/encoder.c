#include "includes.h" 

/*定义采样周期内 最大计数值 永远不会达到*/
#define  MAX_COUNT  0x5000

u8  ENCODERDIR_FLAG; /*方向标志位：0 向上 1 向下*/

s32 nowENCODERCNT;  /*当前编码器位置 *0.02mm 即为当前拉线编码器伸出长度*/

void vENCODERConfig()
{
  prvTIM8_encoder();
} 

signed int s32GetEncoderCNT(void)
{
    static s16 preTIM8CNT; /*定义静态变量 存储上一次计数值*/
    s16 nowTIM8CNT;        /*存储当前计数值*/
    s32 dTIM8CNT;          /*存储10ms间隔内 动作距离*/
    nowTIM8CNT = TIM_GetCounter(TIM8); /*获取当前计数值*/
    dTIM8CNT = nowTIM8CNT - preTIM8CNT;/*计算当前与上一次计数差值*/
    
    if(dTIM8CNT > MAX_COUNT )   /*向下计数 溢出*/         
    {
        dTIM8CNT -= 0x10000;
    }
    else if(dTIM8CNT < -MAX_COUNT) /*向上计数 溢出*/
    {
        dTIM8CNT += 0x10000;
    }
    preTIM8CNT = nowTIM8CNT;/*存储当前计数值*/
    
    /*TIMx_CR1 位4 为计数方向 0：up  1: down*/
    ENCODERDIR_FLAG = ((TIM8->CR1 >> 4) & 0x01) ;
    
    return dTIM8CNT;/*返回10ms内动作计数值*/
        
}
/*TIM8 编码器模式初始化*/
void prvTIM8_encoder(void)
{ 
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_ICInitTypeDef TIM_ICInitStructure;
	/*开启相关时钟 TIM8 PC*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8 , ENABLE); //
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE); //
	/*PC6 PC7初始化*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC,&GPIO_InitStructure);  //
	/*TIM8 时基初始化 溢出值为0xFFFF 向上计数*/
	TIM_TimeBaseStructure.TIM_Prescaler = 0x0;
	TIM_TimeBaseStructure.TIM_Period = 0xFFFF;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM8, &TIM_TimeBaseStructure); // Time base configuration
    
    /*初始化TIM8编码器模式3 TI1FP1 TI2FP2 上升沿计数*/
	TIM_EncoderInterfaceConfig(TIM8, TIM_EncoderMode_TI12, 
				                     TIM_ICPolarity_Rising, 
									 TIM_ICPolarity_Rising);
    /*初始化通道1 2 使用输入捕获1 滤波*/                                
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_1 | TIM_Channel_2 ;
	TIM_ICInitStructure.TIM_ICFilter = 6;//ICx_FILTER;
	TIM_ICInit(TIM8, &TIM_ICInitStructure);

	TIM_ITConfig(TIM8, TIM_IT_Update, ENABLE);
	TIM_SetCounter(TIM8, 0);	//???????   
	TIM_Cmd(TIM8, ENABLE);
}


 
