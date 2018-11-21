#include<reg52.h>
#include<intrins.h>
#define uchar unsigned char
#define uint unsigned int
#define RxIn 110	//定义接收的数组长度为90
uchar code AT[]="AT";	//握手信号
uchar code ATE[]="ATE";	 //关回显
uchar code AT_CNMI[]="AT+CNMI=2,1";//设置这组参数来了新信息直接显示到串口，不作存储
                        //部分模块需设置成AT+CNMI=2,2，建议模块跟PC测试后再编程

uchar code AT_CSCA[]="AT+CSCA=\"+8613010710500\"";//设置服务中心号码
uchar code AT_CMGF[]="AT+CMGF=1";//设置短信的格式为text格式
uchar code AT_CMGR[]="AT+CMGR=";//读取短信指令
uchar code AT_CMGS[]="AT+CMGS=";//发送短信指令
uchar code AT_CMGD[]="AT+CMGD=";//删除保存的短信
uchar code successfully[]="Operate Successfully!";//发送操作成功信息到目标号码
uchar code fail[]="Operate failed,try again!";	//发送操作失败信息到目标号码
uchar AT_delete[12];
uchar AT_Read[12];	//用来存储发送读取短信指令 
uchar AT_SendNumber[25];  //用来存储发送短信号码指令
uchar numberbuf[3];		 //用来保存短信条数 
uchar idata SystemBuf[RxIn];  //储存出口接收数据 
uchar CommandBuf[6];	 //用来储存指令 
uchar idata state[17];	  //用来存储IO口状态 
uchar idata state1[17];	 //用来存储IO口状态 
uchar Rx=0;
uint temp;	//记录状态 	 
uchar temp1;	// 用于记录P0口状态
uchar temp2;   //用于记录P2口状态 
bit check=0;	//查询标志位 
bit receiveready=0;	//接收短信标志位
bit sendready=0;//发送短信准备标志位
bit send=0;		//发送短信标志位
bit flag=0;	//指令标志位
sbit P3_7=P3^7;//启动GSM的启动线连IGT

sbit realy0=P1^4;//继电器1
sbit realy1=P2^6;//继电器2

sbit test4=P1^3;//测试点4
sbit test5=P1^5;//继电器2

sbit L1=P1^6;
sbit L2=P1^7;

sbit realy2=P2^3;//继电器2
sbit realy3=P0^3;//继电器4
sbit realy4=P0^4;  //继电器5
sbit realy5=P0^5;  //继电器6
sbit realy6=P0^6;  //继电器7
sbit realy7=P0^7;//继电器8

sbit key1=P3^4;	//开关1
sbit key2=P3^5; //开关2
sbit key3=P3^4;	//开关3
sbit key4=P2^3;	//开关4
sbit key5=P2^4;	//开关5
sbit key6=P2^5;	//开关6
sbit key7=P2^6;	//开关7
sbit key8=P2^7;	//开关8
void Delay_ms(uint i);
void Start_GSM(void);
void UART_init (void);
void sendchar(uchar ch);
void sendstring(uchar *p);
void GSM_INIT(void);
void receive_ready(void);
void message_read(void);
void read_message(void);
void sendmessage(void);
/******************************************************************************************************************/
///////////////////函数void Delay_ms(uint i);实现功能:进行毫秒延时///////////////////////////////////
/******************************************************************************************************************/
void Delay_ms(uint i)
{
    unsigned int j;
    for(;i>0;i--){
    for(j=0;j<125;j++)
    {;}
}
}
/******************************************************************************************************************/
////////////函数void Start_GSM(void);实现功能:对TC35进行启动,开启TC35///////////////////////////////////
//功能详述:单片机上的P3_7管脚是跟TC35的IGT管脚相连;需要启动TC35,必须在 
//15脚(/IGT)加时长至少为100ms的低电平信号,
///且该信号下降沿时间小于1ms。启动后，15 脚的信号应保持高电平.
/******************************************************************************************************************/
void Start_GSM(void)
{
  P3_7=0;
  Delay_ms(1000);
  P3_7=1;
  Delay_ms(1000);
}
/********函数void UART_init;实现功能:对串口进行初始化////////////////////////////////////////////////
/******************************************************************************************************************/
void UART_init (void)
{
  	SCON = 0x50 ;  //SCON: serail mode 1, 8-bit UART, enable ucvr   
                         //UART为模式1，8位数据，允许接收
          TMOD |= 0x21 ; //TMOD: timer 1, mode 2, 8-bit reload             
                         //定时器1为模式2,8位自动重装
          PCON |= 0x00 ; //SMOD=1; 
          TH1 = 0xFD ;   //Baud:9600 fosc="11".0592MHz 
          IE |= 0x90 ;     //Enable Serial Interrupt 
          TR1 = 1 ;       // timer 1 run 
          TI=1;  
   
  }
/******************************************************************************************************************/
////////////函数void sendchar(uchar ch);实现功能:发送一字节数据////////////////////////////////////////////////
/******************************************************************************************************************/
void sendchar(uchar ch)
{
	SBUF=ch;
	while(TI==0);
	TI=0;
}
/******************************************************************************************************************/
///////////函数void sendstring(uchar *p);实现功能:通过串口发送字符串/////////////////////////////////////
/******************************************************************************************************************/
void sendstring(uchar *p)
{
  while(*p)
  {
  	sendchar(*p);
  	p++;
	}
  sendchar(0x0D);//回车
  sendchar(0x0A); //换行
}
/******************************************************************************************************************/
///////////////////函数void receive(void) interrupt 4 using 1;实现功能:通过串口接收数据///////////////////////////////
/******************************************************************************************************************/
void receive(void) interrupt 4 using 1
{  	
	if(RI)
	{
	  	if(Rx<RxIn)	//RxIn接收数组长度
	  	{
	  		SystemBuf[Rx]=SBUF; //储存出口接收数据
	  		Rx++;
	  	}
	  	RI=0;
  	}
}
/******************************************************************************************************************/
///////////////////函数void GSM_INIT(void);实现功能:初始化TC35模块///////////////////////////////
/******************************************************************************************************************/
void GSM_INIT(void)
{ 
LOOP:
	Delay_ms(1000);
	sendstring(AT);
	Delay_ms(1000);
  	sendstring(ATE);
 	Delay_ms(1000);
 	sendstring(AT_CNMI); //AT_CNMI[]="AT+CNMI=2,1";设置这组参数来了新信息直接显示到串口，
					//不作存储 部分模块需设置成AT+CNMI=2,2，建议模块跟PC测试后再编程
	Delay_ms(1000);
 	sendstring(AT_CSCA);//设置服务中心号码
	Delay_ms(1000);
 	for(Rx=0;Rx<RxIn;Rx++)//RxIn接收数组长度
  	{
      SystemBuf[Rx]=0x00; //储存出口接收数据   
  	}
  	Rx=0; 
  	sendstring(AT_CMGF); //设置短信的格式为text格式
  	Delay_ms(1000);
 	if((SystemBuf[2]=='O')&&(SystemBuf[3]=='K'))//判断是否模块初始化成功,成功的话模块会回复"OK"给单片机
  	{					//如果单片机没有收到OK,就继续发送初始化指令/   
			L2=0;
     	for(Rx=0;Rx<RxIn;Rx++)
     	{
       	 SystemBuf[Rx]=0x00;   
     	}
    	 Rx=0;
  	}
  	else
  	{    
			L1=0;
     	for(Rx=0;Rx<RxIn;Rx++)
     	{
     	    SystemBuf[Rx]=0x00;   
     	}
     	Rx=0; 
     	goto LOOP;           
  	}
}
/******************************************************************************************************************/
///////////////////函数void receive_ready(void);实现功能:接收短信准备///////////////////////////////
/******************************************************************************************************************/ 
void receive_ready(void)
{
	uchar i;
	if((SystemBuf[5]==0x54)&&SystemBuf[6]==0x49)//0x54:T  0x49:I
		//如果有新短信来,模块会通过串口向单片机发送字符串,
	{	 //	此函数的功能是判断是否有新短信来,如果来的话就置位准备接受位标志为1
		receiveready=1;	//如果不是新短信的指令,就舍弃,并将接收数组清零/
	}
	else
	{ 
		for(i=0;i<Rx;i++)
		{
			SystemBuf[i]=0x00;   
		}
		Rx=0; 
	} 
}
/******************************************************************************************************************/
//函数 void message_read(void);实现功能:判断短信,准备是否回复短信给目标号码//////////////////////////
/******************************************************************************************************************/ 
 void message_read(void)
{ 
  if((sendready==1)&&(SystemBuf[5]==0x47)&&(SystemBuf[6]==0x52)) //0x47=G,0x52=R
  send=1;
}
/******************************************************************************************************************/
///////////////////函数 void read_message(void);实现功能:发送读取短信指令,////////////////////////////////////////
/******************************************************************************************************************/ 	 
void read_message(void)
{
	uchar i;
	Delay_ms(1000);
	for(i=0;i<3;i++)
	{
		numberbuf[i]=SystemBuf[14+i];// numberbuf[3];	用来保存短信条数
	}
	for(i=0;i<8;i++)
	{
		AT_Read[i]=AT_CMGR[i];//AT_Read[12];用来存储发送读取短信指令;CMGR：读取短消息 
	}
	for(i=8;i<11;i++)
	{
		AT_Read[i]=numberbuf[i-8];//AT_Read[12];用来存储发送读取短信指令;
	}						//numberbuf[3]:用来保存短信条数				  
	for(Rx=0;Rx<RxIn;Rx++)
	{
		SystemBuf[Rx]=0x00;    
	}
	Rx=0;  
	sendstring(AT_Read);	//发送AT+CMGR=?,?代表短信储存所在位置
}						//AT_Read[12];	//用来存储发送读取短信指令
/******************************************************************************************************************/
////////函数void readcommend(void);实现功能:读取短信内容,判断相应指令是否正确//////////////////////////
/******************************************************************************************************************/ 
void readcommend(void)
{
	uchar i;
	for(i=0;i<25;i++)  //将短信内容中的指令部分截取出来放到
	{					//CommandBuf数组中
		CommandBuf[i]=SystemBuf[78+i];//SystemBuf[RxIn];储存出口接收数据
	}//CommandBuf[6];用来储存指令 
	if((CommandBuf[18]=='o')&&(CommandBuf[19]=='p')&&(CommandBuf[20]=='e')&&(CommandBuf[21]=='n')) 
//判断指令是否为开继电器指令
	{
		switch(CommandBuf[22])
		{
			case 0x31: realy0=0;	//继电器 1开启
			break;
			case 0x32: realy1=0;	//继电器 2开启
			break;
			case 0x33: realy2=0;	//继电器 3开启
			break;
		  case 0x34: realy3=0;	//继电器 4开启
			break;
			case 0x35: realy2=0;	//继电器 5开启
			break;
			case 0x36: realy5=0;	//继电器 6开启
			break;
			case 0x37: realy6=0;//继电器 7开启
			break;
			case 0x38: realy7=0;//继电器 8开启
			break;
			case 0x41: key1=0;	//开关1开启
			break;
			case 0x42: key2=0;	//开关2开启
			break;
			case 0x43: key3=0;	//开关3开启
			break;
			case 0x44: key4=0;	//开关4开启
			break;
			case 0x45: key5=0;	//开关5开启
			break;
			case 0x46: key6=0;	//开关6开启
			break;
			case 0x47: key7=0;	//开关7开启
			break;
			case 0x48: key8=0;	//开关8开启
			break;
			case 0x4f: P0=0;P2=0;//打开所有的继电器和开关
			break;
			case 0x3f: temp1=P0;//记录P0口的状态
			temp2=P2;//记录P2口的状态
			temp=temp2;
			temp<<=8;
			temp=temp|temp1;//将P0口状态作为低8位,P2口状态作为高8位,组成一个16位的字节放到temp中
			check=1;
			break;
			default:flag=1;	//其他指令定义为错误操作                                                            
			break;		//置位错误操作位为1
		}
	}
	else
	{	
		if((CommandBuf[18]=='s')&&(CommandBuf[19]=='h')&&(CommandBuf[20]=='u')&&(CommandBuf[21]=='t'))	
		{													//判断指令是否为关闭电器指令
			
			switch(CommandBuf[22])
			{
				case 0x31: realy0=1;//关闭继电器1
				break;
				case 0x32: realy1=1;//关闭继电器2
				break;
				case 0x33: realy2=1;//关闭继电器3
				break;			
				case 0x34: realy3=1;//关闭继电器4
				break;
				case 0x35: realy4=1;//关闭继电器5
				break;
				case 0x36: realy5=1;//关闭继电器6
				break;
				case 0x37: realy6=1; //关闭继电器7
				break;
				case 0x38: realy7=1;//关闭继电器8
				break;
				case 0x41: key1=1;	//开关1关闭
				break;
				case 0x42: key2=1;	//开关2关闭
				break;
				case 0x43: key3=1;	//开关3关闭
				break;
				case 0x44: key4=1;	//开关4关闭
				break;
				case 0x45: key5=1;	//开关5关闭
				break;
				case 0x46: key6=1;	//开关6关闭
				break;
				case 0x47: key7=1;	//开关7关闭
				break;
				case 0x48: key8=1;	//开关8关闭
				break;
				case 0x53:P0=0xff;P2=0xff;//关闭所有的继电器和开关
				break;
				default:flag=1;	//其他指令定义为错误操作
				break;	//置位错误操作位为1
			}
		}
		else {flag=1;}//如果发送的指令既不是open也不是shut就定义为错误操作
	}
}
/******************************************************************************************************************/
///////////////////函数 void readstate;实现功能:将P0,P2口的状态转化为数组,////////////////////////////////////////
/******************************************************************************************************************/
void readstate(void)
{  
	uint bitcnt,i,j;
	for(bitcnt=0,i=0;bitcnt<16,i<16;bitcnt++,i++)
	{
		if((temp<<bitcnt)&0x8000)
			{
				  state1[i]=0x30;//为什么最高位是1的时候就赋给该位0x30？？？？
			}
		else 
			state1[i]=0x31;//为什么最高位是0的时候就赋给该位0x31？？？？
	}
	for(j=15,i=0;j>=0,i<16;j--,i++)
	{
		state[j]=state1[i];//state[17];用来存储IO口状态
	}				//state1[17];用来存储IO口状态
}
/******************************************************************************************************************/
///////////////////函数 void delete_message;实现功能:删除读短信指令,////////////////////////////////////////
/******************************************************************************************************************/ 
void delete_message(void)
{
	uchar i;
	Delay_ms(1000);
	Delay_ms(1000);
	Delay_ms(5000);
	for(i=0;i<8;i++)
	{
		AT_delete[i]=AT_CMGD[i];// AT_CMGD[]="AT+CMGD=";删除保存的短信
	}
	for(i=8;i<11;i++)
	{
		AT_delete[i]=numberbuf[i-8];	//numberbuf[3];用来保存短信条数 			  
	}
	for(Rx=0;Rx<RxIn;Rx++)
	{
		SystemBuf[Rx]=0x00; //SystemBuf[RxIn];储存出口接收数据   
	}
	Rx=0;  
	sendstring(AT_delete);	//发送AT+CMGR=?,?代表短信储存所在位置
}
/******************************************************************************************************************/
///////////////////函数 void sendmessage(void);实现功能:发送回复短信指令,////////////////////////////////////////
/******************************************************************************************************************/  
void sendmessage(void)
{
	uchar i;
	for(i=0;i<8;i++)
  {
     AT_SendNumber[i]=AT_CMGS[i];// AT_SendNumber[25];用来存储发送短信号码指令
  }									//AT_CMGS[]="AT+CMGS=";发送短信指令
	for(i=8;i<24;i++)
  {
     AT_SendNumber[i]=SystemBuf[14+i];//将对方号码提取用来回复给对方
  }
	sendstring(AT_SendNumber);
	Delay_ms(400);
	if(flag==0)
	{ 
		if(check==1)
		{
			sendstring(state);
		}//如果查询位置1时,发送状态信息 
		sendstring(successfully);
		check=0;
	}
	else
	{
		sendstring(fail);
	}
	Delay_ms(30);
	sendchar(0X1A);
}
/******************************************************************************************************************/
///////////////////函数 void main();实现功能:主函数,////////////////////////////////////////
/******************************************************************************************************************/  
void main()
{	
	Start_GSM();	//开启TC35
	L1=1;
	L2=1;
	Delay_ms(10000);//延时大约10秒 ,等待模块联网/
	UART_init(); //串口初始化/
	GSM_INIT();	//对tc35模块进行初始化
	while(1)
	{
		receive_ready();
		if(receiveready==1)
		{
			test4=0;
			read_message();
			receiveready=0;
			sendready=1;
			Delay_ms(5000);
			test4=1;
		}
		 test5=0;
		Delay_ms(100);
		test5=1;
		Delay_ms(300);
		message_read();
		if(send==1)
		{ 
			Delay_ms(2000);
			readcommend();
			readstate(); 
			sendmessage();
			Delay_ms(1000);
			delete_message();
			flag=0;
			for(Rx=0;Rx<RxIn;Rx++)	//每一次操作完成后对接收数组清零//
			{
				SystemBuf[Rx]=0x00;   
			}
			Rx=0;  
			send=0;
		} 
	}
}