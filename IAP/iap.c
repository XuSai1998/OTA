#include "iap.h"
//include "sys.h"
#include "stm32flash.h"

#include "usart.h"


iapfun jump2app; 
u16 iapbuf[1024];   

//������תǰ�ر���BootLoader�п������ж��Լ�����
static void Close_NVIC_and_GPIO()
{
//	//����USART�жϷ�ʽ
	USART_ITConfig(USART1,USART_IT_RXNE,DISABLE);
	USART_ITConfig(USART2,USART_IT_RXNE,DISABLE);
	USART_ITConfig(USART3,USART_IT_RXNE,DISABLE);
	USART_Cmd(USART1,DISABLE);
	USART_Cmd(USART3,DISABLE);
	USART_Cmd(USART2,DISABLE);
	USART_DeInit(USART1);
	USART_DeInit(USART2);
	USART_DeInit(USART3);

	GPIO_DeInit(GPIOA);
	GPIO_DeInit(GPIOB);
	GPIO_DeInit(GPIOC);
	GPIO_DeInit(GPIOD);
	
	RCC_DeInit();
}


/*
�����ܵ���Bin�ļ�����д��FLASH��
//appxaddr:Ӧ�ó������ʼ��ַ
//appbuf:Ӧ�ó���CODE.
//appsize:Ӧ�ó����С(�ֽ�).
*/
void iap_write_appbin(u32 appxaddr,u8 *appbuf,u32 appsize)
{
	u16 t;
	u16 i=0;
	u16 temp;
	u32 fwaddr=appxaddr;//��ǰд��ĵ�ַ
	u8 *dfu=appbuf;
	for(t=0;t<appsize;t+=2)
	{						    
		temp=(u16)dfu[1]<<8;
		temp+=(u16)dfu[0];	  
		dfu+=2;//ƫ��2���ֽ�
		iapbuf[i++]=temp;	    
		if(i==1024)
		{
			i=0;
			STMFLASH_Write(fwaddr,iapbuf,1024);	
			fwaddr+=2048;//ƫ��2048  16=2*8.����Ҫ����2.
		}
	}
	if(i)STMFLASH_Write(fwaddr,iapbuf,i);//������һЩ�����ֽ�д��ȥ.  
}

//APP��BOOT�໥��ת���ڽ���APPǰ��Ҫ�ر��жϡ�����Ȳ�������Ҫ��ʱ����д��
//ADDRҪ��ת���ĵ�ַ��APP��BOOT����ͨ�ã�:APP_ADDR\IAP_ADDR	
void IAP_APP_Jump (uint32_t ADDR)
{
	uint32_t  SpInitVal; //Ҫ��ת�������SP��ֵ.

	uint32_t  JumpAddr; //Ҫ��ת������ĵ�ַ.��,Ҫ��ת����������

	void (*pFun)(void); //����һ������ָ��.����ָ��APP�������

	Close_NVIC_and_GPIO();
	//RCC_DeInit(); //�ر�����
	
	//NVIC_DeInit ();                           //�ָ�NVICΪ��λ״̬.ʹ�жϲ��ٷ���.

	
	
	SpInitVal = *(uint32_t *)ADDR;//IAP_ADDR IAP��ջ����ַ��0x08000000��

	//��ת��APPʱ APP_ADDR AAP��ջ����ַ���磺0x08003800��

	JumpAddr = *(uint32_t *)( ADDR + 4); //���ø�λ�ж����������������̷�����

	__set_MSP(SpInitVal); //����SP.����ջջ����ַ

	pFun = (void (*)(void))JumpAddr; //������ת����.����λ�ж�������ַ��Ϊ����ָ��

	(*pFun) (); //ִ�к�����ʵ����ת.���ٷ���.

}

