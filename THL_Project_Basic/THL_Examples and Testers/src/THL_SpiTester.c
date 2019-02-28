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


//#define Master_Board
#define Slave_Board

static void testTwoNucleoBoardComm(void) {
	setup();
	char* receivedString;
#ifdef Master_Board


	gpioWrite(led, High);
	while(gpioRead(button));
	gpioWrite(led, Low);
	//Normal Polling Mode
	strcpy(spiBus->TxBuffer, "A message from the Master");
	receivedString = spiReadWrite(spiBus);
	printf_u("\rMessage Sent!!!!!!!!!!!!!!!!!\r\n");
	for(int i = 0; i<5;i++) blink(led, 300);
	printf_u("\rMessag from slave: %s\r\n", receivedString);
	/*//Non-Blocking interrupt mode
	gpioWrite(led, Low);
	strcpy(i2cBus->TxBuffer, "Message B from the Master");
	i2cWrite_IT(i2cBus, MasterMode(Target_Address));
	while(i2cBus->TxStatus != Completed);
*/
	/*
	  //Non-Blocking DMA mode
	gpioWrite(led, Low);
	//Test continous Writeing
	for(int i = 0; i < 3; i++) {
		strcpy(i2cBus->TxBuffer, "Message C from the Master");
		i2cWrite_DMA(i2cBus, MasterMode(Target_Address));
		while(i2cBus->TxStatus != Completed);

		//Give slave board a bit time to print
		delay(1000);
	}*/

#endif
#ifdef Slave_Board

	spiBus->hspi->Init.Mode = SPI_MODE_SLAVE;
	spiBus->hspi->Init.NSS = SPI_NSS_HARD_INPUT;
	HAL_SPI_Init(spiBus->hspi);

	printf_u("\rThis is slave Board\r\n");
	printf_u("\r*******************************\r\n");
	gpioWrite(led, High);

	//Normal Polling Mode
	strcpy(spiBus->TxBuffer, "A message from the Slave.");
	receivedString = spiReadWrite(spiBus);
	toggle(led);
	printf_u("\rMessag from Master: %s\r\n", receivedString);

	/*
	// Non-Blocking interrupt mode


	// Non-Blocking DMA mode
	gpioWrite(led, High);
	i2cRead_DMA(i2cBus, SlaveMode, 25);
	while(1) {
		if(i2cBus->RxStatus == Completed) {
			gpioWrite(led, Low);

			//Printing the previous 25 bytes
			printf_u("\r=================================\r\n");
			printf_u("\rReceived: \r\n");
			printf_u("\r%s\r\n", i2cBus->RxBuffer);
			printf_u("\r=================================\r\n\n");

			//Reading the next 25 bytes
			i2cRead_DMA(i2cBus, SlaveMode, 25);
		}
	}*/

#endif
}


void testSpi(void) {
	testTwoNucleoBoardComm();
}

void Exception_Handler(const char* str) {
	printf_u("\r%s\r\n",str);
}


