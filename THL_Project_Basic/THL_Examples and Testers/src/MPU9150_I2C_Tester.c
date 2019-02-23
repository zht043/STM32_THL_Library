/*
 * THL_I2cMasterTester.c
 *
 *  Created on: Feb 5, 2019
 *      Author: zhang
 */
#include "MPU9150_I2C_Tester.h"

GPIO led_pin;
GPIO* Led;

extern UART_HandleTypeDef huart2;
USART serial_console;


extern I2C_HandleTypeDef hi2c1;


////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MPU9150_H
#define MPU9150_H

#define SELF_TESTX_REG    0x0D
#define SELF_TESTY_REG    0x0E
#define SELF_TESTZ_REG    0x0F
#define SELF_TESTA_REG    0x10

#define SMPLRT_REG        0x19
#define CFG_REG           0x1A
#define GYRO_CFG_REG      0x1B
#define ACCEL_CFG1_REG    0x1C  // Only 1 config register for this device

#define FIFO_EN_REG       0x23
#define I2CMST_CTRL_REG   0x24
#define I2C_SLV0_ADDR_REG 0x25
#define I2C_SLV0_REG_REG  0x26
#define I2C_SLV0_CTRL_REG 0x27

#define I2CMST_STA_REG    0x36
#define INT_PIN_CFG_REG   0x37
#define INT_EN_REG        0x38
#define INT_STA_REG       0x3A

#define ACCEL_XOUTH_REG   0x3B
#define ACCEL_XOUTL_REG   0x3C
#define ACCEL_YOUTH_REG   0x3D
#define ACCEL_YOUTL_REG   0x3E
#define ACCEL_ZOUTH_REG   0x3F
#define ACCEL_ZOUTL_REG   0x40

#define TEMP_OUTH_REG     0x41
#define TEMP_OUTL_REG     0x42

#define GYRO_XOUTH_REG    0x43
#define GYRO_XOUTL_REG    0x44
#define GYRO_YOUTH_REG    0x45
#define GYRO_YOUTL_REG    0x46
#define GYRO_ZOUTH_REG    0x47
#define GYRO_ZOUTL_REG    0x48

#define SIGPATH_RST_REG   0x68
#define USER_CTRL_REG     0x6A
#define PWR_MGMT1_REG     0x6B
#define PWR_MGMT2_REG     0x6C
#define FIFO_CNTH_REG     0x72
#define FIFO_CNTL_REG     0x73
#define FIFO_RW_REG       0x74
#define WHO_AM_I_REG      0x75

#define MPU_ADDR          0x68

#endif

/////////////////////////////////////////////////////////////////////////
/*------------------------------------------------------------*/
//  (2) Define either MPU_USE_I2C or MPU_USE_SPI
/*------------------------------------------------------------*/

#define MPU_USE_I2C
//#define MPU_USE_SPI

/*------------------------------------------------------------*/
//  (3) Define which MPU device to use
/*------------------------------------------------------------*/

//#define MPU_9255
#define MPU_9150

/*------------------------------------------------------------*/
//  (4) Important Macro Definitions and Error Checking
/*------------------------------------------------------------*/

#if defined(MPU_USE_I2C)

 // #include "i2c.h"

  // (4) Ensure the proper i2c peripheral is used
  #define MPU_I2C  hi2c1

#elif defined(MPU_USE_SPI)

  #include "spi.h"

  // (4) Ensure the proper spi peripheral is used
  #define MPU_SPI  hspi1

#else
  #error "Must define either MPU_USE_I2C or MPU_USE_SPI in main.h"
#endif

/*------------------------------------------------------------*/
//  More important macros, DO NOT MODIFY
/*------------------------------------------------------------*/

#if defined(MPU_USE_I2C) & defined(MPU_USE_SPI)
  #error "Must only define one of MPU_USE_I2C or MPU_USE_SPI"
#endif

#if defined(MPU_9255) & defined(MPU_9150)
  #error "Must only define a single MPU device, MPU_9150 or MPU_9255"
#endif

#if defined(MPU_9150) & defined(MPU_USE_SPI)
  #error "MPU 9150 doesn't support SPI"
#endif

/*------------------------------------------------------------*/
//  Address Macros for SPI and I2C
/*------------------------------------------------------------*/

#define SPI_ADDR_ADD_W_BIT(x)    (x)
#define SPI_ADDR_ADD_R_BIT(x)    ((x) | 0x80)

#define MPU_ADDR               0x68
#define MPU_ADDR_W             (MPU_ADDR << 1)
#define MPU_ADDR_R             (MPU_ADDR << 1) | 1

/*------------------------------------------------------------*/
//  Config Options for device registers
/*------------------------------------------------------------*/

// Acceleration Full Scale Range, write to ACCEL_CFG1_REG<4:3>
#define MPU_ACCEL_FS_2G    0x00
#define MPU_ACCEL_FS_4G    0x01
#define MPU_ACCEL_FS_8G    0x02
#define MPU_ACCEL_FS_16G   0x03

// Gyro Full Scale Range, write to GYRO_CFG_REG<4:3>
#define MPU_GYRO_FS_250DPS  0x00
#define MPU_GYRO_FS_500DPS  0x01
#define MPU_GYRO_FS_1000DPS 0x02
#define MPU_GYRO_FS_2000DPS 0x03

/*------------------------------------------------------------*/
//  Internal Data Structures
/*------------------------------------------------------------*/

// Contains all the relevant sensor info with packet headers/footers
// to facilitate with streaming sensor data over a serial interface
typedef struct {
  uint32_t sod;
  float accelData[3];
  float gyroData[3];
  float magData[3];
  uint32_t eod;
} MPUDataPacket_t;

/*------------------------------------------------------------*/
//  Other Constants
/*------------------------------------------------------------*/

#define MPU_DEFAULT_TIMEOUT  200

// Serial Start of Data and End of Data frame headers (arbitrarily chosen)
#define MPU_SERIAL_SOD (0xFFDEADFF)
#define MPU_SERIAL_EOD (0xEEDDAA77)

/*------------------------------------------------------------*/
//  Public Interface Functions
/*------------------------------------------------------------*/

// Interface Funcs
HAL_StatusTypeDef MPU_Init(void);
HAL_StatusTypeDef MPU_GetTemperature(float* tempBuf);
HAL_StatusTypeDef MPU_GetGyroscope(float *gx, float *gy, float *gz);
HAL_StatusTypeDef MPU_GetAccelerations(float *ax, float *ay, float *az);
HAL_StatusTypeDef MPU_SetAccelFSRange(uint8_t range);
HAL_StatusTypeDef MPU_SetGyroFSRange(uint8_t range);

// Data Transportation Funcs
void MPU_SampleAndSerialize(MPUDataPacket_t* packet);

// Debug Funcs
HAL_StatusTypeDef MPU_GetGyroscopeRaw(int16_t *gx, int16_t *gy, int16_t *gz);
HAL_StatusTypeDef MPU_GetAccelerationsRaw(int16_t *ax, int16_t *ay, int16_t *az);
HAL_StatusTypeDef MPU_Surprise(uint16_t* output);

// Utility Funcs, only for Low-Level purposes
HAL_StatusTypeDef MPU_ReadLen(uint8_t reg, uint8_t len, uint8_t *buf);
HAL_StatusTypeDef MPU_WriteLen(uint8_t reg, uint8_t len, uint8_t *buf);
HAL_StatusTypeDef MPU_ReadByte(uint8_t reg, uint8_t* res);
HAL_StatusTypeDef MPU_WriteByte(uint8_t reg, uint8_t data);
HAL_StatusTypeDef MPU_WriteBits(uint8_t reg, uint8_t bitsToWrite, uint8_t bitStart, uint8_t numBits);





/////////////////////////////////////////////////////////////////////////

#include "math.h"

/*------------------------------------------------------------*/
// Private Variables
/*------------------------------------------------------------*/

float accelFSScale;
float gyroFSScale;

#ifdef MPU_USE_SPI
  uint8_t spiTxBuf[32];
#endif

/**
 *  Initializes the accelerometer, and certain private variables
 *  pertaining to the functionality of this driver.
 *
 *  The parameters to MPU_SetAccelFSRange and MPU_SetGyroFSRange
 *  may change depending on the application, but both functions
 *  must be called at least once.
 */
HAL_StatusTypeDef MPU_Init(void) {
  HAL_StatusTypeDef status = 0;

  // Set the clock source to PLL with X Axis Gyroscope as reference
  status |= MPU_WriteBits(PWR_MGMT1_REG, 1, 2, 3);

  status |= MPU_SetAccelFSRange(MPU_ACCEL_FS_2G);
  status |= MPU_SetGyroFSRange(MPU_GYRO_FS_2000DPS);

  // Disable sleep mode
  status |= MPU_WriteBits(PWR_MGMT1_REG, 0, 6, 1);

  return (status == HAL_OK ? HAL_OK : HAL_ERROR);
}

/**
 *  Sets the full scale range of the accelerometer,
 *  and updates the scaling factor for acceleration values read
 *  from the sensor
 *
 *  fsRange:  The new full scale range of the accelerometer. Use the
 *            macros defined in mpu9255.h to ensure correct functionality
 */
HAL_StatusTypeDef MPU_SetAccelFSRange(uint8_t fsRange) {
  accelFSScale = pow(2, fsRange) / 16384;

  return MPU_WriteBits(ACCEL_CFG1_REG, fsRange, 4, 2);
}


/**
 *  Sets the full scale range of the gyroscope,
 *  and updates the scaling factor for gyroscope values read
 *  from the sensor
 *
 *  fsRange:  The new full scale range of the gyroscope. Use the
 *            macros defined in mpu9255.h to ensure correct functionality
 */
HAL_StatusTypeDef MPU_SetGyroFSRange(uint8_t fsRange) {
  gyroFSScale = pow(2, fsRange) / 232;

  return MPU_WriteBits(GYRO_CFG_REG, fsRange, 4, 2);
}

/**
 *  Reads the x, y, and z gyroscope values in burst-read mode, and scales them
 *  according to the full scale range of the gyroscope.
 *
 *  The scaling factor that is applied is calculated during the call to
 *  MPU_SetAccelFSRange(). Therefore no mater what FS range is selected,
 *  the values returned from this function will be very similar, but with
 *  varying measurement precision
 *
 *  ax:  memory location into which to place the x gyroscope value
 *  ay:  memory location into which to place the y gyroscope value
 *  az:  memory location into which to place the z gyroscope value
 */
HAL_StatusTypeDef MPU_GetGyroscope(float *gx, float *gy, float *gz) {
  HAL_StatusTypeDef status;
  int16_t xRaw, yRaw, zRaw;

  status = MPU_GetGyroscopeRaw(&xRaw, &yRaw, &zRaw);

  *gx = ((float) xRaw) * gyroFSScale;
  *gy = ((float) yRaw) * gyroFSScale;
  *gz = ((float) zRaw) * gyroFSScale;

  return status;
}

/**
 *  Reads the x, y, and z accelerometer values in burst-read mode, and scales them
 *  according to the full scale range of the accelerometer.
 *
 *  The scaling factor that is applied is calculated during the call to
 *  MPU_SetAccelFSRange(). Therefore no mater what FS range is selected,
 *  the values returned from this function will be very similar, but with
 *  varying measurement precision
 *
 *  ax:  memory location into which to place the x acceleration
 *  ay:  memory location into which to place the y acceleration
 *  az:  memory location into which to place the z acceleration
 */
HAL_StatusTypeDef MPU_GetAccelerations(float *ax, float *ay, float *az) {
  HAL_StatusTypeDef status;
  int16_t xRaw, yRaw, zRaw;

  status = MPU_GetAccelerationsRaw(&xRaw, &yRaw, &zRaw);

  *ax = ((float) xRaw) * accelFSScale;
  *ay = ((float) yRaw) * accelFSScale;
  *az = ((float) zRaw) * accelFSScale;

  return status;
}


/**
 *  Reads the x, y, and z gyroscope values in burst-read mode
 *  The datasheet guarantees that all readings will be from the same sampling interval
 *  during a burst-read.
 *
 *  The gyroscope values are the raw bit values, and need scaling to be interpreted
 *  properly. The scaling depends on the full-scale range that is set in the init function
 *  of this driver
 *
 *  ax:  memory location into which to place the x gyroscope value
 *  ay:  memory location into which to place the y gyroscope value
 *  az:  memory location into which to place the z gyroscope value
 */
HAL_StatusTypeDef MPU_GetGyroscopeRaw(int16_t *gx, int16_t *gy, int16_t *gz) {
  HAL_StatusTypeDef status;
  uint8_t buf[6];

  status = MPU_ReadLen(GYRO_XOUTH_REG, 6, buf);
  *gx = ((uint16_t) buf[0] << 8) | buf[1];
  *gy = ((uint16_t) buf[2] << 8) | buf[3];
  *gz = ((uint16_t) buf[4] << 8) | buf[5];

  return status;
}

/**
 *  Reads the x, y, and z accelerometer values in burst-read mode
 *  The datasheet guarantees that all readings will be from the same sampling interval
 *  during a burst-read.
 *
 *  The acceleration values are the raw bit values, and need scaling to be interpreted
 *  properly. The scaling depends on the full-scale range that is set in the init function
 *  of this driver
 *
 *  ax:  memory location into which to place the x acceleration
 *  ay:  memory location into which to place the y acceleration
 *  az:  memory location into which to place the z acceleration
 */
HAL_StatusTypeDef MPU_GetAccelerationsRaw(int16_t *ax, int16_t *ay, int16_t *az) {
  HAL_StatusTypeDef status;
  uint8_t buf[6];

  status = MPU_ReadLen(ACCEL_XOUTH_REG, 6, buf);
  *ax = ((uint16_t) buf[0] << 8) | buf[1];
  *ay = ((uint16_t) buf[2] << 8) | buf[3];
  *az = ((uint16_t) buf[4] << 8) | buf[5];

  return status;
}


/**
 * TODO THIS METHOD IS MALFUNCTIONAL
 *
 *  Reads the internal temperature of the sensor
 *
 *  temp:  pointer to a variable into which to store the sensor temperature
 */
HAL_StatusTypeDef MPU_GetTemperature(float* temp) {
  HAL_StatusTypeDef status;
  uint8_t buf[2];
  uint16_t raw;

  status = MPU_ReadLen(TEMP_OUTH_REG, 2, buf);
  raw = ((uint16_t) buf[0] << 8) | buf[1];
  *temp = 36.53 + ((float) raw) / 340;
  *temp *= 100;

  return status;
}

/**
 *  Samples the accelerometer, gyroscope, and magnetometer, and constructs places
 *  all the data in a single "serial packet" such that it can be transmited over any
 *  desired interface.
 *
 *  To use this function:
 *    1) Allocate space for an MPUDataPacket_t type either statically
 *       (preferred) or dynamically (avoid if possible). Then
 *    2) Call this function, passing in the adress of the allocated space
 *    3) Use the desired peripheral (UART, SPI, etc) to stream the entire packet in
 *       one shot, using sizeof(MPUDataPacket_t) as the number of bytes to stream.
 *       A typecast to a char* or uint8_t* might be necessary (see example below).
 *
 *  Example usage to stream sensor data over UART:
 *
 *    MPUDataPacket_t dataPacket;
 *    MPU_SampleAndSerialize(&dataPacket);
 *    UsartSend((char*) &dataPacket, sizeof(dataPacket), &huart2);
 *
 *  packet: pointer to an allocated memory location to place all the sensor data in
 */
void MPU_SampleAndSerialize(MPUDataPacket_t* packet) {

    // Add the start and end of data frame indicators
    packet->sod = MPU_SERIAL_SOD;
    packet->eod = MPU_SERIAL_EOD;

    // Sample the sensor and place the sampled data in the data packet
    MPU_GetAccelerations(&packet->accelData[0], &packet->accelData[1], &packet->accelData[2]);
    MPU_GetGyroscope(&packet->gyroData[0], &packet->gyroData[1], &packet->gyroData[2]);
   // MPU_GetAccelerations(&packet->magData[0], &packet->magData[1], &packet->magData[2]);
}

/**
 *  For lazy debug purposes only, to test singular register reads without implementing
 *  an entire extra method to handle functionality. Has undocumented behaviour
 */

HAL_StatusTypeDef MPU_Surprise(uint16_t* output) {
  HAL_StatusTypeDef status;
  uint8_t buf[2] = {0};

  status = MPU_ReadLen(PWR_MGMT1_REG, 1, buf);
  //*output = ((uint16_t) buf[0] << 8) | buf[1];
  *output = buf[0];

  return status;
}

/**
 *  Writes a bit pattern to a sub-section of a register. This is useful for writing
 *  to registers where a certain configuration option does not span the entire register length.
 *  Ex: To set the Accel Full Scale, only bits <4:3> of ACCEL_CFG1_REG need to be written.
 *
 *  This function works by first reading the value of the register, then super-imposing the bits
 *  to write onto the original register value, and finally writing the full register back to
 *  the device.
 *
 *  Clever bitwise logic is borrowed from the Open-Source I2CDevLib:
 *  https://www.i2cdevlib.com/
 *
 *  reg:          The register to write to
 *  bitsToWrite:  The relevant bits to write to the register sub-section
 *  bitStart:      The location of the 1st bit in the register sub-section (from 0 to 7)
 *  numBits:      The number of bits in this bit string (from 1 to 6)
 */
HAL_StatusTypeDef MPU_WriteBits(uint8_t reg, uint8_t bitsToWrite, uint8_t bitStart, uint8_t numBits) {
  uint8_t prevRegVal;
  HAL_StatusTypeDef status = MPU_ReadByte(reg, &prevRegVal);
  if (status != HAL_OK) {
    return status;
  }

  uint8_t mask = ((1 << numBits) - 1) << (bitStart - numBits + 1);
  bitsToWrite <<= (bitStart - numBits + 1); // shift data into correct position
  bitsToWrite &= mask; // zero all non-important bits in data
  prevRegVal &= ~(mask); // zero all important bits in existing byte
  prevRegVal |= bitsToWrite; // combine data with existing byte

  return MPU_WriteByte(reg, prevRegVal);
}


/**
 *  Writes a single byte to a device register
 *
 *  reg:  The device register to write to
 *  data:  The data to write to the register (note, not a pointer)
 */
HAL_StatusTypeDef MPU_WriteByte(uint8_t reg, uint8_t data) {
  return MPU_WriteLen(reg, 1, &data);
}

/**
 *  Reads a single byte from a device register
 *
 *  reg:  The device register to read from
 *  res:  The destination memory address into which to place the read result
 */
HAL_StatusTypeDef MPU_ReadByte(uint8_t reg, uint8_t* res) {
  return MPU_ReadLen(reg, 1, res);
}


/**
 *  Writes a certain number of bytes from a buffer into a given register
 *
 *  reg:  The register to write to
 *  len:  The number of bytes to write
 *  buf:  The buffer from which to read data (must be properly initialized)
 */
HAL_StatusTypeDef MPU_WriteLen(uint8_t reg, uint8_t len, uint8_t *buf) {
  HAL_StatusTypeDef status;
  #if defined(MPU_USE_I2C)
    status = HAL_I2C_Mem_Write(&MPU_I2C, MPU_ADDR_W, reg, I2C_MEMADD_SIZE_8BIT, buf, len, MPU_DEFAULT_TIMEOUT);
  #elif defined(MPU_USE_SPI)

    // The first byte of the transfer is the register address plus a write bit
    spiTxBuf[0] = SPI_ADDR_ADD_W_BIT(reg);

    // Initiate the SPI transfer
    HAL_GPIO_WritePin(Gyro_CS_GPIO_Port, Gyro_CS_Pin, GPIO_PIN_RESET);

    // Transmit the register address to read from. If not ok status, end
    // SPI transfer early and return
    status = HAL_SPI_Transmit(&MPU_SPI, spiTxBuf, 1, MPU_DEFAULT_TIMEOUT);
    if (status != HAL_OK) {
      HAL_GPIO_WritePin(Gyro_CS_GPIO_Port, Gyro_CS_Pin, GPIO_PIN_SET);
      return status;
    }

    // Write the actual data
    status = HAL_SPI_Transmit(&MPU_SPI, buf, len, MPU_DEFAULT_TIMEOUT);

    // Finalize the SPI transfer
    HAL_GPIO_WritePin(Gyro_CS_GPIO_Port, Gyro_CS_Pin, GPIO_PIN_SET);
  #endif
  return status;
}

/**
 *  Reads a certain number of bytes from a given register into a buffer
 *
 *  reg:  The register to read from
 *  len:  The number of bytes to read
 *  buf:  The buffer into which to place data (must be properly initialized)
 */
HAL_StatusTypeDef MPU_ReadLen(uint8_t reg, uint8_t len, uint8_t *buf) {
  HAL_StatusTypeDef status;
  #if defined(MPU_USE_I2C)
    status = HAL_I2C_Mem_Read(&MPU_I2C, MPU_ADDR_R, reg, I2C_MEMADD_SIZE_8BIT, buf, len, MPU_DEFAULT_TIMEOUT);
  #elif defined(MPU_USE_SPI)

    // The first byte of the transfer is the register address plus a read bit
    spiTxBuf[0] = SPI_ADDR_ADD_R_BIT(reg);

    // Initiate the SPI transfer
    HAL_GPIO_WritePin(Gyro_CS_GPIO_Port, Gyro_CS_Pin, GPIO_PIN_RESET);

    // Transmit the register address to read from. If not ok status, end
    // SPI transfer early and return
    status = HAL_SPI_Transmit(&MPU_SPI, spiTxBuf, 1, MPU_DEFAULT_TIMEOUT);
    if (status != HAL_OK) {
      HAL_GPIO_WritePin(Gyro_CS_GPIO_Port, Gyro_CS_Pin, GPIO_PIN_SET);
      return status;
    }

    // Read the actual data
    status = HAL_SPI_Receive(&MPU_SPI, buf, len, MPU_DEFAULT_TIMEOUT);

    // Finalize the SPI transfer
    HAL_GPIO_WritePin(Gyro_CS_GPIO_Port, Gyro_CS_Pin, GPIO_PIN_SET);
  #endif

  return status;
}


////////////////////////////////////////////////////////////////////////////////////////////



void testI2cMaster(void) {
	Led = newGPIO(&led_pin, LD2_GPIO_Port, LD2_Pin);
	serial_console = *newMainUSART(&huart2);

	MPU_Init();
	MPUDataPacket_t dataPacket;
	while (1) {
	    // Sample the accelerometer, gyroscope, and magnetometer data, and place it all
	    // in a single serial packet to transmit over UART
	    MPU_SampleAndSerialize(&dataPacket);

	    //HAL_UART_Transmit(&huart2, (uint8_t*) &dataPacket, sizeof(MPUDataPacket_t), serial_console.TxTimeOut);
	    //serial_console.TxBuffer = (char*) &dataPacket;
	    //usartSend(&serial_console);
	    //UsartSend((char*) &dataPacket, sizeof(MPUDataPacket_t), &huart2);
	    printf_u("\rGyro[%.6f %.6f %.6f]  Accel[%.6f %.6f %.6f]\r\n",
	    		dataPacket.gyroData[0], dataPacket.gyroData[1], dataPacket.gyroData[2],
	    		dataPacket.accelData[0], dataPacket.accelData[1], dataPacket.accelData[2]);
	    //printf_u("\rxxxxx = %f\r\n", 0.123456);
	    delay(100);
	}
}

void IT_CallBack_UsartTC(USART* Device) {
	turnOn(Led);
}
