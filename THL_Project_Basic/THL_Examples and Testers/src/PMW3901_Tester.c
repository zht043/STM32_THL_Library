/*
 * PMW3901_Tester.c
 *
 *  Created on: Mar 2, 2019
 *      Author: zhang
 */

#include "PMW3901_Tester.h"
#include "PMW3901.h"
#include "main.h"

extern UART_HandleTypeDef huart2;
extern SPI_HandleTypeDef hspi2;

SPI spiBusMem;
SPI* spiBus;

GPIO ledMem;
GPIO* led;

GPIO buttonMem;
GPIO* button;

GPIO spi_csMem;
GPIO* spi_cs;


PMW3901 YSTZmem;
PMW3901* YSTZ;

static void setupSPI(void) {
	newMainUSART(&huart2);
	spiBus = newSPI(&spiBusMem ,&hspi2);
	led = newGPIO(&ledMem, LD2_GPIO_Port, LD2_Pin);
	button = newGPIO(&buttonMem, B1_GPIO_Port, B1_Pin);
	spi_cs = newGPIO(&spi_csMem, SPI2_SoftCS_GPIO_Port, SPI2_SoftCS_Pin);
}

void testPMW3901(void) {
    setupSPI();

	YSTZ = newPMW3901(&YSTZmem, spiBus, spi_cs);


    printf_u("\r%s\r\n", initPMW3901(YSTZ)?"True":"False");

    printf_u("\rYuan Shi Tian Zun\r\n");


    while(1) {
    	getMotion(YSTZ);
    	printf_u("\rX[%6d] Y[%6d]\r\n ", YSTZ->delta_X_Cnt, YSTZ->delta_Y_Cnt);
    	delay(100);
    }

}
