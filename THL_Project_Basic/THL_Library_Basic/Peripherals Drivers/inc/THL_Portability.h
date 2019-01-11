#ifndef __THL_Portability_H
#define __THL_Portability_H
#include "stm32f4xx_hal.h"

//----------------------------------------------//
/*Select the Chip/Product you are using*/
/*Do not define multiple products*/
#define UseNucleoF446
//----------------------------------------------//

#ifdef UseNucleoF446
	#define NumTimers 14
	#define NumUsarts 6
#endif

#ifdef UseNucleoF411
#endif

#ifdef UseSTM32F405RGT
#endif



//-----------------functions--------------------//
uint8_t USARTx(UART_HandleTypeDef *huart);
uint8_t TIMx(TIM_HandleTypeDef *htim);
uint32_t APBx_Div(TIM_HandleTypeDef *htim);
#endif
