#include "BC26Connect.h"
#include "bc26.h"
#include "usart.h"
#include "delay.h"
#include "string.h"
#include "crc16.h"
#include "iap.h"

FLAG Updata_flag;

uint32_t Package_size=0,Package_num=0,Bin_size=0;//Package_size每个包的大小,Package_num包的数量,Bin_size存储整个Bin文件大小
char UpData_Version[3];//云端产品的版本,因为是Vx.xx的格式，而且是十六进制字符串，所以也就是V“0x3*”.“0x3*”“0x3*”，我这里为了方便只存储后面的*那一个字节的数
/*
	这部分主要用于BC26和电信云连接
*/
//#define BC26_Usart_Send(fm,...) BC26_Usart_SendCmd(fm,__VA_ARGS__) 

void BC26_Connect_Net()
{
	int i;
	__IO char  IMEI[20];
	char *BC26_IMEI;//用来存储BC26的IMEI号
	
	CleanBC26Buffer();
	//1.先初始化BC26，防止在APP程序中对Boot程序中BC26联网有干扰
	BC26_Usart_SendCmd("AT+QRST=1\r\n");//复位
	delay_ms(1000);
	delay_ms(1000);
	//2.测试功能是否正常
	CleanBC26Buffer();
	BC26_Usart_SendCmd("AT\r\n");
	delay_ms(1000);
	printf("输入AT后返回%s\n",BC26_Buffer);
	
	//3.
	CleanBC26Buffer();
	BC26_Usart_SendCmd("AT+QSCLK=1\r\n");//禁止BC26进入休眠状态
	delay_ms(300);

	BC26_Usart_SendCmd("AT+CPSMS=0\r\n");//禁止进入PSM（低功耗）
	delay_ms(300);
	
	CleanBC26Buffer();
	BC26_Usart_SendCmd("AT+CGSN=1\r\n");//查询IMEI卡号
	delay_ms(300);

/*****
不能用strstr的原因是，赋值的是地址，而不是，字符串，所以导致数组里面的东西一直在变
BC26_IMEI=strstr(BC26_Buffer,"+CGSN:");
*****/
	BC26_IMEI=strstr(BC26_Buffer,"+CGSN:");
	BC26_IMEI=BC26_IMEI+7;//右移7为，使数组指向IMEI第一位
	
	for(i=0;i<15;i++)
		IMEI[i]=BC26_IMEI[i];
	IMEI[15]='\0';
	
	printf("IMEI号是%s\n",IMEI);
	
	delay_ms(300);

	BC26_Usart_SendCmd("AT*MICCID\r\n");//查询SIM卡号
	delay_ms(300);

	CleanBC26Buffer();
	BC26_Usart_SendCmd("AT+CESQ\r\n");//当前网络状态
	delay_ms(500);
	printf("当前网络状态%s\n第五项为rsrq,范围0-34,数值越高信号越好\n",BC26_Buffer);
	
	CleanBC26Buffer();
	BC26_Usart_SendCmd("AT+CGPADDR=1\r\n");//查询IP地址，确保入网成功
	delay_ms(300);
	printf("当前IP地址为%s\n",BC26_Buffer);
	
	
	CleanBC26Buffer();
	BC26_Usart_SendCmd("AT+CGATT?\r\n");//查询网络附着状态
	delay_ms(300);
	printf("当前网络附着状态%s\n1表示附着成功，0表示未附着成功\n",BC26_Buffer);

	CleanBC26Buffer();
	BC26_Usart_SendCmd("AT+QLWSERV=\"180.101.147.115\",5683\r\n");//连接电信云平台IP地址和端口号，电信平台可查看
	delay_ms(1000);
	printf("连接电信云状态%s\n",BC26_Buffer);
	

	CleanBC26Buffer();
	BC26_Usart_SendCmd("AT+QLWCONF=\"");
	BC26_Usart_SendCmd(IMEI);///设置IOT平台注册的IMEI号
	BC26_Usart_SendCmd("\"\r\n");
	//BC26_Usart_Send("AT+QLWCONF=\"%s\"\r\n",BC26_IMEI);
	delay_ms(1000);
	delay_ms(1000);
	printf("平台注册情况%s\n",BC26_Buffer);
	
	CleanBC26Buffer();
	BC26_Usart_SendCmd("AT+QLWADDOBJ=19,0,1,\"0\"\r\n");//允许上行通信
	delay_ms(1000);

	BC26_Usart_SendCmd("AT+QLWADDOBJ=19,0,1,\"0\"\r\n");//允许下行通信
	delay_ms(1000);

	CleanBC26Buffer();
	BC26_Usart_SendCmd("AT+QLWOPEN=0\r\n");//设备注册，这时设备应该是上线了
	delay_ms(1000);
	printf("平台上线情况%s\n",BC26_Buffer);
	
	
	CleanBC26Buffer();
	BC26_Usart_SendCmd("AT+QLWCFG=\"dataformat\",1,1\r\n");//设置发送和接收数据格式为十六进制的字符串模式
	delay_ms(1000);
	
//	CleanBC26Buffer();
//	BC26_Usart_SendCmd("AT+QLWDATASEND=19,0,0,1,31,0x0100\r\n");//发送和接收数据测试（会收到SEND OK，这个表示数据成功发送到基站）
////	BC26_Usart_SendCmd("AT+QLWDATASEND=19,0,0,5,68656C6C6F,0x0000\r\n");//发送和接收数据测试（没有是否发送到基站确认）
//	delay_ms(1000);
}

//获取现在STM32产品的版本号
//这里的版本号存在内部FLASH里面，按照字符串"Vx.xx"形式存储(这里为了凑成一个2的倍数把V字符删掉了)
void Get_Version(char *Version)
{ 

	uint32_t Address = 0x00;				//记录写入的地址
	int i;
	Address = Version_WRITE_ADDR ;

	for(i=0;i<4;i++)
		Version[i]=(*(__IO uint32_t*) (Address+i));
	
	
}
//将产品的版本写入到FLASH中
void Set_Version(uint8_t *Version)
{
	uint32_t Addres = 0x00;				//记录写入的地址
	int i;
	Addres = Version_WRITE_ADDR;
	
	FLASH_Unlock();
	FLASH_ErasePage(Addres);
	for(i=0;i<2;i++)
		FLASH_ProgramHalfWord((Addres+i*2),(Version[i*2])+(Version[i*2+1]<<8));

	FLASH_Lock();
}



/*
更新流程：
要先在平台上上传更新包，设置好更新任务后
才能开始
如果这时候我上传数据，那么云平台会给我返回FFFE01134C9A0000，表示查询设备版本

该函数用于获取更新包前的设置
告诉云端现在的版本等任务
以及获取云平台包的数量及大小
*/
FLAG BC26_Updata_CMD()
{
	
	char *strx,cmd[100],cmd_len[3],len;//cmd是需要上传的指令（其中还包含CRC校验位）
	char version[10],i;
	//uint32_t Package_size,Package_num;
	char Package_size_str[4],Package_num_str[4];
	
	
	CleanBC26Buffer();
	BC26_Usart_SendCmd("AT+QLWDATASEND=19,0,0,1,31,0x0100\r\n");//发送和接收数据测试（会收到SEND OK，这个表示数据成功发送到基站）
	delay_ms(1000);
	delay_ms(1000);
	delay_ms(1000);
	delay_ms(1000);
	delay_ms(1000);
//	delay_ms(1000);
	strx=strstr(BC26_Buffer,"SEND OK");
	printf("进入版本查询\n");
	if(!strx)
	{
		printf("网络连接不成功\n");
		//这里可以再次重新调用前面的BC26_Connect_Net()来重新连接试试
	}else
	{
		
		
	//如果设备已经能正常通行，这时候应该是接收到了，云端请求设备版本的指令
		printf("网络成功\n");
		printf("%s\n",BC26_Buffer);
		//这条指令只有上传了更新包后，第一次上传指令的时候才能收到
		//有时候收不到是应为设备上传过数据，导致这条指令云端已经下下发过了，那么，第二次就收不到了
		strx=strstr(BC26_Buffer,"FFFE01134C9A0000");
		
		//在实际应用中可能会应为APP程序和云端交互太多，而导致错过了这条消息，
		//所以在云端应该可以设置，进入更新程序后才开始上传更新包
		//这样就不会漏掉该指令
		//这个版本的为了方便测试，代码中就没有加对确认收到该指令的处理
		//否则应该把if下面的函数写在if里面
//		if(strx)		
//		{
//		}
		//我看了一下RCT6的那个开发板上有个FLASH（W25Q16），其实可以把更新的内容写在外部FLASH里面
		//感觉可以根据需要来设置，如果联网下载的程序都在BootLoader里面的话，就可以把更新的程序写在SRAM里面
		//简单又省钱，但是如果把联网下载的程序都写在APP程序里面的话，就可以用外部FLASH，就类似庆科的IAP方案
		
		//先不管这些，我直接把更新程序写到SRAM里面，收到上面指令后需要上传现在产品里面的版本
		//这里我们统一版本格式都为 Vx.xx 如（V2.16）
		//先上报STM32里面现在的程序版本
		Get_Version(version);//获取当前产品的版本号
		/*
		FFFE0113164700110056322E31300000000000000000000000
		FFFE（起始标识）01（版本号）13（消息码）1647（校验码）0011（数据区长度）00 (结果码) 
		56322E31300000000000000000000000（当前版本号，V2.10，规定长度为byte[16]）
		*/
		strcpy(cmd,"FFFE0113000000110056322E31300000000000000000000000");
		
		//把现在产品里面的版本写到字符串里面
		cmd[21]=version[0];
		cmd[25]=version[2];
		cmd[27]=version[3];
		
		BC26_CRC(cmd);//得到带校验码的CMD
		printf("CRC是%s\n",cmd);
	
		len=strlen(cmd)/2;	
		myitoa(len,cmd_len,10);

		CleanBC26Buffer();
		
		printf("字长为%s\n",cmd_len);
		//这里是BC26上报数据的固定格式
		BC26_Usart_SendCmd("AT+QLWDATASEND=19,0,0,");
		BC26_Usart_SendCmd(cmd_len);
		BC26_Usart_SendCmd(",");
		BC26_Usart_SendCmd(cmd);
		BC26_Usart_SendCmd(",0x0100\r\n");
		//上报完版本，云端会下发当前云端的版本
		//以及分片数量和分片大小，
		printf("\n%s\n",BC26_Buffer);
		
		//等待云端下发参数
		strx=strstr(BC26_Buffer,"QLWDATARECV:");
		
		
		//这里后面需要修改不然，当云端没有更新包的时候会卡死在这里
		while(strx==NULL)
		{
			strx=strstr(BC26_Buffer,"QLWDATARECV:");
			delay_ms(1000);
		}
	
	/*
	假设此时在平台上上传的软件包，json文件中定义的版本号为V2.16，
	设备端收到消息码流：FFFE011491B0001656322E3136000000000000000000000001F400813836，
	01F4为分片包大小（500字节），0081为分片总数，3836转换成字符串即为json文件中的versioncheckcode字段值。
		
	还需要把云端的版本号存储下来，方便下面的程序请求数据包
		*/
	
		strx=strstr(BC26_Buffer,"19,1,0,30,FFFE");
		strx=strstr(strx,"FFFE");
		printf("字符串为：%s",strx);
		//应为是固定格式，所以获取指定位置的包大小，和包数量（字符串形式）
		Package_size_str[0]=strx[48];
		Package_size_str[1]=strx[49];
		Package_size_str[2]=strx[50];
		Package_size_str[3]=strx[51];
		
		Package_num_str[0]=strx[52];
		Package_num_str[1]=strx[53];
		Package_num_str[2]=strx[54];
		Package_num_str[3]=strx[55];
		
		//版本号
		UpData_Version[0]=strx[19];
		UpData_Version[1]=strx[23];
		UpData_Version[2]=strx[25];
		printf("云端版本号为V3%c.3%c3%c\n",UpData_Version[0],UpData_Version[1],UpData_Version[2]);
		
		
		
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
		
		//有时候请求的数据包大小这条指令错误，不过可以后期优化
		//多接受几次，然后判断包大小和数量是不是符合规定
		printf("包大小%x,数量%x",Package_size,Package_num);
		
		CleanBC26Buffer();
//		delay_ms(1000);
//告诉云端，确认要升级
		BC26_Usart_SendCmd("AT+QLWDATASEND=19,0,0,9,FFFE0114D768000100,0x0000\r\n");
		delay_ms(1000);
		
		//接下来就可以开始请求数据包了

		
	}
	
}


//对升级包进行下载
//在上面一个函数中已经获取到，每个包的数量，以及大小
//下面就开始获取升级包
void BC26_Download()
{
	int i,j,num=0;
	char cmd[100],num_str[4];
	int Package_num_temp=0,TimeOut=0;
	char *strx;
	uint32_t addr=0;
	//因为云端下发的是十六进制的字符"FF",需要转换成二进制字符
	//所以这个是用来存储处理后的升级包的
	u8 UpdataBin_Buffer[Package_size];
//	u8 Buffer[Package_size*Package_num]	__attribute__ ((at(0X20001000)));//接收缓冲,最大个字节,起始地址为0X20001000(防止SRAM里面)		

	
	printf("包大小%d,数量%d\n",Package_size,Package_num);
	CleanBC26Buffer();
	//Package_num在上面一个程序以及获取到了包的数量
	Bin_size=0;//计算整个文件大小
	for(Package_num_temp=0;Package_num_temp<Package_num;Package_num_temp++)
	{
		
	//	printf("第%d次下载数据\n",Package_num_temp);
		strcpy(cmd,"FFFE01150000001256302E303000000000000000000000000000");
		cmd[48]=(Package_num_temp/16/16/16<=9)?(Package_num_temp/16/16/16+'0'):(Package_num_temp/16/16/16+'A'-10);
		cmd[49]=(Package_num_temp/16/16%16<=9)?(Package_num_temp/16/16%16+'0'):(Package_num_temp/16/16%16+'A'-10);
		cmd[50]=(Package_num_temp/16%16<=9)?(Package_num_temp/16%16+'0'):(Package_num_temp/16%16+'A'-10);
		cmd[51]=(Package_num_temp%16<=9)?(Package_num_temp%16+'0'):(Package_num_temp%16+'A'-10);
		
		//把云端的版本号写入到cmd里面
		cmd[19]=UpData_Version[0];
		cmd[23]=UpData_Version[1];
		cmd[25]=UpData_Version[2];
	
		
		printf("第%c%c%c%c次下载数据",cmd[48],cmd[49],cmd[50],cmd[51]);
		
		BC26_CRC(cmd);//得到带校验码的CMD
		//这里是BC26获取更新包的固定格式
		CleanBC26Buffer();
		BC26_Usart_SendCmd("AT+QLWDATASEND=19,0,0,26,");
		BC26_Usart_SendCmd(cmd);
		BC26_Usart_SendCmd(",0x0100\r\n");
	
		printf("请求数据包的cmd是%s\n",cmd);
		//这里有个问题是我接收到的数据有很多
		//除了包里面的内容，还有发送上一条指令的时候返回的数据
		//所以直接用strstr提取FFFE不行
		//要换一个方法
		/****************+QLWDATARECV这个是根据BC26写的，其他的需要该*********************/
		strx=strstr(BC26_Buffer,"QLWDATARECV");//这时候Buffer里面是没有+QLWDATARECV的
		while(strx==NULL)
		{
			//留有最够的时间给中断接住字符串
			//不过这种处理方法有缺陷，不应该用延时（网速慢的时候可能导致时间很长二跳出中断），
			//应该用一段时间内没有接收到字符来判断
			strx=strstr(BC26_Buffer,"QLWDATARECV");
			delay_ms(1000);
			delay_ms(1000);
			delay_ms(1000);
			delay_ms(1000);
		}
		delay_ms(1000);
		delay_ms(1000);
		//先把字符在中断中全部接住，再进行处理	
		//找到有效字符的位置
		strx=strstr(strx,"FFFE");
	//	printf("接收到完整数据是%s\n",strx);
		
		//先确定需要接收有效字节数量(存在num里面)
		num_str[0]=strx[12];
		num_str[1]=strx[13];
		num_str[2]=strx[14];
		num_str[3]=strx[15];
		
	//	printf("分片是%c%c%c%c",num_str[0],num_str[0],num_str[0],num_str[0]);
		num=0;
		for(i=0;i<4;i++)
		{
			if(num_str[i]<='9') num_str[i]=num_str[i]-'0';
			else {
				num_str[i]=num_str[i]-'A'+10;
			}
			num=num*16+num_str[i];
		}
				
	
		//这里减去三是因为
		num=num-3;//这时候num应该是bin文件的有效字节
		printf("该包有效字节为：%d\n",num);
		//因为下发的是十六进制字符串形，而我们要的bin文件应该是二进制的
		//所以要进行处理
		//而且每次下发包的大小可能不一样，就需要按照要求来处理
		//这里用strx来处理
		strx=strx+22;
		printf("接收到完整的数据包是%s\n",strx);
		
		i=0;
		while(strx[2*i]!='\0'||strx[2*i+1]!='\0')
		{
			//把字符串合并成16进制的字符串
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
	
		printf("接收到字符数量为：%d\n",i-1);
		//判断是否数据丢包，如果丢包就重新在请求一次这个包
		if(i<num)
		{
			TimeOut++;
			printf("数据丢包,重新请求一次\n"); 
			Package_num_temp--;
		}	
		
		//如果请求次数过多超时就结束
		if(TimeOut>=2)
		{
			printf("数据包有问题，请求超时\n");
			break;
		}
		
		
		//现在把接收的数据全部放到SRAM里面
		for(i=0;i<num;i++)
		{
			Buffer[addr++]=UpdataBin_Buffer[i];
		}
		
		Bin_size=Bin_size+num;
		
	}
	
}


/*
	把Bin写入到FLASH里面，然后跳转运行
*/
void Write_To_Flash()
{
	char *str,cmd[100],len,cmd_len[3],Newest_Version[4];
	CleanBC26Buffer();
	//设备请求完分片包后，按照协议设备需上传一条消息码流查询设备的升级包下载状态
	delay_ms(1000);
	BC26_Usart_SendCmd("AT+QLWDATASEND=19,0,0,9,FFFE0116850E000100,0x0000\r\n");
	delay_ms(1000);
	delay_ms(1000);
	
	/*
		上包下载情况后云平台会下发
		+QLWDATARECV: 19,1,0,9,FFFE0116850E000100
		+QLWDATARECV: 19,1,0,8,FFFE0117CF900000
		平台也会转换成downloaded
	*/
	//其实不要下面这部分也可以，直接上报完情况就直接升级就可以了
//	str=strstr(BC26_Buffer,"FFFE01134C9A0000");
//	while(str==NULL)
//	{
//			str=strstr(BC26_Buffer,"FFFE01134C9A0000");
//			delay_ms(1000);

//	}
//	
	delay_ms(1000);
	//上报，表示产品开始升级了
	BC26_Usart_SendCmd("AT+QLWDATASEND=19,0,0,9,FFFE0117B725000100,0x0000\r\n");	
	//等收到之后产品开始升级
	//收到后设备进入升级流程
	//为了防止升级被打断，先关闭总中断
	__disable_irq();
	
	iap_write_appbin(FLASH_APP_ADDR,Buffer,Bin_size);//更新FLASH代码   
	delay_ms(100);
	
	__enable_irq();
	strcpy(cmd,"FFFE0118000000110056302E30300000000000000000000000");
		
	//把现在产品里面的版本写到字符串里面
	cmd[21]=UpData_Version[0];
	cmd[25]=UpData_Version[1];
	cmd[27]=UpData_Version[2];
		
	BC26_CRC(cmd);//得到带校验码的CMD
	printf("CRC是%s\n",cmd);
	
	len=strlen(cmd)/2;	
	myitoa(len,cmd_len,10);

	CleanBC26Buffer();
		
	printf("字长为%s\n",cmd_len);
	//这里是BC26上报数据的固定格式
		BC26_Usart_SendCmd("AT+QLWDATASEND=19,0,0,");
		BC26_Usart_SendCmd(cmd_len);
		BC26_Usart_SendCmd(",");
		BC26_Usart_SendCmd(cmd);
		BC26_Usart_SendCmd(",0x0100\r\n");
		//上报完版本，就把升级后的版本号在写入到FLASH里面为后买你升级做准备
		
		//把最新版本号写入到Flash里
		
	Newest_Version[0]=UpData_Version[0];
	Newest_Version[1]='.';
	Newest_Version[2]=UpData_Version[1];
	Newest_Version[3]=UpData_Version[2];
	Set_Version(Newest_Version);
	
	
	Get_Version(Newest_Version);
	printf("最新版本是%s\n",Newest_Version);
	
	//升级完成，跳转前先上发升级结果
	IAP_APP_Jump (APP_ADDR);
	
	
	
	
}

