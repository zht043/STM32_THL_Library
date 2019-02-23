#include "THL_Library_Basic.h"
#include "THL_I2cTester.h"
__weak void THL_main(void) {
	//testUsart();
	testI2c();
	delay(1000);
}


