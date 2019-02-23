#ifndef __THL_TIMER_H
#define __THL_TIMER_H
#include "THL_Utility.h"
#include "THL_Portability.h"


#define TIM_Channel_1 TIM_CHANNEL_1
#define TIM_Channel_2 TIM_CHANNEL_2
#define TIM_Channel_3 TIM_CHANNEL_3
#define TIM_Channel_4 TIM_CHANNEL_4



typedef struct{
	TIM_HandleTypeDef *htim;
	uint32_t TimerPrescaler;
	uint32_t max_count; //ARR: (Counter) Auto Reload Register value, a.k.a timer period
						//ARR val = max_count = *htim->init->Period
}TIM;



/*=======================Universal Functions=================================*/
TIM *newTIM(TIM* instance, TIM_HandleTypeDef *htim);
/*===========================================================================*/



/*==============================PWM Generation===============================*/
TIM *newTIM_PWM(TIM* instance, TIM_HandleTypeDef *htim, uint32_t max_count, uint32_t pwm_frequency);
void timSetPwmFrequency(TIM* instance, uint32_t max_count, uint32_t pwm_frequency);
void timPwmGenBegin(TIM* instance, uint32_t channel);
void timSetPwmDutyCircle(TIM* instance, uint32_t channel, uint32_t dutyCircleCnt);
void timPwmWrite(TIM* instance, uint32_t channel, double dutyCirclePercent);
/*===========================================================================*/


#endif
