/*
 * THL_I2cMaster.c
 *
 *  Created on: Feb 5, 2019
 *      Author: zhang
 */
#include "THL_I2C.h"
#include "string.h"

#ifdef HAL_I2C_MODULE_ENABLED

#define Max_Num_I2Cs 10
uint16_t numActiveI2Cs = 0;
I2C* ActiveI2Cs[Max_Num_I2Cs];

/*==============================instantiation===============================*/
/* Pseudo-Constructor
 * */
I2C *newI2C(I2C* instance, I2C_HandleTypeDef *hi2c) {
	instance->hi2c = hi2c;
	instance->TxTimeOut = I2C_Default_TxTimeOut;
	instance->RxTimeOut = I2C_Default_RxTimeOut;
	instance->TxStatus = Ready;
	instance->RxStatus = Ready;
	instance->AddressSize8Bit = True;
	for(int i = 0; i < numActiveI2Cs; i++)
		if(ActiveI2Cs[i]->hi2c == hi2c) {
			ActiveI2Cs[i] = instance;
			return instance;
		}
	ActiveI2Cs[numActiveI2Cs++] = instance;
	return instance;
}
/*=========================================================================*/





/*==============================Polling Mode===============================*/
void i2cWrite(I2C* instance, uint16_t Mode) {
	HAL_StatusTypeDef Status;

	if(Mode != I2C_SlaveMode)
		//The Mode variable here in I2C_MasterMode includes devAddress
		Status = HAL_I2C_Master_Transmit(instance->hi2c, Mode /*devAddress*/, (uint8_t*)instance->TxBuffer,
									     strlen(instance->TxBuffer),  instance->TxTimeOut);
	else
		Status =  HAL_I2C_Slave_Transmit(instance->hi2c, (uint8_t*)instance->TxBuffer,
										 strlen(instance->TxBuffer),  instance->TxTimeOut);

	if(Status == HAL_BUSY) instance->TxStatus = InProcess;
	else if(Status == HAL_TIMEOUT) {
		instance->TxStatus = TimeOut;

		__HAL_UNLOCK(instance->hi2c);
		instance->hi2c->State = HAL_I2C_STATE_READY;

		throwException("THL_I2C.c: i2cWrite() | TimeOut");
	}
	else if(Status == HAL_ERROR) {
		instance->TxStatus = Error;
		throwException("THL_I2C.c: i2cWrite() | Error");
	}
	else if(Status == HAL_OK) instance->TxStatus = Completed;
}


char* i2cRead(I2C* instance, uint16_t Mode, uint16_t size) {
	HAL_StatusTypeDef Status;
	memset(instance->RxBuffer, 0, strlen(instance->RxBuffer));

	if(Mode != I2C_SlaveMode)
		//The Mode variable here in I2C_MasterMode includes devAddress
		Status = HAL_I2C_Master_Receive(instance->hi2c, Mode /*devAddress*/,(uint8_t*)instance->RxBuffer,
			 	 	 	   	   	   	   	size,  instance->RxTimeOut);
	else
		Status = HAL_I2C_Slave_Receive(instance->hi2c, (uint8_t*)instance->RxBuffer,
				 	 	 	           size,  instance->RxTimeOut);

	if(Status == HAL_BUSY) instance->RxStatus = InProcess;
	else if(Status == HAL_TIMEOUT) {
		instance->RxStatus = TimeOut;
		//Unlock I2C
		__HAL_UNLOCK(instance->hi2c);
		instance->hi2c->State = HAL_I2C_STATE_READY;

		throwException("THL_I2C.c: I2CRead() | TimeOut");
	}
	else if(Status == HAL_ERROR) {
		instance->RxStatus = Error;
		throwException("THL_I2C.c: I2CRead() | Error");
	}
	else if(Status == HAL_OK) instance->RxStatus = Completed;
	return instance->RxBuffer;
}

/*Write 1 byte only to a particular register, buffer-less, master mode only*/
void i2cWriteReg(I2C* instance, uint16_t devAddress, uint16_t regAddress, uint8_t byte) {
	HAL_StatusTypeDef Status;
	instance->TxByte = byte;
	Status = HAL_I2C_Mem_Write(instance->hi2c, I2C_MasterMode(devAddress), regAddress,
						instance->AddressSize8Bit?I2C_MEMADD_SIZE_8BIT:I2C_MEMADD_SIZE_16BIT ,
						&instance->TxByte, 1, instance->TxTimeOut);
	if(Status == HAL_BUSY) instance->TxStatus = InProcess;
	else if(Status == HAL_TIMEOUT) {
		instance->TxStatus = TimeOut;

		__HAL_UNLOCK(instance->hi2c);
		instance->hi2c->State = HAL_I2C_STATE_READY;

		throwException("THL_I2C.c: i2cWriteReg() | TimeOut");
	}
	else if(Status == HAL_ERROR) {
		instance->TxStatus = Error;
		throwException("THL_I2C.c: i2cWriteReg() | Error");
	}
	else if(Status == HAL_OK) instance->TxStatus = Completed;
}

/*Receive 1 byte only from a particular register, buffer-less, master mode only*/
uint8_t* i2cReadReg(I2C* instance, uint16_t devAddress, uint16_t regAddress) {
	HAL_StatusTypeDef Status;
	Status = HAL_I2C_Mem_Read(instance->hi2c, I2C_MasterMode(devAddress), regAddress,
			instance->AddressSize8Bit?I2C_MEMADD_SIZE_8BIT:I2C_MEMADD_SIZE_16BIT ,
			&instance->RxByte, 1, instance->RxTimeOut);

	if(Status == HAL_BUSY) instance->RxStatus = InProcess;
	else if(Status == HAL_TIMEOUT) {
		instance->RxStatus = TimeOut;
		//Unlock I2C
		__HAL_UNLOCK(instance->hi2c);
		instance->hi2c->State = HAL_I2C_STATE_READY;

		throwException("THL_I2C.c: I2cReadReg() | TimeOut");
	}
	else if(Status == HAL_ERROR) {
		instance->RxStatus = Error;
		throwException("THL_I2C.c: I2cReadReg() | Error");
	}
	else if(Status == HAL_OK) instance->RxStatus = Completed;
	return &instance->RxByte;
}
/*=========================================================================*/

/*==============================Interrupt Mode===============================*/
void i2cWrite_IT(I2C* instance, uint16_t Mode) {
	//check if the previous transmission is completed
	if(instance->TxStatus == InProcess) return;
	HAL_StatusTypeDef Status;
	if(Mode != I2C_SlaveMode)
		//The Mode variable here in I2C_MasterMode includes devAddress
		Status = HAL_I2C_Master_Transmit_IT(instance->hi2c, Mode /*Actually devAddress*/,
			                       (uint8_t*)instance->TxBuffer, strlen(instance->TxBuffer));
	else
		Status =  HAL_I2C_Slave_Transmit_IT(instance->hi2c, (uint8_t*)instance->TxBuffer,
										 strlen(instance->TxBuffer));

	if(Status == HAL_ERROR) {
		instance->TxStatus = Error;
		throwException("THL_I2C.c: i2cWrite_IT() | Error");
		return;
	}
	instance->TxStatus = InProcess;
}

char* i2cRead_IT(I2C* instance, uint16_t Mode, uint16_t size) {
	//check if the previous reception is completed
	if(instance->RxStatus == InProcess) return instance->RxBuffer;
	HAL_StatusTypeDef Status;
	memset(instance->RxBuffer, 0, strlen(instance->RxBuffer));

	if(Mode != I2C_SlaveMode)
			//The Mode variable here in I2C_MasterMode includes devAddress
			Status = HAL_I2C_Master_Receive_IT(instance->hi2c, Mode /*Actually devAddress*/,
				                       (uint8_t*)instance->RxBuffer, size);
		else
			Status =  HAL_I2C_Slave_Receive_IT(instance->hi2c, (uint8_t*)instance->RxBuffer,
											 size);
	if(Status == HAL_ERROR) {
		instance->RxStatus = Error;
		throwException("THL_I2C.c: i2cRead_IT() | Error");
		return instance->RxBuffer;
	}
	instance->RxStatus = InProcess;
	return instance->RxBuffer;
}

void i2cWriteReg_IT(I2C* instance, uint16_t devAddress, uint16_t regAddress, uint8_t byte) {
	//check if the previous transmission is completed
	if(instance->TxStatus == InProcess) return;
	instance->TxByte = byte;
	HAL_StatusTypeDef Status;
	Status = HAL_I2C_Mem_Write_IT(instance->hi2c, devAddress, regAddress,
						instance->AddressSize8Bit?I2C_MEMADD_SIZE_8BIT:I2C_MEMADD_SIZE_16BIT ,
						&instance->TxByte, 1);
	if(Status == HAL_ERROR) {
		instance->TxStatus = Error;
		throwException("THL_I2C.c: i2cWriteReg_IT() | Error");
		return;
	}
	instance->TxStatus = InProcess;
}

uint8_t* i2cReadReg_IT(I2C* instance, uint16_t devAddress, uint16_t regAddress) {
	//check if the previous reception is completed
	if(instance->RxStatus == InProcess) return &instance->RxByte;
	HAL_StatusTypeDef Status;

	Status = HAL_I2C_Mem_Read_IT(instance->hi2c, devAddress, regAddress,
			instance->AddressSize8Bit?I2C_MEMADD_SIZE_8BIT:I2C_MEMADD_SIZE_16BIT ,
			&instance->RxByte, 1);

	if(Status == HAL_ERROR) {
		instance->RxStatus = Error;
		throwException("THL_I2C.c: i2cReadReg_IT() | Error");
		return &instance->RxByte;
	}
	instance->RxStatus = InProcess;
	return &instance->RxByte;
}
/*=========================================================================*/




/*==============================DMA Mode===============================*/
void i2cWrite_DMA(I2C* instance, uint16_t Mode) {
	//check if the previous transmission is completed
	if(instance->TxStatus == InProcess) return;
	HAL_StatusTypeDef Status;
	if(Mode != I2C_SlaveMode)
		//The Mode variable here in I2C_MasterMode includes devAddress
		Status = HAL_I2C_Master_Transmit_DMA(instance->hi2c, Mode /*Actually devAddress*/,
			                       (uint8_t*)instance->TxBuffer, strlen(instance->TxBuffer));
	else
		Status =  HAL_I2C_Slave_Transmit_DMA(instance->hi2c, (uint8_t*)instance->TxBuffer,
										 strlen(instance->TxBuffer));

	if(Status == HAL_ERROR) {
		instance->TxStatus = Error;
		throwException("THL_I2C.c: i2cWrite_DMA() | Error");
		return;
	}
	instance->TxStatus = InProcess;
}

char* i2cRead_DMA(I2C* instance, uint16_t Mode, uint16_t size) {
	//check if the previous reception is completed
	if(instance->RxStatus == InProcess) return instance->RxBuffer;
	HAL_StatusTypeDef Status;
	memset(instance->RxBuffer, 0, strlen(instance->RxBuffer));

	if(Mode != I2C_SlaveMode)
			//The Mode variable here in I2C_MasterMode includes devAddress
			Status = HAL_I2C_Master_Receive_DMA(instance->hi2c, Mode /*Actually devAddress*/,
				                       (uint8_t*)instance->RxBuffer, size);
		else
			Status =  HAL_I2C_Slave_Receive_DMA(instance->hi2c, (uint8_t*)instance->RxBuffer,
											 size);
	if(Status == HAL_ERROR) {
		instance->RxStatus = Error;
		throwException("THL_I2C.c: i2cRead_DMA() | Error");
		return instance->RxBuffer;
	}
	instance->RxStatus = InProcess;
	return instance->RxBuffer;
}

void i2cWriteReg_DMA(I2C* instance, uint16_t devAddress, uint16_t regAddress, uint8_t byte) {
	//check if the previous transmission is completed
	if(instance->TxStatus == InProcess) return;
	instance->TxByte = byte;
	HAL_StatusTypeDef Status;
	Status = HAL_I2C_Mem_Write_DMA(instance->hi2c, devAddress, regAddress,
						instance->AddressSize8Bit?I2C_MEMADD_SIZE_8BIT:I2C_MEMADD_SIZE_16BIT ,
						&instance->TxByte, 1);
	if(Status == HAL_ERROR) {
		instance->TxStatus = Error;
		throwException("THL_I2C.c: i2cWriteReg_DMA() | Error");
		return;
	}
	instance->TxStatus = InProcess;
}

uint8_t* i2cReadReg_DMA(I2C* instance, uint16_t devAddress, uint16_t regAddress) {
	//check if the previous reception is completed
	if(instance->RxStatus == InProcess) return &instance->RxByte;
	HAL_StatusTypeDef Status;

	Status = HAL_I2C_Mem_Read_DMA(instance->hi2c, devAddress, regAddress,
			instance->AddressSize8Bit?I2C_MEMADD_SIZE_8BIT:I2C_MEMADD_SIZE_16BIT ,
			&instance->RxByte, 1);

	if(Status == HAL_ERROR) {
		instance->RxStatus = Error;
		throwException("THL_I2C.c: i2cReadReg_DMA() | Error");
		return &instance->RxByte;
	}
	instance->RxStatus = InProcess;
	return &instance->RxByte;
}
/*=========================================================================*/


/*==============================Native Callback===============================*/
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c) {
	for(int i = 0; i < numActiveI2Cs; i++) {
		if(ActiveI2Cs[i]->hi2c == hi2c) {
			ActiveI2Cs[i]->TxStatus = Completed;
			i2cTC_IT_CallBack(ActiveI2Cs[i]);
		}
	}
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c) {
	for(int i = 0; i < numActiveI2Cs; i++) {
		if(ActiveI2Cs[i]->hi2c == hi2c) {
			ActiveI2Cs[i]->RxStatus = Completed;
			i2cRC_IT_CallBack(ActiveI2Cs[i]);
		}
	}
}

void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c) {
	for(int i = 0; i < numActiveI2Cs; i++) {
		if(ActiveI2Cs[i]->hi2c == hi2c) {
			ActiveI2Cs[i]->TxStatus = Completed;
			i2cTC_IT_CallBack(ActiveI2Cs[i]);
		}
	}
}

void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c) {
	for(int i = 0; i < numActiveI2Cs; i++) {
		if(ActiveI2Cs[i]->hi2c == hi2c) {
			ActiveI2Cs[i]->RxStatus = Completed;
			i2cRC_IT_CallBack(ActiveI2Cs[i]);
		}
	}
}
/*=========================================================================*/


/*==============================Interrupt Handler===============================*/
__weak void i2cTC_IT_CallBack(I2C* instance){
	 UNUSED(instance);
}

__weak void i2cRC_IT_CallBack(I2C* instance){
	 UNUSED(instance);
}
/*=========================================================================*/


#endif




