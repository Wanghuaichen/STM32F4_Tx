
#include "stm32f4xx.h"
#include "delay.h"
#include "led.h"
#include "drv_lcd.h"

/*************************************************************************************
  * �������ƣ�main()
  * ����    ��void
  * ����ֵ  ��void
  * ����    �����������main����
  *************************************************************************************/
int main(void)
{
  SystemInit();			                                   //��ʼ��ϵͳʱ�ӣ�����ʱ��Ϊ168Mhz
  LED_GPIO_Conf();										   //��ʼ��LED��GPIO����
  LCD_Init();
  while (1)
  {
	 LCD_Clear(0xf800);
  	LED0(On);					                           //��LED0
	delay_ms(1000);									   //��ʱԼ1s
	LED0(Off);											   //�ر�LED0
	delay_ms(1000);									   //��ʱԼ1s
		 LCD_Clear(0x001f);
    LED1(On);											   //��LED1
	delay_ms(1000);									   //��ʱԼ1s
	LED1(Off);											   //�ر�LED1
	delay_ms(1000);									   //��ʱԼ1s
  }
}


