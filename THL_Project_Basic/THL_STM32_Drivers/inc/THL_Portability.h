#ifndef __THL_Portability_H
#define __THL_Portability_H


/*========================Global Setting Macros===========================*/
#define USE_STM32F4
#define SysTick_Reserved  //SysTick reserved for RTOS
/*========================================================================*/








#ifdef USE_STM32F1
#include "stm32f1xx_hal.h"
#endif

#ifdef USE_STM32F4
#include "stm32f4xx_hal.h"
#endif

#ifdef USE_STM32F7
#include "stm32f7xx_hal.h"
#endif






#endif
