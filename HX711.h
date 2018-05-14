#ifndef __HX711_H__
#define __HX711_H__


#include <reg52.h>
#include <intrins.h>

//IO����
sbit HX711_DOUT=P1^3; 
sbit HX711_SCK=P1^4; 

//�������߱�������
extern void Delay__hx711_us(void);
extern unsigned long HX711_Read(void);

#endif