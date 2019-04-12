#include "Prj_Main.h"
#include "THL_Library_Basic.h"
#include "ICM20689_I2C_Tester.h"
#include "THL_SpiTester.h"
#include "PMW3901_Tester.h"
#include "THL_TimerTester.h"
#include "THL_AdcTester.h"
#include "ESC_Tester.h"
void prj_main(void) {
	testEsc();
}

