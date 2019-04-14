#include "Prj_Main.h"
#include "THL_Library_Basic.h"
#include "ICM20689_I2C_Tester.h"
#include "THL_SpiTester.h"
#include "PMW3901_Tester.h"
#include "THL_TimerTester.h"
#include "THL_AdcTester.h"

extern UART_HandleTypeDef huart2;

void prj_main(void) {
	newMainUSART(&huart2);
	while(1) {
		printf_u("\rHello World!\r\n");
	}
}

