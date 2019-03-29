/*
 * THL_TimerTester.c
 *
 *  Created on: Mar 26, 2019
 *      Author: zhang
 */

#include "THL_TimerTester.h"
#include "main.h"


extern UART_HandleTypeDef huart2;
extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim5;
extern TIM_HandleTypeDef htim7;
extern TIM_HandleTypeDef htim8;



TIM timer1Mem;
TIM* timer1;

TIM timer5Mem;
TIM* timer5;

TIM timer7Mem;
TIM* timer7;

TIM timer8Mem;
TIM* timer8;


USART* system_console;

GPIO ledMem;
GPIO* led;
GPIO buttonMem;
GPIO* button;
GPIO PB6Mem;
GPIO* PB6;


static void setup(void) {
	system_console = newMainUSART(&huart2);
	led = newGPIO(&ledMem, LD2_GPIO_Port, LD2_Pin);
	PB6 = newGPIO(&PB6Mem, GPIOB, GPIO_PIN_6);
	button = newGPIO(&buttonMem, B1_GPIO_Port, B1_Pin);
	printf_u("\rTimer Testing\r\n");
}


static void testCount(void) {
	timer7 = newTIM(&timer7Mem, &htim7, 2);
	uint32_t ActualFreq = initTIM_BasicCounting(timer7, 10000, 100000);
	printf_u("\r%d\r\n", ActualFreq);
	timCountBegin(timer7);
	uint32_t t0 = millis();
	while(1) {
		printf_u("\r millis = [%d]     timCNT = [%d]\r\n", millis() - t0, timGetCount(timer7) / 100);
	}
}

static void testTIMdelay(void) {
	initSysTime_TIM(&htim7, 2);

	while(1) {
		gpioWrite(PB6, High);

		//delay_us(10);
		delay(10);

		gpioWrite(PB6, Low);

		//delay_us(10);
		delay(10);
	}
}

static void testPWM(void) {
	timer1 = newTIM(&timer1Mem, &htim1, 1); //TIM1 belongs to APB2, HCLK/APB2 = 1
	initTIM_PWM(timer1, 10000, 10000); //max_cnt = 10,000; pwm_freq = 10k;
	timPwmGenBegin(timer1, TIM_CH1);
	timPwmGenBegin(timer1, TIM_CH2);
	timPwmGenBegin(timer1, TIM_CH3);
	timPwmGenBegin(timer1, TIM_CH4);

	gpioWrite(led, High);
	while(gpioRead(button));
	gpioWrite(led, Low);
	delay(500);
	gpioWrite(led, High);

	double dutyCircle = 10.00f, increm = 5.25f, gap = 10.00f, iter = 0.00f;
	double dc1, dc2, dc3, dc4;
	while(1) {

		if(!gpioRead(button)) {

			//Altering freq real-time
			if((uint32_t)iter % 2 == 1) timSetPwmFrequency(timer1, 10000, 10000);
			else timSetPwmFrequency(timer1, 10000, 100);

			dc1 = dutyCircle + 0.00f * gap + iter * increm;
			dc2 = dutyCircle + 1.00f * gap + iter * increm;
			dc3 = dutyCircle + 2.00f * gap + iter * increm;
			dc4 = dutyCircle + 3.00f * gap + iter * increm;
			if(dc1 > 100.00f) dc1 = dc1 - ((uint32_t)(dc1 / 100.00f) * 100.00f);
			if(dc2 > 100.00f) dc2 = dc2 - ((uint32_t)(dc2 / 100.00f) * 100.00f);
			if(dc3 > 100.00f) dc3 = dc3 - ((uint32_t)(dc3 / 100.00f) * 100.00f);
			if(dc4 > 100.00f) dc4 = dc4 - ((uint32_t)(dc4 / 100.00f) * 100.00f);


			timPwmWrite(timer1, TIM_CH1, dc1);
			timPwmWrite(timer1, TIM_CH2, dc2);
			timPwmWrite(timer1, TIM_CH3, dc3);
			timPwmWrite(timer1, TIM_CH4, dc4);

			iter += 1.00f;
			gpioWrite(led, Low);
			delay(1000);
			gpioWrite(led, High);
		}

/*
		timPwmWrite(timer1, TIM_CH1, 10.25);
		timPwmWrite(timer1, TIM_CH2, 30.59);
		timPwmWrite(timer1, TIM_CH3, 50.95);
		timPwmWrite(timer1, TIM_CH4, 75.33);*/
	}
}

void testTimer(void) {
	setup();

	//testPWM();
	UNUSED(testPWM);

	//testCount();
	UNUSED(testCount);

	testTIMdelay();
	//UNUSED(testTIMdelay);

}


