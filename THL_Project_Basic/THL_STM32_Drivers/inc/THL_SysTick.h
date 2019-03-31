#ifndef __THL_SYSTICK_H
#define __THL_SYSTICK_H
#include "THL_Portability.h"
#include "THL_Timer.h"


#ifdef SysTick_Reserved
void initSysTime_TIM(TIM_HandleTypeDef *htim, uint32_t APBx_DivFactor, TIM_Resolution xBitTIM);
void timSysT_IT_CallBack(TIM* instance);
#endif

uint32_t millis(void);
uint32_t micros(void);
void delay_us(uint32_t Time); 
void delay(uint32_t Time);

#endif

