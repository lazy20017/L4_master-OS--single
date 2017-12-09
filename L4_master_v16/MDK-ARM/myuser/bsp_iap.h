#ifndef __BSP_IAP_H__
#define	__BSP_IAP_H__

/* ����ͷ�ļ� ----------------------------------------------------------------*/
#include "stm32l4xx_hal.h"
//#include "my_gloabal_val.h"

/* ���Ͷ��� ------------------------------------------------------------------*/
/************************** IAP �������Ͷ���********************************/
typedef  void ( * pIapFun_TypeDef ) ( void ); //����һ���������͵Ĳ���.

/* �궨�� --------------------------------------------------------------------*/
/************************** IAP ���������********************************/
//�Ƿ���� APP �� FLASH��������µ� RAM
#define User_Flash

#ifdef User_Flash
#define APP_START_ADDR2       	0x8030000  		//Ӧ�ó�����ʼ��ַ(�����FLASH)��bank������
#define APP_START_ADDR         	0x8010000  		//Ӧ�ó�����ʼ��ַ(�����FLASH)��ִ����
#define APP_START_DATA_ADD 			0X807F000     //���ݵ���ʼ����
#endif

/************************** IAP �ⲿ����********************************/
//#define APP_FLASH_LEN  			 (rsbuf3_max-1024) // 51k-1k
#define APP_update_status1_add  APP_START_DATA_ADD+0X20  //0x807F020   0x807F800
#define APP_update_status2_add  APP_START_DATA_ADD+0X28  //0x807F028

#define APP_update_length1_add 	APP_START_DATA_ADD+0X10		//0x807F010
#define APP_update_length2_add 	APP_START_DATA_ADD+0X18		//0x807F018

#define APP_update_password 0xF1F1



/* �������� ------------------------------------------------------------------*/
void IAP_Write_App_Bin( uint32_t appxaddr, uint8_t * appbuf, uint32_t applen);	//��ָ����ַ��ʼ,д��bin
void IAP_ExecuteApp( uint32_t appxaddr );			                              //ִ��flash�����app����

#endif /* __BSP_IAP_H__ */

/******************* (C) COPYRIGHT 2015-2020 ӲʯǶ��ʽ�����Ŷ� *****END OF FILE****/
void my_fun_update_get_data_to_flash(void);
void my_fun_update_send_data(uint8_t my_port);
//__asm void MSR_MSP ( uint32_t ulAddr );

uint8_t my_fun_write_update_data_to_FLASH(void);
void MX_CC1101_CS_Init(uint8_t use_status_on); //��ֹCC1101�˿�©�磬1ΪƬѡ�ź����ã�0λ���ã�����Ϊ����

