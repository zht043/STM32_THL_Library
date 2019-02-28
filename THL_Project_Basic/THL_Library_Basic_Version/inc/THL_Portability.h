#ifndef __THL_Portability_H
#define __THL_Portability_H

#ifndef USE_STM32F4
	#define USE_STM32F4
#endif


#ifdef STM32F1
#include "stm32f1xx_hal.h"
#endif

#ifdef USE_STM32F4
#include "stm32f4xx_hal.h"
#endif

#ifdef STM32F7
#include "stm32f7xx_hal.h"
#endif


#endif
