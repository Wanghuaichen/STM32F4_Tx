 /******************************************************************************
  * �ļ�·��  ��   User/main.c 
  * ����      ��  ɽ�ҵ��ӹ�����
  * �̼���汾��   V1.0.0
  * ����      ��   10-October-2011
  * �ļ�����  ��   ������� main()����
  ******************************************************************************/
/* -------------------������ͷ�ļ�-----------------------------------------------*/
#include "stm32f4xx.h"
#include "delay.h"
#include "led.h"


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
//  USART1_Conf();
//  printf("\r\n��ӭʹ��ɽ�ҵ���STM32F407VG�����壡\r\n");
//  printf("\r\n        ɽ�ҵ��ӹ����ң�\r\n");
//    printf("\r\n            -----רҵ��ֵ��������\r\n");
  while (1)
  {
  	LED0(On);					                           //��LED0
	delay_nms(1000);									   //��ʱԼ1s
	LED0(Off);											   //�ر�LED0
	delay_nms(1000);									   //��ʱԼ1s
    LED1(On);											   //��LED1
	delay_nms(1000);									   //��ʱԼ1s
	LED1(Off);											   //�ر�LED1
	delay_nms(1000);									   //��ʱԼ1s
  }
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

