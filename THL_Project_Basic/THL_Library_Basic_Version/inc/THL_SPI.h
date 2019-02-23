/*
 * THL_SPI.h
 *
 *  Created on: Feb 20, 2019
 *      Author: zhang
 */

#ifndef THL_SPI_H_
#define THL_SPI_H_
#include "THL_Portability.h"
#include "THL_Utility.h"
#include "THL_GPIO.h"


#ifdef HAL_SPI_MODULE_ENABLED

#define SPI_TxBuffer_Size 100
#define SPI_RxBuffer_Size 100
#define SPI_Default_TimeOut 10000  // 10 second
#define SPI_Dummy_Byte 0x00

typedef struct{
	SPI_HandleTypeDef *hspi;
	char TxBuffer[SPI_TxBuffer_Size];
	char RxBuffer[SPI_RxBuffer_Size];
	uint32_t TxRxTimeOut; // in millisecond
	volatile uint8_t Status;
	GPIO* CS; //Chip Select
}SPI;


/*==============================instantiation===============================*/
SPI *newSPI(SPI* instance, SPI_HandleTypeDef *hspi);

/*With Software chip select*/
SPI *newSPI_CS(SPI* instance, SPI_HandleTypeDef *hspi, GPIO* ChipSelect);
/*=========================================================================*/



/*==============================Polling Mode===============================*/
char* spiReadWrite(SPI* instance);
uint8_t spiReadWriteReg(SPI* instance, uint8_t regAddress, uint8_t byte);
/*=========================================================================*/



/*==============================Interrupt Mode===============================*/
char* spiReadWrite_IT(SPI* instance);
uint8_t* spiReadWriteReg_IT(SPI* instance, uint8_t regAddress, uint8_t byte);
/*=========================================================================*/



/*==============================DMA Mode===============================*/
char* spiReadWrite_DMA(SPI* instance);
uint8_t* spiReadWriteReg_DMA(SPI* instance, uint8_t regAddress, uint8_t byte);
/*=========================================================================*/



/*==============================Native Callback===============================*/
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi);
/*=========================================================================*/


/*==============================Interrupt Handler===============================*/
__weak void IT_CallBack_SpiTRC(SPI* instance);
/*=========================================================================*/


#endif

#endif /* PERIPHERALS_DRIVERS_INC_THL_SPI_H_ */
