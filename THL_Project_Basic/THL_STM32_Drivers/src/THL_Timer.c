#include "THL_Timer.h"


#ifdef HAL_TIM_MODULE_ENABLED

#define Max_Num_TIMs 25
uint16_t numActiveTIMs = 0;
TIM* ActiveTIMs[Max_Num_TIMs];


//Each section below has its own instantiation method


/*Abbreviation Notes:
 * ->ARR = Autoreload Register -- Period
 * ->CCR = Compare & Capture Register
 * ->CNT = Counter Value
 */

/*private function declaration*/
static void timPWM_IN_IT_CallBack(TIM* instance, HAL_TIM_ActiveChannel active_channel);
static void timEnc_OverFlow_IT_CallBack(TIM* instance);



/*=======================Universal Function================================*/
/**This section is aimed for timer that multitasks, if a timer
  *is solely focus on e.g. pwm generation, go to PWM Generation
  *in another section below
  */
/**
 * Please use STM32CubeMx to config period and prescaler
 * Period & prescaler setting are applied on all four channels
 */
TIM *newTIM(TIM* instance, TIM_HandleTypeDef *htim, uint32_t APBx_DivFactor, TIM_Resolution xBitTIM) {
	instance->htim = htim;
	instance->APBx_Div_Factor = APBx_DivFactor;
	instance->xBitTIM = xBitTIM;
	instance->isEncMode = False;
	for(int i = 0; i < numActiveTIMs; i++)
		if(ActiveTIMs[i]->htim == htim) {
			ActiveTIMs[i] = instance;
			return instance;
		}
	ActiveTIMs[numActiveTIMs++] = instance;
	return instance;
}

void timSetARR(TIM* instance, uint32_t ARR_val) {
	if(instance->xBitTIM == TIM_16bit) {
		if(ARR_val > 0xFFFF)
			throwException("THL_Timer.c: timSetARR() | AutoReload must < 0xFFFF for a 16 bit timer");
	}
	else if(instance->xBitTIM == TIM_32bit) {
		if(ARR_val > 0xFFFFFFFF)
			throwException("THL_Timer.c: timSetARR() | AutoReload must < 0xFFFFFFFF for a 32 bit timer");
	}


	instance->ARR = ARR_val;
	__HAL_TIM_SET_AUTORELOAD(instance->htim, instance->ARR);
}

uint32_t timGetARR(TIM* instance) {
	instance->ARR = __HAL_TIM_GET_AUTORELOAD(instance->htim);
	return instance->ARR;
}

void timSetCCR(TIM* instance, uint32_t channel, uint32_t CCR_val) {
	instance->CCR = CCR_val;
	__HAL_TIM_SET_COMPARE(instance->htim, channel, CCR_val);
}

uint32_t timGetCCR(TIM* instance, uint32_t channel) {
	instance->CCR = __HAL_TIM_GET_COMPARE(instance->htim, channel);
	return instance->CCR;
}

void timSetCNT(TIM* instance, uint32_t CNT_val) {
	instance->CNT = CNT_val;
	__HAL_TIM_SET_COUNTER(instance->htim, CNT_val);
}

uint32_t timGetCNT(TIM* instance) {
	instance->CNT = __HAL_TIM_GET_COUNTER(instance->htim);
	return instance->CNT;
}

void timSetPrescaler(TIM* instance, uint32_t prescaler_val) {
	instance->TimerPrescaler = prescaler_val;
	__HAL_TIM_SET_PRESCALER(instance->htim, instance->TimerPrescaler);
}

uint32_t timGetPrescaler(TIM* instance) {
	return instance->TimerPrescaler;
}

uint8_t tim_channel_index(uint32_t channel) {
	if(channel == TIM_CH1) return 1;
	if(channel == TIM_CH2) return 2;
	if(channel == TIM_CH3) return 3;
	if(channel == TIM_CH4) return 4;
	return 0;
}
/*===========================================================================*/


/*=======================Basic Counting & Interrupt==========================*/
Bool initTIM_BasicCounting(TIM* instance, uint32_t AutoReload_count, uint32_t timer_frequency) {
	timSetARR(instance, AutoReload_count);
	Bool rtn = timSetFrequency(instance, timer_frequency);
	return rtn;
}


Bool timSetFrequency(TIM* instance, uint32_t timer_frequency) {
	uint32_t TimerMaxFrequency = HAL_RCC_GetHCLKFreq() / instance->APBx_Div_Factor;

	if(timer_frequency > TimerMaxFrequency) {
		throwException("THL_Timer.c: timSetFrequency() | timer_frequency must be less or equal than TimerMaxFrequency");
		return Failed;
	}
	timSetPrescaler(instance, TimerMaxFrequency / timer_frequency - 1);

	//Prescaled frequency is subject to rounding error
	instance->ActualFreq = TimerMaxFrequency / (timGetPrescaler(instance) + 1);
	return Succeeded;
}

void timCountBegin(TIM* instance) {
	HAL_TIM_Base_Start(instance->htim);
}
void timCountEnd(TIM* instance) {
	HAL_TIM_Base_Stop(instance->htim);
}


void timCountBegin_IT(TIM* instance) {
	HAL_TIM_Base_Start_IT(instance->htim);
}
void timCountEnd_IT(TIM* instance) {
	HAL_TIM_Base_Stop_IT(instance->htim);
}
uint32_t timGetCount(TIM* instance) {
	return timGetCNT(instance);
}


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	for(int i = 0; i < numActiveTIMs; i++) {
		if(ActiveTIMs[i]->htim == htim) {
			timPE_IT_CallBack(ActiveTIMs[i]);
			timSysT_IT_CallBack(ActiveTIMs[i]);
		}
	}
}

__weak void timPE_IT_CallBack(TIM* instance) {
	UNUSED(instance);
}

__weak void timSysT_IT_CallBack(TIM* instance) {
	UNUSED(instance);
}
/*===========================================================================*/



/*============================PWM Input/Output===============================*/

//Output: PWM generation

/** PWM Frequency Explaination
 *  - Timer frequency and PWM frequency are two distinct concepts!
 *    [SystemCoreClock] => [TimerClock/Freq] => [pwm_frequency]  (signals are produced from left to right).
 *    Timer clock is system clock divided by a division factor.
 *    Timer clock determines the frequency of each timer tick.
 *
 *    PWM signals are generated by setting a counter value <= ([Max_Count], a.k.a [timer period]), this
 *    counter value is what gets written to TIM->CCRx register. The timer output compare (a hardware function)
 *    pulls the signal line HIGH until the counter counts up to this TIM->CCRx value, and then pull it LOW till
 *    Max_Count. Technically, the [PwmDutyCycle] = [CCR] / [Max_Count] (in percentage).
 *
 * 	- The [TimerPrescaler] corresponds to a register that can be configured to set the desired timer frequency
 * 	  by dividing the "APB bus clock" scaled from [SystemCoreClock], a.k.a [HCLK_Frequency], with the TimerPrescaler value.
 * 	  Details of the clock relations can be looked up in STM32CubeMx software's clock configuration section,
 * 	  where a clock tree is presented.
 *
 * 	- [TimerMaxFrequency] = [HCLK] / [APBx_Bus_DivisionFactor] = [APBx_Bus_Frequency].
 *
 * 	- [TimerPrescaler] = [TimerMaxFrequency] / [TimerFrequency:(the desired frequency by user)].
 *
 * 	- [TimerFrequency:(desired)] = [Max_Count:(refer to 2nd paragraph)] * [pwm_frequency:(desired)].
 * 	  Basically the timer frequency is determined both by max count and pwm frequency,
 * 	  where the max count determines the resolution of this pwm signal.
 *
 * 	- [Max_Count] * [pwm_frequency] must be <= [TimerMaxFrequency], increasing PWM_Freq makes the driver
 * 	  respond quicker at the cost of resolution, while increasing the maximum counter value gives a higher
 * 	  resolution as the expense of responsiveness.
 *
 * 	- Turning the frequency down saves power consumption, but it usually dosen't matter.
 *
 * 	- For regular DC motor, 1kHZ-10kHZ is recommended in general.
 *
 * 	- For drone ESCs, frequency is much lower, refer to the ESC datasheet.
 *
 */

/* max_count determines the resolution,
 * e.g. for .1  precision, max_count is typically 1,000,
 *      for .01 precision, max_count is typically 10,000
 * pwm_frequency refers to the frequency of each PWM Cycle's period,
 * which is the time period between two rising edge of a typical pwm pulse
 */
Bool initTIM_PWM_Out(TIM* instance, uint32_t max_count, uint32_t pwm_frequency) {
	Bool rtn = timSetPwmFrequency(instance, max_count, pwm_frequency);
	return rtn;
}

/*Set PWM frequency at runtime*/
Bool timSetPwmFrequency(TIM* instance, uint32_t max_count, uint32_t pwm_frequency) {
	double TimerMaxFrequency = HAL_RCC_GetHCLKFreq() / instance->APBx_Div_Factor;
	double TimerFrequency = max_count * pwm_frequency;
	if(TimerFrequency > TimerMaxFrequency) {
		throwException("THL_Timer.c: setPwmFrequency() | max_count * pwm_frequency must be less or equal than TimerMaxFrequency");
		return Failed;
	}
	instance->ActualFreq = TimerFrequency;

	timSetPrescaler(instance, TimerMaxFrequency / TimerFrequency - 1);
	timSetARR(instance, ((uint32_t)TimerMaxFrequency / (instance->TimerPrescaler + 1) ) / pwm_frequency);
												//Minimize rounding error
	return Succeeded;
}

/**
 * @param  Channel TIM Channels to be enabled.
 *         This parameter can be one of the following values:
 *           @arg TIM_Channel_1
 *           @arg TIM_Channel_2
 *           @arg TIM_Channel_3
 *           @arg TIM_Channel_4
 */
void timPwmGenBegin(TIM* instance, uint32_t channel) {
	HAL_TIM_PWM_Start(instance->htim, channel);
	timSetPwmDutyCycle(instance, channel, 0);
}

void timPwmGenEnd(TIM* instance, uint32_t channel) {
	HAL_TIM_PWM_Stop(instance->htim, channel);
}

void timSetPwmDutyCycle(TIM* instance, uint32_t channel, uint32_t dutyCycleCnt) {
	timSetCCR(instance, channel, dutyCycleCnt);
}

//Pretty straightforward
void timPwmWrite(TIM* instance, uint32_t channel, double dutyCyclePercent) {
	if(dutyCyclePercent > 100.00f) dutyCyclePercent = 100.00f;
	dutyCyclePercent /= 100.00f;
	timSetPwmDutyCycle(instance, channel, (uint32_t)(dutyCyclePercent * (double)instance->ARR));
}


// Input: Periodic input capture interpreted as PWM pulse

/* PWM input: Typical use cases: PPM radio remote input, SR04 Ultrasonic sensor
 * Timer's counter counts at [timer_freq = max_cnt * pwm_freq]
 * Same as pwm_gen, max_count here is used to determine resolution.
 * Generally it is recommended to set the resolution so that the timer_freq is less than 1,000,000 Hz = 1Mhz
 *
 * Unlike other methods, max_count here is not used to set Autoreload,
 * To reduce the chances of counter overflow, ARR is set to be the largest 16-bit value 0xFFFF,
 * even though this edge case has been taken care.
 * It is required to have [# of Autoreload(ARR) count > # of counts within a pulse],
 * ARR count is 0xFFFF, # of counts within a pulse = pulse_width / (1/timer_freq).
 *
 * Note: Capture interrupt MUST be enabled, use CubeMx to enable it
 */
Bool initTIM_PWM_In(TIM* instance, TIM_IC* IC_fields, uint32_t max_count, uint32_t pwm_frequency) {
	uint32_t timer_frequency = max_count * pwm_frequency;
	uint32_t TimerMaxFrequency = HAL_RCC_GetHCLKFreq() / instance->APBx_Div_Factor;
	if(timer_frequency > TimerMaxFrequency) {
		throwException("THL_Timer.c: timSetFrequency() | timer_frequency must be less or equal than TimerMaxFrequency");
		return Failed;
	}

	initTIM_IC(instance, IC_fields, timer_frequency);

	for(int i = 1; i < TIM_Num_Channels+1; i++) {
		IC_fields->IC_FirstEdge[i] = 0;
		IC_fields->PulseWidth[i] = 0;
	}

	instance->IC_fields->isUsedForPwmInput = True;
	instance->IC_fields->pwm_input_max_count = max_count;

	return Succeeded;
}

//pulse_polarity simply tells whether the pulse signal is High during the pulse or low during the pulse
void timPwmIcBegin(TIM* instance, uint32_t channel, PulseLevel pulse_polarity) {
	if(pulse_polarity == TIM_PulseOnHigh)
		timSetIC_Polarity(instance, channel, TIM_IC_RisingEdge);
	if(pulse_polarity == TIM_PulseOnLow)
		timSetIC_Polarity(instance, channel, TIM_IC_FallingEdge);

	instance->IC_fields->pulse_polarity = pulse_polarity;

	timIcBegin_IT(instance, channel);
}

void timPwmIcEnd(TIM* instance, uint32_t channel) {
	timIcEnd_IT(instance, channel);
}


int32_t timGetPulseWidth(TIM* instance, uint32_t channel) {
	return instance->IC_fields->PulseWidth[tim_channel_index(channel)];
}

double timPwmRead(TIM* instance, uint32_t channel) {
	return ((double)timGetPulseWidth(instance, channel) / (double)instance->IC_fields->pwm_input_max_count) * 100.00f;
}

static void timPWM_IN_IT_CallBack(TIM* instance, HAL_TIM_ActiveChannel active_channel) {
	uint32_t channel;
	if(instance->IC_fields->isUsedForPwmInput == True) {
		if(active_channel == TIM_Active_CH1) channel = TIM_CH1;
		if(active_channel == TIM_Active_CH2) channel = TIM_CH2;
		if(active_channel == TIM_Active_CH3) channel = TIM_CH3;
		if(active_channel == TIM_Active_CH4) channel = TIM_CH4;

		if(instance->IC_fields->pulse_polarity == TIM_PulseOnHigh) {
			if(instance->IC_fields->ICpolarity[tim_channel_index(channel)] == TIM_IC_RisingEdge) {
				instance->IC_fields->IC_FirstEdge[tim_channel_index(channel)]
												  = timGetCapVal(instance, channel);

				timSetIC_Polarity(instance, channel, TIM_IC_FallingEdge);
			}
			else if(instance->IC_fields->ICpolarity[tim_channel_index(channel)] == TIM_IC_FallingEdge) {
				instance->IC_fields->PulseWidth[tim_channel_index(channel)]
												= timGetCapVal(instance, channel)
												  - instance->IC_fields->IC_FirstEdge[tim_channel_index(channel)];
				if(instance->IC_fields->PulseWidth[tim_channel_index(channel)] < 0) {
					instance->IC_fields->PulseWidth[tim_channel_index(channel)]
													+= instance->ARR+1;
				}

				timSetIC_Polarity(instance, channel, TIM_IC_RisingEdge);
			}
		}
		else {
			if(instance->IC_fields->ICpolarity[tim_channel_index(channel)] == TIM_IC_FallingEdge) {
				instance->IC_fields->IC_FirstEdge[tim_channel_index(channel)]
												  = timGetCapVal(instance, channel);

				timSetIC_Polarity(instance, channel, TIM_IC_RisingEdge);
			}
			else if(instance->IC_fields->ICpolarity[tim_channel_index(channel)] == TIM_IC_RisingEdge) {
				instance->IC_fields->PulseWidth[tim_channel_index(channel)]
												= timGetCapVal(instance, channel)
												  - instance->IC_fields->IC_FirstEdge[tim_channel_index(channel)];
				if(instance->IC_fields->PulseWidth[tim_channel_index(channel)] < 0) {
					instance->IC_fields->PulseWidth[tim_channel_index(channel)]
													+= instance->ARR+1;
				}

				timSetIC_Polarity(instance, channel, TIM_IC_FallingEdge);
			}
		}
	}
}


/*=========================================================================*/



/*================Input Capture(Interrupt Mode Only)=======================*/
uint32_t initTIM_IC(TIM* instance, TIM_IC* IC_fields, uint32_t timer_frequency) {
	if(instance->xBitTIM == TIM_32bit) timSetARR(instance, 0xFFFFFFFF);
	else if(instance->xBitTIM == TIM_16bit) timSetARR(instance, 0xFFFF);
	timSetFrequency(instance, timer_frequency);

	instance->IC_fields = IC_fields;
	instance->IC_fields->isUsedForPwmInput = False;
	return instance->ActualFreq;
}

void timSetIC_Polarity(TIM* instance, uint32_t channel, uint32_t ICpolarity) {
	instance->IC_fields->ICpolarity[tim_channel_index(channel)] = ICpolarity;
	__HAL_TIM_SET_CAPTUREPOLARITY(instance->htim, channel, ICpolarity);
}
void timIcBegin_IT(TIM* instance, uint32_t channel) {
	HAL_TIM_IC_Start_IT(instance->htim, channel);
}
void timIcEnd_IT(TIM* instance, uint32_t channel) {
	HAL_TIM_IC_Stop_IT(instance->htim, channel);
}


uint32_t timGetCapVal(TIM* instance, uint32_t channel) {
	return HAL_TIM_ReadCapturedValue(instance->htim, channel);
}


void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
	for(int i = 0; i < numActiveTIMs; i++) {
		if(ActiveTIMs[i]->htim == htim) {
			HAL_TIM_ActiveChannel active_channel = htim->Channel;
			timPWM_IN_IT_CallBack(ActiveTIMs[i], active_channel);
			timEnc_OverFlow_IT_CallBack(ActiveTIMs[i]);
			timIC_IT_CallBack(ActiveTIMs[i], active_channel);
		}
	}
}

__weak void timIC_IT_CallBack(TIM* instance, HAL_TIM_ActiveChannel active_channel) {
	UNUSED(instance);
	UNUSED(active_channel);
}
/*=========================================================================*/

/*======================(Quadrature) Encoder Mode==========================*/
//Reminder: Enable Encoder Mode in CubeMx

void initTIM_Enc(TIM* instance) {
	if(instance->xBitTIM == TIM_16bit) timSetARR(instance, 0xFFFF);
	else if(instance->xBitTIM == TIM_32bit) timSetARR(instance, 0xFFFFFFFF);
	instance->isEncMode = True;
	instance->ENC_CNT   = 0;
	instance->ENC_OverFlow = 0;
}

void timEncBegin(TIM* instance) {
	HAL_TIM_Encoder_Start(instance->htim, TIM_CHANNEL_ALL);
	timResetEnc(instance);
}

//Enable interrupt to handle counter overflow, usually applied on 16bit timers
void timEncBegin_IT(TIM* instance) {
	HAL_TIM_Encoder_Start_IT(instance->htim, TIM_CHANNEL_ALL);
	timResetEnc(instance);
}

void timEncEnd(TIM* instance) {
	HAL_TIM_Encoder_Stop(instance->htim, TIM_CHANNEL_ALL);
}
void timEncEnd_IT(TIM* instance) {
	HAL_TIM_Encoder_Stop_IT(instance->htim, TIM_CHANNEL_ALL);
}

void timResetEnc(TIM* instance) {
	timSetCNT(instance, 0);
	instance->ENC_CNT = 0;
	instance->ENC_OverFlow = 0;
}

int32_t timGetEncCNT(TIM* instance) {
	if(instance->xBitTIM == TIM_16bit) {
		instance->ENC_CNT = (int16_t)__HAL_TIM_GET_COUNTER(instance->htim) + instance->ENC_OverFlow;
	}
	else {
		instance->ENC_CNT = (int32_t)__HAL_TIM_GET_COUNTER(instance->htim);
	}
	return instance->ENC_CNT;
}

int32_t timGetOverFlowPart_32bit(TIM* instance) {
	return instance->ENC_OverFlow;
}


static void timEnc_OverFlow_IT_CallBack(TIM* instance) {
	if(instance->isEncMode == True) {
		if(instance->xBitTIM == TIM_32bit) {

			if((int16_t)__HAL_TIM_GET_COUNTER(instance->htim) <= -1000000000) {  //1,000,000,000 using decimal base here for simplicity
				instance->ENC_OverFlow--;
				timSetCNT(instance, 0);
			}
			else if((int16_t)__HAL_TIM_GET_COUNTER(instance->htim) >= 1000000000) {
				instance->ENC_OverFlow++;
				timSetCNT(instance, 0);
			}
		}
		else if(instance->xBitTIM == TIM_16bit) {

			// 32767 == 0x7FFF == 0xFFFF/2, setting it to be a bit less than 0x7FFF in case of failure of interrupt capture (very rare)
			if((int16_t)__HAL_TIM_GET_COUNTER(instance->htim) <= -32760) {
				instance->ENC_OverFlow -= 32760;
				timSetCNT(instance, 0);
			}
			else if((int16_t)__HAL_TIM_GET_COUNTER(instance->htim) >= 32760) {
				instance->ENC_OverFlow += 32760;
				timSetCNT(instance, 0);
			}
		}
	}
}
/*=========================================================================*/





#endif


