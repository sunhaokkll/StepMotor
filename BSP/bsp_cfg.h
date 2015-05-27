#ifndef __BSP_CFG_H__
#define __BSP_CFG_H__
//中断优先级定义

#define SR04_EXTI_PRIO     2       //SR04 ECHO回波接收中断          外部中断
#define ADC_DMA_PRIO       5       //ADC   DMA 通道转换完毕中断     DMA通道中断
#define UART_RX_PRIO       3        //UART接收中断                   UART中断
#define LWIP_TIM_PRIO      6        //周期调用lwip底层接收函数       定时器中断 
#define AM2305_EXTI_PRIO   1         //AM2305 数据接收中断            外部中断
#define PWM_TIM_PRIO       7         //定时器PWM个数中断           TIM4
#define PWM_TIM1_PRIO      10

#define LS_EXTI_PRIO 			 9 				//限位开关外部中断
#define LS_EXTI_PRIO1 			 8				//限位开关外部中断


#endif
