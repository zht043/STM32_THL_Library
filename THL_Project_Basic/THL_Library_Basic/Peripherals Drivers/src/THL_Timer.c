#include "THL_Timer.h"
#include "THL_SysTick.h"


#ifdef fixthemlater
// internal helper function

uint8_t chDec(uint32_t channel_macro) {
	if(channel_macro == TIM_CHANNEL_1) return 1;
	if(channel_macro == TIM_CHANNEL_2) return 2;
	if(channel_macro == TIM_CHANNEL_3) return 3;
	if(channel_macro == TIM_CHANNEL_4) return 4;
	return 0;
}
uint32_t chEnc(uint8_t channel_num) {
	if(channel_num == 1) return TIM_CHANNEL_1;
	if(channel_num == 2) return TIM_CHANNEL_2;	
	if(channel_num == 3) return TIM_CHANNEL_3;
	if(channel_num == 4) return TIM_CHANNEL_4;
	return 0;

}

/*========================SETUP AND DATA STRUCTURE========================*/
#define Max_Num_TIMs 20
uint16_t numActiveTIMs = 0;
TIM* ActiveTIMs[Max_Num_TIMs];

TIM *newTIM_PWM(TIM* obj, TIM_HandleTypeDef *htim, uint32_t frequency, uint32_t resolustion_cnt) {
	obj->htim = htim;

	for(int i = 0; i < numActiveTIMs; i++)
		if(ActiveTIMs[i]->htim == htim) {
			ActiveTIMs[i] = obj;
			return obj;
		}
	ActiveTIMs[numActiveTIMs++] = obj;
	return obj;
}
void resetTIM_Frequency() {

}
void resetTIM_CounterPeriod() {

}


/*=========================================================================*/


/**********************************PWM************************************/

#define CounterPeriod Init.Period
#define NumChannel 4
#define INACTIVE 0
#define ACTIVE 1

void setPwmFrequency(TIM_HandleTypeDef* htimx, uint32_t frequency) { //unit???
	uint32_t prescalar_val; 
	// PrescalerValue = ((HCLK/(APB1 or APB2 division factor))/Perioid/Frequency) - 1;
	/* Refer the "Clock Three" and datasheets for specific APBx_Div, which is the same factor to get PCLK1 or PCLK2
	   different timers might connect to different APB bus with different frequencies*/	
	prescalar_val = ( (HAL_RCC_GetHCLKFreq()/APBx_Div(htimx)) / htimx->CounterPeriod / frequency) - 1;
	setTimerPrescalar(htimx, prescalar_val);
}
void setTimerPrescalar(TIM_HandleTypeDef* htimx, uint32_t prescalar_val) {
	__HAL_TIM_SET_PRESCALER(htimx, prescalar_val);
	//htimx->Instance->PSC = prescalar_val;  //the same as the above line
}


/*Init & start pwm with setting up its period*/
void startPwmGeneration(TIM_HandleTypeDef* htimx, uint32_t channel, uint32_t period) {
	htimx->Init.Period = period;
	if (HAL_TIM_PWM_Init(htimx) != HAL_OK)
	{
		Error_Handler2();
	}
	HAL_TIM_PWM_Start(htimx, channel);
}
void setPwmDutyCircle(TIM_HandleTypeDef* htimx, uint32_t channel, uint32_t dutyCircleCnt) {
	//equivalent to htimx->instance->CCRx = dutyCircle
	__HAL_TIM_SET_COMPARE(htimx, channel, dutyCircle);
}

void pwm(TIM_HandleTypeDef* htimx, uint32_t channel, double dutyCirclePercent) {
	uint32_t cnt = (int)(dutyCirclePercent * (double)htimx->CounterPeriod);
	setPwmDutyCircle(htimx, channel, cnt);
}






#ifdef xxx
/***************************Input Capture*********************************/
volatile int PulseWidth[NumTimers + 1][NumChannel + 1] = {0};
volatile int PulseWidthtmp[NumTimers + 1][NumChannel + 1] = {0};

/*******Input Capture Blocking Mode**********/
volatile int *startInputCapture(TIM_HandleTypeDef* htimx, uint32_t channel) {
	if(HAL_TIM_IC_Start(htimx, channel) != HAL_OK) Error_Handler2();
	return PulseWidth[TIMx(htimx)];
}
int getTimCapturedVal(TIM_HandleTypeDef* htimx, uint32_t channel) {
	return (int)HAL_TIM_ReadCapturedValue(htimx, channel);
}
void MeasurePulseWidth(TIM_HandleTypeDef* htimx, uint32_t channel, uint32_t MaxDelay_us) {
	uint32_t t0 = micros(); 
	int tmp_IC_CapVal = getTimCapturedVal(htimx, channel);
	while(getTimCapturedVal(htimx, channel) == tmp_IC_CapVal) if(micros() - t0 > MaxDelay_us) return;
	tmp_IC_CapVal = getTimCapturedVal(htimx, channel);
	while(getTimCapturedVal(htimx, channel) == tmp_IC_CapVal) if(micros() - t0 > MaxDelay_us) return;
  PulseWidth[TIMx(htimx)][chDec(channel)] = getTimCapturedVal(htimx, channel) - tmp_IC_CapVal;
}
/********************************************/



/*******Input Capture Interrupt Mode*********/
volatile uint8_t TIM_IC_IT_index[NumTimers + 1][NumChannel + 1] = {0};
volatile uint8_t TIM_IC_IT_Channel[NumTimers + 1][NumChannel + 1] = {INACTIVE};
volatile int *startInputCapture_IT(TIM_HandleTypeDef* htimx, uint32_t channel) {
	if(HAL_TIM_IC_Start_IT(htimx, channel) != HAL_OK) Error_Handler2();
	TIM_IC_IT_index[TIMx(htimx)][chDec(channel)] = 0;
	TIM_IC_IT_Channel[TIMx(htimx)][chDec(channel)] = ACTIVE;
	return PulseWidth[TIMx(htimx)];
}
void stopInputCapture_IT(TIM_HandleTypeDef* htimx, uint32_t channel) {
	HAL_TIM_IC_Stop_IT(htimx, channel);
	TIM_IC_IT_Channel[TIMx(htimx)][chDec(channel)] = INACTIVE;
}
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htimx)
{
	for(int channel = 1; channel <= NumChannel; channel++) {
		if(TIM_IC_IT_Channel[TIMx(htimx)][channel] == INACTIVE) continue;
		if(TIM_IC_IT_index[TIMx(htimx)][channel] == 0) {
			PulseWidthtmp[TIMx(htimx)][channel] = (int)HAL_TIM_ReadCapturedValue(htimx, chEnc(channel));
			TIM_IC_IT_index[TIMx(htimx)][channel] = 1;
		}
		else if(TIM_IC_IT_index[TIMx(htimx)][channel] == 1) {
			PulseWidth[TIMx(htimx)][channel] = 
				(int)HAL_TIM_ReadCapturedValue(htimx, chEnc(channel)) - PulseWidthtmp[TIMx(htimx)][channel];
			TIM_IC_IT_index[TIMx(htimx)][channel] = 0;
		}			
	}
}
/**********************************************/
int getInputCapturePulseWidth(TIM_HandleTypeDef* htimx, uint32_t channel) {
	return PulseWidth[TIMx(htimx)][chDec(channel)];
}
int TIM_ICval(TIM_HandleTypeDef* htimx, uint32_t channel) {
	return PulseWidth[TIMx(htimx)][chDec(channel)];
}
uint8_t isChannelActive(TIM_HandleTypeDef* htimx, uint32_t channel) {
	return TIM_IC_IT_Channel[TIMx(htimx)][channel];
}
/*************************************************************************/

#endif
#endif



