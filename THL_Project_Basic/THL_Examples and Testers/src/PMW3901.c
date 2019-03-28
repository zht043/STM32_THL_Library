/*
 * PMW3901.c
 *
 *  Created on: Mar 2, 2019
 *      Author: zhang
 */

#include "PMW3901.h"


#define PMW3901_WriteReg(regAddr, byte) __PMW3901_WriteReg(instance, regAddr, byte)
#define PMW3901_ReadReg(regAddr) __PMW3901_ReadReg(instance, regAddr)


static void __PMW3901_WriteReg(PMW3901* instance, uint8_t regAddr, uint8_t byte) {
    spiBeginDevice(instance->CS);
    delay_us(50);
	spiReadWriteByte(instance->spiBus, SPI_WriteMode(regAddr));
	spiReadWriteByte(instance->spiBus, byte);
	delay_us(50);
    spiEndDevice(instance->CS);
    delay_us(200);
}

static uint8_t __PMW3901_ReadReg(PMW3901* instance, uint8_t regAddr) {
	spiBeginDevice(instance->CS);
	delay_us(50);
	spiReadWriteByte(instance->spiBus, SPI_ReadMode(regAddr));
	delay_us(60);
	uint8_t rtn = *spiReadWriteByte(instance->spiBus, SPI_Dummy_Byte);
	delay_us(50);
	spiEndDevice(instance->CS);
	delay_us(200);
	return rtn;
}

PMW3901* newPMW3901(PMW3901* instance, SPI* spiBus, GPIO* chip_select) {
	instance->spiBus = spiBus;
	instance->CS = chip_select;
	instance->Status = Ready;
	return instance;
}

uint8_t initPMW3901(PMW3901* instance) {
	//reset SPI bus
	delay(40);
	spiEndDevice(instance->CS);
	delay(2);
	spiBeginDevice(instance->CS);
	delay(2);
	spiEndDevice(instance->CS);

    // Power on reset
    PMW3901_WriteReg(0x3A, 0x5A);
    delay(5);

    // Test the SPI communication, checking chipId and inverse chipId
    instance->chip_id = PMW3901_ReadReg(0x00);
    instance->inverse_produc_id = PMW3901_ReadReg(0x5F);

    if (instance->chip_id != 0x49 && instance->inverse_produc_id != 0xB8) return False;

     // Reading the motion registers one time
    PMW3901_ReadReg(0x02);
    PMW3901_ReadReg(0x03);
    PMW3901_ReadReg(0x04);
    PMW3901_ReadReg(0x05);
    PMW3901_ReadReg(0x06);
    delay(1);

    // Initialize registers
    PMW3901_WriteReg(0x7F, 0x00);
    PMW3901_WriteReg(0x61, 0xAD);
    PMW3901_WriteReg(0x7F, 0x03);
    PMW3901_WriteReg(0x40, 0x00);
    PMW3901_WriteReg(0x7F, 0x05);
    PMW3901_WriteReg(0x41, 0xB3);
    PMW3901_WriteReg(0x43, 0xF1);
    PMW3901_WriteReg(0x45, 0x14);
    PMW3901_WriteReg(0x5B, 0x32);
    PMW3901_WriteReg(0x5F, 0x34);
    PMW3901_WriteReg(0x7B, 0x08);
    PMW3901_WriteReg(0x7F, 0x06);
    PMW3901_WriteReg(0x44, 0x1B);
    PMW3901_WriteReg(0x40, 0xBF);
    PMW3901_WriteReg(0x4E, 0x3F);
    PMW3901_WriteReg(0x7F, 0x08);
    PMW3901_WriteReg(0x65, 0x20);
    PMW3901_WriteReg(0x6A, 0x18);
    PMW3901_WriteReg(0x7F, 0x09);
    PMW3901_WriteReg(0x4F, 0xAF);
    PMW3901_WriteReg(0x5F, 0x40);
    PMW3901_WriteReg(0x48, 0x80);
    PMW3901_WriteReg(0x49, 0x80);
    PMW3901_WriteReg(0x57, 0x77);
    PMW3901_WriteReg(0x60, 0x78);
    PMW3901_WriteReg(0x61, 0x78);
    PMW3901_WriteReg(0x62, 0x08);
    PMW3901_WriteReg(0x63, 0x50);
    PMW3901_WriteReg(0x7F, 0x0A);
    PMW3901_WriteReg(0x45, 0x60);
    PMW3901_WriteReg(0x7F, 0x00);
    PMW3901_WriteReg(0x4D, 0x11);
    PMW3901_WriteReg(0x55, 0x80);
    PMW3901_WriteReg(0x74, 0x1F);
    PMW3901_WriteReg(0x75, 0x1F);
    PMW3901_WriteReg(0x4A, 0x78);
    PMW3901_WriteReg(0x4B, 0x78);
    PMW3901_WriteReg(0x44, 0x08);
    PMW3901_WriteReg(0x45, 0x50);
    PMW3901_WriteReg(0x64, 0xFF);
    PMW3901_WriteReg(0x65, 0x1F);
    PMW3901_WriteReg(0x7F, 0x14);
    PMW3901_WriteReg(0x65, 0x60);
    PMW3901_WriteReg(0x66, 0x08);
    PMW3901_WriteReg(0x63, 0x78);
    PMW3901_WriteReg(0x7F, 0x15);
    PMW3901_WriteReg(0x48, 0x58);
    PMW3901_WriteReg(0x7F, 0x07);
    PMW3901_WriteReg(0x41, 0x0D);
    PMW3901_WriteReg(0x43, 0x14);
    PMW3901_WriteReg(0x4B, 0x0E);
    PMW3901_WriteReg(0x45, 0x0F);
    PMW3901_WriteReg(0x44, 0x42);
    PMW3901_WriteReg(0x4C, 0x80);
    PMW3901_WriteReg(0x7F, 0x10);
    PMW3901_WriteReg(0x5B, 0x02);
    PMW3901_WriteReg(0x7F, 0x07);
    PMW3901_WriteReg(0x40, 0x41);
    PMW3901_WriteReg(0x70, 0x00);

    delay(100);
    PMW3901_WriteReg(0x32, 0x44);
    PMW3901_WriteReg(0x7F, 0x07);
    PMW3901_WriteReg(0x40, 0x40);
    PMW3901_WriteReg(0x7F, 0x06);
    PMW3901_WriteReg(0x62, 0xf0);
    PMW3901_WriteReg(0x63, 0x00);
    PMW3901_WriteReg(0x7F, 0x0D);
    PMW3901_WriteReg(0x48, 0xC0);
    PMW3901_WriteReg(0x6F, 0xd5);
    PMW3901_WriteReg(0x7F, 0x00);
    PMW3901_WriteReg(0x5B, 0xa0);
    PMW3901_WriteReg(0x4E, 0xA8);
    PMW3901_WriteReg(0x5A, 0x50);
    PMW3901_WriteReg(0x40, 0x80);

    return True;
}

PMW3901* getMotion(PMW3901* instance) {
    spiBeginDevice(instance->CS);

	PMW3901_ReadReg(0x02);
    instance->delta_X_Cnt = (int16_t)PMW3901_ReadReg(0x04) << 8 | PMW3901_ReadReg(0x03);
    instance->delta_Y_Cnt = (int16_t)PMW3901_ReadReg(0x06) << 8 | PMW3901_ReadReg(0x05);

    spiEndDevice(instance->CS);
    return instance;
}

