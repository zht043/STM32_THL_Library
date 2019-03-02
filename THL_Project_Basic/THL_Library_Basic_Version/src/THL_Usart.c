#include "THL_Usart.h"
#include "string.h"
#include <stdarg.h>

#ifdef HAL_UART_MODULE_ENABLED

/*========================SETUP AND DATA STRUCTURE========================*/
#define Max_Num_USARTs 20 /*Just throwing a large number here since
						*normally no stm32 chip has more than 20 usarts.
						*User may overwrite this macro to save memory space
						*when the specific ST chip model is known*/
uint16_t numActiveUSARTs = 0;
USART* ActiveUSARTs[Max_Num_USARTs]; /*For keeping a internal record of all usarts that are intialized
								   *The theoretical ideal implementation here should be a hash-table/hash-map.
								   *However, in order to reduce memory consumption and dev complexity, 
								   *an array tracker is introduced instead*/
USART DefaultUSART;

/**Pseudo-new (the "new" in oop context)
  *Initialize USART Struct with default parameters
  *The actual initiation of the usart circuit takes place by HAL 
  *before THL_Main.c gets called by main.c
  *Treated it as Pseudo-Constructor
  *Note: no memory allocation occurs
  */
USART *newUSART(USART* instance, UART_HandleTypeDef *huart) {
	instance->huart = huart;
	instance->TxTimeOut = USART_Default_TxTimeOut;
	instance->RxTimeOut = USART_Default_RxTimeOut;
	instance->TxStatus = Ready;
	instance->RxStatus = Ready;
	for(int i = 0; i < numActiveUSARTs; i++) 
		if(ActiveUSARTs[i]->huart == huart) {
			ActiveUSARTs[i] = instance;
			return instance;
		}
	ActiveUSARTs[numActiveUSARTs++] = instance;
	return instance;
}

/**Pseudo-Constructor for Default/Main USART
  *It sets the usart
  */
USART *newMainUSART(UART_HandleTypeDef *huart) { 
	return newUSART(&DefaultUSART, huart);
}
/*=========================================================================*/



/*==============================Transmission===============================*/
void usartWrite(USART* instance) {
	HAL_StatusTypeDef Status;
	Status = HAL_UART_Transmit(instance->huart, (uint8_t*)instance->TxBuffer, strlen(instance->TxBuffer), instance->TxTimeOut);
	if(Status == HAL_BUSY) instance->TxStatus = InProcess;
	else if(Status == HAL_TIMEOUT) {
		instance->TxStatus = TimeOut;
		//Unlock Usart
		__HAL_UNLOCK(instance->huart);
		instance->huart->gState = HAL_UART_STATE_READY;
		
		throwException("THL_Usart.c: usartWrite() | TimeOut");
	}
	else if(Status == HAL_ERROR) {
		instance->TxStatus = Error;
		throwException("THL_Usart.c: usartWrite() | Error");
	}
	else if(Status == HAL_OK) instance->TxStatus = Completed;
}


//Test Me!!!
void usartWrite_IT(USART* instance) {
	//check if the previous transmission is completed
	if(instance->TxStatus == InProcess) return;
	HAL_StatusTypeDef Status;
	Status = HAL_UART_Transmit_IT(instance->huart, (uint8_t*)instance->TxBuffer, strlen(instance->TxBuffer));
	if(Status == HAL_ERROR) {
		instance->TxStatus = Error;
		throwException("THL_Usart.c: usartWrite_IT() | Error");
		return;
	}
	instance->TxStatus = InProcess;
}

void usartWrite_DMA(USART* instance) {
	//check if the previous transmission is completed
	if(instance->TxStatus == InProcess) return;
	HAL_StatusTypeDef Status;
	Status = HAL_UART_Transmit_DMA(instance->huart, (uint8_t*)instance->TxBuffer, strlen(instance->TxBuffer));
	if(Status == HAL_ERROR) {
		instance->TxStatus = Error;
		throwException("THL_Usart.c: usartWrite_DMA() | Error");
		return;
	}
	instance->TxStatus = InProcess;
} 
/** Interrupt handler call back function
  */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
	for(int i = 0; i < numActiveUSARTs; i++) {
		if(ActiveUSARTs[i]->huart == huart) {
			usartTC_IT_CallBack(ActiveUSARTs[i]);
			ActiveUSARTs[i]->TxStatus = Completed;
		}
	}
}
/**Prototype function for transimission completion interrupt service. 
  *User overwrites it to define what to do when 
  *interrupt is triggered by usart completing 
  *tranfering data
  */
__weak void usartTC_IT_CallBack(USART* instance){
	 UNUSED(instance);
}

void print(USART* instance, Format_Param) {
	formatStrings(instance->TxBuffer);
	usartWrite(instance);
}
void printf_u(Format_Param) {
	formatStrings(DefaultUSART.TxBuffer);
	usartWrite(&DefaultUSART);
}


//Test Me !!!
/** Non-Blocking mode print with interrupt
  */
void print_IT(USART* instance, Format_Param) {
	//check if the previous transmission is completed
	if(instance->TxStatus == InProcess) return;
	formatStrings(instance->TxBuffer);
	usartWrite_IT(instance);
}
/** Non-Blocking mode print with dma
  */
void print_DMA(USART* instance, Format_Param) {
	//check if the previous transmission is completed
	if(instance->TxStatus == InProcess) return;
	formatStrings(instance->TxBuffer);
	usartWrite_DMA(instance);
}

//Test Me !!!
void printf_IT(Format_Param) {
	//check if the previous transmission is completed
	if(DefaultUSART.TxStatus == InProcess) return;
	formatStrings(DefaultUSART.TxBuffer);
	usartWrite_IT(&DefaultUSART);
}


void printf_DMA(Format_Param) {
	//check if the previous transmission is completed
	if(DefaultUSART.TxStatus == InProcess) return;
	formatStrings(DefaultUSART.TxBuffer);
	usartWrite_DMA(&DefaultUSART);
}
/** Blocking Mode print_dma 
  * printing with dma is 
  * generally faster than with CPU
  */
void fastPrint(USART* instance, const char*format, ...) {
	print_DMA(instance, format);
	uint32_t fP_t0 = millis();
	while(instance->TxStatus != Completed)
		if(millis() - fP_t0 > instance->TxTimeOut) break;
}
void fastPrintf(Format_Param) {
	printf_DMA(format);
	uint32_t fP_t0 = millis();
	while(DefaultUSART.TxStatus != Completed)
		if(millis() - fP_t0 > DefaultUSART.TxTimeOut) break;
}

/*=========================================================================*/


/*================================Reception================================*/
char* usartRead(USART* instance, uint16_t size) {
	HAL_StatusTypeDef Status;
	memset(instance->RxBuffer, 0, strlen(instance->RxBuffer));
	Status = HAL_UART_Receive(instance->huart, (uint8_t*)instance->RxBuffer, size, instance->RxTimeOut);
	if(Status == HAL_BUSY) instance->RxStatus = InProcess;
	else if(Status == HAL_TIMEOUT) {
		instance->RxStatus = TimeOut;
		//Unlock Usart
		__HAL_UNLOCK(instance->huart);
		instance->huart->gState = HAL_UART_STATE_READY;
		
		throwException("THL_Usart.c: usartRead() | TimeOut");
	}
	else if(Status == HAL_ERROR) {
		instance->RxStatus = Error;
		throwException("THL_Usart.c: usartRead() | Error");
	}
	else if(Status == HAL_OK) instance->RxStatus = Completed;
	return instance->RxBuffer;
}


//Test Me!!!
char* usartRead_IT(USART* instance, uint16_t size) {
	//check if the previous reception is completed
	if(instance->RxStatus == InProcess) return instance->RxBuffer;
	HAL_StatusTypeDef Status;
	memset(instance->RxBuffer, 0, strlen(instance->RxBuffer));
	Status = HAL_UART_Receive_IT(instance->huart, (uint8_t*)instance->RxBuffer, size);
	if(Status == HAL_ERROR) {
		instance->RxStatus = Error;
		throwException("THL_Usart.c: usartRead_IT() | Error");
		return instance->RxBuffer;
	}
	instance->RxStatus = InProcess;
	return instance->RxBuffer;
}

char* usartRead_DMA(USART* instance, uint16_t size) {
	//check if the previous reception is completed
	if(instance->RxStatus == InProcess) return instance->RxBuffer;
	HAL_StatusTypeDef Status;
	memset(instance->RxBuffer, 0, strlen(instance->RxBuffer));
	Status = HAL_UART_Receive_DMA(instance->huart, (uint8_t*)instance->RxBuffer, size);
	if(Status == HAL_ERROR) {
		instance->RxStatus = Error;
		throwException("THL_Usart.c: usartRead_DMA() | Error");
		return instance->RxBuffer;
	}
	instance->RxStatus = InProcess;
	return instance->RxBuffer;
} 



/** Interrupt handler call back function
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	for(int i = 0; i < numActiveUSARTs; i++) {
		if(ActiveUSARTs[i]->huart == huart) {
			usartRC_IT_CallBack(ActiveUSARTs[i]);
			ActiveUSARTs[i]->RxStatus = Completed;
		}
	}
}

__weak void usartRC_IT_CallBack(USART* instance){
	 UNUSED(instance);
}


void readWord(USART* instance, char* str) {
	int i = 0;
	str[i] = usartRead(instance, 1)[0];
	while(str[i] != ' ' && str[i] != '\r' && str[i] != '\n') 
		str[++i] = usartRead(instance, 1)[0];
	str[i] = '\0';
}
void readLine(USART* instance, char* str) {
	int i = 0;
	str[i] = usartRead(instance, 1)[0];
	while(str[i] != '\r' && str[i] != '\n') 
		str[++i] = usartRead(instance, 1)[0];
	str[i] = '\0';
}

void readWordf(char* str) {
	int i = 0;
	str[i] = usartRead(&DefaultUSART, 1)[0];
	while(str[i] != ' ' && str[i] != '\r' && str[i] != '\n') 
		str[++i] = usartRead(&DefaultUSART, 1)[0];
	str[i] = '\0';
}
void readLinef(char* str) {
	int i = 0;
	str[i] = usartRead(&DefaultUSART, 1)[0];
	while(str[i] != '\r' && str[i] != '\n') 
		str[++i] = usartRead(&DefaultUSART, 1)[0];
	str[i] = '\0';
}
int scan(USART* instance, Format_Param) {
	char scan_str[USART_RxBuffer_Size];
	readLine(instance, scan_str);
	va_list args; 
    va_start(args, format); 
	int rtn = vsscanf(scan_str, format, args);
	va_end(args); 
	return rtn;
}
/*Implement usart-scanf by overloading fgetc is quite troublesome,
 *while C is not oop and scanf() is not overwrite/overload-able, 
 *thus we provide scanf_u instead, use it as if you are using scanf.
 *Here, 'u' in scanf_u stands for usart.	
 */
int scanf_u(Format_Param) {
	char scanf_str[USART_RxBuffer_Size];
	readLine(&DefaultUSART, scanf_str);
	va_list args; 
    va_start(args, format); 
	int rtn = vsscanf(scanf_str, format, args);
	va_end(args); 
	return rtn;
}
/*******
Read & scan with dma is not impossible to implement but too troublesome, 
it would require rewriting the underlying HAL library with significant efforts.
Best to use multitask/threading instead, e.g. vPortTask with FreeRTOS, in case
a non-block read/scan implementation is needed. And if reception completion interrupt
is demanded, please include the use of LL library or perform register operation
directly with CMSIS. 
Normally these advanced demands described above are rare.
*******/

/*=========================================================================*/

/*
||*******************************************************************************************************************************||
||*******************************************************************************************************************************||
||*******************************************************************************************************************************||
*/

/*=============================REDIRECT STDIO==============================*/
/**Overwrite the fputc function which is called by printf
  *in order to redirect stdout > the main usart(Serial) port
  */
/* In IDE [Atollic Truestudio] printf is preserved for debugging, use printf_u instead.
 * Uncomment this to enable prinf when compiling on other IDEs such as IAR or KEIL
int fputc(int c, FILE *f)  
{ 
	print(&DefaultUSART, "%c", c);
	return c; 
} 
*/

/*=========================================================================*/



void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {
	throwException("THL_Usart.c: HAL_UART_ErrorCallback()");
}

#endif
