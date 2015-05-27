#include "includes.h"
#define  ADC1_DR_Address    ((u32)0x40012400+0x4c) 
vu16 AD_Value[1];   //
float Current_Temp; 

void vADCConfig(void)
{
    /*SR04      管脚配置
	*TRIG(PC8)  推挽输出
	*ECHO(PC7)  浮空输入  外部中断
    */
    
    ADC_InitTypeDef ADC_InitStructure;   
	 /*开启时钟ADC1,DMA1*/
	//启动DMA时钟 
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);   
    //启动ADC1时钟    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    
   
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent; //转换模式为独立，还有交叉等非常多样的选择   
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;   
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;  //连续转换开启    
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;   
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;   
    ADC_InitStructure.ADC_NbrOfChannel = 1;     //设置转换序列长度为1   
    ADC_Init(ADC1, &ADC_InitStructure);   
      
    //ADC内置温度传感器使能（要使用片内温度传感器，切忌要开启它）    
    ADC_TempSensorVrefintCmd(ENABLE);   
       
    //常规转换序列1：通道16（内部温度传感器），采样时间>2.2us,(239cycles)    
    ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 1, ADC_SampleTime_239Cycles5); 

     //输入参数：ADC外设，ADC通道，转换序列顺序，采样时间 
    // Enable ADC1    
    ADC_Cmd(ADC1, ENABLE);   
    // 开启ADC的DMA支持（要实现DMA功能，还需独立配置DMA通道等参数）    
    ADC_DMACmd(ADC1, ENABLE);   
      
    // 下面是ADC自动校准，开机后需执行一次，保证精度    
    // Enable ADC1 reset calibaration register    
    ADC_ResetCalibration(ADC1);   
    // Check the end of ADC1 reset calibration register    
    while(ADC_GetResetCalibrationStatus(ADC1));   
   
    // Start ADC1 calibaration    
    ADC_StartCalibration(ADC1);   
    // Check the end of ADC1 calibration    
    while(ADC_GetCalibrationStatus(ADC1));
    // ADC自动校准结束---------------    
    ADC_SoftwareStartConvCmd(ADC1, ENABLE); //ADC启动    
     
    prvDMAConfig();
    prvADCNVICInit();
}
void prvDMAConfig()
{
    DMA_InitTypeDef DMA_InitStructure;   
      
    DMA_DeInit(DMA1_Channel1);   
    DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;   
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&AD_Value;   
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;    
    //如此设置，使序列1结果放在AD_Value[0]    
    DMA_InitStructure.DMA_BufferSize = 1;   
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;   
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;   
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;   
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;   
    //循环模式开启，Buffer写满后，自动回到初始地址开始传输    
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;   
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;   
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;   
    DMA_Init(DMA1_Channel1, &DMA_InitStructure);   
    //配置完成后，启动DMA通道    
    DMA_Cmd(DMA1_Channel1, ENABLE);
    DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE); //使能DMA传输完成中断 
}


static void prvADCNVICInit(void)
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
//	
	 /*使能DMA1_Channel1_IRQn 中断请求 4级占先式优先级*/
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn; 
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = ADC_DMA_PRIO; 
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; 
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
    NVIC_Init(&NVIC_InitStructure);          // Enable the DMA Interrupt 
}

void GetTemp(u16 advalue)   
{   
    float Vtemp_sensor;
      
//    ADC转换结束以后，读取ADC_DR寄存器中的结果，转换温度值计算公式如下：    
//          V25 - VSENSE    
//  T(℃) = ------------  + 25    
//           Avg_Slope    
//   V25：  温度传感器在25℃时 的输出电压，典型值1.43 V。    
//  VSENSE：温度传感器的当前输出电压，与ADC_DR 寄存器中的结果ADC_ConvertedValue之间的转换关系为：    
//            ADC_ConvertedValue * Vdd    
//  VSENSE = --------------------------    
//            Vdd_convert_value(0xFFF)    
//  Avg_Slope：温度传感器输出电压和温度的关联参数，典型值4.3 mV/℃。
//  Current_Temp = (V25 - Vtemp_sensor)/Avg_Slope + 25;     
   
    Vtemp_sensor = advalue * 3.3 / 4096;   
    Current_Temp = (1.43 - Vtemp_sensor)*100/4.3 + 25;   
}    

void DMA1_Channel1_IRQHandler(void)
{
    OSIntEnter();

    if(DMA_GetITStatus(DMA1_IT_TC1) != RESET)
    {
        GetTemp(AD_Value[0]);
        DMA_ClearITPendingBit(DMA1_IT_TC1);
    }

    OSIntExit();
}


