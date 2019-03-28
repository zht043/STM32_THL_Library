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
#define SPI_Default_TimeOut 0xFFFF
#define SPI_Dummy_Byte 0x00

typedef struct{
	SPI_HandleTypeDef *hspi;
	char TxBuffer[SPI_TxBuffer_Size];
	char RxBuffer[SPI_RxBuffer_Size];
	uint8_t TxByte;
	uint8_t RxByte;
	uint32_t TxRxTimeOut; // in millisecond
	volatile uint8_t Status;
}SPI;

/*==========================Set Read/Write Bit=============================*/
//Master
//Read + 0 at MSB, Write + 1 at MSB
#define SPI_ReadMode(regAddr) regAddr & ~0x80   //~0x80 = 0111,1111 in binary
#define SPI_WriteMode(regAddr) regAddr | 0x80   // 0x80 = 1000,0000 in binary

//Device/Slave
#define SPI_isRequestingRead(byte) !(byte >> 7)
#define SPI_isRequestingWrite(byte) (byte >> 7)
#define SPI_getRegAddr(byte) byte & ~0x80
/*=========================================================================*/




/*==============================instantiation===============================*/
SPI *newSPI(SPI* instance, SPI_HandleTypeDef *hspi);
/*=========================================================================*/


/*==============================Chip Select================================*/
/*For a SPI bus shared by multiple devices,
 *BeginDevice/EndDevice functions enable/disable
 *the particular chip_select pin wired to
 *each of the devices. Only one device can be enabled at one times*/
void spiBeginDevice(GPIO* chip_select);
void spiEndDevice(GPIO* chip_select);

Bool spiReconfigHardParam(SPI* instance);
/*=========================================================================*/


/*==============================Polling Mode===============================*/
char* spiReadWrite(SPI* instance);
uint8_t* spiReadWriteByte(SPI* instance, uint8_t byte);
/*=========================================================================*/



/*==============================Interrupt Mode===============================*/
char* spiReadWrite_IT(SPI* instance);
uint8_t* spiReadWriteByte_IT(SPI* instance, uint8_t byte);
/*=========================================================================*/



/*==============================DMA Mode===============================*/
char* spiReadWrite_DMA(SPI* instance);
uint8_t* spiReadWriteByte_DMA(SPI* instance, uint8_t byte);
/*=========================================================================*/



/*==============================Native Callback===============================*/
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi);
/*=========================================================================*/


/*==============================Interrupt Handler===============================*/
__weak void spiTRC_IT_CallBack(SPI* instance);
/*=========================================================================*/


#endif

#endif /* PERIPHERALS_DRIVERS_INC_THL_SPI_H_ */
