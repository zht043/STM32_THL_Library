/*
 * THL_Utility.c
 *
 *      Author: Hongtao Zhang
 */


#include "THL_Utility.h"

/*==========================Exception Handling==============================*/
__weak void Exception_Handler(const char* str) {
	UNUSED(str);
}
void throwException(const char* str) {
	Exception_Handler(str);
}

__weak void DebugStr_Handler(const char* str) {
	UNUSED(str);
}
void debugString(Format_Param) {
	char str[30];
	formatStrings(str);
	DebugStr_Handler(str);
}
/*==========================================================================*/
