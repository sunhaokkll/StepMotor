#include "includes.h"


static OS_STK startup_task_stk[STARTUP_TASK_STK_SIZE];  //∂®“Â’ª

OS_MEM *CoMboxBuffer;
u8 CoMboxPart[16][256];


int main(void)
{ 
 //   u8 err;
   
	BSP_Init();

    OSInit();

//	CoMboxBuffer = OSMemCreate(CoMboxPart, 16, 256, &err);

//	if(err != OS_ERR_NONE)
//		printf("ƒ⁄¥Ê…Í«Î ß∞‹!");
     
    OSTaskCreate(Task_Start,(void *)0,
    (OS_STK *)&startup_task_stk[STARTUP_TASK_STK_SIZE - 1], STARTUP_TASK_PRIO);
    OSStart(); 

    return  0;
}

int fputc(int ch,FILE *f)
{
    USART_SendData(USART1, (u8)ch);
	while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
	return ch;  
}

