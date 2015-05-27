#ifndef __TCP_CLIENT_DEMO_H
#define __TCP_CLIENT_DEMO_H

#include "includes.h"
   
 
 
#define TCP_CLIENT_RX_BUFSIZE	2048	//接收缓冲区长度
#define REMOTE_PORT				8087	    //定义远端主机端口
#define LWIP_SEND_DATA			0X80    //定义有数据发送

#define xNETCONN 0
#define xSOCKET 1


extern u8 tcp_client_recvbuf[TCP_CLIENT_RX_BUFSIZE];	//TCP客户端接收数据缓冲区
extern u8 tcp_client_flag;		//TCP客户端数据发送标志位

u8 tcp_client_init(u8 flag);  //tcp客户端初始化(创建tcp客户端线程) 
                              //参数：flag xNETCONN(0) NETCONN方式 xSOCKET(1) socket方式
                              
                              
static void tcp_client_thread_socket (void *arg);
static void tcp_client_thread_netconn(void *arg);
#endif

