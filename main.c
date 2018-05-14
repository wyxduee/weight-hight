#include "main.h"
#include "LCD12864.h"
#include "HX711.h"
#include <stdio.h>  

#define uchar unsigned char 
#define uint  unsigned int
 sbit c_send   = P3^6;		//超声波发射
sbit c_recive = P3^7;		//超声波接收
uchar flag_hc_value;        //超声波中间变量


unsigned int distance;	        //距离
uint set_d;	            //距离
bit flag_csb_juli;        //超声波超出量程
uint  flag_time0;     //用来保存定时器0的时候的

unsigned long HX711_Buffer = 0;
unsigned long Weight_Maopi = 0;
unsigned int Weight_Shiwu = 0;

void delay()
{
	_nop_(); 		           //执行一条_nop_()指令就是1us
	_nop_(); 
	_nop_(); 
	_nop_(); 
	_nop_(); 
	_nop_(); 
	_nop_();  
}

/*********************定时器0、定时器1初始化******************/
void time_init()	  
{
	EA  = 1;	 	  //开总中断
	TMOD = 0X01;	  //定时器0、定时器1工作方式1
	ET0 = 1;		  //开定时器0中断 
	TR0 = 1;		  //允许定时器0定时
//	ET1 = 1;		  //开定时器1中断 
//	TR1 = 1;		  //允许定时器1定时	
//串口的初始化
TMOD |=0x20;
TH1=0xFD;
TL1=0xFD;
SCON=0x50;
ES=1;
TI=0;
RI=0;	
TR1=1;
	
	
		
			
}


/*********************串口发送程序*****************************/
	unsigned char str_send_buff[10];
unsigned char receive_buff[10];
void send_str(unsigned char *s)
{
	ES=0;
	if(*s=='\0')
	{
			return;
	}
	else 
	{
		while(*s!=0){
			SBUF=*s;
			while(TI==0);
			TI=0;
			s++;		
	}
	ES=1;
	
}
return;
}

void send_char(unsigned char c)
{
		ES=0;
		SBUF=c;
		while(TI==0);
		TI=0;
		ES=1;
		return;
}
//接受
unsigned char i;
unsigned int count;
void rsint() interrupt 4 using 0
{		
		i=0;
		count=0;
		receive_buff[i]=SBUF;
		RI=0;
	loop:
		i++;
		while(!RI){
				count++;
				if(count>130){
					receive_buff[i]='\0';
					send_str(receive_buff);
					return ;
				}
			receive_buff[i]=SBUF;
		}
		RI=0;
		count=0;
		goto loop;
}


/*********************超声波测距程序*****************************/
void send_wave()
{
   EA=1;
	c_send = 1;		           //10us的高电平触发 
	delay();
	c_send = 0;	 
	TH0 = 0;		          //给定时器0清零
	TL0 = 0;
	TR0 = 0;				  //关定时器0定时
	flag_hc_value = 0;
	while(!c_recive);		  //当c_recive为零时等待
	TR0=1;
	while(c_recive)		      //当c_recive为1计数并等待
	{
		flag_time0 = TH0 * 256 + TL0;
		if((flag_hc_value > 1) || (flag_time0 > 65000))      //当超声波超过测量范围时，显示3个888
		{
			TR0 = 0;
			flag_csb_juli = 2;
			distance = 777;
			flag_hc_value = 0;
			break ;		
		}
		else 
		{
			flag_csb_juli = 1;	
		}
	}
	if(flag_csb_juli == 1)
	{	
		TR0=0;							 //关定时器0定时
	//	distance = TH0;		 		     //读出定时器0的时间
		distance = TH0 * 256 + TL0;
		distance +=( flag_hc_value * 65536);//算出超声波测距的时间	 得到单位是ms
		distance *= 0.017;               // 0.017 = 340M / 2 = 170M = 0.017M 算出来是米
		if(distance > 300)				 //距离 = 速度 * 时间
		{	
			distance = 888;				 //如果大于4.0m就超出超声波的量程 
		}
	} 
	EA=0; 
}



float weight_reg;
int k=0;
//****************************************************
//主函数
//****************************************************
void main()
{	
	
	
  time_init();
	send_wave();
	LCD12864_Reset();								//初始化液晶
	LCD12864_HAIZI_SET();							//设置为普通模式

	LCD12864_NoWaitIdle_COM_Write(0x80);						//指针设置
	LCD12864_write_word("※※※※※※※※");
	LCD12864_NoWaitIdle_COM_Write(0x90);						//指针设置
	LCD12864_write_word("※※欢迎使用※※");			
	LCD12864_NoWaitIdle_COM_Write(0x88);						//指针设置
	LCD12864_write_word("身高体重测量仪器");
	LCD12864_NoWaitIdle_COM_Write(0x98);						//指针设置
	LCD12864_write_word("※※※※※※※※");
				
	Delay_ms(1000);		 //延时1s,等待传感器稳定

	Get_Maopi();				//称毛皮重量
	LCD12864_NoWaitIdle_COM_Write(0x01);			//清空

	while(1)
	{
	    send_wave();
		Get_Weight();			//称重
		LCD12864_NoWaitIdle_COM_Write(0x80);						//指针设置
		LCD12864_write_word("※※欢迎使用※※");
		LCD12864_NoWaitIdle_COM_Write(0x90);
		LCD12864_write_word("身高体重测量仪器");
		//显示当前重量
		LCD12864_NoWaitIdle_COM_Write(0x88);
		LCD12864_write_word("身高：");
		
		LCD12864_Data_Write(' ');
		LCD12864_Data_Write(' ');
		LCD12864_Data_Write(' ');
		LCD12864_Data_Write(distance%1000/100 + 0x30);
		LCD12864_Data_Write('.');
		LCD12864_Data_Write(distance%100/10 + 0x30);
		LCD12864_Data_Write(distance%10 + 0x30);
			LCD12864_write_word(" m ");
		LCD12864_NoWaitIdle_COM_Write(0x98);
		LCD12864_write_word("体重：");
		if( Weight_Shiwu/10000 != 0)
		{
			LCD12864_Data_Write(Weight_Shiwu/100000 + 0x30);
		}
		else
		{
			LCD12864_Data_Write(' ');	
		}
		LCD12864_Data_Write(Weight_Shiwu%100000/10000 + 0x30);
		LCD12864_Data_Write(Weight_Shiwu%10000/1000 + 0x30);
		LCD12864_Data_Write(Weight_Shiwu%1000/100 + 0x30);
		LCD12864_Data_Write('.');
		LCD12864_Data_Write(Weight_Shiwu%100/10 + 0x30);
		LCD12864_Data_Write(Weight_Shiwu%10 + 0x30);
		LCD12864_write_word(" Kg ");
		Delay_ms(1000);
		
		if(Weight_Shiwu!=weight_reg){
		weight_reg=Weight_Shiwu;	
		sprintf(str_send_buff,"%f",(float)(distance)/100);
		while(str_send_buff[k]!='.')
		{
			k++;
		}
		str_send_buff[k+3]='\0';
		k=0;
		send_str("HIGHT IS:");
		send_str(str_send_buff);
		send_str(" m");
		send_str("\n");
		sprintf(str_send_buff,"%f",(float)(Weight_Shiwu)/100);
		while(str_send_buff[k]!='.')
		{
			k++;
		}
		str_send_buff[k+3]='\0';
		k=0;
		send_str("WEIGHT IS:");
		send_str(str_send_buff);
		send_str(" Kg");
		send_str("\n");
		}
	}
}

//****************************************************
//称重
//****************************************************
void Get_Weight()
{
	HX711_Buffer = HX711_Read();
	HX711_Buffer = HX711_Buffer/100;
	if(HX711_Buffer > Weight_Maopi)			
	{
		Weight_Shiwu = HX711_Buffer;
		Weight_Shiwu = Weight_Shiwu - Weight_Maopi;				//获取实物的AD采样数值。
	
		Weight_Shiwu = (unsigned long)((float)Weight_Shiwu/4.00+0.05); 	//计算实物的实际重量
																		//因为不同的传感器特性曲线不一样，因此，每一个传感器需要矫正这里的2.15这个除数。
																		//当发现测试出来的重量偏大时，增加该数值。
																		//如果测试出来的重量偏小时，减小改数值。
																		//该数值一般在2.15附近调整之间。因传感器不同而定。
																		//+0.05是为了四舍五入百分位
		
		if(  Weight_Shiwu > 100000 )
		{
			Buzzer = 0;				//打开警报	
		}
		else
		{
			Buzzer = 1;				//关闭警报
		}
	}
	else if(HX711_Buffer < Weight_Maopi - 30)
	{
		Buzzer = 0;				//负重量报警
	}

}

//****************************************************
//获取毛皮重量
//****************************************************
void Get_Maopi()
{
	unsigned char i = 0;
	unsigned int Temp_Weight = 0;

	Weight_Maopi = 0;

	for( i = 0 ; i < 10 ; i++)
	{
		HX711_Buffer = HX711_Read();
		Temp_Weight = HX711_Buffer/100;

		if( Temp_Weight > Weight_Maopi)
		{
			Weight_Maopi = Temp_Weight; 	
		}
	}		
} 

//****************************************************
//MS延时函数(12M晶振下测试)
//****************************************************
void Delay_ms(unsigned int n)
{
	unsigned int  i,j;
	for(i=0;i<n;i++)
		for(j=0;j<123;j++);
}

/*********************定时器0中断服务程序 用做超声波测距的************************/
void time0_int() interrupt 1  
{						   
	flag_hc_value ++;		 //	TH0 TL0 到65536后溢出中断
}		  
