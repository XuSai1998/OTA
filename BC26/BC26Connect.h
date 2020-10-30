#ifndef _BC26CONNECT_H_
#define _BC26CONNECT_H_

#include "stm32f10x.h"
#include "bc26.h"

typedef enum{
	YES=1,
	NO=!YES
}FLAG;//typedef ����˼����FLAG����ʾǰ�������

extern FLAG Updata_flag;


//д��汾�ŵ���ʼ��ַ
#define Version_WRITE_ADDR  ((uint32_t)0x08009000)
//#define WRITE_END_ADDR    ((uint32_t)0x0800C000)

void Set_Version(uint8_t  *Version);
void Get_Version(char *Version);


void BC26_Connect_Net();


FLAG BC26_Updata_CMD();
void BC26_Download();
void Write_To_Flash();

#endif
