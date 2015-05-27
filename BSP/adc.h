#ifndef __ADC_H__
#define __ADC_H__
void vADCConfig(void);

static void prvDMAConfig(void);
static void prvADCNVICInit(void);
static void GetTemp(unsigned short advalue);   
#endif
