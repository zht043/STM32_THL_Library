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

GPIO spi_csMem;
GPIO* spi_cs;

/*uncomment the board u wanna flash to, then comment the other board*/
#define Master_Board    //code to be downloaded to the Master board
//#define Slave_Board     //code to be downloaded the Slave  board



static void setup(void) {
	newMainUSART(&huart2);
	spiBus = newSPI(&spiBusMem ,&hspi2);
	led = newGPIO(&ledMem, LD2_GPIO_Port, LD2_Pin);
	button = newGPIO(&buttonMem, B1_GPIO_Port, B1_Pin);
	spi_cs = newGPIO(&spi_csMem, SPI2_SoftCS_GPIO_Port, SPI2_SoftCS_Pin);
}


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
	for(int i = 0; i < 3; i++) {
	//while(1) {
		spiReadWrite_DMA(spiBus);
		//spiReadWrite_IT(spiBus);

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

	spiReadWrite_DMA(spiBus);
	//spiReadWrite_IT(spiBus);

	int i = 0;
	while(1) {
		if(spiBus->Status == Completed) {
			gpioWrite(led, Low);
			printf_u("\n\rMessage from Master: %s\r\n", spiBus->RxBuffer);

			spiReadWrite_DMA(spiBus);
			//spiReadWrite_IT(spiBus);
		}
		else {
			i++;
			printf_u("\r%d",i);
		}
	}

#endif
}


//Test with a Logic Analyzer
static void testByteWiseReadWrite(void) {
	setup();

	uint8_t regAddrA = 0x1A;
	uint8_t regAddrB = 0x1B;
	uint8_t byteA = 0x0A;
	uint8_t byteB = 0x0B;

	gpioWrite(led, High);
	while(gpioRead(button));
	gpioWrite(led, Low);

	printf_u("\r==============================================================\r\n");
	uint8_t egWriteByte = 0xAE; //0xAE 1010,1110
	uint8_t egReadByte = 0x2F;  //0x2F 0010,1111
	printf_u("\rExample Write Byte: %#02x\r\n", egWriteByte);
	printf_u("\r>>> Is this incoming byte by master/mcu requesting a WRITE operation to a slave/device? T/F\r\n");
	printf_u("\r%s\r\n", SPI_isRequestingWrite(egWriteByte)?"True":"False");
	printf_u("\r>>> Is this incoming byte by master/mcu requesting a READ  operation to a slave/device? T/F\r\n");
	printf_u("\r%s\r\n", SPI_isRequestingRead(egWriteByte)?"True":"False");
	printf_u("\r\nExtract RegAddress from Example Byte: %#02x\r\n\n", SPI_getRegAddr(egWriteByte));
	printf_u("\r---------------------------------\r\n\n");
	printf_u("\rExample Read Byte: %#02x\r\n", egReadByte);
	printf_u("\r>>> Is this incoming byte by master/mcu requesting a WRITE operation to a slave/device? T/F\r\n");
	printf_u("\r%s\r\n", SPI_isRequestingWrite(egReadByte)?"True":"False");
	printf_u("\r>>> Is this incoming byte by master/mcu requesting a READ  operation to a slave/device? T/F\r\n");
	printf_u("\r%s\r\n", SPI_isRequestingRead(egReadByte)?"True":"False");
	printf_u("\r\nExtract RegAddress from Example Byte: %#02x\r\n", SPI_getRegAddr(egReadByte));
	printf_u("\r==============================================================\r\n");
	while(1) {
		spiBeginDevice(spi_cs);

		spiReadWriteByte(spiBus, SPI_WriteMode(regAddrA));
		spiReadWriteByte(spiBus, byteA);

		spiReadWriteByte(spiBus, SPI_ReadMode(regAddrB));
		spiReadWriteByte(spiBus, byteB);

		/*

		spiReadWriteByte_IT(spiBus, SPI_WriteMode(regAddrA));
		while(spiBus->Status != Completed);
		spiReadWriteByte_IT(spiBus, byteA);
		while(spiBus->Status != Completed);

		spiReadWriteByte_DMA(spiBus, SPI_ReadMode(regAddrB));
		while(spiBus->Status != Completed);
		spiReadWriteByte_DMA(spiBus, byteB);
		while(spiBus->Status != Completed);

		*/

		spiEndDevice(spi_cs);

		delay(10); //10 ms
	}
}

void testSpi(void) {
	testByteWiseReadWrite();
	//testTwoNucleoBoardComm();
	UNUSED(testTwoNucleoBoardComm); //shut the compiler warning
}

void Exception_Handler(const char* str) {
	printf_u("\r%s\r\n",str);
}


