#include "iap.h"
//include "sys.h"
#include "stm32flash.h"

#include "usart.h"


iapfun jump2app; 
u16 iapbuf[1024];   

//用于跳转前关闭在BootLoader中开启的中断以及外设
static void Close_NVIC_and_GPIO()
{
//	//设置USART中断方式
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
将接受到的Bin文件内容写到FLASH钟
//appxaddr:应用程序的起始地址
//appbuf:应用程序CODE.
//appsize:应用程序大小(字节).
*/
void iap_write_appbin(u32 appxaddr,u8 *appbuf,u32 appsize)
{
	u16 t;
	u16 i=0;
	u16 temp;
	u32 fwaddr=appxaddr;//当前写入的地址
	u8 *dfu=appbuf;
	for(t=0;t<appsize;t+=2)
	{						    
		temp=(u16)dfu[1]<<8;
		temp+=(u16)dfu[0];	  
		dfu+=2;//偏移2个字节
		iapbuf[i++]=temp;	    
		if(i==1024)
		{
			i=0;
			STMFLASH_Write(fwaddr,iapbuf,1024);	
			fwaddr+=2048;//偏移2048  16=2*8.所以要乘以2.
		}
	}
	if(i)STMFLASH_Write(fwaddr,iapbuf,i);//将最后的一些内容字节写进去.  
}

//APP或BOOT相互跳转（在进入APP前需要关闭中断、外设等操作，需要的时候再写）
//ADDR要跳转到的地址（APP和BOOT程序通用）:APP_ADDR\IAP_ADDR	
void IAP_APP_Jump (uint32_t ADDR)
{
	uint32_t  SpInitVal; //要跳转到程序的SP初值.

	uint32_t  JumpAddr; //要跳转到程序的地址.即,要跳转到程序的入口

	void (*pFun)(void); //定义一个函数指针.用于指向APP程序入口

	Close_NVIC_and_GPIO();
	//RCC_DeInit(); //关闭外设
	
	//NVIC_DeInit ();                           //恢复NVIC为复位状态.使中断不再发生.

	
	
	SpInitVal = *(uint32_t *)ADDR;//IAP_ADDR IAP的栈顶地址（0x08000000）

	//跳转到APP时 APP_ADDR AAP的栈顶地址（如：0x08003800）

	JumpAddr = *(uint32_t *)( ADDR + 4); //设置复位中断向量（如上面流程分析）

	__set_MSP(SpInitVal); //设置SP.，堆栈栈顶地址

	pFun = (void (*)(void))JumpAddr; //生成跳转函数.将复位中断向量地址做为函数指针

	(*pFun) (); //执行函数，实现跳转.不再返回.

}

