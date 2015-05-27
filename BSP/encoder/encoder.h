#ifndef __ENCODER_H
#define __ENCODER_H
#include "includes.h"
 
void vENCODERConfig(void);

signed int s32GetEncoderCNT(void);
void prvTIM8_encoder(void); 
#endif
