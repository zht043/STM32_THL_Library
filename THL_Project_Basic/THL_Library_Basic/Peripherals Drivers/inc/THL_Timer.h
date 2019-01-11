#ifndef __THL_TIMER_H
#define __THL_TIMER_H
#include "THL_Utility.h"
#include "THL_Portability.h"





typedef struct{
	TIM_HandleTypeDef *htim;
	uint32_t frequency;

}TIM;


void setTimerPrescalar(TIM_HandleTypeDef* htimx, uint32_t prescalar_val);
void setPwmFrequency(TIM_HandleTypeDef* htimx, uint32_t frequency);
void startPwmGeneration(TIM_HandleTypeDef* htimx, uint32_t channel, uint32_t period);
void setPwmDutyCircle(TIM_HandleTypeDef* htimx, uint32_t channel, uint32_t dutyCircle);
void pwm(TIM_HandleTypeDef* htimx, uint32_t channel, double dutyCirclePercent);

volatile int *startInputCapture(TIM_HandleTypeDef* htimx, uint32_t channel);
int getTimCapturedVal(TIM_HandleTypeDef* htimx, uint32_t channel);
void MeasurePulseWidth(TIM_HandleTypeDef* htimx, uint32_t channel, uint32_t MaxDelay_us);

volatile int *startInputCapture_IT(TIM_HandleTypeDef* htimx, uint32_t channel);
void stopInputCapture_IT(TIM_HandleTypeDef* htimx, uint32_t channel);
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htimx);

int getInputCapturePulseWidth(TIM_HandleTypeDef* htimx, uint32_t channel);
int TIM_ICval(TIM_HandleTypeDef* htimx, uint32_t channel);
#endif
