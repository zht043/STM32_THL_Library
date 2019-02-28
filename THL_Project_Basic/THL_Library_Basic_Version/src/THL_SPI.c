/*
 * THL_SPI.c
 *
 *  Created on: Feb 20, 2019
 *      Author: zhang
 */

#include "THL_SPI.h"
#include "string.h"

#ifdef HAL_SPI_MODULE_ENABLED

#define DummyByte 0x00
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
/* Pseudo-Constructor with Chip Select
 * */
SPI *newSPI_CS(SPI* instance, SPI_HandleTypeDef *hspi, GPIO* ChipSelect) {
	if(hspi->Init.NSS != SPI_NSS_SOFT) {
		throwException("THL_SPI.c: newSPI_CS | Plz use newSPI instead");
		return instance;
	}
	instance->hspi = hspi;
	instance->TxRxTimeOut = SPI_Default_TimeOut;
	instance->Status = Ready;
	instance->CS = ChipSelect;

	for(int i = 0; i < numActiveSPIs; i++)
		if(ActiveSPIs[i]->hspi == hspi) {
			ActiveSPIs[i] = instance;
			return instance;
		}
	ActiveSPIs[numActiveSPIs++] = instance;

	gpioWrite(instance->CS, High);
	return instance;
}
/*=========================================================================*/



/*==============================Polling Mode===============================*/
char* spiReadWrite(SPI* instance) {
	if(instance->hspi->Init.NSS == SPI_NSS_SOFT) gpioWrite(instance->CS, Low);
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
	if(instance->hspi->Init.NSS == SPI_NSS_SOFT) gpioWrite(instance->CS, High);
	return instance->RxBuffer;
}


void spiWriteReg(SPI* instance, uint8_t regAddress, uint8_t byte) {
	instance->TxBuffer[0] = (char)(regAddress | 0x80); //0x80 = 1000,0000 in binary
	instance->TxBuffer[1] = (char)byte;
	instance->TxBuffer[2] = '\0';
	spiReadWrite(instance);
}

uint8_t* spiReadReg(SPI* instance, uint8_t regAddress) {
	instance->TxBuffer[0] = (char)(regAddress & ~0x80); //0x80 = 1000,0000 in binary
	instance->TxBuffer[1] = DummyByte;
	instance->TxBuffer[2] = '\0';
	return (uint8_t*)spiReadWrite(instance);
}

/*=========================================================================*/



/*==============================Interrupt Mode===============================*/
char* spiReadWrite_IT(SPI* instance) {
	//check if the previous reception is completed
	if(instance->Status == InProcess) return instance->RxBuffer;

	if(instance->hspi->Init.NSS == SPI_NSS_SOFT) gpioWrite(instance->CS, Low);

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

void spiWriteReg_IT(SPI* instance, uint8_t regAddress, uint8_t byte) {
	instance->TxBuffer[0] = (char)(regAddress | 0x80); //0x80 = 1000,0000 in binary
	instance->TxBuffer[1] = (char)byte;
	instance->TxBuffer[2] = '\0';
	spiReadWrite(instance);
}

uint8_t* spiReadReg_IT(SPI* instance, uint8_t regAddress) {
	instance->TxBuffer[0] = (char)(regAddress & ~0x80); //0x80 = 1000,0000 in binary
	instance->TxBuffer[1] = DummyByte;
	instance->TxBuffer[2] = '\0';
	return (uint8_t*)spiReadWrite(instance);
}

/*=========================================================================*/



/*==============================DMA Mode===============================*/
char* spiReadWrite_DMA(SPI* instance) {
	//check if the previous reception is completed
	if(instance->Status == InProcess) return instance->RxBuffer;

	if(instance->hspi->Init.NSS == SPI_NSS_SOFT) gpioWrite(instance->CS, Low);

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

void spiWriteReg_DMA(SPI* instance, uint8_t regAddress, uint8_t byte) {
	instance->TxBuffer[0] = (char)(regAddress | 0x80); //0x80 = 1000,0000 in binary
	instance->TxBuffer[1] = (char)byte;
	instance->TxBuffer[2] = '\0';
	spiReadWrite(instance);
}

uint8_t* spiReadReg_DMA(SPI* instance, uint8_t regAddress) {
	instance->TxBuffer[0] = (char)(regAddress & ~0x80); //0x80 = 1000,0000 in binary
	instance->TxBuffer[1] = DummyByte;
	instance->TxBuffer[2] = '\0';
	return (uint8_t*)spiReadWrite(instance);
}

/*=========================================================================*/



/*==============================Native Callback===============================*/
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi) {
	for(int i = 0; i < numActiveSPIs; i++) {
		if(ActiveSPIs[i]->hspi == hspi) {
			IT_CallBack_SpiTRC(ActiveSPIs[i]);
			ActiveSPIs[i]->Status = Completed;
			if(hspi->Init.NSS == SPI_NSS_SOFT) gpioWrite(ActiveSPIs[i]->CS, High);
		}
	}
}
/*=========================================================================*/



/*==============================Interrupt Handler===============================*/
__weak void IT_CallBack_SpiTRC(SPI* instance) {
	UNUSED(instance);
}
/*=========================================================================*/

#endif
