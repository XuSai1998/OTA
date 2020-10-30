#ifndef __USART_H
#define __USART_H
#include "stdio.h"
#include "string.h"
#include "sys.h" 


extern	char 	DebugBuffer[100];
extern 	uint32_t  DebugBufferCounter;




//DebugÓÃµÄ´®¿ÚUsart1
void DebugUsart_init(u32 bound);
void DebugUsart_SendData(u8 Data);
void DebugUsart_SendStr(char*SendBuf);

void CleanDebugBuffer(void);
#endif


