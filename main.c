#include "main.h"
#include "LCD12864.h"
#include "HX711.h"
#include <stdio.h>  

#define uchar unsigned char 
#define uint  unsigned int
 sbit c_send   = P3^6;		//����������
sbit c_recive = P3^7;		//����������
uchar flag_hc_value;        //�������м����


unsigned int distance;	        //����
uint set_d;	            //����
bit flag_csb_juli;        //��������������
uint  flag_time0;     //�������涨ʱ��0��ʱ���

unsigned long HX711_Buffer = 0;
unsigned long Weight_Maopi = 0;
unsigned int Weight_Shiwu = 0;

void delay()
{
	_nop_(); 		           //ִ��һ��_nop_()ָ�����1us
	_nop_(); 
	_nop_(); 
	_nop_(); 
	_nop_(); 
	_nop_(); 
	_nop_();  
}

/*********************��ʱ��0����ʱ��1��ʼ��******************/
void time_init()	  
{
	EA  = 1;	 	  //�����ж�
	TMOD = 0X01;	  //��ʱ��0����ʱ��1������ʽ1
	ET0 = 1;		  //����ʱ��0�ж� 
	TR0 = 1;		  //����ʱ��0��ʱ
//	ET1 = 1;		  //����ʱ��1�ж� 
//	TR1 = 1;		  //����ʱ��1��ʱ	
//���ڵĳ�ʼ��
TMOD |=0x20;
TH1=0xFD;
TL1=0xFD;
SCON=0x50;
ES=1;
TI=0;
RI=0;	
TR1=1;
	
	
		
			
}


/*********************���ڷ��ͳ���*****************************/
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
//����
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


/*********************������������*****************************/
void send_wave()
{
   EA=1;
	c_send = 1;		           //10us�ĸߵ�ƽ���� 
	delay();
	c_send = 0;	 
	TH0 = 0;		          //����ʱ��0����
	TL0 = 0;
	TR0 = 0;				  //�ض�ʱ��0��ʱ
	flag_hc_value = 0;
	while(!c_recive);		  //��c_reciveΪ��ʱ�ȴ�
	TR0=1;
	while(c_recive)		      //��c_reciveΪ1�������ȴ�
	{
		flag_time0 = TH0 * 256 + TL0;
		if((flag_hc_value > 1) || (flag_time0 > 65000))      //������������������Χʱ����ʾ3��888
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
		TR0=0;							 //�ض�ʱ��0��ʱ
	//	distance = TH0;		 		     //������ʱ��0��ʱ��
		distance = TH0 * 256 + TL0;
		distance +=( flag_hc_value * 65536);//�������������ʱ��	 �õ���λ��ms
		distance *= 0.017;               // 0.017 = 340M / 2 = 170M = 0.017M ���������
		if(distance > 300)				 //���� = �ٶ� * ʱ��
		{	
			distance = 888;				 //�������4.0m�ͳ��������������� 
		}
	} 
	EA=0; 
}



float weight_reg;
int k=0;
//****************************************************
//������
//****************************************************
void main()
{	
	
	
  time_init();
	send_wave();
	LCD12864_Reset();								//��ʼ��Һ��
	LCD12864_HAIZI_SET();							//����Ϊ��ͨģʽ

	LCD12864_NoWaitIdle_COM_Write(0x80);						//ָ������
	LCD12864_write_word("����������������");
	LCD12864_NoWaitIdle_COM_Write(0x90);						//ָ������
	LCD12864_write_word("������ӭʹ�á���");			
	LCD12864_NoWaitIdle_COM_Write(0x88);						//ָ������
	LCD12864_write_word("������ز�������");
	LCD12864_NoWaitIdle_COM_Write(0x98);						//ָ������
	LCD12864_write_word("����������������");
				
	Delay_ms(1000);		 //��ʱ1s,�ȴ��������ȶ�

	Get_Maopi();				//��ëƤ����
	LCD12864_NoWaitIdle_COM_Write(0x01);			//���

	while(1)
	{
	    send_wave();
		Get_Weight();			//����
		LCD12864_NoWaitIdle_COM_Write(0x80);						//ָ������
		LCD12864_write_word("������ӭʹ�á���");
		LCD12864_NoWaitIdle_COM_Write(0x90);
		LCD12864_write_word("������ز�������");
		//��ʾ��ǰ����
		LCD12864_NoWaitIdle_COM_Write(0x88);
		LCD12864_write_word("��ߣ�");
		
		LCD12864_Data_Write(' ');
		LCD12864_Data_Write(' ');
		LCD12864_Data_Write(' ');
		LCD12864_Data_Write(distance%1000/100 + 0x30);
		LCD12864_Data_Write('.');
		LCD12864_Data_Write(distance%100/10 + 0x30);
		LCD12864_Data_Write(distance%10 + 0x30);
			LCD12864_write_word(" m ");
		LCD12864_NoWaitIdle_COM_Write(0x98);
		LCD12864_write_word("���أ�");
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
//����
//****************************************************
void Get_Weight()
{
	HX711_Buffer = HX711_Read();
	HX711_Buffer = HX711_Buffer/100;
	if(HX711_Buffer > Weight_Maopi)			
	{
		Weight_Shiwu = HX711_Buffer;
		Weight_Shiwu = Weight_Shiwu - Weight_Maopi;				//��ȡʵ���AD������ֵ��
	
		Weight_Shiwu = (unsigned long)((float)Weight_Shiwu/4.00+0.05); 	//����ʵ���ʵ������
																		//��Ϊ��ͬ�Ĵ������������߲�һ������ˣ�ÿһ����������Ҫ���������2.15���������
																		//�����ֲ��Գ���������ƫ��ʱ�����Ӹ���ֵ��
																		//������Գ���������ƫСʱ����С����ֵ��
																		//����ֵһ����2.15��������֮�䡣�򴫸�����ͬ������
																		//+0.05��Ϊ����������ٷ�λ
		
		if(  Weight_Shiwu > 100000 )
		{
			Buzzer = 0;				//�򿪾���	
		}
		else
		{
			Buzzer = 1;				//�رվ���
		}
	}
	else if(HX711_Buffer < Weight_Maopi - 30)
	{
		Buzzer = 0;				//����������
	}

}

//****************************************************
//��ȡëƤ����
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
//MS��ʱ����(12M�����²���)
//****************************************************
void Delay_ms(unsigned int n)
{
	unsigned int  i,j;
	for(i=0;i<n;i++)
		for(j=0;j<123;j++);
}

/*********************��ʱ��0�жϷ������ ��������������************************/
void time0_int() interrupt 1  
{						   
	flag_hc_value ++;		 //	TH0 TL0 ��65536������ж�
}		  
