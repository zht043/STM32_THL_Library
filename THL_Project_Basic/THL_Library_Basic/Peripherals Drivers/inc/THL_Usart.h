#ifndef __THL_USART_H
#define __THL_USART_H
#include "THL_Portability.h"
#include "THL_Utility.h"
#include "THL_SysTick.h"

#ifdef HAL_UART_MODULE_ENABLED

#define TxBuffer_Size 100
#define RxBuffer_Size 100
#define Default_TxTimeOut 0xFFFF //~ 1 sec
#define Default_RxTimeOut 0xFFFFFFFF // 2^32 milliseconds


typedef struct{
	UART_HandleTypeDef *huart; //HAL Library Asynchronous Serial Handler's pointer 
	char TxBuffer[TxBuffer_Size];
	char RxBuffer[RxBuffer_Size];
	uint32_t TxTimeOut; // in millisecond
	uint32_t RxTimeOut;
	volatile uint8_t TxStatus;
	volatile uint8_t RxStatus;
}USART;


USART *newUSART(USART* obj, UART_HandleTypeDef *huart);
USART *newMainUSART(UART_HandleTypeDef *huart);
void usartSend(USART* obj);
void print(USART* obj, const char*format, ...);
void printf_u(const char*format, ...);
void usartSend_DMA(USART* obj);
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);
__weak void IT_CallBack_UsartTC(USART* obj);
void print_DMA(USART* obj, const char*format, ...);
void printf_DMA(Format_Param);
void fastPrint(USART* obj, const char*format, ...);
void fastPrintf(Format_Param);
int fputc(int c, FILE *f);


char* usartRead(USART* obj, uint16_t size);
char* usartRead_DMA(USART* obj, uint16_t size);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
__weak void IT_CallBack_UsartRC(USART* obj);
void readWord(USART* obj, char* str);
void readLine(USART* obj, char* str);
void readWordf(char* str);
void readLinef(char* str);
int scan(USART* obj, Format_Param);
int scanf_u(Format_Param);
#endif

#endif
