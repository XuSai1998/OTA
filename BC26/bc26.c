/*
这部分用于对BC26所使用的串口进行初始化
*/

#include "bc26.h"
#include "stdio.h"
#include "string.h"
#include "sys.h" 




/*
#define LOG_TYPE1(format, ...) do{ 		\
		printf(format, __VA_ARGS__); 	\
										\
} while(0)
*/

//这个是用来暂存升级包的
char Buffer[30*1024]	__attribute__ ((at(0X20001000)));//接收缓冲,最大USART_REC_LEN个字节,起始地址为0X20001000(防止SRAM里面)
char BC26_Buffer[1500];
uint32_t BC26BufferCounter;


/*
这里的BC26用的串口是USART2
stm32的引脚
TX--------PA2
RX--------PA3
*/
void BC26_Usart_init(u32 bound)
{
		GPIO_InitTypeDef GPIO_InitStructure;
		USART_InitTypeDef USART_InitStructure;
		NVIC_InitTypeDef NVIC_InitStructure;
		 
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	//使能USART2，GPIOA时钟
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);	//使能USART2时钟
		
		//USART2_TX   GPIOA.2
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //PB10
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
		GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.2
		 
		//USART2_RX	  GPIOA.3初始化
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;//PA3
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
		GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.3

		//Usart2 NVIC 配置
		NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0 ;//抢占优先级3
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		//子优先级3
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
		NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
		
		
		//USART 初始化设置
		USART_InitStructure.USART_BaudRate = bound;//串口波特率
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
		USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
		USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
		USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
		USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

		USART_Init(USART2, &USART_InitStructure); //初始化串口3
		USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//开启串口接受中断
		USART_Cmd(USART2, ENABLE);              //使能串口1 
}


void BC26_Usart_SendData(u8 Data)
{
		 USART_SendData(USART2,Data);	  //发送一个字节数据
		 while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=1);  //等待发送数据寄存器为空
}

void BC26_Usart_SendCmd(u8 *Data)
{
		u8 i;
    for(i=0;Data[i]!='\0';i++)
    {
        USART_SendData(USART2, Data[i]);
			  //Usart2_SendData(Data[i]);
        while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);//?????? 
    }
}
void CleanBC26Buffer(void)
{
		memset(BC26_Buffer,0,1500);
		BC26BufferCounter = 0;
		
}


void USART2_IRQHandler(void)                	//串口3中断服务程序
{
		if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
		{	
			BC26_Buffer[BC26BufferCounter++] = USART_ReceiveData(USART2);			//将串口接收到的数据存放在数组中	
    } 
}



