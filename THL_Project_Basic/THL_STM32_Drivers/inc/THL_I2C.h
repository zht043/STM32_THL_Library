/*
 * THL_I2cMaster.h
 *
 *  Created on: Feb 5, 2019
 *      Author: zhang
 */

#ifndef THL_I2C_H_
#define THL_I2C_H_
#include "THL_Portability.h"
#include "THL_Utility.h"


#ifdef HAL_I2C_MODULE_ENABLED

#define I2C_TxBuffer_Size 100
#define I2C_RxBuffer_Size 100
#define I2C_Default_TxTimeOut 10000  // 10 second
#define I2C_Default_RxTimeOut 10000  // 10 second

#define I2C_MasterMode(devAddress) devAddress << 1
#define I2C_SlaveMode 0xFFFF

typedef struct{
	I2C_HandleTypeDef *hi2c;
	char TxBuffer[I2C_TxBuffer_Size];
	char RxBuffer[I2C_RxBuffer_Size];
	uint8_t TxByte;
	uint8_t RxByte;
	uint32_t TxTimeOut; // in millisecond
	uint32_t RxTimeOut;
	volatile uint8_t TxStatus;
	volatile uint8_t RxStatus;
	Bool AddressSize8Bit;
}I2C;



/*==============================instantiation===============================*/
I2C *newI2C(I2C* instance, I2C_HandleTypeDef *hi2c);
/*=========================================================================*/



/*==============================Polling Mode===============================*/
void i2cWrite(I2C* instance, uint16_t Mode);
char* i2cRead(I2C* instance, uint16_t Mode, uint16_t size);
void i2cWriteReg(I2C* instance, uint16_t devAddress, uint16_t regAddress, uint8_t byte);
uint8_t* i2cReadReg(I2C* instance, uint16_t devAddress, uint16_t regAddress);
/*=========================================================================*/



/*==============================Interrupt Mode===============================*/
void i2cWrite_IT(I2C* instance, uint16_t Mode);
char* i2cRead_IT(I2C* instance, uint16_t Mode, uint16_t size);
void i2cWriteReg_IT(I2C* instance, uint16_t devAddress, uint16_t regAddress, uint8_t byte);
uint8_t* i2cReadReg_IT(I2C* instance, uint16_t devAddress, uint16_t regAddress);
/*=========================================================================*/



/*==============================DMA Mode===============================*/
void i2cWrite_DMA(I2C* instance, uint16_t Mode);
char* i2cRead_DMA(I2C* instance, uint16_t Mode, uint16_t size);
/*=========================================================================*/



/*==============================Native Callback===============================*/
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c);
void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c);
void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c);
void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c);
/*=========================================================================*/


/*==============================Interrupt Handler===============================*/
__weak void i2cTC_IT_CallBack(I2C* instance);
__weak void i2cRC_IT_CallBack(I2C* instance);
/*=========================================================================*/


#endif

#endif /* THL_I2CMASTER_H_ */
