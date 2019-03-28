#include "THL_GPIO.h"
#include "THL_SysTick.h"



/*========================SETUP AND DATA STRUCTURE========================*/
GPIO *newGPIO(GPIO* obj, GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin) {
	obj->GPIO_Pin = GPIO_Pin;
	obj->GPIOx = GPIOx;
	return obj;
}
/*=========================================================================*/

/*========================General GPIO Methods================================*/
void gpioWrite(GPIO* obj, uint8_t Bit) {
	if(Bit == High) turnOn(obj);
	else turnOff(obj);
}
Bool gpioRead(GPIO* obj) {
	return HAL_GPIO_ReadPin(obj->GPIOx, obj->GPIO_Pin) == GPIO_PIN_SET?High:Low;
}
/*=============================================================================*/


/*========================Convenience Purposed Methods========================*/
void turnOn(GPIO* obj) {
	HAL_GPIO_WritePin(obj->GPIOx, obj->GPIO_Pin, GPIO_PIN_SET);
}

void turnOff(GPIO* obj) {
	HAL_GPIO_WritePin(obj->GPIOx, obj->GPIO_Pin, GPIO_PIN_RESET);
}
void blink(GPIO* obj, uint32_t blkPeriod) {
	turnOn(obj);
	delay(blkPeriod / 2);
	turnOff(obj);
	delay(blkPeriod / 2);
}

void toggle(GPIO* obj) {
	HAL_GPIO_TogglePin(obj->GPIOx, obj->GPIO_Pin);
}
/*=============================================================================*/


