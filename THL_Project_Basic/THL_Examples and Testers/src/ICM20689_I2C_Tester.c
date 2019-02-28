/*
                       _oo0oo_
                      o8888888o
                      88" . "88
                      (| -_- |)
                      0\  =  /0
                    ___/`---'\___
                  .' \\|     |// '.
                 / \\|||  :  |||// \
                / _||||| -:- |||||- \
               |   | \\\  -  /// |   |
               | \_|  ''\---/''  |_/ |
               \  .-\__  '-'  ___/-. /
             ___'. .'  /--.--\  `. .'___
          ."" '<  `.___\_<|>_/___.' >' "".
         | | :  `- \`.;`\ _ /`;.`/ - ` : | |
         \  \ `_.   \_ __\ /__ _/   .-` /  /
     =====`-.____`.___ \_____/___.-`___.-'=====
                       `=---='
*/
#include "ICM20689_I2C_Tester.h"

#define ICM_ADDR 0x68    // AD0 should be low

#define SELF_TEST_X_GYRO_REG    0x00
#define SELF_TEST_Y_GYRO_RE     0x01
#define SELF_TEST_Z_GYRO_REG    0x02

#define SELF_TEST_X_ACCEL_REG   0x0D
#define SELF_TEST_Y_ACCEL_REG   0x0E
#define SELF_TEST_Z_ACCEL_REG   0x0F

#define SMPLRT_REG        0x19
#define CFG_REG           0x1A
#define GYRO_CFG_REG      0x1B
#define ACCEL_CFG1_REG    0x1C
#define ACCEL_CFG2_REG    0x1D

#define FIFO_EN_REG       0x23
#define FSYNC_INT_REG     0x36

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

GPIO led_pin;
GPIO* Led;

extern UART_HandleTypeDef huart2;
USART serial_console_mem;
USART* serial_console;

extern I2C_HandleTypeDef hi2c1;
I2C ICM20689_mem;
I2C* ICM20689;

#define ICM20689_WriteReg(RegAdd, Byte) i2cWriteReg(ICM20689, ICM_ADDR, (RegAdd), (Byte))
#define ICM20689_ReadReg(RegAdd) *i2cReadReg(ICM20689, ICM_ADDR, (RegAdd))

// Init ICM20689
uint8_t initICM20689(void) {
    ICM20689 = newI2C(&ICM20689_mem, &hi2c1);

	if(ICM20689_ReadReg(WHO_AM_I_REG) != 0x98) {
		return Failed;
	}

    ICM20689_WriteReg(PWR_MGMT1_REG, 0X80); // Reset
	delay(100);

    if((ICM20689_ReadReg(PWR_MGMT1_REG) >> 7) != 0) {
        return Failed; // Fail to reset in 100 ms
    } else {
    	ICM20689_WriteReg(PWR_MGMT1_REG, 0X00);
    }

    ICM20689_WriteReg(GYRO_CFG_REG, 3<<3);    // Gyro Full Scale -> ±2000dps
    ICM20689_WriteReg(ACCEL_CFG1_REG, 0<<3);  // Accel Full Scale -> ±2g
	ICM20689_WriteReg(ACCEL_CFG2_REG, 0x07);  // Disable DLPF, sample rate = 1k

	ICM20689_WriteReg(SMPLRT_REG, 0x00);
	//ICM20689_WriteReg(CFG_REG, 0x07); // Disable DLPF, sample rate = 8k
	ICM20689_WriteReg(CFG_REG, 0x07);

	ICM20689_WriteReg(INT_EN_REG, 0x00);	// Disable Interrupt
	ICM20689_WriteReg(FIFO_EN_REG, 0x00);  // Disable FIFO

	ICM20689_WriteReg(PWR_MGMT1_REG, 0x01);	  // Set up x-axis PLL if ready
	ICM20689_WriteReg(PWR_MGMT2_REG, 0x00);   // Turn on gyro and acce

	return Succeeded;
}






void getAccel(int16_t *ax, int16_t *ay, int16_t *az)
{
    uint8_t buf[6];

    buf[0] = ICM20689_ReadReg(ACCEL_XOUTH_REG);
    buf[1] = ICM20689_ReadReg(ACCEL_XOUTL_REG);
    buf[2] = ICM20689_ReadReg(ACCEL_YOUTH_REG);
    buf[3] = ICM20689_ReadReg(ACCEL_YOUTL_REG);
    buf[4] = ICM20689_ReadReg(ACCEL_ZOUTH_REG);
    buf[5] = ICM20689_ReadReg(ACCEL_ZOUTL_REG);

	*ax = (int16_t)(buf[0] << 8) | buf[1];
	*ay = (int16_t)(buf[2] << 8) | buf[3];
	*az = (int16_t)(buf[4] << 8) | buf[5];
}





uint8_t getWhoAmI(void) {
	return ICM20689_ReadReg(WHO_AM_I_REG);
}

void testICM20689(void) {
	newMainUSART(&huart2);

	uint8_t result = initICM20689();
	printf_u("\r result = %d\r\n", result);
	printf_u("\r WhoAmI = %#04x\r\n", ICM20689_ReadReg(CFG_REG));

    int16_t ax, ay, az;

    while(1) {
        getAccel(&ax, &ay, &az);
        printf_u("\r[%d]   [%d]   [%d]\r\n", ax, ay, az);
        delay(300);
    }
}
