/*
�ⲿ�����ڶ�BC26��ʹ�õĴ��ڽ��г�ʼ��
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

//����������ݴ���������
char Buffer[30*1024]	__attribute__ ((at(0X20001000)));//���ջ���,���USART_REC_LEN���ֽ�,��ʼ��ַΪ0X20001000(��ֹSRAM����)
char BC26_Buffer[1500];
uint32_t BC26BufferCounter;


/*
�����BC26�õĴ�����USART2
stm32������
TX--------PA2
RX--------PA3
*/
void BC26_Usart_init(u32 bound)
{
		GPIO_InitTypeDef GPIO_InitStructure;
		USART_InitTypeDef USART_InitStructure;
		NVIC_InitTypeDef NVIC_InitStructure;
		 
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	//ʹ��USART2��GPIOAʱ��
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);	//ʹ��USART2ʱ��
		
		//USART2_TX   GPIOA.2
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //PB10
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
		GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA.2
		 
		//USART2_RX	  GPIOA.3��ʼ��
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;//PA3
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
		GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA.3

		//Usart2 NVIC ����
		NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0 ;//��ռ���ȼ�3
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		//�����ȼ�3
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
		NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
		
		
		//USART ��ʼ������
		USART_InitStructure.USART_BaudRate = bound;//���ڲ�����
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
		USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
		USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
		USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
		USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

		USART_Init(USART2, &USART_InitStructure); //��ʼ������3
		USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//�������ڽ����ж�
		USART_Cmd(USART2, ENABLE);              //ʹ�ܴ���1 
}


void BC26_Usart_SendData(u8 Data)
{
		 USART_SendData(USART2,Data);	  //����һ���ֽ�����
		 while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=1);  //�ȴ��������ݼĴ���Ϊ��
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


void USART2_IRQHandler(void)                	//����3�жϷ������
{
		if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)  //�����ж�(���յ������ݱ�����0x0d 0x0a��β)
		{	
			BC26_Buffer[BC26BufferCounter++] = USART_ReceiveData(USART2);			//�����ڽ��յ������ݴ����������	
    } 
}



