/*
 * THL_ADC.h
 *
 *  Created on: Apr 1, 2019
 *      Author: zhang
 */

#ifndef THL_ADC_H_
#define THL_ADC_H_


#include "THL_Utility.h"
#include "THL_Portability.h"

#ifdef HAL_ADC_MODULE_ENABLED

/* Make sure to config hardware settings properly with CubeMx software!
 *
 * For multi-channel scan, [Scan Conversion Mode] must be enabled
 * */

//In case of THL library consuming too much RAM space, user may lower this amount if only a few channels are used
#define ADC_Max_Num_Channels 20

#define ADC_Polling_Mode 0
#define ADC_IT_Mode 1
#define ADC_DMA_Mode 2


typedef struct {
	ADC_HandleTypeDef *hadc;

	uint8_t mode;
	volatile uint32_t NumConv;

	//Conversion result stored in this array are ordered by the channel RANK set in CubeMx
	volatile uint16_t ConvertedVal[ADC_Max_Num_Channels];

	//Used only in polling mode
	uint32_t TimeOut;

	volatile CommStatus ConvStatus;
}_ADC;  //Naming conflict, keyword "ADC" has already been used



/*=============================instantiation===============================*/
_ADC *newADC(_ADC* instance, ADC_HandleTypeDef *hadc);
/*=========================================================================*/



/*==================Polling Mode (Single Channel Only)=====================*/
void adcConvBegin(_ADC* instance, uint32_t time_out_ms);
void adcConvEnd(_ADC* instance);

volatile uint16_t adcGetVal(_ADC* instance);
/*=========================================================================*/



/*================Interrupt Mode (Single Channel Only)=====================*/
void adcConvBegin_IT(_ADC* instance);
void adcConvEnd_IT(_ADC* instance);
void adcRequestVal_IT(_ADC* instance);
volatile uint16_t adcGetVal_IT(_ADC* instance);
/*=========================================================================*/



/*================DMA Mode (Continuous Conversion Only)====================*/
void adcConvBegin_DMA(_ADC* instance);
void adcConvEnd_DMA(_ADC* instance);
volatile uint16_t *adcGetVal_DMA(_ADC* instance);
uint32_t adcGetNumChannel(_ADC* instance);
/*=========================================================================*/



/*==============================Native Callback============================*/
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc);
/*=========================================================================*/


/*==============================Interrupt Handler==========================*/
__weak void adcConvCplt_IT_CallBack(_ADC* instance);
/*=========================================================================*/



#endif

#endif /* THL_ADC_H_ */
