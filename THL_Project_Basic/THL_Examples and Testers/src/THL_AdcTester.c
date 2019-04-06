/*
 * THL_AdcTester.c
 *
 *  Created on: Apr 2, 2019
 *      Author: zhang
 */

#include "THL_AdcTester.h"
#include "main.h"

extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;

_ADC adc1_mem;
_ADC* adc1;

_ADC adc2_mem;
_ADC* adc2;

extern UART_HandleTypeDef huart2;
USART* system_console;

GPIO led_mem;
GPIO* led;
GPIO button_mem;
GPIO* button;

static void setup(void) {
	system_console = newMainUSART(&huart2);
	led = newGPIO(&led_mem, LD2_GPIO_Port, LD2_Pin);
	button = newGPIO(&button_mem, B1_GPIO_Port, B1_Pin);


	printf_u("\rADC Testing\r\n");
}

static void testAdc_Polling(void) {
	adc2 = newADC(&adc2_mem, &hadc2);

	adcConvBegin(adc2, 10);

	uint16_t val;
	while(1) {
		val = adcGetVal(adc2);
		printf_u("\r[%5d]\r\n", val);
	}

	adcConvEnd(adc2);
}

static void testAdc_IT(void) {
	adc2 = newADC(&adc2_mem, &hadc2);

	adcConvBegin_IT(adc2);

	adcRequestVal_IT(adc2);
	uint16_t val = 0;
	while(1) {
		if(adc2->ConvStatus == Completed) {
			val = adcGetVal_IT(adc2);
			adcRequestVal_IT(adc2);
		}
		printf_u("\r[%5d]\r\n", val);
	}

	adcConvEnd(adc2);
}

static void testAdc_DMA(void) {
	adc1 = newADC(&adc1_mem, &hadc1);

	adcConvBegin_DMA(adc1);

	volatile uint16_t* val;
	while(1) {
		val = adcGetVal_DMA(adc1);

		printf_u("\r");
		for(int i = 0; i < adcGetNumChannel(adc1); i++) {
			printf_u("[%5d]  ", val[i]);
		}
		printf_u("\r\n");
	}

	adcConvEnd(adc1);
}

void testAdc(void) {
	setup();

	//testAdc_Polling();
	UNUSED(testAdc_Polling);

	//testAdc_IT();
	UNUSED(testAdc_IT);

	//Most recommended approach for ADCs
	testAdc_DMA();
	//UNUSED(testAdc_DMA);

}

void Exception_Handler(const char* str) {

	printf_u("\r%s\r\n",str);
}

