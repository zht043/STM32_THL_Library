#include "THL_SysTick.h"
#include "THL_Portability.h"

#ifdef Operating_System_Enabled
/*
__weak uint32_t millis(void) {
}
__weak uint32_t micros(void) {
}
void delay_us(uint32_t Time) {
		uint32_t T_init = micros();
		while(micros() - T_init < Time);
}
void delay(uint32_t Time) {
		uint32_t T_init = millis();
		while(millis() - T_init < Time);
}*/
#else
__weak uint32_t millis(void) {
		return HAL_GetTick();
}
__weak uint32_t micros(void) {
	  return HAL_GetTick()*1000 + 1000 - (uint32_t)((SysTick->VAL)/(HAL_RCC_GetHCLKFreq() / 1000000)); 
}
void delay_us(uint32_t Time) {
		uint32_t T_init = micros(); 
		while(micros() - T_init < Time); 
}
void delay(uint32_t Time) {
		uint32_t T_init = millis(); 
		while(millis() - T_init < Time); 
}
#endif
