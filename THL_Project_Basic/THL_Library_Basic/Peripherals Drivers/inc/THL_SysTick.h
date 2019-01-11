#ifndef __THL_SYSTICK_H
#define __THL_SYSTICK_H
#include "THL_Portability.h"

uint32_t millis(void);
uint32_t micros(void);
void delay_us(uint32_t Time); 
void delay(uint32_t Time);

#endif

