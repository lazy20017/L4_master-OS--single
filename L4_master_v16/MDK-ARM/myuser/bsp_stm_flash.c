#include "bsp_stm_flash.h"
/*
�ڲ�FLASH����Ĺ���
#define APP_START_ADDR2       	0x8030000  		//Ӧ�ó�����ʼ��ַ(�����FLASH)��bank������
#define APP_START_ADDR         	0x8010000  		//Ӧ�ó�����ʼ��ַ(�����FLASH)��ִ����
#define APP_START_DATA_ADD 			0X807F000     //���ݵ���ʼ����,�ֶ���ʹ�ã�

��BSP_IAP.h�ļ����ж��壬�ص㿴����ļ�

��д�ķ�����������ʹ��8���ֽ�Ϊһ����λ�ķ�������Ϊ8���ֽ���һ�������������ȽϺò�����
���ÿ����������8���ֽڡ�


*/





/* ˽�����Ͷ��� --------------------------------------------------------------*/
/* ˽�к궨�� ----------------------------------------------------------------*/
//#if STM32_FLASH_SIZE < 256
//  #define STM_SECTOR_SIZE  1024 //�ֽ�
//#else
//  #define STM_SECTOR_SIZE	 2048
//#endif


/* ˽�б��� ------------------------------------------------------------------*/
#if STM32_FLASH_WREN	//���ʹ����д 
static uint64_t STMFLASH_BUF [ STM_SECTOR_SIZE / 8 ];//�����2K�ֽ�
static FLASH_EraseInitTypeDef EraseInitStruct;
#endif

/* ��չ���� ------------------------------------------------------------------*/
/* ˽�к���ԭ�� --------------------------------------------------------------*/
/* ������ --------------------------------------------------------------------*/
/**
  * ��������: ��ȡָ����ַ�İ���(16λ����)
  * �������: faddr:����ַ(�˵�ַ����Ϊ2�ı���!!)
  * �� �� ֵ: ����ֵ:��Ӧ����.
  * ˵    ������
  */
uint16_t STMFLASH_ReadHalfWord ( uint32_t faddr )
{
    return *(__IO uint16_t*)faddr;
}
uint64_t STMFLASH_ReadDoubleWord ( uint32_t faddr )
{
    return *(__IO uint64_t*)faddr;
}
#if STM32_FLASH_WREN	//���ʹ����д   
/**
  * ��������: ������д��
  * �������: WriteAddr:��ʼ��ַ
  *           pBuffer:����ָ��
  *           NumToWrite:����(16λ)��
  * �� �� ֵ: ��
  * ˵    ������
  */
void STMFLASH_Write_NoCheck ( uint32_t WriteAddr, uint16_t * pBuffer, uint16_t NumToWrite )
{
    uint16_t i;

    for(i=0; i<NumToWrite; i++)
    {
        //HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,WriteAddr,pBuffer[i]);  FLASH_TYPEPROGRAM_DOUBLEWORD

        WriteAddr+=2;                                    //��ַ����2.
    }
}

void STMFLASH_Write_NoCheck_64bit ( uint32_t WriteAddr, uint64_t * pBuffer, uint16_t NumToWrite )
{
    uint16_t i;

    for(i=0; i<NumToWrite; i++)
    {

        HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD,WriteAddr,pBuffer[i]);
        WriteAddr+=2;                                    //��ַ����2.
    }
}

/**
  * ��������: ��ָ����ַ��ʼд��ָ�����ȵ�����
  * �������: WriteAddr:��ʼ��ַ(�˵�ַ����Ϊ2�ı���!!)
  *           pBuffer:����ָ��
  *           NumToWrite:����(16λ)��(����Ҫд���16λ���ݵĸ���.)
  * �� �� ֵ: ��
  * ˵    ������
  */
//uint64_t my_buf64[256]= {0};
void STMFLASH_Write ( uint32_t WriteAddr, uint64_t * pBuffer, uint16_t NumToWrite )
{
    uint32_t SECTORError = 0;
    uint16_t secoff;	   //������ƫ�Ƶ�ַ(64λ�ּ���)
    uint16_t secremain; //������ʣ���ַ(64λ�ּ���)
    uint16_t i;
    uint32_t secpos;	   //������ַ
    uint32_t offaddr;   //ȥ��0X08000000��ĵ�ַ

    if(WriteAddr<FLASH_BASE||(WriteAddr>=(FLASH_BASE+1024*STM32_FLASH_SIZE)))return;//�Ƿ���ַ

    HAL_FLASH_Unlock();						//����

    offaddr=WriteAddr-FLASH_BASE;		//ʵ��ƫ�Ƶ�ַ.
    secpos=offaddr/STM_SECTOR_SIZE;			//������ַ  0~127 for STM32F103RBT6
    secoff=(offaddr%STM_SECTOR_SIZE)/8;		//�������ڵ�ƫ��(8���ֽ�Ϊ������λ.)
    secremain=STM_SECTOR_SIZE/8-secoff;		//����ʣ��ռ��С
    if((NumToWrite)<=secremain)secremain=NumToWrite;//�����ڸ�������Χ

    while(1)
    {
        STMFLASH_Read(secpos*STM_SECTOR_SIZE+FLASH_BASE,STMFLASH_BUF,STM_SECTOR_SIZE/8);//������������������
        //STMFLASH_Read(secpos*STM_SECTOR_SIZE+FLASH_BASE,my_buf64,STM_SECTOR_SIZE/8);//@@@@@
        for(i=0; i<secremain; i++) //У������
        {
            if(STMFLASH_BUF[secoff+i]!=0XFFFFFFFFFFFFFFFF)
                break;//��Ҫ����
        }
        if(i<secremain)//��Ҫ����
        {
            //�����������
            /* Fill EraseInit structure*/
            EraseInitStruct.Banks=FLASH_BANK_1;
            EraseInitStruct.TypeErase     = FLASH_TYPEERASE_PAGES;//FLASH_TYPEERASE_PAGES;
            EraseInitStruct.Page   = secpos;//*STM_SECTOR_SIZE+FLASH_BASE;
            EraseInitStruct.NbPages       = 1;

            if(secpos>127)
            {
                secpos=secpos+128;
                EraseInitStruct.Page   = secpos;
                EraseInitStruct.Banks=FLASH_BANK_2;
            }

            //HAL_FLASH
            HAL_FLASHEx_Erase(&EraseInitStruct, &SECTORError);

            //STMFLASH_Read(secpos*STM_SECTOR_SIZE+FLASH_BASE,my_buf64,STM_SECTOR_SIZE/8);//@@@@@
						if(secpos>127)
            {
                secpos=secpos-128;
                            
						}
            for(i=0; i<secremain; i++) //����
            {
                STMFLASH_BUF[i+secoff]=pBuffer[i];
            }
            STMFLASH_Write_NoCheck_64bit(secpos*STM_SECTOR_SIZE+FLASH_BASE,STMFLASH_BUF,STM_SECTOR_SIZE/8);//д����������
            //STMFLASH_Read(secpos*STM_SECTOR_SIZE+FLASH_BASE,my_buf64,STM_SECTOR_SIZE/8);//@@@@@
        }
        else
        {
            STMFLASH_Write_NoCheck_64bit(WriteAddr,pBuffer,secremain);//д�Ѿ������˵�,ֱ��д������ʣ������.
            //STMFLASH_Read(secpos*STM_SECTOR_SIZE+FLASH_BASE,my_buf64,STM_SECTOR_SIZE/8);//@@@@@
        }

        if(NumToWrite==secremain)break;//д�������
        else//д��δ����
        {
            secpos++;				//������ַ��1
            secoff=0;				//ƫ��λ��Ϊ0
            pBuffer+=secremain;  	//ָ��ƫ��
            WriteAddr+=secremain;	//д��ַƫ��
            NumToWrite-=secremain;	//�ֽ�(16λ)���ݼ�
            if(NumToWrite>(STM_SECTOR_SIZE/8))secremain=STM_SECTOR_SIZE/8;//��һ����������д����
            else secremain=NumToWrite;//��һ����������д����
        }
    };
    HAL_FLASH_Lock();//����
}
#endif

/**
  * ��������: ��ָ����ַ��ʼ����ָ�����ȵ�����
  * �������: ReadAddr:��ʼ��ַ
  *           pBuffer:����ָ��
  *           NumToRead:����(16λ)��
  * �� �� ֵ: ��
  * ˵    ������
  */
void STMFLASH_Read ( uint32_t ReadAddr, uint64_t *pBuffer, uint16_t NumToRead )
{
    uint16_t i;

    for(i=0; i<NumToRead; i++)
    {
        pBuffer[i]=STMFLASH_ReadDoubleWord(ReadAddr);//��ȡ2���ֽ�.
        ReadAddr+=8;//ƫ��8���ֽ�.
    }
}


