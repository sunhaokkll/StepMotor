#include "includes.h"

/*全局变量*/
s16 Tem_Now, Hum_now;
/*END*/

/*局部变量*/
u8  u8BitTHBuff[40];
u8  u8THBuff[5];
u32 EXTI_LINE0_FLAG = 0; 
u32 THBuff[44];

OS_EVENT *AM2305Sem; /*2305测量完毕信号量*/
/*END*/

#define EXTI0_OFF EXTI->IMR &= (~EXTI_IMR_MR0)
#define EXTI0_ON  EXTI->IMR |= EXTI_IMR_MR0

void SDA_LOW(void)
{
  GPIOB->CRL = GPIOB->CRL & 0xFFFFFFF0;
  GPIOB->CRL = GPIOB->CRL | 0x00000003;
  GPIOB->BSRR |= (0x0001 << 16);  
}
void SDA_LOW1(void)
{
  GPIOB->CRL = GPIOB->CRL & 0xFFFFFFF0;
  GPIOB->CRL = GPIOB->CRL | 0x00000003;
  GPIOB->BSRR |= (0x0001);  
}
void SDA_HIGH(void)
{
  GPIOB->CRL = GPIOB->CRL & 0xFFFFFFF0;
  GPIOB->CRL = GPIOB->CRL | 0x00000004; 
}
void vTask2305(void *p_arg)
{ 
    u8 i = 0, j =0;
    u8 Tempp = 0;
    u8 err;
    (void)p_arg;    /*'p_arg' 并没有用到，防止编译器提示警告*/
    AM2305Sem = OSSemCreate(0); /*创建信号量*/
    OSTimeDlyHMSM(0, 0, 3, 50);
    EXTI0_OFF; 
    for(;;)
    {
        memset(u8BitTHBuff, 0, 40);
        memset(THBuff, 0, 44);
        memset(u8THBuff, 0, 5);
        SDA_LOW();
        delay_ms(1);
        EXTI0_ON;  //打开中断
        SDA_HIGH(); 
               
        
        
        OSSemPend(AM2305Sem, 20, &err);
        //while(EXTI_LINE0_FLAG != 42);
        if(EXTI_LINE0_FLAG == 42)
        {
            printf("OK %d\n", EXTI_LINE0_FLAG );
            for( i = 0; i < 40 ; i ++)
            {
                u8BitTHBuff[i] = THBuff[i+3] - THBuff[i+2];   
            }

            for(j = 0; j < 5; j++)
            {
                for(i = 0; i < 8; i++)
                {
                    if((u8BitTHBuff[i + (j * 8)] < 90) && (u8BitTHBuff[i + (j * 8)] > 60))
                        u8THBuff[j] |= (0 << (7 - i));
                    else if((u8BitTHBuff[i + (j * 8)] < 150) && (u8BitTHBuff[i + (j * 8)] > 110))
                        u8THBuff[j] |= (1 << (7 - i));                     
                }                
            }
            
            for(j = 0; j < 4; j ++)
            {
                Tempp += u8THBuff[j];
            }
            if(Tempp == u8THBuff[4])
            {
                Tem_Now = u8THBuff[2] * 256 + u8THBuff[3]; 
                Hum_now = u8THBuff[0] * 256 + u8THBuff[1];
            }
        }
        else if(EXTI_LINE0_FLAG != 42)
        {
            printf("error %d\n", EXTI_LINE0_FLAG );
        }
        
        EXTI_LINE0_FLAG = 0;
        EXTI0_OFF;  //关闭中断
        TIM_Cmd(TIM3, DISABLE);
        TIM_SetCounter(TIM3, 0);        
        Tempp = 0;
        
        OSTimeDlyHMSM(0, 0, 5, 0);                   
    }
}

void vtask2305Handle(void)
{
    EXTI_LINE0_FLAG ++;
    if(EXTI_LINE0_FLAG == 0x01)
    {
        TIM_Cmd(TIM3, ENABLE);
    }
    THBuff[EXTI_LINE0_FLAG] = TIM_GetCounter(TIM3);
    if(EXTI_LINE0_FLAG == 42)
    {
        TIM_Cmd(TIM3, DISABLE);
        TIM_SetCounter(TIM3, 0);
        OSSemPost(AM2305Sem); 
    }
      
}

void EXTI0_IRQHandler(void)
{
    OSIntEnter();
	
	if(EXTI_GetITStatus(EXTI_Line0) == SET)
	{
		vtask2305Handle();   /*调用处理函数*/
		EXTI_ClearITPendingBit(EXTI_Line0);
	}  
	OSIntExit();
}
