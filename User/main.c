#include "stm32f10x.h"
#include "usart.h"
#include "delay.h"
#include "bc26.h"
#include "BC26Connect.h"

int main(void)
{
	
	char Version[10],i;
	char crc[100]="FFFE0113000000110056322E31300000000000000000000000";
	
	
	delay_init();
  NVIC_Configuration();
	DebugUsart_init(115200);
	BC26_Usart_init(115200);
	
	printf("开始\n");
	delay_ms(100);
	CleanDebugBuffer();
	CleanBC26Buffer();
	
	BC26_Usart_SendCmd("AT\r\n");
	delay_ms(100);
	printf("返回%s\n",BC26_Buffer);

	BC26_Connect_Net();
	
	Set_Version("2.10");

	
	
	
	BC26_Updata_CMD();
	
	BC26_Download();
	Write_To_Flash();
	
	
	
	
	
//	Get_Version(Version);
//	for(i=0;i<4;i++)
//		printf("版本号是%c\n",Version[i]);
	
//	BC26_CRC(crc);//测试CRC功能
	
//	printf("%s",crc);
	
	
	while(1)
	{

	}
		
	
	
}




