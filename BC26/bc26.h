#ifndef _BC26_H_
#define _BC26_H_

#include "stm32f10x.h"


extern char BC26_Buffer[1500];
extern uint32_t BC26BufferCounter;

extern char Buffer[30*1024];


void BC26_Usart_init(u32 bound);
void BC26_Usart_SendData(u8 Data);
void BC26_Usart_SendCmd(u8 *Data);
void CleanBC26Buffer(void);






#endif

