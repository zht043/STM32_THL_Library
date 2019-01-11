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
USART *newUSART(USART* obj, UART_HandleTypeDef *huart) {
	obj->huart = huart;
	obj->TxTimeOut = Default_TxTimeOut;
	obj->RxTimeOut = Default_RxTimeOut;
	obj->TxStatus = Ready;
	obj->RxStatus = Ready;
	for(int i = 0; i < numActiveUSARTs; i++) 
		if(ActiveUSARTs[i]->huart == huart) {
			ActiveUSARTs[i] = obj;
			return obj;
		}
	ActiveUSARTs[numActiveUSARTs++] = obj;
	return obj;
}

/**Pseudo-Constructor for Default/Main USART
  *It sets the usart
  */
USART *newMainUSART(UART_HandleTypeDef *huart) { 
	return newUSART(&DefaultUSART, huart);
}
/*=========================================================================*/



/*==============================Transmission===============================*/
void usartSend(USART* obj) {
	HAL_StatusTypeDef Status;
	Status = HAL_UART_Transmit(obj->huart, (uint8_t*)obj->TxBuffer, strlen(obj->TxBuffer), obj->TxTimeOut);
	if(Status == HAL_BUSY) obj->TxStatus = InProcess;
	else if(Status == HAL_TIMEOUT) {
		obj->TxStatus = TimeOut;
		//Unlock Usart
		__HAL_UNLOCK(obj->huart);
		obj->huart->gState = HAL_UART_STATE_READY; 	
		
		throwException("THL_Usart.c: usartSend() | TimeOut");
	}
	else if(Status == HAL_ERROR) {
		obj->TxStatus = Error;
		throwException("THL_Usart.c: usartSend() | Error");
	}
	else if(Status == HAL_OK) obj->TxStatus = Completed;
}
void usartSend_DMA(USART* obj) {
	//check if the previous transmission is completed
	if(obj->TxStatus == InProcess) return;
	HAL_StatusTypeDef Status;
	Status = HAL_UART_Transmit_DMA(obj->huart, (uint8_t*)obj->TxBuffer, strlen(obj->TxBuffer));
	if(Status == HAL_ERROR) {
		obj->TxStatus = Error;
		throwException("THL_Usart.c: usartSend_DMA() | Error");
		return;
	}
	obj->TxStatus = InProcess;
} 
/** Interrupt handler call back function
  */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
	for(int i = 0; i < numActiveUSARTs; i++) {
		if(ActiveUSARTs[i]->huart == huart) {
			IT_CallBack_UsartTC(ActiveUSARTs[i]);
			ActiveUSARTs[i]->TxStatus = Completed;
		}
	}
}
/**Prototype function for transimission completion interrupt service. 
  *User overwrites it to define what to do when 
  *interrupt is triggered by usart completing 
  *tranfering data
  */
__weak void IT_CallBack_UsartTC(USART* obj){
	 UNUSED(obj);
}

void print(USART* obj, Format_Param) {
	formatStrings(obj->TxBuffer);
	usartSend(obj);
}
void printf_u(Format_Param) {
	formatStrings(DefaultUSART.TxBuffer);
	usartSend(&DefaultUSART);
}
/** Non-Blocking mode print with dma
  */
void print_DMA(USART* obj, Format_Param) {
	//check if the previous transmission is completed
	if(obj->TxStatus == InProcess) return;
	formatStrings(obj->TxBuffer);
	usartSend_DMA(obj);
}
void printf_DMA(Format_Param) {
	//check if the previous transmission is completed
	if(DefaultUSART.TxStatus == InProcess) return;
	formatStrings(DefaultUSART.TxBuffer);
	usartSend_DMA(&DefaultUSART);
}
/** Blocking Mode print_dma 
  * printing with dma is 
  * generally faster than with CPU
  */
void fastPrint(USART* obj, const char*format, ...) {
	print_DMA(obj, format);
	uint32_t fP_t0 = millis();
	while(obj->TxStatus != Completed)
		if(millis() - fP_t0 > obj->TxTimeOut) break;
}
void fastPrintf(Format_Param) {
	printf_DMA(format);
	uint32_t fP_t0 = millis();
	while(DefaultUSART.TxStatus != Completed)
		if(millis() - fP_t0 > DefaultUSART.TxTimeOut) break;
}

/*=========================================================================*/


/*================================Reception================================*/
char* usartRead(USART* obj, uint16_t size) {
	HAL_StatusTypeDef Status;
	memset(obj->RxBuffer, 0, strlen(obj->RxBuffer));
	Status = HAL_UART_Receive(obj->huart, (uint8_t*)obj->RxBuffer, size, obj->RxTimeOut);
	if(Status == HAL_BUSY) obj->RxStatus = InProcess;
	else if(Status == HAL_TIMEOUT) {
		obj->RxStatus = TimeOut;
		//Unlock Usart
		__HAL_UNLOCK(obj->huart);
		obj->huart->gState = HAL_UART_STATE_READY; 	
		
		throwException("THL_Usart.c: usartRead() | TimeOut");
	}
	else if(Status == HAL_ERROR) {
		obj->RxStatus = Error;
		throwException("THL_Usart.c: usartRead() | Error");
	}
	else if(Status == HAL_OK) obj->RxStatus = Completed;
	return obj->RxBuffer;
}
char* usartRead_DMA(USART* obj, uint16_t size) {
	//check if the previous reception is completed
	if(obj->RxStatus == InProcess) return obj->RxBuffer;
	HAL_StatusTypeDef Status;
	memset(obj->RxBuffer, 0, strlen(obj->RxBuffer));
	Status = HAL_UART_Receive_DMA(obj->huart, (uint8_t*)obj->RxBuffer, size);
	if(Status == HAL_ERROR) {
		obj->RxStatus = Error;
		throwException("THL_Usart.c: usartRead_DMA() | Error");
		return obj->RxBuffer;
	}
	obj->RxStatus = InProcess;
	return obj->RxBuffer;
} 
/** Interrupt handler call back function
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	for(int i = 0; i < numActiveUSARTs; i++) {
		if(ActiveUSARTs[i]->huart == huart) {
			IT_CallBack_UsartRC(ActiveUSARTs[i]);
			ActiveUSARTs[i]->RxStatus = Completed;
		}
	}
}

__weak void IT_CallBack_UsartRC(USART* obj){
	 UNUSED(obj);
}


void readWord(USART* obj, char* str) {
	int i = 0;
	str[i] = usartRead(obj, 1)[0];
	while(str[i] != ' ' && str[i] != '\r' && str[i] != '\n') 
		str[++i] = usartRead(obj, 1)[0];
	str[i] = '\0';
}
void readLine(USART* obj, char* str) {
	int i = 0;
	str[i] = usartRead(obj, 1)[0];
	while(str[i] != '\r' && str[i] != '\n') 
		str[++i] = usartRead(obj, 1)[0];
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
int scan(USART* obj, Format_Param) {
	char scan_str[RxBuffer_Size];
	readLine(obj, scan_str);
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
	char scanf_str[RxBuffer_Size];
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
