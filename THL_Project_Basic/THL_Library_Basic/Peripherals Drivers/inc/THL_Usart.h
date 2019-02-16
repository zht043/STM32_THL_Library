#ifndef __THL_USART_H
#define __THL_USART_H
#include "THL_Portability.h"
#include "THL_Utility.h"
#include "THL_SysTick.h"

#ifdef HAL_UART_MODULE_ENABLED

#define USART_TxBuffer_Size 100
#define USART_RxBuffer_Size 100
#define USART_Default_TxTimeOut 1000  // 1 second
#define USART_Default_RxTimeOut 1000  // 1 second


typedef struct{
	UART_HandleTypeDef *huart; //HAL Library Asynchronous Serial Handler's pointer 
	char TxBuffer[USART_TxBuffer_Size];
	char RxBuffer[USART_RxBuffer_Size];
	uint32_t TxTimeOut; // in millisecond
	uint32_t RxTimeOut;
	volatile uint8_t TxStatus;
	volatile uint8_t RxStatus;
}USART;


/*==============================instantiation===============================*/
USART *newUSART(USART* instance, UART_HandleTypeDef *huart);
USART *newMainUSART(UART_HandleTypeDef *huart);
/*=========================================================================*/



/*==============================Polling Mode===============================*/
/*All are blocking mode*/
void usartSend(USART* instance);
char* usartRead(USART* instance, uint16_t size);

void print(USART* instance, const char*format, ...);
void printf_u(const char*format, ...);

void readWord(USART* instance, char* str);
void readLine(USART* instance, char* str);
void readWordf(char* str);
void readLinef(char* str);
int scan(USART* instance, Format_Param);
int scanf_u(Format_Param);
/*=========================================================================*/


/*==============================Interrupt Mode===============================*/
/*All non-blocking mode*/
void usartSend_IT(USART* instance);
char* usartRead_IT(USART* instance, uint16_t size);
void print_IT(USART* instance, Format_Param);
void printf_IT(Format_Param);
/*=========================================================================*/


/*==============================DMA Mode===============================*/
/*Non-blocking mode*/
void usartSend_DMA(USART* instance);
char* usartRead_DMA(USART* instance, uint16_t size);

void print_DMA(USART* instance, const char*format, ...);
void printf_DMA(Format_Param);

/*Blocking mode*/
void fastPrint(USART* instance, const char*format, ...);
void fastPrintf(Format_Param);
/*=========================================================================*/


/*==============================Interrupt Handler===============================*/
/*Overload this callback function to add user-defined handling program upon Transmission Completion */
__weak void IT_CallBack_UsartTC(USART* instance);
/*Overload this callback function to add user-defined handling program upon Reception Completion */
__weak void IT_CallBack_UsartRC(USART* instance);
/*=========================================================================*/



/*==============================Native Callback===============================*/
//For internal use, not setting it private with static keyword due to overloading _weak def
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);
/*=========================================================================*/

/*Overloading standard printf's fputc function, this method does not support Atollic Truestudio IDE
 * due to coinciding with its built-in debug tool*/
int fputc(int c, FILE *f);



#endif

#endif
