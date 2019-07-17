/*P
 * ESC_Tester.c
 *
 *  Created on: Apr 7, 2019
 *      Author: zhang
 */
#include "ESC_Tester.h"

extern TIM_HandleTypeDef htim1;

TIM timer1_mem;
TIM* timer1;


#define esc1 timer1, TIM_CH1
#define esc2 timer1, TIM_CH2



void setEscSpeed(TIM* esc, uint32_t channel, double percent) {
	percent = percent * 0.100f;
	timPwmWrite(esc, channel, percent);
}

void testEsc(void) {
	timer1 = newTIM(&timer1_mem, &htim1, 1, TIM_16bit);
	initTIM_PWM_Out(timer1, 10000, 50); //max_cnt = 10,000; pwm_freq = 500hz;
	timPwmGenBegin(timer1, TIM_CH1);
	timPwmGenBegin(timer1, TIM_CH2);
	timPwmGenBegin(timer1, TIM_CH3);
	timPwmGenBegin(timer1, TIM_CH4);

	while(1) {
		setEscSpeed(esc1, 20.00f);
	}

}

