/*
 * THL_SPI.c
 *
 *  Created on: Feb 20, 2019
 *      Author: zhang
 */

#include "THL_SPI.h"
#include "string.h"

#ifdef HAL_SPI_MODULE_ENABLED

#define DummyByte 0xFF
#define Max_Num_SPIs 10
uint16_t numActiveSPIs = 0;
SPI* ActiveSPIs[Max_Num_SPIs];



/*==============================instantiation===============================*/
/* Pseudo-Constructor
 * */
SPI *newSPI(SPI* instance, SPI_HandleTypeDef *hspi) {
	instance->hspi = hspi;
	instance->TxRxTimeOut = SPI_Default_TimeOut;
	instance->Status = Ready;
	for(int i = 0; i < numActiveSPIs; i++)
		if(ActiveSPIs[i]->hspi == hspi) {
			ActiveSPIs[i] = instance;
			return instance;
		}
	ActiveSPIs[numActiveSPIs++] = instance;
	return instance;
}
/*=========================================================================*/



/*==============================Chip Select/Shared bus================================*/
/*For a SPI bus shared by multiple devices,
 *BeginDevice/EndDevice functions enable/disable
 *the particular chip_select pin wired to
 *each of the devices. Only one device can be enabled at one times*/
void spiBeginDevice(GPIO* chip_select) {
	gpioWrite(chip_select, Low);  //Pull CS low to enable device
}
void spiEndDevice(GPIO* chip_select) {
	gpioWrite(chip_select, High); //Pull CS High to disable device for another device
}

/* Parameter such as SPI bus clock, polarity etc can be reconfigured via this method
 * This is useful when multiple devices that requires different settings shares the
 * same bus
 * change settings in "instance->hspi->Init.xxxx" then pass the instance in*/
Bool spiReconfigHardParam(SPI* instance) {
	if (HAL_SPI_Init(instance->hspi) != HAL_OK) return False;
	return True;
}
/*=========================================================================*/



/*==============================Polling Mode===============================*/
char* spiReadWrite(SPI* instance) {
	HAL_StatusTypeDef Status;
	Status = HAL_SPI_TransmitReceive(instance->hspi,
									(uint8_t*)instance->TxBuffer,
									(uint8_t*)instance->RxBuffer,
									strlen(instance->TxBuffer), instance->TxRxTimeOut);
	if(Status == HAL_BUSY) instance->Status = InProcess;
	else if(Status == HAL_TIMEOUT) {
		instance->Status = TimeOut;

		__HAL_UNLOCK(instance->hspi);
		instance->hspi->State = HAL_SPI_STATE_READY;

		throwException("THL_SPI.c: spiReadWrite() | TimeOut");
	}
	else if(Status == HAL_ERROR) {
		instance->Status = Error;
		throwException("THL_SPI.c: spiReadWrite() | Error");
	}
	else if(Status == HAL_OK) instance->Status = Completed;
	return instance->RxBuffer;
}

uint8_t* spiReadWriteByte(SPI* instance, uint8_t byte) {
	HAL_StatusTypeDef Status;
	instance->TxByte = byte;
	Status = HAL_SPI_TransmitReceive(instance->hspi,
									&instance->TxByte,
									&instance->RxByte,
									1, instance->TxRxTimeOut);
	if(Status == HAL_BUSY) instance->Status = InProcess;
	else if(Status == HAL_TIMEOUT) {
		instance->Status = TimeOut;

		__HAL_UNLOCK(instance->hspi);
		instance->hspi->State = HAL_SPI_STATE_READY;

		throwException("THL_SPI.c: spiReadWriteByte() | TimeOut");
	}
	else if(Status == HAL_ERROR) {
		instance->Status = Error;
		throwException("THL_SPI.c: spiReadWriteByte() | Error");
	}
	else if(Status == HAL_OK) instance->Status = Completed;
	return &instance->RxByte;
}
/*=========================================================================*/



/*==============================Interrupt Mode===============================*/
char* spiReadWrite_IT(SPI* instance) {
	//check if the previous reception is completed
	if(instance->Status == InProcess) return instance->RxBuffer;

	memset(instance->RxBuffer, 0, strlen(instance->RxBuffer));
	HAL_StatusTypeDef Status;
	Status = HAL_SPI_TransmitReceive_IT(instance->hspi,
									(uint8_t*)instance->TxBuffer,
									(uint8_t*)instance->RxBuffer,
									strlen(instance->TxBuffer));

	if(Status == HAL_ERROR) {
		instance->Status = Error;
		throwException("SPI.c: spiReadWrite_IT() | Error");
		return instance->RxBuffer;
	}
	instance->Status = InProcess;
	return instance->RxBuffer;
}
uint8_t* spiReadWriteByte_IT(SPI* instance, uint8_t byte) {
	//check if the previous reception is completed
	if(instance->Status == InProcess) return &instance->RxByte;
	HAL_StatusTypeDef Status;
	instance->TxByte = byte;
	Status = HAL_SPI_TransmitReceive_IT(instance->hspi, &instance->TxByte, &instance->RxByte, 1);
	if(Status == HAL_ERROR) {
		instance->Status = Error;
		throwException("SPI.c: spiReadWriteByte_IT() | Error");
		return &instance->RxByte;
	}
	instance->Status = InProcess;
	return &instance->RxByte;
}
/*=========================================================================*/



/*==============================DMA Mode===============================*/
char* spiReadWrite_DMA(SPI* instance) {
	//check if the previous reception is completed
	if(instance->Status == InProcess) return instance->RxBuffer;

	memset(instance->RxBuffer, 0, strlen(instance->RxBuffer));
	HAL_StatusTypeDef Status;
	Status = HAL_SPI_TransmitReceive_DMA(instance->hspi,
									(uint8_t*)instance->TxBuffer,
									(uint8_t*)instance->RxBuffer,
									strlen(instance->TxBuffer));

	if(Status == HAL_ERROR) {
		instance->Status = Error;
		throwException("SPI.c: spiReadWrite_DMA() | Error");
		return instance->RxBuffer;
	}
	instance->Status = InProcess;
	return instance->RxBuffer;
}

uint8_t* spiReadWriteByte_DMA(SPI* instance, uint8_t byte) {
	//check if the previous reception is completed
	if(instance->Status == InProcess) return &instance->RxByte;
	HAL_StatusTypeDef Status;
	instance->TxByte = byte;
	Status = HAL_SPI_TransmitReceive_DMA(instance->hspi, &instance->TxByte, &instance->RxByte, 1);
	if(Status == HAL_ERROR) {
		instance->Status = Error;
		throwException("SPI.c: spiReadWriteByte_DMA() | Error");
		return &instance->RxByte;
	}
	instance->Status = InProcess;
	return &instance->RxByte;
}
/*=========================================================================*/



/*==============================Native Callback===============================*/
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi) {
	for(int i = 0; i < numActiveSPIs; i++) {
		if(ActiveSPIs[i]->hspi == hspi) {
			ActiveSPIs[i]->Status = Completed;
			spiTRC_IT_CallBack(ActiveSPIs[i]);
		}
	}
}
/*=========================================================================*/



/*==============================Interrupt Handler===============================*/
__weak void spiTRC_IT_CallBack(SPI* instance) {
	UNUSED(instance);
}
/*=========================================================================*/

#endif
