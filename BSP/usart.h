#ifndef __USART_H__
#define __USART_H__

#include "includes.h"
/**********************************************/

#define uDEBUG   1u

//#define RS485_R  GPIOA->BSRR = GPIO_Pin_8
//#define RS485_T  GPIOA->BRR = GPIO_Pin_8
/**********************************************/
/*定义串口接收缓冲区长度*/
#define FRAMEBUFMAX 256

/**********************************************/
static u8 FrameBuf[FRAMEBUFMAX];     /*帧缓存数组*/
static u8 FrameBufWptr = 0;          /*缓冲区写指针*/
static u8 FrameBufRptr = 0;          /*缓冲区读指针*/
//static u8 FrameSData;                /*待发送数组*/


void USART_Config(void);

static void USART_NVIC_Config(void);
static void USART_RCC_Config(void);
static void USART_GPIO_Config(void);
static void USART_MODE_Config(void);


void FrameBufWrite(void);
u8 FrameBufRead(u8 *data);

void UART1_Sendbyte(INT8U data);
void UART1_Sendarray(INT8U *buf, INT8U buflen);

void UART4_Sendbyte(INT8U data);
void UART4_Sendarray(INT8U *buf, INT8U buflen);

#if uDEBUG > 0u
	void UART4_Sendbyte(INT8U data);
//	int fputc(int ch,FILE *f);
#endif

#endif

