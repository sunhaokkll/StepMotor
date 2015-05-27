#ifndef __PTO_H__
#define __PTO_H__
void vPTOConfig(void);
void PTO(unsigned short Cycle, unsigned short Pulse_Num);
void MotorF(unsigned short Freq, unsigned short Pulse_Num);
void MotorB(unsigned short Freq, unsigned short Pulse_Num);
void MotorS(void);

void PTOWithAcceleration(unsigned short Freq);	/*带加速度启动PWM输出  500HZ / 200MS*/
void PTOStopWithDeceleration(void);	/*带减速度关闭PWM输出  500HZ / 200MS*/

static void PTO_Stop(void);/*关闭PWM输出*/ 

void PTO_HZ_NONum(unsigned short Freq);/*输出指定频率PWM 无时间限制*/
void PTO_HZ_Period(unsigned short Freq,unsigned short period);	/*输出20ms指定频率PWM*/  

void PTO_HZ_NUM(unsigned short Freq, unsigned int Pulse_Num);
static void PTO_Cycle(unsigned short Cycle);
static void PTO_HZ_NUM_Start(unsigned char Flag, signed int pPulseNum_Rel);

#define EXTI1_OFF EXTI->IMR &= (~EXTI_IMR_MR1)
#define EXTI1_ON  EXTI->IMR |= EXTI_IMR_MR1

#define EXTI2_OFF EXTI->IMR &= (~EXTI_IMR_MR2)
#define EXTI2_ON  EXTI->IMR |= EXTI_IMR_MR2

/*Step自动模式标志组*/
#define AutoF 0x01
#define AutoB 0x02
#define AutoS 0x04

/*加减速参数定义
*加速度为：ACC_HZ_MIN / ACC_PERIOD  HZ/MS 
*/
/*加减速最小时间单位  ms*/
#define ACC_PERIOD   100
#define DEC_PERIOD   100
/*加减速最小频率单位  hz*/
#define ACC_HZ_MIN   200
#define DEC_HZ_MIN   200

#endif
