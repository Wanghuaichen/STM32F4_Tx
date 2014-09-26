#include "led.h"

/******************************************************************************************
*��������LED_GPIO_Conf()
* ������void
* ����ֵ��void
* ���ܣ�LED�Ĺܽ�����
*********************************************************************************************/
void LED_GPIO_Conf(void)
{
   GPIO_InitTypeDef GPIO_InitStructure;		                                              //����GPIO��ʼ���ṹ��
   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOB, ENABLE);			  //ʹ��GPIOA��GPIOB��AHP1ʱ��
   GPIO_InitStructure.GPIO_Pin=GPIO_Pin_1;												  //�����ʼ���Ĺܽ�ΪPin1
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;										  //���ó�ʼ��ģʽΪ���ģʽ
   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;										  //�����������Ϊ�������
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;									  //��������ٶ�Ϊ100Mhz
   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;										  
   GPIO_Init(GPIOA, &GPIO_InitStructure);												  //��ʼ��GPIOA��Pin_1�ܽ�
   GPIO_InitStructure.GPIO_Pin=GPIO_Pin_14;												  //�����ʼ���Ĺܽ�ΪPin14
   GPIO_Init(GPIOB, &GPIO_InitStructure);												  //��ʼ��GPIOB��Pin_14�ܽ�
   GPIO_SetBits(GPIOA,GPIO_Pin_1);														  //ʹGPIOA��Pin_1����ߵ�ƽ
   GPIO_SetBits(GPIOB,GPIO_Pin_14);														  //ʹGPIOB��Pin_14����ߵ�ƽ
}