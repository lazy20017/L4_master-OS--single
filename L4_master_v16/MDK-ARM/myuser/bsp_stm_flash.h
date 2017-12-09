#include "stm32l4xx_hal.h"



/* ���Ͷ��� ------------------------------------------------------------------*/
/* �궨�� --------------------------------------------------------------------*/
/************************** STM32 �ڲ� FLASH ���� *****************************/
#define STM32_FLASH_SIZE        512  // ��ѡSTM32��FLASH������С(��λΪK)
#define STM32_FLASH_WREN        1    // stm32оƬ����FLASH д��ʹ��(0������;1��ʹ��)


#if STM32_FLASH_SIZE < 256
#define STM_SECTOR_SIZE  1024 //�ֽ�
#else
#define STM_SECTOR_SIZE	 2048
#endif

/* ��չ���� ------------------------------------------------------------------*/
/* �������� ------------------------------------------------------------------*/
uint16_t STMFLASH_ReadHalfWord(uint32_t faddr);		  //��������

//void STMFLASH_WriteLenByte(uint32_t WriteAddr, uint32_t DataToWrite, uint16_t Len );	 //ָ����ַ��ʼд��ָ�����ȵ�����
//uint32_t STMFLASH_ReadLenByte(uint32_t ReadAddr, uint16_t Len );					                    	//ָ����ַ��ʼ��ȡָ����������
void STMFLASH_Write( uint32_t WriteAddr, uint64_t * pBuffer, uint16_t NumToWrite );		//��ָ����ַ��ʼд��ָ�����ȵ�����
void STMFLASH_Read( uint32_t ReadAddr, uint64_t * pBuffer, uint16_t NumToRead );   	//��ָ����ַ��ʼ����ָ�����ȵ�����

void my_fun_flash_test1(void);

