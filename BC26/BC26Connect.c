#include "BC26Connect.h"
#include "bc26.h"
#include "usart.h"
#include "delay.h"
#include "string.h"
#include "crc16.h"
#include "iap.h"

FLAG Updata_flag;

uint32_t Package_size=0,Package_num=0,Bin_size=0;//Package_sizeÿ�����Ĵ�С,Package_num��������,Bin_size�洢����Bin�ļ���С
char UpData_Version[3];//�ƶ˲�Ʒ�İ汾,��Ϊ��Vx.xx�ĸ�ʽ��������ʮ�������ַ���������Ҳ����V��0x3*��.��0x3*����0x3*����������Ϊ�˷���ֻ�洢�����*��һ���ֽڵ���
/*
	�ⲿ����Ҫ����BC26�͵���������
*/
//#define BC26_Usart_Send(fm,...) BC26_Usart_SendCmd(fm,__VA_ARGS__) 

void BC26_Connect_Net()
{
	int i;
	__IO char  IMEI[20];
	char *BC26_IMEI;//�����洢BC26��IMEI��
	
	CleanBC26Buffer();
	//1.�ȳ�ʼ��BC26����ֹ��APP�����ж�Boot������BC26�����и���
	BC26_Usart_SendCmd("AT+QRST=1\r\n");//��λ
	delay_ms(1000);
	delay_ms(1000);
	//2.���Թ����Ƿ�����
	CleanBC26Buffer();
	BC26_Usart_SendCmd("AT\r\n");
	delay_ms(1000);
	printf("����AT�󷵻�%s\n",BC26_Buffer);
	
	//3.
	CleanBC26Buffer();
	BC26_Usart_SendCmd("AT+QSCLK=1\r\n");//��ֹBC26��������״̬
	delay_ms(300);

	BC26_Usart_SendCmd("AT+CPSMS=0\r\n");//��ֹ����PSM���͹��ģ�
	delay_ms(300);
	
	CleanBC26Buffer();
	BC26_Usart_SendCmd("AT+CGSN=1\r\n");//��ѯIMEI����
	delay_ms(300);

/*****
������strstr��ԭ���ǣ���ֵ���ǵ�ַ�������ǣ��ַ��������Ե�����������Ķ���һֱ�ڱ�
BC26_IMEI=strstr(BC26_Buffer,"+CGSN:");
*****/
	BC26_IMEI=strstr(BC26_Buffer,"+CGSN:");
	BC26_IMEI=BC26_IMEI+7;//����7Ϊ��ʹ����ָ��IMEI��һλ
	
	for(i=0;i<15;i++)
		IMEI[i]=BC26_IMEI[i];
	IMEI[15]='\0';
	
	printf("IMEI����%s\n",IMEI);
	
	delay_ms(300);

	BC26_Usart_SendCmd("AT*MICCID\r\n");//��ѯSIM����
	delay_ms(300);

	CleanBC26Buffer();
	BC26_Usart_SendCmd("AT+CESQ\r\n");//��ǰ����״̬
	delay_ms(500);
	printf("��ǰ����״̬%s\n������Ϊrsrq,��Χ0-34,��ֵԽ���ź�Խ��\n",BC26_Buffer);
	
	CleanBC26Buffer();
	BC26_Usart_SendCmd("AT+CGPADDR=1\r\n");//��ѯIP��ַ��ȷ�������ɹ�
	delay_ms(300);
	printf("��ǰIP��ַΪ%s\n",BC26_Buffer);
	
	
	CleanBC26Buffer();
	BC26_Usart_SendCmd("AT+CGATT?\r\n");//��ѯ���總��״̬
	delay_ms(300);
	printf("��ǰ���總��״̬%s\n1��ʾ���ųɹ���0��ʾδ���ųɹ�\n",BC26_Buffer);

	CleanBC26Buffer();
	BC26_Usart_SendCmd("AT+QLWSERV=\"180.101.147.115\",5683\r\n");//���ӵ�����ƽ̨IP��ַ�Ͷ˿ںţ�����ƽ̨�ɲ鿴
	delay_ms(1000);
	printf("���ӵ�����״̬%s\n",BC26_Buffer);
	

	CleanBC26Buffer();
	BC26_Usart_SendCmd("AT+QLWCONF=\"");
	BC26_Usart_SendCmd(IMEI);///����IOTƽ̨ע���IMEI��
	BC26_Usart_SendCmd("\"\r\n");
	//BC26_Usart_Send("AT+QLWCONF=\"%s\"\r\n",BC26_IMEI);
	delay_ms(1000);
	delay_ms(1000);
	printf("ƽ̨ע�����%s\n",BC26_Buffer);
	
	CleanBC26Buffer();
	BC26_Usart_SendCmd("AT+QLWADDOBJ=19,0,1,\"0\"\r\n");//��������ͨ��
	delay_ms(1000);

	BC26_Usart_SendCmd("AT+QLWADDOBJ=19,0,1,\"0\"\r\n");//��������ͨ��
	delay_ms(1000);

	CleanBC26Buffer();
	BC26_Usart_SendCmd("AT+QLWOPEN=0\r\n");//�豸ע�ᣬ��ʱ�豸Ӧ����������
	delay_ms(1000);
	printf("ƽ̨�������%s\n",BC26_Buffer);
	
	
	CleanBC26Buffer();
	BC26_Usart_SendCmd("AT+QLWCFG=\"dataformat\",1,1\r\n");//���÷��ͺͽ������ݸ�ʽΪʮ�����Ƶ��ַ���ģʽ
	delay_ms(1000);
	
//	CleanBC26Buffer();
//	BC26_Usart_SendCmd("AT+QLWDATASEND=19,0,0,1,31,0x0100\r\n");//���ͺͽ������ݲ��ԣ����յ�SEND OK�������ʾ���ݳɹ����͵���վ��
////	BC26_Usart_SendCmd("AT+QLWDATASEND=19,0,0,5,68656C6C6F,0x0000\r\n");//���ͺͽ������ݲ��ԣ�û���Ƿ��͵���վȷ�ϣ�
//	delay_ms(1000);
}

//��ȡ����STM32��Ʒ�İ汾��
//����İ汾�Ŵ����ڲ�FLASH���棬�����ַ���"Vx.xx"��ʽ�洢(����Ϊ�˴ճ�һ��2�ı�����V�ַ�ɾ����)
void Get_Version(char *Version)
{ 

	uint32_t Address = 0x00;				//��¼д��ĵ�ַ
	int i;
	Address = Version_WRITE_ADDR ;

	for(i=0;i<4;i++)
		Version[i]=(*(__IO uint32_t*) (Address+i));
	
	
}
//����Ʒ�İ汾д�뵽FLASH��
void Set_Version(uint8_t *Version)
{
	uint32_t Addres = 0x00;				//��¼д��ĵ�ַ
	int i;
	Addres = Version_WRITE_ADDR;
	
	FLASH_Unlock();
	FLASH_ErasePage(Addres);
	for(i=0;i<2;i++)
		FLASH_ProgramHalfWord((Addres+i*2),(Version[i*2])+(Version[i*2+1]<<8));

	FLASH_Lock();
}



/*
�������̣�
Ҫ����ƽ̨���ϴ����°������úø��������
���ܿ�ʼ
�����ʱ�����ϴ����ݣ���ô��ƽ̨����ҷ���FFFE01134C9A0000����ʾ��ѯ�豸�汾

�ú������ڻ�ȡ���°�ǰ������
�����ƶ����ڵİ汾������
�Լ���ȡ��ƽ̨������������С
*/
FLAG BC26_Updata_CMD()
{
	
	char *strx,cmd[100],cmd_len[3],len;//cmd����Ҫ�ϴ���ָ����л�����CRCУ��λ��
	char version[10],i;
	//uint32_t Package_size,Package_num;
	char Package_size_str[4],Package_num_str[4];
	
	
	CleanBC26Buffer();
	BC26_Usart_SendCmd("AT+QLWDATASEND=19,0,0,1,31,0x0100\r\n");//���ͺͽ������ݲ��ԣ����յ�SEND OK�������ʾ���ݳɹ����͵���վ��
	delay_ms(1000);
	delay_ms(1000);
	delay_ms(1000);
	delay_ms(1000);
	delay_ms(1000);
//	delay_ms(1000);
	strx=strstr(BC26_Buffer,"SEND OK");
	printf("����汾��ѯ\n");
	if(!strx)
	{
		printf("�������Ӳ��ɹ�\n");
		//��������ٴ����µ���ǰ���BC26_Connect_Net()��������������
	}else
	{
		
		
	//����豸�Ѿ�������ͨ�У���ʱ��Ӧ���ǽ��յ��ˣ��ƶ������豸�汾��ָ��
		printf("����ɹ�\n");
		printf("%s\n",BC26_Buffer);
		//����ָ��ֻ���ϴ��˸��°��󣬵�һ���ϴ�ָ���ʱ������յ�
		//��ʱ���ղ�����ӦΪ�豸�ϴ������ݣ���������ָ���ƶ��Ѿ����·����ˣ���ô���ڶ��ξ��ղ�����
		strx=strstr(BC26_Buffer,"FFFE01134C9A0000");
		
		//��ʵ��Ӧ���п��ܻ�ӦΪAPP������ƶ˽���̫�࣬�����´����������Ϣ��
		//�������ƶ�Ӧ�ÿ������ã�������³����ſ�ʼ�ϴ����°�
		//�����Ͳ���©����ָ��
		//����汾��Ϊ�˷�����ԣ������о�û�мӶ�ȷ���յ���ָ��Ĵ���
		//����Ӧ�ð�if����ĺ���д��if����
//		if(strx)		
//		{
//		}
		//�ҿ���һ��RCT6���Ǹ����������и�FLASH��W25Q16������ʵ���԰Ѹ��µ�����д���ⲿFLASH����
		//�о����Ը�����Ҫ�����ã�����������صĳ�����BootLoader����Ļ����Ϳ��԰Ѹ��µĳ���д��SRAM����
		//����ʡǮ������������������صĳ���д��APP��������Ļ����Ϳ������ⲿFLASH����������Ƶ�IAP����
		
		//�Ȳ�����Щ����ֱ�ӰѸ��³���д��SRAM���棬�յ�����ָ�����Ҫ�ϴ����ڲ�Ʒ����İ汾
		//��������ͳһ�汾��ʽ��Ϊ Vx.xx �磨V2.16��
		//���ϱ�STM32�������ڵĳ���汾
		Get_Version(version);//��ȡ��ǰ��Ʒ�İ汾��
		/*
		FFFE0113164700110056322E31300000000000000000000000
		FFFE����ʼ��ʶ��01���汾�ţ�13����Ϣ�룩1647��У���룩0011�����������ȣ�00 (�����) 
		56322E31300000000000000000000000����ǰ�汾�ţ�V2.10���涨����Ϊbyte[16]��
		*/
		strcpy(cmd,"FFFE0113000000110056322E31300000000000000000000000");
		
		//�����ڲ�Ʒ����İ汾д���ַ�������
		cmd[21]=version[0];
		cmd[25]=version[2];
		cmd[27]=version[3];
		
		BC26_CRC(cmd);//�õ���У�����CMD
		printf("CRC��%s\n",cmd);
	
		len=strlen(cmd)/2;	
		myitoa(len,cmd_len,10);

		CleanBC26Buffer();
		
		printf("�ֳ�Ϊ%s\n",cmd_len);
		//������BC26�ϱ����ݵĹ̶���ʽ
		BC26_Usart_SendCmd("AT+QLWDATASEND=19,0,0,");
		BC26_Usart_SendCmd(cmd_len);
		BC26_Usart_SendCmd(",");
		BC26_Usart_SendCmd(cmd);
		BC26_Usart_SendCmd(",0x0100\r\n");
		//�ϱ���汾���ƶ˻��·���ǰ�ƶ˵İ汾
		//�Լ���Ƭ�����ͷ�Ƭ��С��
		printf("\n%s\n",BC26_Buffer);
		
		//�ȴ��ƶ��·�����
		strx=strstr(BC26_Buffer,"QLWDATARECV:");
		
		
		//���������Ҫ�޸Ĳ�Ȼ�����ƶ�û�и��°���ʱ��Ῠ��������
		while(strx==NULL)
		{
			strx=strstr(BC26_Buffer,"QLWDATARECV:");
			delay_ms(1000);
		}
	
	/*
	�����ʱ��ƽ̨���ϴ����������json�ļ��ж���İ汾��ΪV2.16��
	�豸���յ���Ϣ������FFFE011491B0001656322E3136000000000000000000000001F400813836��
	01F4Ϊ��Ƭ����С��500�ֽڣ���0081Ϊ��Ƭ������3836ת�����ַ�����Ϊjson�ļ��е�versioncheckcode�ֶ�ֵ��
		
	����Ҫ���ƶ˵İ汾�Ŵ洢��������������ĳ����������ݰ�
		*/
	
		strx=strstr(BC26_Buffer,"19,1,0,30,FFFE");
		strx=strstr(strx,"FFFE");
		printf("�ַ���Ϊ��%s",strx);
		//ӦΪ�ǹ̶���ʽ�����Ի�ȡָ��λ�õİ���С���Ͱ��������ַ�����ʽ��
		Package_size_str[0]=strx[48];
		Package_size_str[1]=strx[49];
		Package_size_str[2]=strx[50];
		Package_size_str[3]=strx[51];
		
		Package_num_str[0]=strx[52];
		Package_num_str[1]=strx[53];
		Package_num_str[2]=strx[54];
		Package_num_str[3]=strx[55];
		
		//�汾��
		UpData_Version[0]=strx[19];
		UpData_Version[1]=strx[23];
		UpData_Version[2]=strx[25];
		printf("�ƶ˰汾��ΪV3%c.3%c3%c\n",UpData_Version[0],UpData_Version[1],UpData_Version[2]);
		
		
		
		for(i=0;i<4;i++)
		{
			if(Package_size_str[i]<='9') Package_size_str[i]=Package_size_str[i]-'0';
			else {
				Package_size_str[i]=Package_size_str[i]-'A'+10;
			}
			Package_size=Package_size*16+Package_size_str[i];
			
			if(Package_num_str[i]<='9') Package_num_str[i]=Package_num_str[i]-'0';
			else {
				Package_num_str[i]=Package_num_str[i]-'A'+10;
			}
			
			Package_num=Package_num*16+Package_num_str[i];
		}
		
		//��ʱ����������ݰ���С����ָ����󣬲������Ժ����Ż�
		//����ܼ��Σ�Ȼ���жϰ���С�������ǲ��Ƿ��Ϲ涨
		printf("����С%x,����%x",Package_size,Package_num);
		
		CleanBC26Buffer();
//		delay_ms(1000);
//�����ƶˣ�ȷ��Ҫ����
		BC26_Usart_SendCmd("AT+QLWDATASEND=19,0,0,9,FFFE0114D768000100,0x0000\r\n");
		delay_ms(1000);
		
		//�������Ϳ��Կ�ʼ�������ݰ���

		
	}
	
}


//����������������
//������һ���������Ѿ���ȡ����ÿ�������������Լ���С
//����Ϳ�ʼ��ȡ������
void BC26_Download()
{
	int i,j,num=0;
	char cmd[100],num_str[4];
	int Package_num_temp=0,TimeOut=0;
	char *strx;
	uint32_t addr=0;
	//��Ϊ�ƶ��·�����ʮ�����Ƶ��ַ�"FF",��Ҫת���ɶ������ַ�
	//��������������洢��������������
	u8 UpdataBin_Buffer[Package_size];
//	u8 Buffer[Package_size*Package_num]	__attribute__ ((at(0X20001000)));//���ջ���,�����ֽ�,��ʼ��ַΪ0X20001000(��ֹSRAM����)		

	
	printf("����С%d,����%d\n",Package_size,Package_num);
	CleanBC26Buffer();
	//Package_num������һ�������Լ���ȡ���˰�������
	Bin_size=0;//���������ļ���С
	for(Package_num_temp=0;Package_num_temp<Package_num;Package_num_temp++)
	{
		
	//	printf("��%d����������\n",Package_num_temp);
		strcpy(cmd,"FFFE01150000001256302E303000000000000000000000000000");
		cmd[48]=(Package_num_temp/16/16/16<=9)?(Package_num_temp/16/16/16+'0'):(Package_num_temp/16/16/16+'A'-10);
		cmd[49]=(Package_num_temp/16/16%16<=9)?(Package_num_temp/16/16%16+'0'):(Package_num_temp/16/16%16+'A'-10);
		cmd[50]=(Package_num_temp/16%16<=9)?(Package_num_temp/16%16+'0'):(Package_num_temp/16%16+'A'-10);
		cmd[51]=(Package_num_temp%16<=9)?(Package_num_temp%16+'0'):(Package_num_temp%16+'A'-10);
		
		//���ƶ˵İ汾��д�뵽cmd����
		cmd[19]=UpData_Version[0];
		cmd[23]=UpData_Version[1];
		cmd[25]=UpData_Version[2];
	
		
		printf("��%c%c%c%c����������",cmd[48],cmd[49],cmd[50],cmd[51]);
		
		BC26_CRC(cmd);//�õ���У�����CMD
		//������BC26��ȡ���°��Ĺ̶���ʽ
		CleanBC26Buffer();
		BC26_Usart_SendCmd("AT+QLWDATASEND=19,0,0,26,");
		BC26_Usart_SendCmd(cmd);
		BC26_Usart_SendCmd(",0x0100\r\n");
	
		printf("�������ݰ���cmd��%s\n",cmd);
		//�����и��������ҽ��յ��������кܶ�
		//���˰���������ݣ����з�����һ��ָ���ʱ�򷵻ص�����
		//����ֱ����strstr��ȡFFFE����
		//Ҫ��һ������
		/****************+QLWDATARECV����Ǹ���BC26д�ģ���������Ҫ��*********************/
		strx=strstr(BC26_Buffer,"QLWDATARECV");//��ʱ��Buffer������û��+QLWDATARECV��
		while(strx==NULL)
		{
			//�������ʱ����жϽ�ס�ַ���
			//�������ִ�������ȱ�ݣ���Ӧ������ʱ����������ʱ����ܵ���ʱ��ܳ��������жϣ���
			//Ӧ����һ��ʱ����û�н��յ��ַ����ж�
			strx=strstr(BC26_Buffer,"QLWDATARECV");
			delay_ms(1000);
			delay_ms(1000);
			delay_ms(1000);
			delay_ms(1000);
		}
		delay_ms(1000);
		delay_ms(1000);
		//�Ȱ��ַ����ж���ȫ����ס���ٽ��д���	
		//�ҵ���Ч�ַ���λ��
		strx=strstr(strx,"FFFE");
	//	printf("���յ�����������%s\n",strx);
		
		//��ȷ����Ҫ������Ч�ֽ�����(����num����)
		num_str[0]=strx[12];
		num_str[1]=strx[13];
		num_str[2]=strx[14];
		num_str[3]=strx[15];
		
	//	printf("��Ƭ��%c%c%c%c",num_str[0],num_str[0],num_str[0],num_str[0]);
		num=0;
		for(i=0;i<4;i++)
		{
			if(num_str[i]<='9') num_str[i]=num_str[i]-'0';
			else {
				num_str[i]=num_str[i]-'A'+10;
			}
			num=num*16+num_str[i];
		}
				
	
		//�����ȥ������Ϊ
		num=num-3;//��ʱ��numӦ����bin�ļ�����Ч�ֽ�
		printf("�ð���Ч�ֽ�Ϊ��%d\n",num);
		//��Ϊ�·�����ʮ�������ַ����Σ�������Ҫ��bin�ļ�Ӧ���Ƕ����Ƶ�
		//����Ҫ���д���
		//����ÿ���·����Ĵ�С���ܲ�һ��������Ҫ����Ҫ��������
		//������strx������
		strx=strx+22;
		printf("���յ����������ݰ���%s\n",strx);
		
		i=0;
		while(strx[2*i]!='\0'||strx[2*i+1]!='\0')
		{
			//���ַ����ϲ���16���Ƶ��ַ���
			if(strx[2*i]<='9') strx[2*i]=strx[2*i]-'0';
			else {
				strx[2*i]=strx[2*i]-'A'+10;
			}
			if(strx[2*i+1]<='9') strx[2*i+1]=strx[2*i+1]-'0';
			else {
				strx[2*i+1]=strx[2*i+1]-'A'+10;
			}
			
			UpdataBin_Buffer[i]=((strx[2*i])<<4)+strx[2*i+1];
			i++;
		}
	
		printf("���յ��ַ�����Ϊ��%d\n",i-1);
		//�ж��Ƿ����ݶ������������������������һ�������
		if(i<num)
		{
			TimeOut++;
			printf("���ݶ���,��������һ��\n"); 
			Package_num_temp--;
		}	
		
		//�������������೬ʱ�ͽ���
		if(TimeOut>=2)
		{
			printf("���ݰ������⣬����ʱ\n");
			break;
		}
		
		
		//���ڰѽ��յ�����ȫ���ŵ�SRAM����
		for(i=0;i<num;i++)
		{
			Buffer[addr++]=UpdataBin_Buffer[i];
		}
		
		Bin_size=Bin_size+num;
		
	}
	
}


/*
	��Binд�뵽FLASH���棬Ȼ����ת����
*/
void Write_To_Flash()
{
	char *str,cmd[100],len,cmd_len[3],Newest_Version[4];
	CleanBC26Buffer();
	//�豸�������Ƭ���󣬰���Э���豸���ϴ�һ����Ϣ������ѯ�豸������������״̬
	delay_ms(1000);
	BC26_Usart_SendCmd("AT+QLWDATASEND=19,0,0,9,FFFE0116850E000100,0x0000\r\n");
	delay_ms(1000);
	delay_ms(1000);
	
	/*
		�ϰ������������ƽ̨���·�
		+QLWDATARECV: 19,1,0,9,FFFE0116850E000100
		+QLWDATARECV: 19,1,0,8,FFFE0117CF900000
		ƽ̨Ҳ��ת����downloaded
	*/
	//��ʵ��Ҫ�����ⲿ��Ҳ���ԣ�ֱ���ϱ��������ֱ�������Ϳ�����
//	str=strstr(BC26_Buffer,"FFFE01134C9A0000");
//	while(str==NULL)
//	{
//			str=strstr(BC26_Buffer,"FFFE01134C9A0000");
//			delay_ms(1000);

//	}
//	
	delay_ms(1000);
	//�ϱ�����ʾ��Ʒ��ʼ������
	BC26_Usart_SendCmd("AT+QLWDATASEND=19,0,0,9,FFFE0117B725000100,0x0000\r\n");	
	//���յ�֮���Ʒ��ʼ����
	//�յ����豸������������
	//Ϊ�˷�ֹ��������ϣ��ȹر����ж�
	__disable_irq();
	
	iap_write_appbin(FLASH_APP_ADDR,Buffer,Bin_size);//����FLASH����   
	delay_ms(100);
	
	__enable_irq();
	strcpy(cmd,"FFFE0118000000110056302E30300000000000000000000000");
		
	//�����ڲ�Ʒ����İ汾д���ַ�������
	cmd[21]=UpData_Version[0];
	cmd[25]=UpData_Version[1];
	cmd[27]=UpData_Version[2];
		
	BC26_CRC(cmd);//�õ���У�����CMD
	printf("CRC��%s\n",cmd);
	
	len=strlen(cmd)/2;	
	myitoa(len,cmd_len,10);

	CleanBC26Buffer();
		
	printf("�ֳ�Ϊ%s\n",cmd_len);
	//������BC26�ϱ����ݵĹ̶���ʽ
		BC26_Usart_SendCmd("AT+QLWDATASEND=19,0,0,");
		BC26_Usart_SendCmd(cmd_len);
		BC26_Usart_SendCmd(",");
		BC26_Usart_SendCmd(cmd);
		BC26_Usart_SendCmd(",0x0100\r\n");
		//�ϱ���汾���Ͱ�������İ汾����д�뵽FLASH����Ϊ������������׼��
		
		//�����°汾��д�뵽Flash��
		
	Newest_Version[0]=UpData_Version[0];
	Newest_Version[1]='.';
	Newest_Version[2]=UpData_Version[1];
	Newest_Version[3]=UpData_Version[2];
	Set_Version(Newest_Version);
	
	
	Get_Version(Newest_Version);
	printf("���°汾��%s\n",Newest_Version);
	
	//������ɣ���תǰ���Ϸ��������
	IAP_APP_Jump (APP_ADDR);
	
	
	
	
}

