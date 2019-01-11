#include "THL_UsartTesters.h"



GPIO led_pin;
GPIO* Led;


extern UART_HandleTypeDef huart2;
USART uart2; /*Allocate Global Var Memory for USART Pseudo-Object (struct)
			  *It is tricky for allocating heap memory in an OS-free embedded system,
			  *therefore, we alloacte static memory instead, for it is not an 
			  *authentic oop environment.*/


void IT_CallBack_UsartTC(USART* Device) {
	turnOn(Led);
}

void testPrints(void) {
	//For example, on nuleo board F446, uart2 -> stlink-v2's stm32f103 -> usb -> COM on ur X86/X64 computer
	USART* Terminal = newUSART(&uart2, &huart2); 
	print(Terminal,"\r**************************************************************\n\r");
	print(Terminal,"\r**************************************************************\n\r");
	print(Terminal,"\r**************************************************************\n\r");
	print(Terminal,"\r[Atollic Truestudio - C/C++] \n\r");

	print(Terminal,"\n\r=============Begin:Test Basic Outputtings==================\r");
		//test usartSend() 
		strcpy(Terminal->TxBuffer, "\n\rTest[usartSend]\r\n\r[*****]usartSend: Hello World!\r\n");
		usartSend(Terminal);

		//test print()
		print(Terminal, "\n\rTest[print]\r\n");
		print(Terminal, "\r[*****]print: Hello World!\r\n");
		print(Terminal, "\r[*****]Test format string: %d + %d = %d\r\n",1,1,1+1);

		//test printf_u()
		USART* MainTerminal = newMainUSART(&huart2);  //[1]
		printf_u("\n\rTest[printf_u]\r\n");
		printf_u("\r[*****]printf_u: Hello World!\r\n");
	print(MainTerminal,  "\r======================End==========================\r\n");
 
	
	printf_u("\n\r===============Begin:Test Outputtings with DMA=================\r");
		
		/*Terminal defined above is disabled when in [1] MainUSART access the same husart
		  Thus, redefine it again*/  //[2]
		Terminal = newUSART(&uart2, &huart2);
		
		//test usartSend_DMA()
		strcpy(Terminal->TxBuffer, "\n\rTest[usartSend_DMA]\r\n\r[*****]usartSend_DMA: Hello World!\r\n");
		usartSend_DMA(Terminal);
		while(Terminal->TxStatus != Completed);

		//test print_DMA()
		print_DMA(Terminal, "\n\rTest[print_DMA]\r\n");
		while(Terminal->TxStatus != Completed);
		print_DMA(Terminal, "\r[*****]print_DMA: Hello World!\r\n");
		while(Terminal->TxStatus != Completed);
		
		//test printf_DMA()
		
		/*Same reason as [2]*/
		MainTerminal = newMainUSART(&huart2);
		
		printf_DMA("\n\rTest[printf_DMA]\r\n");
		while(MainTerminal->TxStatus != Completed);
		printf_DMA("\r[*****]printf_DMA: Hello World!\r\n");
		while(MainTerminal->TxStatus != Completed);
		
		//test if DMA printing(nonblocking mode) disables follow-up prints
		printf_DMA("\n\rThere should be one empty line below\r\n");
		//delay(100);  
		/*if delayed, "Error" below will be printed because the 
		  printf_DMA above will definitely finish within 100 milliseconds*/
		printf_DMA("\rError!\r");
		while(MainTerminal->TxStatus != Completed);
		printf_DMA("\r\n-------------------------\r\n");
		while(MainTerminal->TxStatus != Completed);
		
		//test fastPrint & fastPrintf, they are simply print(f)_dma under blocking mode
		Terminal = newUSART(&uart2, &huart2); //same reason as [2]
		
		fastPrint(Terminal, "\n\rTest[fastPrint]\r\n");
		fastPrint(Terminal, "\r[*****]fastPrint: Hello World!\r\n");

		MainTerminal = newMainUSART(&huart2); //same reason as [2]
		fastPrintf("\n\rTest[fastPrintf]\r\n");
		fastPrintf("\r[*****]fastPrintf: Hello World!\r\n");
	printf_u("\r======================End==========================\r\n");
	
	delay(500);
	turnOff(Led);
}
void testScans(void) {
	USART* Terminal = newUSART(&uart2, &huart2);
	char* str;
	print(Terminal,"\r++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n\r");
	print(Terminal,"\r++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n\r");
	print(Terminal,"\r++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n\r");
	print(Terminal,"\n\r=============Begin:Test Basic Inputtings==================\r");
		//test usartRead() 
		for(int i = 1; i <= 5; i+=4) {	
			print(Terminal, "\n\rTest[usartRead]\r\n");
			print(Terminal,"\r[*****]Please enter EXACTLY %d letters\r\n\r>>>>>>> ",i);
			str = usartRead(Terminal, i+1); //+1 due to [Enter] key counting as one character at the end of stdin
			print(Terminal,"\r[Echo]: %s\r\n",str);
		}
		for(int i = 3; i > 0; i-=2) {	
			print(Terminal, "\n\rTest[usartRead]\r\n");
			print(Terminal,"\r[*****]Please enter EXACTLY %d letters\r\n\r>>>>>>> ",i);
			str = usartRead(Terminal, i+1); //+1 due to [Enter] key counting as one character at the end of stdin
			print(Terminal,"\r[Echo]: %s\r\n",str);
		}
	
		//test readWord() & readLine()
		print(Terminal, "\n\rTest[readWord]\r\n");
		print(Terminal,"\r[*****]Please enter two words (space-sperated) below\r\n");
		print(Terminal,"\r>>>>>>>>>>>>>>>>>>>\r\n");
		char one_word[RxBuffer_Size];
		readWord(Terminal, one_word);
		char another_word[RxBuffer_Size];
		readWord(Terminal, another_word);
		print(Terminal,"\r<<<<<<<<<<<<<<<<<<<\r\n");
		print(Terminal, "\r[Echo]:\r\n\r%s\r\n", one_word);
		print(Terminal, "\r%s\r\n", another_word);

		print(Terminal, "\n\rTest[readLine]\r\n");
		print(Terminal,"\r[*****]Please enter anything below (no more than [RxBuffer_Size] number of characters)\r\n");
		print(Terminal,"\r>>>>>>>>>>>>>>>>>>>\r\n");
		char one_line[RxBuffer_Size];
		readLine(Terminal, one_line);
		print(Terminal,"\r<<<<<<<<<<<<<<<<<<<\r\n");
		print(Terminal, "\r[Echo]:\r\n\r%s\r\n", one_line);
		
		//test readWordf & readLinef
		newMainUSART(&huart2);
		printf_u("\n\rTest[readWordf]\r\n");
		printf_u("\r[*****]Please enter two words (space-sperated) below\r\n");
		printf_u("\r>>>>>>>>>>>>>>>>>>>\r\n");
		readWordf(one_word);
		readWordf(another_word);
		printf_u("\r<<<<<<<<<<<<<<<<<<<\r\n");
		printf_u("\r[Echo]:\r\n\r%s\r\n", one_word);
		printf_u("\r%s\r\n", another_word);
		
		printf_u("\n\rTest[readLinef]\r\n");
		printf_u("\r[*****]Please enter anything below (no more than [RxBuffer_Size] number of characters)\r\n");
		printf_u("\r>>>>>>>>>>>>>>>>>>>\r\n");
		readLinef(one_line);
		printf_u("\r<<<<<<<<<<<<<<<<<<<\r\n");
		printf_u("\r[Echo]:\r\n\r%s\r\n", one_line);
	
		//test scan
	
		Terminal = newUSART(&uart2, &huart2);
	//while(1) { // uncomment it to do stress testing
		print(Terminal, "\n\rTest[scan]\r\n");
		print(Terminal, "\r[*****]Please enter two integers (space-sperated) below\r\n");
		uint32_t A, B;
		scan(Terminal, "%d %d", &A, &B);
		print(Terminal, "\r[Echo]:\r\n");
		print(Terminal, "\r%d\r\n",A);
		print(Terminal, "\r%d\r\n",B);
		print(Terminal, "\r[Sum]: %d\r\n", A+B);
		
		print(Terminal, "\r[*****]Please enter two strings (space-sperated) below\r\n");
		char string1[RxBuffer_Size], string2[RxBuffer_Size];
		scan(Terminal, "%s %s", string1, string2);
		print(Terminal, "\r[Echo]:\r\n");
		print(Terminal, "\r%s\r\n",string1);
		print(Terminal, "\r%s\r\n",string2);
		print(Terminal, "\r[Concatenation]: %s%s\r\n", string1, string2);
	//}
	
		//test scanf_u
		newMainUSART(&huart2);
	//while(1) { // uncomment it to do stress testing
		printf_u("\n\rTest[scanf_u]\r\n");
		printf_u("\r[*****]Please enter two integers (space-sperated) below\r\n");
		scanf_u("%d %d", &A, &B);
		printf_u("\r[Echo]:\r\n");
		printf_u("\r%lu\r\n",A);
		printf_u("\r%lu\r\n",B);
		printf_u("\r[Sum]: %lu\r\n", A+B);
		
		printf_u("\r[*****]Please enter two strings (space-sperated) below\r\n");
		scanf_u("%s %s", string1, string2);
		printf_u("\r[Echo]:\r\n");
		printf_u("\r%s\r\n",string1);
		printf_u("\r%s\r\n",string2);
		printf_u("\r[Concatenation]: %s%s\r\n", string1, string2);
	//}
	printf_u("\r======================End==========================\r\n");
	
	printf_u("\n\r===============Begin:Test Inputtings with DMA=================\r");
		Terminal = newUSART(&uart2, &huart2);
		//test usartRead_DMA()
		for(int i = 1; i <= 5; i+=4) {
			print_DMA(Terminal, "\n\rTest[usartRead_DMA]\r\n");
			while(Terminal->TxStatus != Completed);
			print_DMA(Terminal,"\r[*****]Please enter EXACTLY %d letters\r\n\r>>>>>>> ",i);
			while(Terminal->TxStatus != Completed);
			str = usartRead_DMA(Terminal, i+1);
			while(Terminal->RxStatus != Completed);
			print(Terminal,"\r[Echo]: %s\r\n",str);
			while(Terminal->TxStatus != Completed);
		}
		for(int i = 3; i > 0; i-=2) {
			print_DMA(Terminal, "\n\rTest[usartRead_DMA]\r\n");
			while(Terminal->TxStatus != Completed);
			print_DMA(Terminal,"\r[*****]Please enter EXACTLY %d letters\r\n\r>>>>>>> ",i);
			while(Terminal->TxStatus != Completed);
			str = usartRead_DMA(Terminal, i+1);
			while(Terminal->RxStatus != Completed);
			print(Terminal,"\r[Echo]: %s\r\n",str);
			while(Terminal->TxStatus != Completed);
		}
	printf_u("\r======================End==========================\r\n");
}
//Treat it as the main() of this tester file 
void testUsart(void) {
	/*In the above code, "MainTerminal = newMainUSART(&huart2);" 
	 *assigns huart2 to the private struct named DefaultUSART in
     *THL_Usart.c. In case the user want to access this struct directly,
	 *just declare it with "extern USART DefaultUSART;" 
	 **/
	/*The default buffer size for both Tx & Rx is 100
	 *If wanting to expand this size limit when there is 
	 *enough RAM size, redefine the macros with the following code:
	 *
	 *#undef TxBuffer_Size
	 *#undef RxBuffer_Size
	 *#define TxBuffer_Size ???
     *#define RxBuffer_Size ???
	 *
	 *Replace ??? with the number of desire buffer sizes*/
	Led = newGPIO(&led_pin, LD2_GPIO_Port, LD2_Pin);
	testPrints();
	testScans();
}
void Exception_Handler(const char* str) {
	USART* Terminal = newUSART(&uart2, &huart2);
	print(Terminal, "\r%s\r\n",str);
}

