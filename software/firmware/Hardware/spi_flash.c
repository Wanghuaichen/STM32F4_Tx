/*
    This project is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Deviation is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Deviation.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "stm32f4xx.h"
#include "common.h"

#define SPI_FLASH_PageSize              256                          //ҳ��С
#define SPI_FLASH_PerWritePageSize      256                          //ÿҳ��С

/*W25Xָ���*/
#define W25X_WriteEnable		            0x06                            //дʹ��ָ��
#define W25X_WriteDisable		            0x04                            //дʧ��ָ��
#define W25X_ReadStatusReg		          0x05                            //��״̬�Ĵ���
#define W25X_WriteStatusReg		          0x01                            //д״̬�Ĵ���
#define W25X_ReadData			              0x03                            //������ָ��
#define W25X_FastReadData		            0x0B                            //���ٶ�ģʽָ��
#define W25X_FastReadDual		            0x3B                            //���˫���ָ��    
#define W25X_PageProgram		            0x02                            //ҳд��ָ��
#define W25X_BlockErase			            0xD8                            //�����ָ��
#define W25X_SectorErase		            0x20                            //��������ָ��
#define W25X_ChipErase			            0xC7                            //Ƭ����ָ��
#define W25X_PowerDown			            0xB9                            //�������ģʽָ��
#define W25X_ReleasePowerDown	          0xAB                            //�˳�����ģʽ
#define W25X_DeviceID			              0xAB                            //��ȡоƬID
#define W25X_ManufactDeviceID        	  0x90                            //��ȡ����ID
#define W25X_JedecDeviceID		          0x9F 

#define WIP_Flag                        0x01                            //д��æ��־λ
#define Dummy_Byte                      0xFF                            //������

#define SPIFLASH_CS_LOW()       GPIO_ResetBits(GPIOA, GPIO_Pin_4)       //ʹ��Ƭѡ
#define SPIFLASH_CS_HIGH()      GPIO_SetBits(GPIOA, GPIO_Pin_4)         //ʧ��Ƭѡ


/**
* SPIFLASH_CS    <--->  PA4
* SPIFLASH_MOSI  <--->  PA7
* SPIFLASH_MISO  <--->  PA6
* SPIFLASH_CLK   <--->  PA5
**/
/******************************************************************************************
*��������SPI_FLASH_Init()
* ������void
* ����ֵ��void
* ���ܣ�SPIFLASH��ʼ���������ⲿ����
*********************************************************************************************/
void SPIFLASH_Init(void)
{
	  GPIO_InitTypeDef GPIO_InitStructure;
  	SPI_InitTypeDef  SPI_InitStructure;
  	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
  	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA ,ENABLE);
  	GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_SPI1);
  	GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_SPI1);
  	GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_SPI1);
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
  	GPIO_Init(GPIOA, &GPIO_InitStructure);
	  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	  GPIO_Init(GPIOA, &GPIO_InitStructure);
	  SPIFLASH_CS_HIGH();
	
  	SPI_I2S_DeInit(SPI1);
  	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;           //ȫ˫��
  	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;                            //8λ����ģʽ
  	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;                                    //����ģʽ��SCKΪ1
  	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;                                  //���ݲ����ӵ�2��ʱ����ؿ�ʼ
  	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;                                     //NSS�������
  	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;           //������
  	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;                             //���ģʽ
  	SPI_InitStructure.SPI_CRCPolynomial = 7;                                       //CRC����ʽ
  	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;                                  //����ģʽ
  	SPI_Init(SPI1, &SPI_InitStructure);
  	SPI_Cmd(SPI1, ENABLE);
}

/******************************************************************************************
*��������SPI_FLASH_SendByte()
* ������u8 byte        д�������
* ����ֵ��u8 8λ����
* ���ܣ�SPIFLASH��дһ���ֽں������ⲿ����
*********************************************************************************************/
static u8 SPIFLASH_SendByte(u8 byte)
{
  /*�ȴ�SPI���Ϳ���*/
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
  /*����8λ���� */
  SPI_I2S_SendData(SPI1, byte);
 /* �ȴ�����8λ���� */
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
  /* ���ؽ��յ�8λ���� */
  return SPI_I2S_ReceiveData(SPI1);
}

/******************************************************************************************
*��������SPIFLASH_WriteEnable()
* ������void
* ����ֵ��void
* ���ܣ�SPIFLASHдʹ�ܺ������ⲿ����
*********************************************************************************************/
static void SPIFLASH_WriteEnable(void)
{
   /* ʹ��Ƭѡ */
  SPIFLASH_CS_LOW();
  /*����дʹ��ָ��*/
  SPIFLASH_SendByte(W25X_WriteEnable);
  /*ʧ��Ƭѡ*/
  SPIFLASH_CS_HIGH();
}
/******************************************************************************************
*��������SPIFLASH_WriteDisable()
* ������void
* ����ֵ��void
* ���ܣ�SPIFLASHдʹ�ܺ������ⲿ����
*********************************************************************************************/
static void SPIFLASH_WriteDisable(void)
{
   /* ʹ��Ƭѡ */
  SPIFLASH_CS_LOW();
  /*����дʹ��ָ��*/
  SPIFLASH_SendByte(W25X_WriteDisable);
  /*ʧ��Ƭѡ*/
  SPIFLASH_CS_HIGH();
}

/******************************************************************************************
*��������SPI_FLASH_ReadDeviceID()
* ������void
* ����ֵ��u32 �豸ID
* ���ܣ�SPIFLASH��ȡ�豸ID�������ⲿ����
*********************************************************************************************/
u32 SPIFLASH_ReadDeviceID(void)
{
  u32 Temp = 0;
   /* ʹ��Ƭѡ */
  SPIFLASH_CS_LOW();
  /*���Ͷ�ȡIDָ��*/
  SPIFLASH_SendByte(W25X_DeviceID);
  SPIFLASH_SendByte(Dummy_Byte);
  SPIFLASH_SendByte(Dummy_Byte);
  SPIFLASH_SendByte(Dummy_Byte);
  /*��ȡ8λ����*/
  Temp = SPIFLASH_SendByte(Dummy_Byte);
  /*ʧ��Ƭѡ*/
  SPIFLASH_CS_HIGH();
  return Temp;
}

/******************************************************************************************
*��������SPI_FLASH_WaitForWriteEnd()
* ������void
* ����ֵ��void
* ���ܣ�SPIFLASH�ȴ�д��Ϻ������ⲿ����
*********************************************************************************************/
void SPIFLASH_WaitForWriteEnd(void)
{
  u8 FLASH_Status = 0;
   /* ʹ��Ƭѡ */
  SPIFLASH_CS_LOW();
  /*���Ͷ�״ָ̬�� */
  SPIFLASH_SendByte(W25X_ReadStatusReg);
  /*ѭ�����Ϳ�����ֱ��FLASHоƬ����*/
  do
  {
    /* ���Ϳ��ֽ� */
    FLASH_Status = SPIFLASH_SendByte(Dummy_Byte);
  }
  while ((FLASH_Status & WIP_Flag) == SET); /* ����Ƿ����*/
  /*ʧ��Ƭѡ*/
  SPIFLASH_CS_HIGH();
}

/******************************************************************************************
*��������SPI_Flash_PowerDown()
* ������void
* ����ֵ��void
* ���ܣ�SPIFLASH�������ģʽ�������ⲿ����
*********************************************************************************************/
void SPIFlash_PowerDown(void)   
{ 
  /* ʹ��Ƭѡ */
  SPIFLASH_CS_LOW();
  /*���͵���ָ�� */
  SPIFLASH_SendByte(W25X_PowerDown);
  /*ʧ��Ƭѡ*/
  SPIFLASH_CS_HIGH();
}   

/******************************************************************************************
*��������SPI_Flash_WAKEUP()
* ������void
* ����ֵ��void
* ���ܣ�SPIFLASH���ѵ���ģʽ�������ⲿ����
*********************************************************************************************/
void SPIFlash_WAKEUP(void)   
{
  /* ʹ��Ƭѡ */
  SPIFLASH_CS_LOW();
  /* �����˳�����ģʽָ�� */
  SPIFLASH_SendByte(W25X_ReleasePowerDown);
  /*ʧ��Ƭѡ*/
  SPIFLASH_CS_HIGH();              
}  

/******************************************************************************************
*��������SPI_FLASH_SectorErase()
* ������u32 SectorAddr   ���ַ
* ����ֵ��void
* ���ܣ�SPIFLASH���������������ⲿ����
*********************************************************************************************/
void SPIFLASH_SectorErase(u32 SectorAddr)
{
  /*����д����ʹ��ָ��*/
  SPIFLASH_WriteEnable();
	/*�ȴ�����д�꣬��֤д�����ǿ��е�*/
  SPIFLASH_WaitForWriteEnd();
  /* ʹ��Ƭѡ */
  SPIFLASH_CS_LOW();
  /*������������ָ��*/
  SPIFLASH_SendByte(W25X_SectorErase);
  /*���Ϳ��ַ��8λ*/
  SPIFLASH_SendByte((SectorAddr & 0xFF0000) >> 16);
  /*���Ϳ��ַ��8λ*/
  SPIFLASH_SendByte((SectorAddr & 0xFF00) >> 8);
  /*���Ϳ��ַ��8λ*/
  SPIFLASH_SendByte(SectorAddr & 0xFF);
  /*ʧ��Ƭѡ*/
  SPIFLASH_CS_HIGH();
  /* �ȴ�д���*/
  SPIFLASH_WaitForWriteEnd();
}

/******************************************************************************************
*��������SPI_FLASH_BulkErase()
* ������void
* ����ֵ��void
* ���ܣ�SPIFLASH��Ƭ�����������ⲿ����
*********************************************************************************************/
void SPIFLASH_BulkErase(void)
{
  /*ʹ��д��*/
  SPIFLASH_WriteEnable();
   /* ʹ��Ƭѡ */
  SPIFLASH_CS_LOW();
  /*������Ƭ����ָ��*/
  SPIFLASH_SendByte(W25X_ChipErase);
  /*ʧ��Ƭѡ*/
  SPIFLASH_CS_HIGH();
  /* �ȴ�д���*/
  SPIFLASH_WaitForWriteEnd();
}

/******************************************************************************************
*��������SPI_FLASH_PageWrite()
* ������u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite ����ָ�룬д���ַ��д��ĸ���
* ����ֵ��void
* ���ܣ�SPIFLASHҳд�����ݺ������ⲿ����
*********************************************************************************************/
void SPIFLASH_PageWrite(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite)
{
   /*ʹ��д��*/
  SPIFLASH_WriteEnable();
  /*ʹ��Ƭѡ*/
  SPIFLASH_CS_LOW();
  /* ����ҳд��ָ��*/
  SPIFLASH_SendByte(W25X_PageProgram);
  /*���͸�8λ���ݵ�ַ*/
  SPIFLASH_SendByte((WriteAddr & 0xFF0000) >> 16);
  /*������8λ���ݵ�ַ*/
  SPIFLASH_SendByte((WriteAddr & 0xFF00) >> 8);
  /*���͵�8λ���ݵ�ַ*/
  SPIFLASH_SendByte(WriteAddr & 0xFF);
  /*���д��������Ƿ񳬳�ҳ��������С*/
  if(NumByteToWrite > SPI_FLASH_PerWritePageSize)
  {
     NumByteToWrite = SPI_FLASH_PerWritePageSize;
  }
  /*ѭ��д������*/
  while (NumByteToWrite--)
  {
    /*��������*/
    SPIFLASH_SendByte(*pBuffer);
    /* ָ���Ƶ���һ��д������ */
    pBuffer++;
  }
  /*ʧ��Ƭѡ*/
  SPIFLASH_CS_HIGH();
  /* �ȴ�д���*/
  SPIFLASH_WaitForWriteEnd();
}

/******************************************************************************************
*��������SPI_FLASH_BufferWrite()
* ������u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite ����ָ�룬д���ַ��д��ĸ���
* ����ֵ��void
* ���ܣ�SPIFLASH������ݺ������ⲿ����
*********************************************************************************************/
void SPIFLASH_BufferWrite(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite)
{
  u8 NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;
  Addr = WriteAddr % SPI_FLASH_PageSize;                           //����д���ҳ�Ķ�Ӧ��ʼ��ַ
  count = SPI_FLASH_PageSize - Addr;
  NumOfPage =  NumByteToWrite / SPI_FLASH_PageSize;                //�����ܹ�Ҫд��ҳ��
  NumOfSingle = NumByteToWrite % SPI_FLASH_PageSize;               //����ʣ�൥��ҳд�����ݸ���
  if (Addr == 0) /* ���Ҫд���ҳ��ַΪ0��˵��������ҳд���ݣ�û��ƫ��*/
  {
    if (NumOfPage == 0) /* ��������д��ҳ��Ϊ0��˵����������һ��ҳ�ķ�Χ�ڣ���ֱ�ӽ���ҳ��д*/
    {
      SPIFLASH_PageWrite(pBuffer, WriteAddr, NumByteToWrite);      //����ҳд����
    }
    else /* ���Ҫд��ҳ������0*/
    { 
			/*�Ƚ���ͷ���ݽ�����ҳд��*/
      while (NumOfPage--)
      { 
				//��ҳд��
        SPIFLASH_PageWrite(pBuffer, WriteAddr, SPI_FLASH_PageSize);
				//��ַƫ��
        WriteAddr +=  SPI_FLASH_PageSize;
				//����ָ��ƫ��
        pBuffer += SPI_FLASH_PageSize;
      }
       //��ʣ�����ݸ���д��
      SPIFLASH_PageWrite(pBuffer, WriteAddr, NumOfSingle);
    }
  }
  else /*���д��ĵ�ַ����ҳ�Ŀ�ͷλ��*/
  {
    if (NumOfPage == 0) /*���д������ҳ�ĸ���Ϊ0��������С��һҳ����*/
    {
      if (NumOfSingle > count) /*���ʣ�����ݴ��ڵ�ǰҳ��ʣ������*/
      {
        temp = NumOfSingle - count;     //���㳬�������ݸ���
				/*д����ǰҳ*/
        SPIFLASH_PageWrite(pBuffer, WriteAddr, count);
				/*���õ�ַƫ��*/
        WriteAddr +=  count;
				/*��������ָ��ƫ��*/
        pBuffer += count;
        /*��ʣ����д���µ�ҳ*/
        SPIFLASH_PageWrite(pBuffer, WriteAddr, temp);
      }
      else  /*���ʣ������С�ڵ�ǰҳ��ʣ������*/
      {
				/*ֱ��д�뵱ǰҳ*/
        SPIFLASH_PageWrite(pBuffer, WriteAddr, NumByteToWrite);
      }
    }
    else /*���д������ҳ�ĸ�������0�������ݴ���һҳ����*/
    {
      NumByteToWrite -= count;         //�����ݼ�ȥ��ǰҳʣ�������
      NumOfPage =  NumByteToWrite / SPI_FLASH_PageSize;  //����Ҫд����ҳ����
      NumOfSingle = NumByteToWrite % SPI_FLASH_PageSize; //����ʣ�����ݸ���
      /*����ͷ����д�뵱ǰҳʣ���ֽڸ���*/
      SPIFLASH_PageWrite(pBuffer, WriteAddr, count);
			/*���õ�ַƫ��*/
      WriteAddr +=  count;
			/*��������ָ��ƫ��*/
      pBuffer += count;
       /*��ʼʣ�����ݵ���ҳд��*/
      while (NumOfPage--)
      {
				/*д��һ��ҳ���ֽ���*/
        SPIFLASH_PageWrite(pBuffer, WriteAddr, SPI_FLASH_PageSize);
				/*���õ�ַƫ��*/
        WriteAddr +=  SPI_FLASH_PageSize;
				/*����ָ��ƫ��*/
        pBuffer += SPI_FLASH_PageSize;
      }
      /*���ʣ�����ݴ���0����ʣ��ĸ���д����һ��ҳ*/
      if (NumOfSingle != 0)
      {
        SPIFLASH_PageWrite(pBuffer, WriteAddr, NumOfSingle);
      }
    }
  }
}

/******************************************************************************************
*��������SPI_FLASH_BufferRead()
* ������u8* pBuffer, u32 ReadAddr, u16 NumByteToRead ����ָ�룬�����ĵ�ַ�������ĸ���
* ����ֵ��void
* ���ܣ�SPIFLASH������ݺ������ⲿ����
*********************************************************************************************/
void SPIFLASH_BufferRead(u8* pBuffer, u32 ReadAddr, u16 NumByteToRead)
{
   /* ʹ��Ƭѡ */
  SPIFLASH_CS_LOW();
  /*���Ͷ�����ָ��*/
  SPIFLASH_SendByte(W25X_ReadData);
  /*����24λ���ݵ�ַ*/
  /* ���͸�8λ���ݵ�ַ*/
  SPIFLASH_SendByte((ReadAddr & 0xFF0000) >> 16);
  /*������8λ���ݵ�ַ*/
  SPIFLASH_SendByte((ReadAddr& 0xFF00) >> 8);
  /*���͵�8λ���ݵ�ַ*/
  SPIFLASH_SendByte(ReadAddr & 0xFF);
  while (NumByteToRead--) /* ѭ����ȡ����*/
  {
    /*��ȡһ���ֽ�����*/
    *pBuffer = SPIFLASH_SendByte(Dummy_Byte);
    /*����ָ���1*/
    pBuffer++;
  }
  /*ʧ��Ƭѡ*/
  SPIFLASH_CS_HIGH();
}

//int SPIFlash_ReadBytesStopCR(u32 readAddress, u32 length, u8 * buffer)
//{
//    u32 i;
//    SPIFlash_SetAddr(0x03, readAddress);

//    for(i=0;i<length;i++)
//    {
//        buffer[i] = ~spi_xfer(SPI1, 0);
//        if (buffer[i] == '\n') {
//            i++;
//            break;
//        }
//    }

//    CS_HI();
//    return i;
//}
