/*
 * PMW3901.h
 *
 *  Created on: Mar 2, 2019
 *      Author: zhang
 */

#ifndef PMW3901_H_
#define PMW3901_H_

#include "THL_Library_Basic.h"
#include <string.h>


typedef struct{
	SPI* spiBus;
	GPIO* CS;
	volatile uint8_t Status;
	volatile int16_t delta_X_Cnt;
	volatile int16_t delta_Y_Cnt;
	volatile double delta_X;
	volatile double delta_Y;
	uint8_t chip_id;
	uint8_t inverse_produc_id;
}PMW3901;

PMW3901* newPMW3901(PMW3901* instance, SPI* spiBus, GPIO* chip_select);

uint8_t initPMW3901(PMW3901* instance);

PMW3901* getMotion(PMW3901* instance);

#endif /* PMW3901_H_ */
