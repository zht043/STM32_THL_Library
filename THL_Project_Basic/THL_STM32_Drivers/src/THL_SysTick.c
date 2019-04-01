#include "THL_SysTick.h"
#include "THL_Portability.h"
#include "THL_Timer.h"

uint32_t SysTime_T0 = 0;

#ifdef SysTick_Reserved

TIM sys_timer_mem;
TIM* sys_timer;

volatile uint32_t millis_tick = 0;

void initSysTime_TIM(TIM_HandleTypeDef *htim, uint32_t APBx_DivFactor, TIM_Resolution xBitTIM) {
	SysTime_T0 = 0;
	millis_tick = 0;
	sys_timer = newTIM(&sys_timer_mem, htim, APBx_DivFactor, xBitTIM);
	initTIM_BasicCounting(sys_timer, 1000, 1000000); // AutoReload at every 1000 count, counting at 1000,000Hz = 1Mhz
	timCountBegin_IT(sys_timer);
}

void timSysT_IT_CallBack(TIM* instance) {
	if(instance == sys_timer) {
		millis_tick++;
	}
}

uint32_t millis(void) {
		return millis_tick - SysTime_T0;
}
uint32_t micros(void) {
	  return millis()*1000 + timGetCount(sys_timer);
}

#else
void initSysTime_SysTick(void) {
	SysTime_T0 = 0;
}

uint32_t millis(void) {
	return HAL_GetTick() - SysTime_T0;
}
uint32_t micros(void) {
	//the second 1000 here corresponds to the default 1ms interrupt for teh Systick timer
	return HAL_GetTick()*1000 + 1000 - (SysTick->VAL)/(HAL_RCC_GetHCLKFreq() / 1000000);
}
#endif


void delay_us(uint32_t Time) {
	uint32_t T_init = micros();
	while(micros() - T_init < Time);
}
void delay(uint32_t Time) {
	uint32_t T_init = millis();
	while(millis() - T_init < Time);
}
