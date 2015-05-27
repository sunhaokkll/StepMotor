#include "includes.h"

/*
* 函数名：USART_Config
* 描述  ：配置USART
* 输入  ：无
* 输出  ：无
*/ 
void USART_Config()
{
  
    USART_RCC_Config(); 
    USART_GPIO_Config();
    USART_MODE_Config();
    USART_NVIC_Config();
}

/*
* 函数名：USART_NVIC_Config
* 描述  ：配置USART中断
* 输入  ：无
* 输出  ：无
*/ 
void USART_NVIC_Config()
{
    /*定义NVIC初始化结构体 NVIC_InitStructure*/
    NVIC_InitTypeDef NVIC_InitStructure;
    
    /*选择NVIC优先级分组4( 0~15抢占优先级)*/
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    
    /*使能串口1接收中断，抢占优先级 3 响应优先级 0*/
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = UART_RX_PRIO;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/*
* 函数名：USART_RCC_Config
* 描述  ：配置USART管脚时钟
* 输入  ：无
* 输出  ：无
*/
void USART_RCC_Config()
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC
                       | RCC_APB2Periph_AFIO, ENABLE); 
	
	#if uDEBUG > 0u
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
	#endif
}

/*
* 函数名：USART_GPIO_Config
* 描述  ：配置USART管脚状态
* 输入  ：无
* 输出  ：无
*/
void USART_GPIO_Config()
{
    /*定义GPIO初始化结构体 GPIO_InitStructure*/
    GPIO_InitTypeDef GPIO_InitStructure;
    
    //******************************************************************************
    //串口1所使用管脚输出输入定义
    //******************************************************************************
    //定义UART1 TX (PA.09)脚为复用推挽输出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;         //IO口的第九脚
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //IO口速度
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;   //IO口复用推挽输出
    GPIO_Init(GPIOA, &GPIO_InitStructure);            //初始化串口1输出IO口
    
    // 定义 USART1 Rx (PA.10)为悬空输入
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;           //IO口的第十脚
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//IO口悬空输入
    GPIO_Init(GPIOA, &GPIO_InitStructure);               //初始化串口1输入IO口

	#if uDEBUG > 0u
	//******************************************************************************
    //串口2所使用管脚输出输入定义
    //******************************************************************************
    //定义UART2 TX (PA.2)脚为复用推挽输出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;         //
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //IO口速度
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;   //IO口复用推挽输出
    GPIO_Init(GPIOA, &GPIO_InitStructure);            //初始化串口1输出IO口
    
    // 定义 USART2 Rx (PA.3)为悬空输入
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;           //
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//IO口悬空输入
    GPIO_Init(GPIOA, &GPIO_InitStructure);               //初始化串口2输入IO口
    
    
    //******************************************************************************
    //串口4所使用管脚输出输入定义
    //******************************************************************************
    //定义UART4 TX (PC.10)脚为复用推挽输出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;         //
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //IO口速度
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;   //IO口复用推挽输出
    GPIO_Init(GPIOC, &GPIO_InitStructure);            //初始化串口4输出IO口
    
    // 定义 USART4 Rx (PC.11)为悬空输入
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;           //
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//IO口悬空输入
    GPIO_Init(GPIOC, &GPIO_InitStructure);               //初始化串口4输入IO口
    #endif
    
    // 定义 485 EN (PA.8)为推挽输出
    //	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    //  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    //  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    //  GPIO_Init(GPIOA, &GPIO_InitStructure);
}

/*
* 函数名：USART_MODE_Config
* 描述  ：配置USART模式（波特率、数据个数、停止、检验等）
* 输入  ：无
* 输出  ：无
*/
void USART_MODE_Config()
{
    USART_InitTypeDef USART_InitStructure;
    
    USART_InitStructure.USART_BaudRate = 115200;                                      //设定传输速率
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;                     //设定传输数据位数
    USART_InitStructure.USART_StopBits = USART_StopBits_1;                          //设定停止位个数
    USART_InitStructure.USART_Parity = USART_Parity_No ;                            //无校验
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //不用流量控制
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;                 //使用接收和发送功能
    USART_Init(USART1, &USART_InitStructure);                                       //初始化串口1
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);                                  //使能串口1接收中断
    USART_Cmd(USART1, ENABLE);                                                      //使能串口1
    //  RS485_R;                                                                        //使能接收

	#if uDEBUG > 0u
		USART_Init(USART2, &USART_InitStructure); 
		USART_Cmd(USART2, ENABLE);
        USART_InitStructure.USART_BaudRate = 9600; 
    	USART_Init(UART4, &USART_InitStructure); 
		USART_Cmd(UART4, ENABLE);
	#endif
}
/*
* 函数名：FrameBufWrite
* 描述  ：缓冲区写函数（由通信接口接收中断服务调用）
* 输入  ：无
* 输出  ：无
*/
void FrameBufWrite()
{
    if(FrameBufWptr == (FrameBufRptr - 1))     /*防止套圈*/
    {
      return;
    }
    FrameBuf[FrameBufWptr] = USART_ReceiveData(USART1);
    FrameBufWptr++;
    FrameBufWptr %= FRAMEBUFMAX;
}

/*
* 函数名：FrameBufRead
* 描述  ：缓冲区读函数
* 输入  ：data,待存放读出数据的内存空间指针
* 输出  ：0：无数据  1：有数据
*/
u8 FrameBufRead(u8 *data)
{
    if(FrameBufRptr == FrameBufWptr)
    {
      return 0;
    }
    *data = FrameBuf[FrameBufRptr];
    FrameBufRptr++;
    FrameBufRptr %= FRAMEBUFMAX;
    return 1;
}


/*
* 函数名：UART1_Sendbyte
* 描述  ：单字节发送
* 输入  ：data：待发送数据
* 输出  ：无
*/
void UART1_Sendbyte(INT8U data)
{
    //  RS485_T;
    USART_SendData(USART1, (u8)data);
	while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);

    
//    OSTimeDly(100); //保证发送稳定性
    
    //  RS485_R;
}

/*
* 函数名：UART1_Sendarray
* 描述  ：发送数组
* 输入  ：指向数组首地址指针，数组长度
* 输出  ：无
*/
void UART1_Sendarray(INT8U *buf, INT8U buflen)
{
    u8 i;
    for(i = 0; i < buflen; i++)
    {
      UART1_Sendbyte(buf[i]);
    }
}

/*
* 函数名：UART4_Sendbyte
* 描述  ：单字节发送
* 输入  ：data：待发送数据
* 输出  ：无
*/
void UART4_Sendbyte(INT8U data)
{

//    USART_SendData(UART4, data);
//    OSTimeDlyHMSM(0, 0, 0, 1);
    
    USART_SendData(UART4, (u8)data);
	while(USART_GetFlagStatus(UART4, USART_FLAG_TC) == RESET);

}
void UART4_Sendarray(INT8U *buf, INT8U buflen)
{
    u8 i;
    for(i = 0; i < buflen; i++)
    {
      UART4_Sendbyte(buf[i]);
    }
}


