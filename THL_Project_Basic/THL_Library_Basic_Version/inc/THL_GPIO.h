#ifndef __THL_GPIO_H
#define __THL_GPIO_H
#include "THL_Portability.h"

typedef struct{
	GPIO_TypeDef* GPIOx;
	uint16_t GPIO_Pin;
}GPIO;

GPIO *newGPIO(GPIO* obj, GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
void turnOn(GPIO* obj);
void turnOff(GPIO* obj);
void gpioWrite(GPIO* obj, uint8_t Bit);
uint8_t gpioRead(GPIO* obj);
void blink(GPIO* obj, uint32_t blkPeriod_ms);
void toggle(GPIO* obj);

#endif


