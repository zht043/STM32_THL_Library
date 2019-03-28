#ifndef __THL_Utility_H
#define __THL_Utility_H
#include "THL_Portability.h"
#include <stdarg.h>
#include <string.h>

/*=============================COMMON STATES===============================*/
typedef enum {
	True = 1,
	False = 0,

	Succeeded = 1,
	Failed = 0,

	High = 1,
	Low = 0,

	Enabled = 1,
	Disabled = 0,

	On = 1,
	Off = 0
}Bool;



typedef enum {
	NotReady = 0,
	Ready = 1,
	InProcess = 2,
	Completed = 3,
	TimeOut = 4,
	Error = 5
}CommStatus;



/*=========================================================================*/

/*=============================FORMAT STRINGS==============================*/
#define Format_Param const char* format, ...	
#define formatStrings(bufferPointer) do{ \
	va_list args; \
    va_start(args, format); \
    vsprintf(bufferPointer, format, args);\
	va_end(args); \
}while(0)


/*=========================================================================*/


__weak void Exception_Handler(const char* str);
void throwException(const char* str);

__weak void DebugStr_Handler(const char* str);
void debugString(Format_Param);

#endif
