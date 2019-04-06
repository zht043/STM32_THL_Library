#ifndef __THL_TIMER_H
#define __THL_TIMER_H
#include "THL_Utility.h"
#include "THL_Portability.h"



#ifdef HAL_TIM_MODULE_ENABLED


#define TIM_CH1 TIM_CHANNEL_1
#define TIM_CH2 TIM_CHANNEL_2
#define TIM_CH3 TIM_CHANNEL_3
#define TIM_CH4 TIM_CHANNEL_4

#define TIM_Active_CH1 HAL_TIM_ACTIVE_CHANNEL_1
#define TIM_Active_CH2 HAL_TIM_ACTIVE_CHANNEL_2
#define TIM_Active_CH3 HAL_TIM_ACTIVE_CHANNEL_3
#define TIM_Active_CH4 HAL_TIM_ACTIVE_CHANNEL_4

#define TIM_IC_RisingEdge  TIM_INPUTCHANNELPOLARITY_RISING
#define TIM_IC_FallingEdge TIM_INPUTCHANNELPOLARITY_FALLING
#define TIM_IC_BothEdge    TIM_INPUTCHANNELPOLARITY_BOTHEDGE

typedef enum {
  TIM_PulseOnHigh = 1,
  TIM_PulseOnLow  = 0
}PulseLevel;


//Refer to datasheet
typedef enum {
  TIM_16bit = 0,
  TIM_32bit = 1
}TIM_Resolution;


#define TIM_Num_Channels 4



//Aux-struct for saving RAM space in case input capture is not used
typedef struct{
	volatile uint32_t ICpolarity[TIM_Num_Channels + 1];
	volatile int32_t IC_FirstEdge[TIM_Num_Channels + 1];
	volatile int32_t PulseWidth[TIM_Num_Channels + 1];
	volatile Bool isUsedForPwmInput;
	volatile PulseLevel pulse_polarity;
	uint32_t pwm_input_max_count;

}TIM_IC;

typedef struct{
	TIM_HandleTypeDef *htim;
	volatile uint32_t TimerPrescaler;
	volatile uint32_t ARR; //ARR: (Counter) Auto Reload Register value, a.k.a timer period
				  //ARR = max_count = *htim->init->Period
	volatile uint32_t CCR; //Capture/Compare (context dependent)
	volatile uint32_t CNT;

	uint32_t APBx_Div_Factor;
	uint32_t ActualFreq;
	TIM_Resolution xBitTIM;

	volatile TIM_IC* IC_fields;

	volatile Bool isEncMode;
	volatile int32_t ENC_CNT;
	volatile int32_t ENC_OverFlow;
}TIM;



/*=======================Universal Functions=================================*/
TIM *newTIM(TIM* instance, TIM_HandleTypeDef *htim, uint32_t APBx_DivFactor, TIM_Resolution xBitTIM);
void timSetARR(TIM* instance, uint32_t ARR_val);
uint32_t timGetARR(TIM* instance);
void timSetCCR(TIM* instance, uint32_t channel, uint32_t CCR_val);
uint32_t timGetCCR(TIM* instance, uint32_t channel);
void timSetCNT(TIM* instance, uint32_t CNT_val);
uint32_t timGetCNT(TIM* instance);
void timSetPrescaler(TIM* instance, uint32_t prescaler_val);
uint32_t timGetPrescaler(TIM* instance);

uint8_t tim_channel_index(uint32_t channel);
/*===========================================================================*/

/*=======================Basic Counting & Interrupt==========================*/

/*Warning!: For 16bit timers, AutoRelaod_count must be less than 2^16
 * & timer frequency must be greater than a certain
 * threshold such that make prescaler value less than 2^16 (refer to source code)*/
Bool initTIM_BasicCounting(TIM* instance, uint32_t AutoReload_count, uint32_t timer_frequency);

Bool timSetFrequency(TIM* instance, uint32_t timer_frequency);
void timCountBegin(TIM* instance);
void timCountEnd(TIM* instance);

void timCountBegin_IT(TIM* instance);
void timCountEnd_IT(TIM* instance);
uint32_t timGetCount(TIM* instance);

//Interrupt handler
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
__weak void timPE_IT_CallBack(TIM* instance);
__weak void timSysT_IT_CallBack(TIM* instance);
/*===========================================================================*/

/*=============================PWM Input/Output==============================*/

//PWM Generation
Bool initTIM_PWM_Out(TIM* instance, uint32_t max_count, uint32_t pwm_frequency);
Bool timSetPwmFrequency(TIM* instance, uint32_t max_count, uint32_t pwm_frequency);
void timPwmGenBegin(TIM* instance, uint32_t channel);
void timSetPwmDutyCycle(TIM* instance, uint32_t channel, uint32_t dutyCycleCnt);
void timPwmWrite(TIM* instance, uint32_t channel, double dutyCyclePercent);

//PWM Capture (Essentially Input Capture interpreted as PWM signal)
Bool initTIM_PWM_In(TIM* instance, TIM_IC* IC_fields, uint32_t max_count, uint32_t pwm_frequency);
void timPwmIcBegin(TIM* instance, uint32_t channel, PulseLevel pulse_polarity);
void timPwmIcEnd(TIM* instance, uint32_t channel);
int32_t timGetPulseWidth(TIM* instance, uint32_t channel);
double timPwmRead(TIM* instance, uint32_t channel);
/*===========================================================================*/

/*================Input Capture(Interrupt Mode Only)========================*/
uint32_t initTIM_IC(TIM* instance, TIM_IC* IC_fields, uint32_t timer_frequency);
void timSetIC_Polarity(TIM* instance, uint32_t channel, uint32_t ICpolarity);
void timIcBegin_IT(TIM* instance, uint32_t channel);
void timIcEnd_IT(TIM* instance, uint32_t channel);
uint32_t timGetCapVal(TIM* instance, uint32_t channel);

//Interrupt handler
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim);
__weak void timIC_IT_CallBack(TIM* instance, HAL_TIM_ActiveChannel active_channel);
/*===========================================================================*/

/*======================(Quadrature) Encoder Mode============================*/
void initTIM_Enc(TIM* instance);
void timEncBegin(TIM* instance);
void timEncBegin_IT(TIM* instance);
void timEncEnd(TIM* instance);
void timEncEnd_IT(TIM* instance);
void timResetEnc(TIM* instance);
int32_t timGetEncCNT(TIM* instance);
int32_t timGetOverFlowPart_32bit(TIM* instance);
/*===========================================================================*/


#endif
#endif
