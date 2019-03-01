/*
 * THL_SpiTester.c
 *
 *  Created on: Feb 28, 2019
 *      Author: zhang
 */

#include "THL_SpiTester.h"
#include "main.h"

extern UART_HandleTypeDef huart2;
extern SPI_HandleTypeDef hspi2;

SPI spiBusMem;
SPI* spiBus;

GPIO ledMem;
GPIO* led;
GPIO buttonMem;
GPIO* button;



static void setup(void) {
	newMainUSART(&huart2);
	spiBus = newSPI(&spiBusMem ,&hspi2);
	led = newGPIO(&ledMem, LD2_GPIO_Port, LD2_Pin);
	button = newGPIO(&buttonMem, B1_GPIO_Port, B1_Pin);
}

/*uncomment the board u wanna flash to, then comment the other board*/
#define Master_Board    //code to be downloaded to the Master board
//#define Slave_Board     //code to be downloaded the Slave  board

static void testTwoNucleoBoardComm(void) {
	setup();
	char* receivedString;

#ifdef Master_Board
	gpioWrite(led, High);
	while(gpioRead(button));
	gpioWrite(led, Low);

	//Normal Polling Mode
	strcpy(spiBus->TxBuffer, "A message from the Master [Blocking Mode]");
	receivedString = spiReadWrite(spiBus);
	printf_u("\rMessage Sent!!!!!!!!!!!!!!!!!\r\n");
	for(int i = 0; i<5;i++) blink(led, 300);
	printf_u("\rMessag from slave: %s\r\n", receivedString);

	// Non-Blocking interrupt mode or Non-Blocking DMA mode
	strcpy(spiBus->TxBuffer, "A message from the Master [Non Blocking Mode]");
	//for(int i = 0; i < 3; i++) {
	while(1) {
		//spiReadWrite_DMA(spiBus);
		spiReadWrite_IT(spiBus);

		printf_u("\rMessage Sent!!!!!!!!!!!!!!!!!\r\n");
		while(spiBus->Status == InProcess);
		printf_u("\rMessag from slave: %s\r\n", spiBus->RxBuffer);
		delay(100);
	}

#endif

#ifdef Slave_Board

	spiBus->hspi->Init.Mode = SPI_MODE_SLAVE;
	spiBus->hspi->Init.NSS = SPI_NSS_HARD_INPUT;
	HAL_SPI_Init(spiBus->hspi);

	printf_u("\rThis is slave Board\r\n");
	printf_u("\r*******************************\r\n");
	gpioWrite(led, High);

	//Normal Polling Mode
	strcpy(spiBus->TxBuffer, "A message from the Slave! [Blocking Mode]");
	receivedString = spiReadWrite(spiBus);
	toggle(led);
	printf_u("\rMessage from Master: %s\r\n", receivedString);

	// Non-Blocking interrupt mode or Non-Blocking DMA mode
	gpioWrite(led, High);
	strcpy(spiBus->TxBuffer, "A message from the Slave! [Non Blocking Mode]");

	//spiReadWrite_DMA(spiBus);
	spiReadWrite_IT(spiBus);

	int i = 0;
	while(1) {
		if(spiBus->Status == Completed) {
			gpioWrite(led, Low);
			printf_u("\n\rMessage from Master: %s\r\n", spiBus->RxBuffer);

			//spiReadWrite_DMA(spiBus);
			spiReadWrite_IT(spiBus);
		}
		else {
			i++;
			printf_u("\r%d",i);
		}
	}

#endif
}


void testSpi(void) {
	testTwoNucleoBoardComm();
}

void Exception_Handler(const char* str) {
	printf_u("\r%s\r\n",str);
}


