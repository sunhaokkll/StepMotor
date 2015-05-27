#include "includes.h"
extern s32 PreSet_Position[512];
extern u32 FLASH_PAGE_ADDR;
/*
* 函数名：BSP_Init
* 描述  ：时钟初始化、硬件初始化
* 输入  ：无
* 输出  ：无
*/
void BSP_Init(void)
{  
    u16 i;
    SystemInit();       /*配置系统时钟为72M*/
    SysTick_init();     /*初始化并使能SysTick定时器*/
    USART_Config();
//  vSR04Config();
//  vADCConfig();
//  vRTCConfig();
//	MSD0_SPI_Configuration();
//  vAM2305Config();
    //delay_ms(1000);
    vPTOConfig();
    
//////    /*正交编码相关*/
//////    vENCODERConfig(); /*编码器初始化*/           
//////    TIM6_Int_Init(1000,719);//100Khz计数频率，计数到1000为10ms  用于周期性调用 编码器模式下计数值  读取函数
//////    /*end*/
//////    
//////    /*读取FLASH中预置位信息*/
//////    for(i = 0; i < 512; i++)
//////    {
//////        PreSet_Position[i] = *(__IO uint32_t*)(FLASH_PAGE_ADDR + i * 4);
//////    }
    
}

/*
* 函数名：SysTick_init
* 描述  ：配置SysTick定时器
* 输入  ：无
* 输出  ：无
*/
void SysTick_init(void)
{
    SysTick_Config(SystemCoreClock / OS_TICKS_PER_SEC); //初始化并使能SysTick定时器
}
