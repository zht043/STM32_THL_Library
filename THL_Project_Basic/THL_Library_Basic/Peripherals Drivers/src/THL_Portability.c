#include "THL_Portability.h"
#include "THL_Usart.h"
#include "THL_Timer.h"
#include <string.h>

#ifdef UseNucleoF446
	#ifdef __THL_USART_H
		uint8_t USARTx(UART_HandleTypeDef *huart) {
			if(huart->Instance == USART1) return 1;
			if(huart->Instance == USART2) return 2;
			if(huart->Instance == USART3) return 3;
			if(huart->Instance == UART4) return 4;
			if(huart->Instance == UART5) return 5;
			if(huart->Instance == USART6) return 6;
			return 0;
		}
	#endif
		
	#ifdef __THL_TIMER_H
		uint32_t APBx_Div(TIM_HandleTypeDef *htim) {
			/*Refer to
				https://www.st.com/resource/en/datasheet/stm32f446re.pdf
			  Page 16*/
			// timer 1,8,9,10,11 are linked to APB2 bus, the rest are linked to APB1 bus
			
			//APBx_divsion factor depends on what you set it up to, in the CubeMx "Clock Tree" 
			#define APB2_div 2
			#define APB1_div 4
			
			if(htim->Instance == TIM1) return APB2_div;
			if(htim->Instance == TIM2) return APB1_div;
			if(htim->Instance == TIM3) return APB1_div;
			if(htim->Instance == TIM4) return APB1_div;
			if(htim->Instance == TIM5) return APB1_div;
			if(htim->Instance == TIM6) return APB1_div;
			if(htim->Instance == TIM7) return APB1_div;
			if(htim->Instance == TIM8) return APB2_div;
			if(htim->Instance == TIM9) return APB2_div;
			if(htim->Instance == TIM10) return APB2_div;
			if(htim->Instance == TIM11) return APB2_div;
			if(htim->Instance == TIM12) return APB1_div;
			if(htim->Instance == TIM13) return APB1_div;
			if(htim->Instance == TIM14) return APB1_div;
			return 1;
		}
		
		uint8_t TIMx(TIM_HandleTypeDef *htim) {
			if(htim->Instance == TIM1) return 1;
			if(htim->Instance == TIM2) return 2;
			if(htim->Instance == TIM3) return 3;
			if(htim->Instance == TIM4) return 4;
			if(htim->Instance == TIM5) return 5;
			if(htim->Instance == TIM6) return 6;
			if(htim->Instance == TIM7) return 7;
			if(htim->Instance == TIM8) return 8;
			if(htim->Instance == TIM9) return 9;
			if(htim->Instance == TIM10) return 10;
			if(htim->Instance == TIM11) return 11;
			if(htim->Instance == TIM12) return 12;
			if(htim->Instance == TIM13) return 13;
			if(htim->Instance == TIM14) return 14;
			return 0;
		}
	#endif
#endif

#ifdef UseNucleoF411
#endif

#ifdef UseSTM32F405RGT
#endif		
		
char ModelType[15];
char *getChipModelType(void) {
#ifdef UseNucleoF446
		strcpy(ModelType, "NucleoF446");
#endif	
		return ModelType;
}
