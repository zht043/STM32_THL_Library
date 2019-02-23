#include "THL_Utility.h"
#include "stm32f4xx_hal_def.h"


/*==========================Exception Handling==============================*/
__weak void Exception_Handler(const char* str) {
	UNUSED(str);
}
void throwException(const char* str) {
	Exception_Handler(str);
}
/*==========================================================================*/
