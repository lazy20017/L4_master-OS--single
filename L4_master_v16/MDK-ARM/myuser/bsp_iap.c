/**
  ******************************************************************************
  * �ļ�����: bsp_iap.c
  * ��    ��: ӲʯǶ��ʽ�����Ŷ�
  * ��    ��: V1.0
  * ��д����: 2015-10-04
  * ��    ��: IAP�ײ�����ʵ��
  ******************************************************************************
  * ˵����
  * ����������Ӳʯstm32������YS-F1Proʹ�á�
  *
  * �Ա���
  * ��̳��http://www.ing10bbs.com
  * ��Ȩ��ӲʯǶ��ʽ�����Ŷ����У��������á�
  ******************************************************************************
  */
/* ����ͷ�ļ� ----------------------------------------------------------------*/
#include "bsp_iap.h"
#include "bsp_stm_flash.h"
#include "my_cc1101.h"
#include "my_usart.h"
#include "bsp_iap.h"
/* ˽�����Ͷ��� --------------------------------------------------------------*/
/* ˽�к궨�� ----------------------------------------------------------------*/
/* ˽�б��� ------------------------------------------------------------------*/



uint64_t ulBuf_Flash_App[256];



/* ��չ���� ------------------------------------------------------------------*/
/* ˽�к���ԭ�� --------------------------------------------------------------*/
/* ������ --------------------------------------------------------------------*/


__asm void MSR_MSP ( uint32_t ulAddr )
{
    MSR MSP, r0 			                   //set Main Stack value
    BX r14
}
//��ת��Ӧ�ó����
//ulAddr_App:�û�������ʼ��ַ.
void IAP_ExecuteApp ( uint32_t ulAddr_App )
{
    pIapFun_TypeDef pJump2App;
    uint32_t my_add=( * ( __IO uint32_t * ) ulAddr_App ) ;

    if ( ( my_add& 0x2FFE0000 ) == 0x20000000 )	  //���ջ����ַ�Ƿ�Ϸ�.
    {
        printf("ջ����ȷ  %XH\r\n",my_add);
        pJump2App = ( pIapFun_TypeDef ) * ( __IO uint32_t * ) ( ulAddr_App + 4 );	//�û��������ڶ�����Ϊ����ʼ��ַ(��λ��ַ)
        MSR_MSP( * ( __IO uint32_t * ) ulAddr_App );					                            //��ʼ��APP��ջָ��(�û��������ĵ�һ�������ڴ��ջ����ַ)
        pJump2App ();								                                    	//��ת��APP.
    }
    else
    {
        printf("ջ������  %XH\r\n",my_add);
    }
}




/*
�������ܣ���ָ�����ȵ����ݣ�д�뵽FLASH��
��������һ��Ϊ��FLASH�׵�ַ����2��Ϊ���ݻ��������ֽ�Ϊ��λ����3��д���ֽ���
*/

void IAP_Write_App_Bin ( uint32_t ulStartAddr, uint8_t * pBin_DataBuf, uint32_t ulBufLength )
{


    uint16_t us, usCtr=0;
    uint64_t usTemp;
    uint32_t ulAdd_Write = ulStartAddr;                                //��ǰд��ĵ�ַ
    uint8_t * pData = pBin_DataBuf;

    for ( us = 0; us < ulBufLength; us += 4 )
    {
        usTemp =  ( uint64_t ) pData[3];
        usTemp=(usTemp<<8)+( uint64_t ) pData[2];
        usTemp=(usTemp<<8)+( uint64_t ) pData[1];
        usTemp=(usTemp<<8)+( uint64_t ) pData[0];

        pData += 4;                                                      //ƫ��4���ֽ�
        ulBuf_Flash_App [ usCtr ++ ] = usTemp;
        if ( usCtr == 256 )
        {
            usCtr = 0;
            STMFLASH_Write ( ulAdd_Write, ulBuf_Flash_App, 256 );	//д�ĵ�λΪ2���ֽڣ����Ǹ��ڵ�ַ����1���ֽ�Ϊ��λ��һ��д1������2048���ֽ�
            ulAdd_Write += 2048;                                           //ƫ��2048  16=2*8.����Ҫ����2.
        }
    }
    if ( usCtr )
        STMFLASH_Write ( ulAdd_Write, ulBuf_Flash_App, usCtr );//������һЩ�����ֽ�д��ȥ.


}



/*

*/
extern uint8_t my_CC1101_COM_Fram_buf[];
uint32_t my_update_length=0;
uint8_t my_fun_write_update_data_to_FLASH(void)
{
    uint8_t temp_status=0;
    uint8_t my_block=0;
    uint16_t my_length=0;

    if(my_CC1101_COM_Fram_buf[0]==0x10)
    {
        temp_status=my_CC1101_COM_Fram_buf[1];
        my_fun_display_buf_16(my_CC1101_COM_Fram_buf,8,0);  //����ʹ�ã���ʾ���յ�������8���ֽ�
    }
    else if (my_CC1101_COM_Fram_buf[0]==0x68)
    {
        temp_status=my_CC1101_COM_Fram_buf[6];
        my_block=my_CC1101_COM_Fram_buf[7];  //��ÿ��

        my_length=my_CC1101_COM_Fram_buf[2];
        my_length=(my_length<<8)+my_CC1101_COM_Fram_buf[1]-2; //��ó���

        my_fun_display_buf_16(my_CC1101_COM_Fram_buf,8,0);   //����ʹ�ã���ʾ���յ�������8���ֽ�
    }

    //------
    if(temp_status==0xF0)
    {
        printf("�����������ݿ�ʼ  F0\r\n");
        my_update_length=0;
        return 1;

    }
    else if(temp_status==0xF2)
    {
        if(my_length==STM_SECTOR_SIZE)
        {
            printf("���յ����ݰ�������д��FLASH  block=%d   F2\r\n",my_block);
            IAP_Write_App_Bin(APP_START_ADDR2+my_block*STM_SECTOR_SIZE,(my_CC1101_COM_Fram_buf+8),my_length);

            printf("FLASH д��ɹ�-----block=%d   length=%d    F2\r\n",my_block,my_length);
            my_update_length=my_update_length+my_length;
            return 1;
        }
        else
        {
            printf("���յ������ݿⳤ�ȴ���  block=%d  length=%d\r\n   F4",my_block,my_length);
            return 0;
        }

    }
    else if(temp_status==0xF4)
    {
        printf("�����������ݰ�   F4\r\n");
        return 1;
    }
    else if(temp_status==0xF6)
    {

        if(my_length<=STM_SECTOR_SIZE)
        {
            printf("���յ�--���---���ݰ�������д��FLASH  block=%d   F6\r\n",my_block);
            IAP_Write_App_Bin(APP_START_ADDR2+my_block*STM_SECTOR_SIZE,(my_CC1101_COM_Fram_buf+8),my_length);
            printf("FLASH --������ݰ�---д��ɹ�-----block=%d   length=%d     F6\r\n",my_block,my_length);
            my_update_length=my_update_length+my_length;
            if(my_update_length==(my_block*STM_SECTOR_SIZE+my_length))
            {
                printf("==д��FLASH����   OK   data=[%XH]\r\n",my_update_length);
                return 1;
            }
            else
            {
                printf("==д��FLASH����  ERROR!!   right_data=[%XH], error_data=[%XH\r\n",(my_block*STM_SECTOR_SIZE+my_length),my_update_length);
                return 0;
            }

        }
        else
        {
            printf("���յ����������ݳ��ȴ���  block=%d  length=%d\r\n   F6",my_block,my_length);

            return 0;
        }



    }
    else if(temp_status==0xF8)
    {
        uint64_t passsword=APP_update_password;
        uint64_t length_L=my_update_length;


        STMFLASH_Write(APP_update_status1_add,&passsword,1);
        STMFLASH_Write(APP_update_status2_add,&passsword,1);

        STMFLASH_Write(APP_update_length1_add,&length_L,1);

        STMFLASH_Write(APP_update_length2_add,&length_L,1);


        printf("�������ݴ������   F8\r\n");
        return 1;
    }

    return 0;

}

