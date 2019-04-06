/*
 * THL_ADC.c
 *
 *  Created on: Apr 1, 2019
 *      Author: zhang
 */



#include "THL_ADC.h"
#include "THL_SysTick.h"


#ifdef HAL_ADC_MODULE_ENABLED

#define Max_Num_ADCs 5
uint16_t numActiveADCs = 0;
_ADC* ActiveADCs[Max_Num_ADCs];

/**
 * Scan conversion mode must be enabled when there are more than one channel needed to be scanned,
 * in this case, either DMA mode, injected channel mode, or discontinuous mode with some tricks must be used.
 * DMA mode is the most straightforward and efficient way. (Time efficiency, not spacial nor hardware resources nor power efficiency).
 * Here only ADC w/ DMA API is provided since it is the most commonly used one.
 *
 *
 * Continuous conversion mode automatically restart the next conversion upon completing the current one.
 *
 * if scan mode is enabled but continuous mode is disabled, each single conversion scans all channels then stop;
 */




/*=============================instantiation===============================*/
_ADC *newADC(_ADC* instance, ADC_HandleTypeDef *hadc) {
	instance->hadc = hadc;
	instance->TimeOut = 0xFFFF;
	instance->NumConv = instance->hadc->Init.NbrOfConversion;

	//instance->ConvStatus = Ready;
	instance->ConvStatus = InProcess;
	for(int i = 0; i < ADC_Max_Num_Channels; i++) instance->ConvertedVal[i] = 0;
	for(int i = 0; i < numActiveADCs; i++)
		if(ActiveADCs[i]->hadc == hadc) {
			ActiveADCs[i] = instance;
			return instance;
		}
	ActiveADCs[numActiveADCs++] = instance;
	return instance;
}
/*=========================================================================*/



/*==================Polling Mode (Single Channel Only)=====================*/
void adcConvBegin(_ADC* instance, uint32_t time_out_ms) {
	instance->TimeOut = time_out_ms;
	instance->mode = ADC_Polling_Mode;
	if(instance->hadc->Init.ContinuousConvMode == ENABLE) {
		HAL_ADC_Start(instance->hadc);
	}
}

void adcConvEnd(_ADC* instance) {
	HAL_ADC_Stop(instance->hadc);
}

volatile uint16_t adcGetVal(_ADC* instance) {
	instance->ConvStatus = InProcess;
	uint32_t t0;
	if(instance->hadc->Init.ContinuousConvMode == DISABLE) {
		HAL_ADC_Start(instance->hadc);
	}
	HAL_ADC_PollForConversion(instance->hadc, instance->TimeOut);
	t0 = millis();
	while(!HAL_IS_BIT_CLR(HAL_ADC_GetState(instance->hadc), HAL_ADC_STATE_REG_EOC)) {
		if(millis() - t0 > instance->TimeOut) break;
		instance->ConvertedVal[0] = HAL_ADC_GetValue(instance->hadc);
	}
	instance->ConvStatus = Completed;
	return instance->ConvertedVal[0];
}
/*=========================================================================*/


/*================Interrupt Mode (Single Channel Only)=====================*/
void adcConvBegin_IT(_ADC* instance) {
	instance->mode = ADC_IT_Mode;
	if(instance->hadc->Init.ContinuousConvMode == ENABLE) {
		HAL_ADC_Start_IT(instance->hadc);
	}
}
void adcConvEnd_IT(_ADC* instance) {
	HAL_ADC_Stop_IT(instance->hadc);
}

//Not needed if ContinuousConv mode is enabled
void adcRequestVal_IT(_ADC* instance) {
	if(instance->hadc->Init.ContinuousConvMode == DISABLE) {
		HAL_ADC_Start_IT(instance->hadc);
	}
}

volatile uint16_t adcGetVal_IT(_ADC* instance) {
	if(instance->ConvStatus == Completed) {
		instance->ConvStatus = InProcess;
	}
	return instance->ConvertedVal[0];
}
/*=========================================================================*/




/*================DMA Mode (Continuous Conversion Only)====================*/
void adcConvBegin_DMA(_ADC* instance) {
	instance->mode = ADC_DMA_Mode;
	HAL_ADC_Start_DMA(instance->hadc, (uint32_t*)instance->ConvertedVal, instance->NumConv);
}
void adcConvEnd_DMA(_ADC* instance) {
	HAL_ADC_Stop_DMA(instance->hadc);
}

volatile uint16_t *adcGetVal_DMA(_ADC* instance) {
	/* This if is only meaningful if adc global interrupt is enabled,
	 * if not enabled, just ignore the ->ConvStatus field in your application code
	 * */
	if(instance->ConvStatus == Completed) {
		instance->ConvStatus = InProcess;
	}
	return instance->ConvertedVal;
}

uint32_t adcGetNumChannel(_ADC* instance) {
	return instance->NumConv;
}

/*=========================================================================*/



/*==============================Native Callback============================*/
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
	for(int i = 0; i < numActiveADCs; i++) {
		if(ActiveADCs[i]->hadc == hadc) {
			if(ActiveADCs[i]->mode == ADC_IT_Mode) {
				ActiveADCs[i]->ConvertedVal[0] = HAL_ADC_GetValue(ActiveADCs[i]->hadc);
			}
			ActiveADCs[i]->ConvStatus = Completed;
			adcConvCplt_IT_CallBack(ActiveADCs[i]);
		}
	}
}
/*=========================================================================*/


/*==============================Interrupt Handler==========================*/
__weak void adcConvCplt_IT_CallBack(_ADC* instance) {
	UNUSED(instance);
}
/*=========================================================================*/






#endif


