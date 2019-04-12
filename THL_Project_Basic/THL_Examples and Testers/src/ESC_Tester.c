/*P
 * ESC_Tester.c
 *
 *  Created on: Apr 7, 2019
 *      Author: zhang
 */
#include "ESC_Tester.h"

extern TIM_HandleTypeDef htim1;

TIM esc1_mem;
TIM* esc1;



void setEscSpeed(TIM* esc, uint32_t channel, double percent) {
	percent = percent * 0.100f;
	timPwmWrite(esc, channel, percent);
}

void testEsc(void) {
	esc1 = newTIM(&esc1_mem, &htim1, 1, TIM_16bit);
	initTIM_PWM_Out(esc1, 10000, 50); //max_cnt = 10,000; pwm_freq = 500hz;
	timPwmGenBegin(esc1, TIM_CH1);
	timPwmGenBegin(esc1, TIM_CH2);
	timPwmGenBegin(esc1, TIM_CH3);
	timPwmGenBegin(esc1, TIM_CH4);

	while(1) {
		setEscSpeed(esc1, TIM_CH1, 20.00f);
	}

}

