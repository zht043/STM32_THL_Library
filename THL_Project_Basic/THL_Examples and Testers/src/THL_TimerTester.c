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
TIM_IC timer8_IC_fieldsMem;


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
	timer7 = newTIM(&timer7Mem, &htim7, 2, TIM_16bit);
	initTIM_BasicCounting(timer7, 10000, 100000);
	printf_u("\r%d\r\n", timer7->ActualFreq);
	timCountBegin(timer7);
	uint32_t t0 = millis();
	while(1) {
		printf_u("\r millis = [%d]     timCNT = [%d]\r\n", millis() - t0, (timGetCount(timer7)+1) / 100);
	}
}

static void testTIMdelay(void) {
#ifdef	SysTick_Reserved
	initSysTime_TIM(&htim7, 2, TIM_16bit);
#endif
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
	timer1 = newTIM(&timer1Mem, &htim1, 1, TIM_16bit); //TIM1 belongs to APB2, HCLK/APB2 = 1
	initTIM_PWM_Out(timer1, 10000, 10000); //max_cnt = 10,000; pwm_freq = 10k;
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

volatile int32_t ICval[2] = {0};
volatile int32_t PulseWidth[2] = {0};

static void testIC(void) {
	timer1 = newTIM(&timer1Mem, &htim1, 1, TIM_16bit); //TIM1 belongs to APB2, HCLK/APB2 = 1
	initTIM_PWM_Out(timer1, 100, 1000); //max_cnt = 10,000; pwm_freq = 10k;
	timPwmGenBegin(timer1, TIM_CH1);
	timPwmGenBegin(timer1, TIM_CH2);

	timPwmWrite(timer1, TIM_CH1, 39.00);
	timPwmWrite(timer1, TIM_CH2, 61.00);

	//Wire TIM1 CH1 pin to TIM8 CH2 pin,
	//     TIM1 CH2 pin to TIM8 CH3 pin for experimenting

	timer8 = newTIM(&timer8Mem, &htim8, 1, TIM_16bit); //TIM8 belongs to APB2, HCLK/APB2 = 1
	initTIM_IC(timer8, &timer8_IC_fieldsMem, 100000);
	timSetIC_Polarity(timer8, TIM_CH2, TIM_IC_RisingEdge);
	timSetIC_Polarity(timer8, TIM_CH3, TIM_IC_RisingEdge);
	timIcBegin_IT(timer8, TIM_CH2);
	timIcBegin_IT(timer8, TIM_CH3);

	while(1) {
		printf_u("\rPulse 1 = [%d]   Pulse 2 = [%d]\r\n", PulseWidth[0], PulseWidth[1]);
	}
}

void timIC_IT_CallBack(TIM* instance, HAL_TIM_ActiveChannel active_channel) {
	if(instance == timer8) {
		if(active_channel == TIM_Active_CH2) {
			if(instance->IC_fields->ICpolarity[tim_channel_index(TIM_CH2)] == TIM_IC_RisingEdge) {
				ICval[0] = timGetCapVal(timer8, TIM_CH2);

				timSetIC_Polarity(timer8, TIM_CH2, TIM_IC_FallingEdge);
			}
			else if(instance->IC_fields->ICpolarity[tim_channel_index(TIM_CH2)] == TIM_IC_FallingEdge) {
				PulseWidth[0] = timGetCapVal(timer8, TIM_CH2) - ICval[0];
				if(PulseWidth[0] < 0) {
					PulseWidth[0] += ICval[0] + instance->ARR+1 - ICval[0];
				}

				timSetIC_Polarity(timer8, TIM_CH2, TIM_IC_RisingEdge);
			}
		}
		if(active_channel == TIM_Active_CH3) {
			if(instance->IC_fields->ICpolarity[tim_channel_index(TIM_CH3)] == TIM_IC_RisingEdge) {
				ICval[1] = timGetCapVal(timer8, TIM_CH3);

				timSetIC_Polarity(timer8, TIM_CH3, TIM_IC_FallingEdge);
			}
			else if(instance->IC_fields->ICpolarity[tim_channel_index(TIM_CH3)] == TIM_IC_FallingEdge) {
				PulseWidth[1] = timGetCapVal(timer8, TIM_CH3) - ICval[1];
				if(PulseWidth[1] < 0) {
					PulseWidth[1] += ICval[1] + instance->ARR+1 - ICval[1];
				}

				timSetIC_Polarity(timer8, TIM_CH3, TIM_IC_RisingEdge);
			}
		}
	}
}


static void testPWM_Input(void) {
	timer1 = newTIM(&timer1Mem, &htim1, 1, TIM_16bit); //TIM1 belongs to APB2, HCLK/APB2 = 1
	initTIM_PWM_Out(timer1, 100, 1000); //max_cnt = 10,000; pwm_freq = 10k;
	timPwmGenBegin(timer1, TIM_CH1);
	timPwmGenBegin(timer1, TIM_CH2);

	timPwmWrite(timer1, TIM_CH1, 43.00);
	timPwmWrite(timer1, TIM_CH2, 61.00);

	//Wire TIM1 CH1 pin to TIM8 CH2 pin,
	//     TIM1 CH2 pin to TIM8 CH3 pin for experimenting

	timer8 = newTIM(&timer8Mem, &htim8, 1, TIM_16bit); //TIM8 belongs to APB2, HCLK/APB2 = 1

	initTIM_PWM_In(timer8, &timer8_IC_fieldsMem, 100, 1000); //match max_cnt & pwm_freq with timer1's
															 //for proper interpretation upon capturing

	timPwmIcBegin(timer8, TIM_CH2, TIM_PulseOnHigh);
	timPwmIcBegin(timer8, TIM_CH3, TIM_PulseOnHigh);


	while(1) {
		printf_u("\rPulse 1 = [%d]   Pulse 2 = [%d]\r\n",
				timGetPulseWidth(timer8, TIM_CH2), timGetPulseWidth(timer8, TIM_CH3));
	}
}


void testTimer(void) {
	setup();

	//testPWM();
	UNUSED(testPWM);

	//testCount();
	UNUSED(testCount);

	//testTIMdelay();
	UNUSED(testTIMdelay);

	//testIC();
	UNUSED(testIC);

	testPWM_Input();
	//UNUSED(testPWM_Input);
}


