#include "my_cc1101.h"
#include "cmsis_os.h"
//#include "my_def_value.h"
#include "my_gloabal_val.h"
#include "my_led.h"
#include "my_extern_val.h"
#include "main.h"






#define RF_reg_count 47
#define my_CC1101_default_channer 0x02    //оƬĬ��CC1101�ŵ�





uint16_t my_CC1101_sync_word = 0x0102; //CC1101ͬ���֣�Ĭ��Ϊ0x8799
uint8_t my_CC1101_change_channer = my_CC1101_default_channer; //��̬�ŵ���

uint8_t my_CC1101_chip_address = 0x01; //����Դ��ַ01 02 03 ZSQ��ַ1,2,3��---9
uint8_t my_CC1101_dest_address = 0xFE; //����Ŀ���ַ��0XFEΪDTU��0xFDΪ��������0x00Ϊ�㲥

//ָʾ����ַ���ɣ�ͬ���ֵ��ֽ�+ͬ���ָ��ֽ�+�ŵ��ֽ�+�ŵ��ڵ�ַ�ֽ�(01��02��03)
//���� 01-02-03-01



//10, 7, 5, 0, -5, -10, -15, -20, dbm output power, 0x12 == -30dbm
//INT8U PaTabel[] = { 0xc0, 0xC8, 0x84, 0x60, 0x68, 0x34, 0x1D, 0x0E};
INT8U PaTabel[] = { 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0};

//extern uint8_t MY_CC1101_STATUS;
//extern uint8_t Cmd_CC1101_RX[];
//====================
uint8_t MY_CC1101_STATUS = 0; //��ʶCC1101��ǰ״̬������Ĭ��00��TX Ϊ01��RXΪ02��WORΪ03
uint8_t MY_TX_RX_STEP = 0; //����Ĳ���

//uint8_t Cmd_CC1101_TX[70]={0};
//uint8_t Cmd_CC1101_RX[70]={0};
//uint16_t my_cmd_cc1101_step=0;
uint8_t my_get_client_add = 0; //CC1101���յ����ݵĿͻ��˵�ַ��

#define my_CC1101_RX_MAX  5000
uint8_t my_CC1101_re_buf[my_CC1101_RX_MAX]; //CC1101�Ļ�����,1������
uint16_t my_CC1101_re_buf_pt_write = 0;
uint16_t my_CC1101_re_buf_pt_read = 0;
uint8_t my_CC1101_COM_Fram_buf[2000]; //ָ�����
uint8_t my_CC1101_Frame_status = 0;
INT8U RS_buf_status[2] = {0}; //�����洢���յ������ݵ�RSSI��crc���
uint8_t my_cc1101_tx_buf[64] = {0x10, 0x20, 0x13, 0x14, 0x15, 0x16};
extern uint8_t my_get_count;



extern uint8_t rsbuf2[];
extern	uint8_t USART2_FRAME_status;
extern	uint8_t USART2_my_frame[];
extern uint16_t rsbuf2pt_write;
extern uint16_t rsbuf2pt_read;

//=========
extern osMutexId myMutex01Handle;

static const uint8_t CC1101InitData[RF_reg_count][2] =
{   /*
    {CC1101_IOCFG2,      0x0E},  //
    {CC1101_IOCFG1,      0x2E},
    {CC1101_IOCFG0,      0x07},  //
    {CC1101_FIFOTHR,     0x47},  //Ĭ��47���������
    {CC1101_PKTCTRL1,    0x06},  //06
    {CC1101_PKTCTRL0,    0x05},  //
    {CC1101_CHANNR,      0x01},  //
    {CC1101_FSCTRL1,     0x06},  //
    {CC1101_FREQ2,       0x0F},
    {CC1101_FREQ1,       0x62},
    {CC1101_FREQ0,       0x76},
    {CC1101_MDMCFG4,     0xF6},
    {CC1101_MDMCFG3,     0x43},
    {CC1101_MDMCFG2,     0x13},
    {CC1101_DEVIATN,     0x15},
    {CC1101_MCSM0,       0x18},  //
    {CC1101_FOCCFG,      0x16},
    {CC1101_WORCTRL,     0xFB},  //
    {CC1101_FSCAL3,      0xE9},
    {CC1101_FSCAL2,      0x2A},
    {CC1101_FSCAL1,      0x00},
    {CC1101_FSCAL0,      0x1F},
    {CC1101_TEST2,       0x81},
    {CC1101_TEST1,       0x35},
    {CC1101_MCSM1,       0x3B},//============
    */
    {CC1101_IOCFG2,      0x0E},
    {CC1101_IOCFG1,      0x2E},
    {CC1101_IOCFG0,      0x06},  //0X06��������ʱΪ0��������λ�ߵ�ƽ��0X46��������Ϊ�ߵ�ƽ��������λ�͵�ƽ
    {CC1101_FIFOTHR,     0x4E},
    {CC1101_SYNC1,       0x87},
    {CC1101_SYNC0,       0x99},
    {CC1101_PKTLEN,      0xFF},
    {CC1101_PKTCTRL1,    0x06},  //0x06������ַУ�飬04����������ַУ��
    {CC1101_PKTCTRL0,    0x05},  //05 0000 0101
    {CC1101_ADDR,        0x05},
    {CC1101_CHANNR,      my_CC1101_default_channer},//�ŵ�
    {CC1101_FSCTRL1,     0x06},
    {CC1101_FSCTRL0,     0x00},
    {CC1101_FREQ2,       0x0F},
    {CC1101_FREQ1,       0x62},
    {CC1101_FREQ0,       0x76},
    {CC1101_MDMCFG4,     0xFA},  //2.4K-0xF6 40K-FA  50K-0XFA 60k-FB
    {CC1101_MDMCFG3,     0x93},  //2.4K-0x83 40K-93  50K-0XF8 60K-2E
    {CC1101_MDMCFG2,     0x13},  //Ĭ��Ϊ0X13
    {CC1101_MDMCFG1,     0x22},
    {CC1101_MDMCFG0,     0xF8},
    {CC1101_DEVIATN,     0x15},
    {CC1101_MCSM2,       0x07},
    {CC1101_MCSM1,       0x30},
    {CC1101_MCSM0,       0x18},
    {CC1101_FOCCFG,      0x16},
    {CC1101_BSCFG,       0x6C},
    {CC1101_AGCCTRL2,    0x03},
    {CC1101_AGCCTRL1,    0x40},
    {CC1101_AGCCTRL0,    0x91},
    {CC1101_WOREVT1,     0x87},
    {CC1101_WOREVT0,     0x6B},
    {CC1101_WORCTRL,     0xFB},
    {CC1101_FREND1,      0x56},
    {CC1101_FREND0,      0x10},  //���͹���ѡ��00
    {CC1101_FSCAL3,      0xE9},
    {CC1101_FSCAL2,      0x2A},
    {CC1101_FSCAL1,      0x00},
    {CC1101_FSCAL0,      0x1F},
    {CC1101_RCCTRL1,     0x41},
    {CC1101_RCCTRL0,     0x00},
    {CC1101_FSTEST,      0x59},
    {CC1101_PTEST,       0x7F},
    {CC1101_AGCTEST,     0x3F},
    {CC1101_TEST2,       0x81},
    {CC1101_TEST1,       0x35},
    {CC1101_TEST0,       0x0B},
};






/*===========================================================================
* ���� ��SPI_ExchangeByte() => ͨ��SPI�������ݽ���                          *
* ���� ����Ҫд��SPI��ֵ                                                    *
* ��� ��ͨ��SPI������ֵ                                                    *
============================================================================*/
uint8_t SPI_ExchangeByte(uint8_t input)
{
    uint8_t re_data = 0;
    //while (RESET == SPI_GetFlagStatus(SPI_FLAG_TXE));   // �ȴ����ݴ������
    //SPI_SendData(input);
    //while (RESET == SPI_GetFlagStatus(SPI_FLAG_RXNE));  // �ȴ����ݽ������
    //return (SPI_ReceiveData());

    while(HAL_SPI_GetState(&hspi3) == HAL_SPI_STATE_BUSY_TX); // �ȴ����ݴ������
    HAL_SPI_TransmitReceive(&hspi3, &input, &re_data, 1, 1000);


    return re_data;

}



/*===========================================================================
* ���� ��RF_Initial() => ��ʼ��RFоƬ                                       *
* ˵�� ��CC1101�Ĳ������Ѿ�������C�⣬��CC1101.c�ļ��� �ṩSPI��CSN������	*
         ���ɵ������ڲ����к����û������ٹ���CC1101�ļĴ����������⡣       *
============================================================================*/
void RF_Initial(void)
{
    CC1101Init();               // ��ʼ��CC1101�Ĵ���
    CC1101SetTRMode(RX_MODE);   // ����ģʽ
}

/*
================================================================================
Function : CC1101RecPacket( )
    Receive a packet
INPUT    : rxBuffer, A buffer store the received data
OUTPUT   : 1:received count, 0:no data
================================================================================
*/


INT8U CC1101RecPacket( INT8U *rxBuffer )
{

    INT8U pktLen = 0;
//    INT16U x;
    //j = 0;

    if ( CC1101GetRXCnt( ) != 0 )
    {
        pktLen = CC1101ReadReg(CC1101_RXFIFO);           // Read length byte

        if( pktLen == 0 ||  pktLen > 60)
        {

            printf("CC1101 get length [%d] data!! error\r\n", pktLen);
            return 0;
        }
        else
        {
            ;
        } // { pktLen --; }
        CC1101ReadMultiReg(CC1101_RXFIFO, rxBuffer, pktLen); // Pull data
        CC1101ReadMultiReg(CC1101_RXFIFO, RS_buf_status, 2);   // Read  status bytes
        //my_fun_display_buf_16(rxBuffer,pktLen+2);//��ʾ���յ�������

        CC1101ClrRXBuff( );


        if( RS_buf_status[1] & CRC_OK )  //������ȷ
        {

            int my_rssi_dbm = 0;
            my_rssi_dbm = RS_buf_status[0]; //RSSI
            if(my_rssi_dbm >= 128)
                my_rssi_dbm = (my_rssi_dbm - 256) / 2 - 75;
            else
                my_rssi_dbm = (my_rssi_dbm) / 2 - 75;

            printf("*** RSSI=[%d] ***\n", my_rssi_dbm);
            return pktLen;
        }
        else
        { int my_rssi_dbm = 0;
            my_rssi_dbm = RS_buf_status[0]; //RSSI
            if(my_rssi_dbm >= 128)
                my_rssi_dbm = (my_rssi_dbm - 256) / 2 - 75;
            else
                my_rssi_dbm = (my_rssi_dbm) / 2 - 75;  
					

            printf("CC1101 CRC error\r\n");
            printf("RSSI=[%d]--CRC=[%XH]-length=[%d]\r\n",my_rssi_dbm,RS_buf_status[1],pktLen);
            pktLen = 0;
            return 0;
        }
    }
    else
    {
        printf("CC1101 get cnt 0 data!!\r\n");
        return 0;    // Error
    }
}





/*
================================================================================
Function : CC1101ClrTXBuff( )
    Flush the TX buffer of CC1101
INPUT    : None
OUTPUT   : None
================================================================================
*/
void CC1101ClrTXBuff( void )
{
    CC1101SetIdle();//MUST BE IDLE MODE
    CC1101WriteCmd( CC1101_SFTX );
}

/*
================================================================================
Function : CC1101SetIdle( )
    Set the CC1101 into IDLE mode
INPUT    : None
OUTPUT   : None
================================================================================
*/
void CC1101SetIdle( void )
{
    CC1101WriteCmd(CC1101_SIDLE);
}

/*
================================================================================
Function : CC1101ClrRXBuff( )
    Flush the RX buffer of CC1101
INPUT    : None
OUTPUT   : None
================================================================================
*/
void CC1101ClrRXBuff( void )
{
    CC1101SetIdle();//MUST BE IDLE MODE
    CC1101WriteCmd( CC1101_SFRX );
}

/*
================================================================================
Function : CC1101ReadMultiReg( )
    Read some bytes from the rigisters continously
INPUT    : addr, The address of the register
           buff, The buffer stores the data
           size, How many bytes should be read
OUTPUT   : None
================================================================================
*/
void CC1101ReadMultiReg( INT8U addr, INT8U *buff, INT8U size )
{
    INT8U i, j;
    CC_CSN_LOW( );
    SPI_ExchangeByte( addr | READ_BURST);
    for( i = 0; i < size; i ++ )
    {
        for( j = 0; j < 20; j ++ );
        *( buff + i ) = SPI_ExchangeByte( 0xFF );
    }
    CC_CSN_HIGH( );
}



/*
================================================================================
Function : CC1101GetRXCnt( )
    Get received count of CC1101
INPUT    : None
OUTPUT   : How many bytes hae been received
================================================================================
*/
INT8U CC1101GetRXCnt( void )
{
    return ( CC1101ReadStatus( CC1101_RXBYTES )  & BYTES_IN_RXFIFO );
}

/*
================================================================================
Function : CC1101SetTRMode( )
    Set the device as TX mode or RX mode
INPUT    : mode selection
OUTPUT   : None
================================================================================
*/
void CC1101SetTRMode( TRMODE mode )
{
    if( mode == TX_MODE )
    {
        CC1101WriteReg(CC1101_IOCFG0, 0x06);
        CC1101WriteCmd( CC1101_STX );
    }
    else if( mode == RX_MODE )
    {
        CC1101WriteReg(CC1101_IOCFG0, 0x06);
        CC1101WriteCmd( CC1101_SRX );
    }
}



/*
================================================================================
Function : CC1101Init( )
    Initialize the CC1101, User can modify it
INPUT    : None
OUTPUT   : None
================================================================================
*/
void CC1101Init( void )
{
    volatile uint8_t i, j;

    CC1101Reset( );

    for( i = 0; i < RF_reg_count; i++ )  //д��23���Ĵ��� RF_reg_count
    {
        CC1101WriteReg( CC1101InitData[i][0], CC1101InitData[i][1] );
    }


    CC1101SetAddress( my_CC1101_chip_address, BROAD_0AND255 );
    CC1101SetSYNC( my_CC1101_sync_word );//0X8799
    CC1101WriteReg(CC1101_MDMCFG1,   0x72); //Modem Configuration

    CC1101WriteMultiReg(CC1101_PATABLE, PaTabel, 8 );

    i = CC1101ReadStatus( CC1101_PARTNUM );//for test, must be 0x80
    printf("PARTNUM=[%XH]  ", i);
    i = CC1101ReadStatus( CC1101_VERSION );//for test, refer to the datasheet
    printf("VERSION=[%XH]  \n", i);
}

/*
================================================================================
Function : CC1101ReadStatus( )
    Read a status register
INPUT    : addr, The address of the register
OUTPUT   : the value read from the status register
================================================================================
*/
INT8U CC1101ReadStatus( INT8U addr )
{
    INT8U i;
    CC_CSN_LOW( );
    SPI_ExchangeByte( addr | READ_BURST);
    i = SPI_ExchangeByte( 0xFF );
    CC_CSN_HIGH( );
    return i;
}



/*
================================================================================
Function : CC1101WriteMultiReg( )
    Write some bytes to the specified register
INPUT    : addr, The address of the register
           buff, a buffer stores the values
           size, How many byte should be written
OUTPUT   : None
================================================================================
*/
void CC1101WriteMultiReg( INT8U addr, INT8U *buff, INT8U size )
{
    INT8U i;
    CC_CSN_LOW( );
    SPI_ExchangeByte( addr | WRITE_BURST );
    for( i = 0; i < size; i ++ )
    {
        SPI_ExchangeByte( *( buff + i ) );
    }
    CC_CSN_HIGH( );
}




/*
================================================================================
Function : CC1101SetSYNC( )
    Set the SYNC bytes of the CC1101
INPUT    : sync, 16bit sync
OUTPUT   : None
================================================================================
*/
void CC1101SetSYNC( INT16U sync )
{
    CC1101WriteReg(CC1101_SYNC1, 0xFF & ( sync >> 8 ) );
    CC1101WriteReg(CC1101_SYNC0, 0xFF & sync );
}


/*
================================================================================
Function : CC1101SetAddress( )
    Set the address and address mode of the CC1101
INPUT    : address, The address byte
           AddressMode, the address check mode
OUTPUT   : None
================================================================================
*/
void CC1101SetAddress( uint8_t address, ADDR_MODE AddressMode)
{
    uint8_t btmp = CC1101ReadReg( CC1101_PKTCTRL1 ) & ~0x03;
    CC1101WriteReg(CC1101_ADDR, address);
    if     ( AddressMode == BROAD_ALL )     {}
    else if( AddressMode == BROAD_NO  )     {
        btmp |= 0x01;
    }
    else if( AddressMode == BROAD_0   )     {
        btmp |= 0x02;
    }
    else if( AddressMode == BROAD_0AND255 ) {
        btmp |= 0x03;
    }
}


/*
================================================================================
Function : CC1101ReadReg( )
    read a byte from the specified register
INPUT    : addr, The address of the register
OUTPUT   : the byte read from the rigister
================================================================================
*/
uint8_t CC1101ReadReg( uint8_t addr )
{
    uint8_t i;
    CC_CSN_LOW( );
    SPI_ExchangeByte( addr | READ_SINGLE);
    i = SPI_ExchangeByte( 0xFF );
    CC_CSN_HIGH( );
    return i;
}


/*
================================================================================
Function : CC1101Reset( )
    Reset the CC1101 device
INPUT    : None
OUTPUT   : None
================================================================================
*/
void CC1101Reset( void )
{
    uint8_t x;

    CC_CSN_HIGH( );
    HAL_Delay(50);
    CC_CSN_LOW( );
    HAL_Delay(50);
    CC_CSN_HIGH( );
    for( x = 0; x < 100; x ++ );
    CC1101WriteCmd( CC1101_SRES );
}

void CC_CSN_LOW(void)
{

    HAL_GPIO_WritePin(PORT_CC_CSN, PIN_CC_CSN, GPIO_PIN_RESET);
    while( HAL_GPIO_ReadPin(PORT_CC_CSN, PIN_CC_CSN) != GPIO_PIN_RESET);

}
void CC_CSN_HIGH( void)
{
    HAL_GPIO_WritePin(PORT_CC_CSN, PIN_CC_CSN, GPIO_PIN_SET);
}

/*
================================================================================
Function : CC1101WriteCmd( )
    Write a command byte to the device
INPUT    : command, the byte you want to write
OUTPUT   : None
================================================================================
*/
void CC1101WriteCmd( uint8_t command )
{   // uint8_t x1=0;
    CC_CSN_LOW( );
    // x1=SPI_ExchangeByte( command );
    SPI_ExchangeByte( command );
    CC_CSN_HIGH( );
    //printf("writeCmd -[%XH]-read:[%XH]\n",command,x1);
}

/*
================================================================================
Function : CC1101WriteReg( )
    Write a byte to the specified register
INPUT    : addr, The address of the register
           value, the byte you want to write
OUTPUT   : None
================================================================================
*/
void CC1101WriteReg( uint8_t addr, uint8_t value )
{
//		uint8_t x1=0,x2=0;
    CC_CSN_LOW( );
    SPI_ExchangeByte( addr );
    SPI_ExchangeByte( value );
    CC_CSN_HIGH( );
    //printf("write reg read data:[%XH]-[%XH]\n",x1,x2);

}

/*
================================================================================
Function : CC1101WORInit( )
    Initialize the WOR function of CC1101
INPUT    : None
OUTPUT   : None
================================================================================
*/
void  CC1101WORInit( void )
{
    CC1101WriteReg(CC1101_MCSM2, 0x00); //0x16
    CC1101WriteReg(CC1101_MCSM0, 0x18); //0X18
    CC1101WriteReg(CC1101_WOREVT1, 0x8C); //0x1E
    CC1101WriteReg(CC1101_WOREVT0, 0xA0); //0x1f
    CC1101WriteReg(CC1101_WORCTRL, 0x78); //0X20   Wake On Radio Control

    CC1101WriteCmd( CC1101_SWORRST );
}



void my_read_all_reg(void)
{
    int i = 0;
    uint8_t read_buf[RF_reg_count] = {0};
    for(i = 0; i < RF_reg_count; i++)
    {
        read_buf[i] = CC1101ReadReg( CC1101InitData[i][0]);

        // CC1101WriteReg( CC1101InitData[i][0], CC1101InitData[i][1] );

    }

    for(i = 0; i < RF_reg_count; i++)
    {
        printf("address-[%2X] write-[%2X] read-[%2X]\n", CC1101InitData[i][0], CC1101InitData[i][1], read_buf[i]);

    }


}


/*
================================================================================
Function : CC1101SendPacket_add( )  ��Ŀ���ַ�ķ��ͺ���
    Send a packet
INPUT    : txbuffer, The buffer stores data to be sent
           size, How many bytes should be sent
           mode, Broadcast or address check packet
OUTPUT   : None

����address���Ŀ���ַ��һ���ֽ�
================================================================================
*/
void CC1101SendPacket_add( INT8U *txbuffer, INT8U size, TX_DATA_MODE mode, INT8U address)
{
    uint32_t kk = 0;
    //xSemaphoreTake(myMutex01Handle,3000);

    HAL_NVIC_DisableIRQ(EXIT_CC_IRQ_GD0); //�ر��жϣ������������ж�
    HAL_NVIC_DisableIRQ(EXIT_jiedi_EXTI_IRQn);
    //�ز�����
    kk = 0;
    while(my_fun_CC1101_re_CCA_status() == 1)
    {
        HAL_Delay(100);  //����ŵ��������ݣ�����ʱһ��ʱ��
        kk++;
        if(kk >= 100)
            return ;
    }
    //




    if( mode == BROADCAST )             {
        address = 0;
    }
    //else if( mode == ADDRESS_CHECK )    { address = CC1101ReadReg( CC1101_ADDR ); }

    CC1101ClrTXBuff( );

    if( ( CC1101ReadReg( CC1101_PKTCTRL1 ) & ~0x03 ) != 0 )
    {
        CC1101WriteReg( CC1101_TXFIFO, size + 1 ); //1���ֽڳ���+��1���ֽڵĵ�ַ+��Ч����,���ⳤ�ȷ���
        CC1101WriteReg( CC1101_TXFIFO, address );  //��ַ+��Ч���ݣ��̶����ȷ���
    }
    else
    {
        CC1101WriteReg( CC1101_TXFIFO, size ); //���ӵ�ַ
    }

    CC1101WriteMultiReg( CC1101_TXFIFO, txbuffer, size ); //д����Ч����
    CC1101SetTRMode( TX_MODE );  //����ģʽ
    // while( HAL_GPIO_ReadPin(PORT_CC_IRQ, PIN_CC_IRQ) != 1 ); //�������ݲ����ߵ�ƽ�ж�

    kk = 0;
    while( HAL_GPIO_ReadPin(PORT_CC_IRQ, PIN_CC_IRQ) != 1 ) //�������ݲ����ߵ�ƽ�ж�
    {
        kk++;
        if(kk >= 0X3FFFF)
        {
            break;
        }
    }
    //while( HAL_GPIO_ReadPin(PORT_CC_IRQ, PIN_CC_IRQ) == 1 ) ;//�ߵ�ƽ��Ϊ�͵�ƽ
    kk = 0;
    while( HAL_GPIO_ReadPin(PORT_CC_IRQ, PIN_CC_IRQ) == 1 ) //�ߵ�ƽ��Ϊ�͵�ƽ
    {
        kk++;
        if(kk >= 0X3FFFF)
        {
            break;
        }

    }

    CC1101ClrTXBuff( );  //�������FIFO
    kk = 0;
    while(CC1101ReadStatus(CC1101_MARCSTATE) != 0x01)
    {
        kk++;
        if(kk > 0X3FFFF)
        {
            break;
        }

    }

    //while(CC1101ReadStatus(CC1101_MARCSTATE) != 0x01);
    //printf("1--CC1101 status=[%XH] \n",CC1101ReadStatus(CC1101_MARCSTATE));
    CC1101SetTRMode(RX_MODE);  // �������ģʽ

    //osDelay(50);
    //HAL_Delay(1000);
    //printf("2--CC1101 status=[%XH] \n",CC1101ReadStatus(CC1101_MARCSTATE));
    __HAL_GPIO_EXTI_CLEAR_IT(PIN_CC_IRQ); //�����������ⲿ�жϣ�ÿ���жϵ������������һ������Ҫ�ֱ����
    HAL_NVIC_EnableIRQ(EXIT_CC_IRQ_GD0); //---------���������ж�

    __HAL_GPIO_EXTI_CLEAR_IT(EXIT_jiedi_EXTI_IRQn);
		__HAL_GPIO_EXTI_CLEAR_FLAG(EXIT_jiedi_EXTI_IRQn);
    HAL_NVIC_EnableIRQ(EXIT_jiedi_EXTI_IRQn);
    my_cc_Efied_count = 1;

    //xSemaphoreGive(myMutex01Handle);


}


/*
���ܣ���CC1101���յ����ݷŵ������ݻ�������
*/

int  RF_RecvHandler_intrrupt_get_data_to_buf(void)
{
    INT8U length = 0, recv_buffer[64] = {0};
    uint8_t ii = 0;
    uint32_t my_temp32 = 0;

    //int RSSI_dBm=0;

    //xSemaphoreTake(myMutex01Handle,3000);
    if(MY_CC1101_STATUS == 0x01 || MY_CC1101_STATUS == 0x02)
        return 0;
    else
        MY_CC1101_STATUS = 0X02;



    while (CC_IRQ_READ() == 1)
    {
        my_temp32++;
        if(my_temp32 >= 0x3FFFFF)
        {
            printf("\n*****2 CC_IRQ_READ() == 1 ******\n");
            CC1101ClrRXBuff( );
            return 0;
        }
    }

    // ��ȡ���յ������ݳ��Ⱥ���������
    length = CC1101RecPacket(recv_buffer);  //��ǰ���յ���RX�����ֽ�����������1+��ַ1+��ЧN+CRCУ��2
    //printf("get length=%d\n",length);

    // �жϽ��������Ƿ���ȷ
    if (length <= SEND_MAX && length > 0)
    {
        //LED3_ON;               // LED��˸������ָʾ�յ�����
        my_get_client_add = recv_buffer[0];
        for(ii = 1; ii < length; ii++)
        {
            my_CC1101_re_buf[my_CC1101_re_buf_pt_write] = recv_buffer[ii];
            my_CC1101_re_buf_pt_write++;
            if(my_CC1101_re_buf_pt_write >= my_CC1101_RX_MAX)
                my_CC1101_re_buf_pt_write = 0;
        }
    }
    else
    {

        printf("CRC ERROR cc1101 status=[%XH] \n", CC1101ReadStatus(CC1101_MARCSTATE));
    }

    CC1101SetTRMode(RX_MODE);           // �������ģʽ
    MY_CC1101_STATUS = 0X00;
    //CC1101WriteCmd(CC1101_SWOR); //����WORģʽ
    //xSemaphoreGive(myMutex01Handle);
    return length;
}


//Э����������һ֡����,��1������Ϊ�˿ںţ���2��Ϊ��ַ2�ֽ�
//��3������Ϊ�Ƿ���CRCУ�飬1Ϊ����
int my_101frame_analyse(uint8_t port_number, uint8_t length_long_status, uint8_t CRC_check_status)
{

    int ii = 0;
    uint16_t my_temp = 0;
    uint16_t my_start_add = 0;
    uint16_t my_end_add = 0;
    uint16_t my_temp_length1 = 0;
    uint16_t my_temp_length2 = 0;
    uint8_t my_status1 = 0;
    uint8_t my_status2 = 0;
    uint16_t my_chazhi = 0;
    int x = 0, x11 = 0, x12 = 0, x21 = 0, x22 = 0;
    uint16_t y = 0;
    volatile	  uint8_t my_point = 0;

    uint8_t *my_pro1_status = NULL;
    uint16_t *my_buf_read_count = NULL;
    uint16_t *my_buf_write_count = NULL;
    uint16_t re_max = 0;
    uint8_t *my_re_buf = NULL;
    uint8_t *my_com_buf1 = NULL; //������ָ��
    uint8_t my_length_max = 0;
    //===CC1101
    if(port_number == 4 )
    {
        my_pro1_status = &my_CC1101_Frame_status;
        my_buf_read_count = &my_CC1101_re_buf_pt_read;
        my_buf_write_count = &my_CC1101_re_buf_pt_write;
        re_max = my_CC1101_RX_MAX;
        my_re_buf = my_CC1101_re_buf;
        my_com_buf1 = my_CC1101_COM_Fram_buf; //������ָ��
    }

    else if(port_number == 2)
    {
        my_pro1_status = &USART2_FRAME_status;
        my_buf_read_count = &rsbuf2pt_read;
        my_buf_write_count = &rsbuf2pt_write;
        re_max = rsbuf2_max ;
        my_re_buf = rsbuf2;
        my_com_buf1 = USART2_my_frame; //������ָ��
    }

    //===========
    if(length_long_status == 1)
    {
        my_length_max = 5;
    }
    else
        my_length_max = 3;

    //=====

    if(*my_pro1_status == 1)
    {
        my_point = 1;
        return 1;
    }

    //-----------1�ж� ֡ͷ��λ��--------



xx0:
    while(*my_buf_read_count != *my_buf_write_count)
    {
        if(my_re_buf[*my_buf_read_count] == 0X68) //��֡ 68H-xx-xx-68H-ID-XX-XX... CRC-16H
        {   my_status2 = 1;
            break;
        }
        else if(my_re_buf[*my_buf_read_count] == 0x10) //�̶����ȶ�֡ 10H-ID-XX-XX-CRC-16H
        {   my_status2 = 2;
            break;
        }
        else
        {   (*my_buf_read_count)++;
            if(*my_buf_read_count >= re_max)
                *my_buf_read_count = 0;
        }
    }


    //--------2 �ж϶�ָ���λ��
    if(*my_buf_read_count == *my_buf_write_count)
    {   //my_point=2;
        return 0;
    }
    else if(*my_buf_read_count < *my_buf_write_count)
    {
        my_status1 = 1;  //���ڵ͵�ַ
    }
    else if(*my_buf_read_count > *my_buf_write_count)
    {
        my_status1 = 2;  //���ڸߵ�ַ
    }

    //-----3 ȡ��ǰʱ�̵�ָ���ַ

    my_start_add = *my_buf_read_count;
    my_end_add = *my_buf_write_count;


    //--------4 ��֡�����ж� //����ǰ��д�ں�
    if(my_status2 == 1 && my_status1 == 1) //���ڵ͵�ַ��д�ڸߵ�ַ
    {
        my_chazhi = my_end_add - my_start_add;
        if(my_chazhi < 10) //��ֵС����С����
        {   my_point = 3;
            return 0;
        }
        //ȡ������һ��֡ͷ

        if(my_re_buf[my_start_add + my_length_max] != 0X68)
        {
            (*my_buf_read_count)++;
            my_point = 4;
            goto xx0;
            //return 0;

        }


        if(length_long_status == 1)
        {
            //ȡ��֡�ĳ��ȣ���ID��ʼ�㣬һֱ��CRC������CRC��֡���ܳ���Ϊlength+6+2   68 xx xx yy yy 68 (....length..) crc 16
            my_temp_length1 = my_re_buf[my_start_add + 1];
            my_temp_length1 = my_temp_length1 + ((uint16_t)(my_re_buf[my_start_add + 2]) << 8);

            my_temp_length2 = my_re_buf[my_start_add + 3];
            my_temp_length2 = my_temp_length2 + ((uint16_t)(my_re_buf[my_start_add + 4]) << 8);

            if(my_temp_length1 != my_temp_length2)
            {
                (*my_buf_read_count)++;
                return 0;
            }
        }
        else
        {
            //ȡ��֡�ĳ��ȣ���ID��ʼ�㣬һֱ��CRC������CRC��֡���ܳ���Ϊlength+6+2   68 xx xx yy yy 68 (....length..) crc 16
            my_temp_length1 = my_re_buf[my_start_add + 1];
            my_temp_length2 = my_re_buf[my_start_add + 2];
            if(my_temp_length1 != my_temp_length2)
            {
                (*my_buf_read_count)++;
                return 0;
            }
        }

        if(my_start_add + my_temp_length1 + my_length_max + 2 >= my_end_add)
        {
            my_point = 31;
            return 0;
        }


        if((my_start_add + my_temp_length1 + my_length_max + 2) >= re_max)
        {

            my_point = 5;
            return 0;
        }

        if(my_chazhi < my_temp_length1) //��ֵ����С��֡����
        {
            my_point = 6;
            return 0;
        }
        //ȡ֡β
        x = my_re_buf[my_start_add + my_length_max + 2 + my_temp_length1];
        if(x != 0X16)
        {
            (*my_buf_read_count)++;
            my_point = 7;
            goto xx0;
            //return 0;

        }
        else
        {
            for(ii = 0; ii < (my_temp_length1 + my_length_max + 2 + 1); ii++)
            {

                my_com_buf1[ii] = my_re_buf[my_start_add + ii]; //ȡһ֡����������

            }
            *my_buf_read_count = my_start_add + my_temp_length1 + my_length_max + 2 + 1;


//            *my_pro1_status = 1; //��ʾȡ��һ֡����������
//            my_point = 8;
//						my_fun_101check_verify(my_com_buf1,length_long_status);
//            return 1;
            if(CRC_check_status == 1)
            {
                *my_pro1_status = my_fun_101check_verify(my_com_buf1, length_long_status);
                return *my_pro1_status;
            }
            else
            {

                *my_pro1_status = 1; //��ʾȡ��һ֡����������
                my_point = 22;
                return 1;
            }
        }


    }
    //--------5 ��֡�����ж� //д��ǰ�����ں�
    if(my_status2 == 1 && my_status1 == 2) //д�ڵ͵�ַ�����ڸߵ�ַ
    {
        my_temp = re_max - my_start_add;
        my_chazhi = (my_temp + my_end_add);
        if(my_chazhi < 8) //��ֵС����С����
        {   my_point = 9;
            return 0;
        }
        if(my_temp >= (my_length_max + 1))
            x = my_start_add + my_length_max;  //xΪ��2��֡ͷ�ĵ�ַ
        else
            x = my_length_max - my_temp;

        //ȡ������һ��֡ͷ
        if(my_re_buf[x] != 0X68)
        {
            (*my_buf_read_count)++;
            if(*my_buf_read_count >= re_max)
                *my_buf_read_count = 0;

            my_point = 10;
            goto xx0;
            //return 0;
        }
        //ȡ��֡�ĳ���

        if(length_long_status == 1) //˫�ֽڵ�ַ
        {
            if(x > 5)
            {
                x11 = my_start_add + 1;
                x12 = my_start_add + 2;
                x21 = my_start_add + 3;
                x22 = my_start_add + 4;
            }
            else
            {
                x11 = my_start_add + 1;
                if(x11 >= re_max)
                    x11 = x11 - re_max;

                x12 = my_start_add + 2;
                if(x12 >= re_max)
                    x12 = x12 - re_max;

                x21 = my_start_add + 3;
                if(x21 >= re_max)
                    x21 = x21 - re_max;

                x22 = my_start_add + 4;
                if(x22 >= re_max)
                    x22 = x22 - re_max;

            }

            my_temp_length1 = my_re_buf[x11];
            my_temp_length1 = my_temp_length1 + ((uint16_t)(my_re_buf[x12]) << 8);

            my_temp_length2 = my_re_buf[x21];
            my_temp_length2 = my_temp_length2 + ((uint16_t)(my_re_buf[x22]) << 8);
        }
        else //���ֽڵ�ַ
        {
            if(x > 3)
            {
                x11 = my_start_add + 1;
                x21 = my_start_add + 2;

            }
            else
            {
                x11 = my_start_add + 1;
                if(x11 >= re_max)
                    x11 = x11 - re_max;

                x21 = my_start_add + 2;
                if(x21 >= re_max)
                    x21 = x21 - re_max;

            }

            my_temp_length1 = my_re_buf[x11];
            my_temp_length2 = my_re_buf[x21];

        }


        if(my_temp_length1 != my_temp_length2)
        {
            (*my_buf_read_count)++;
            goto xx0;
            // return 0;

        }

        if(my_chazhi < my_temp_length1)
        {   my_point = 11;
            return 0;
        }


        //=======

        //ȡ��֡β
        y = my_start_add + my_temp_length1 + my_length_max + 2; //;
        if(y >= re_max) //֡β���������ĩβ
        {
            y = y - re_max;

            if(y >= my_end_add)
            {
                my_point = 32;
                return 0;
            }
        }


        x = my_re_buf[y]; //ȡ��֡β����

        //ȡһ֡����������
        if(x != 0X16)
        {
            (*my_buf_read_count)++;
            if(*my_buf_read_count >= re_max)
                *my_buf_read_count = 0;
            my_point = 18;
            goto xx0;
            // return 0;

        }
        else
        {
            for(ii = 0; ii < my_temp_length1 + 6; ii++)
            {
                y = my_start_add + ii;
                if(y >= re_max)
                {
                    y = y - re_max;
                }
                my_com_buf1[ii] = my_re_buf[y]; //ȡһ֡����������

            }
            *my_buf_read_count = *my_buf_read_count + my_temp_length1 + my_length_max + 2 + 1;
            if(*my_buf_read_count >= re_max)
            {
                *my_buf_read_count = *my_buf_read_count - re_max;
            }


//            *my_pro1_status = 1; //��ʾȡ��һ֡����������
//            my_point = 19;
//						my_fun_101check_verify(my_com_buf1,length_long_status);
//            return 1;

            if(CRC_check_status == 1)
            {
                *my_pro1_status = my_fun_101check_verify(my_com_buf1, length_long_status);
                return *my_pro1_status;
            }
            else
            {

                *my_pro1_status = 1; //��ʾȡ��һ֡����������
                my_point = 22;
                return 1;
            }
        }

    }

    //-------6����֡�ж�  ����ǰ��д�ں�

    if(my_status2 == 2 && my_status1 == 1) //����ǰ��д�ں�
    {
        my_chazhi = my_end_add - my_start_add;
        if(my_chazhi < 5) //��ֵС����С����
        {   my_point = 20;
            return 0;
        }

        //ȡ��֡�ĳ���
        if(length_long_status == 1)
            my_temp_length1 = 7;
        else
            my_temp_length1 = 5;
        if(my_start_add + my_temp_length1 >= my_end_add)
        {
            my_point = 33;
            return 0;
        }

        //ȡ֡β
        x = my_re_buf[my_start_add + my_temp_length1];
        if(x != 0X16)
        {
            (*my_buf_read_count)++;
            my_point = 21;
            goto xx0;
            //return 0;

        }
        else
        {
            for(ii = 0; ii < my_temp_length1 + 1; ii++)
            {

                my_com_buf1[ii] = my_re_buf[my_start_add + ii]; //ȡһ֡����������

            }
            *my_buf_read_count = *my_buf_read_count + my_temp_length1 + 1;


            if(CRC_check_status == 1)
            {
                *my_pro1_status = my_fun_101check_verify(my_com_buf1, length_long_status);
                return *my_pro1_status;
            }
            else
            {

                *my_pro1_status = 1; //��ʾȡ��һ֡����������
                my_point = 22;
                return 1;
            }

        }


    }


//--------7 �̶�֡�ж�  д��ǰ�����ں�

    if(my_status2 == 2 && my_status1 == 2) //д��ǰ�����ں�
    {
        my_temp = re_max - my_start_add;
        my_chazhi = (my_temp + my_end_add);
        if(my_chazhi < 5) //��ֵС����С����
            return 0;

        //ȡ��֡�ĳ���
        if(length_long_status == 1)
            my_temp_length1 = 7;
        else
            my_temp_length1 = 5;

        //ȡ��֡β
        y = my_start_add + my_temp_length1; //;
        if(y >= re_max) //֡β���������ĩβ
        {
            y = y - re_max;
            if(y > my_end_add)
            {
                my_point = 35;
                return 0;
            }

        }

        x = my_re_buf[y]; //ȡ��֡β����

        //ȡһ֡����������
        if(x != 0X16)
        {
            (*my_buf_read_count)++;
            if(*my_buf_read_count >= re_max)
                *my_buf_read_count = 0;
            my_point = 36;
            goto xx0;
            //return 0;

        }
        else
        {
            for(ii = 0; ii < my_temp_length1 + 1; ii++)
            {
                y = my_start_add + ii;
                if(y >= re_max)
                {
                    y = y - re_max;
                }
                my_com_buf1[ii] = my_re_buf[y]; //ȡһ֡����������

            }
            *my_buf_read_count = *my_buf_read_count + my_temp_length1 + 1;
            if(*my_buf_read_count >= re_max)
                *my_buf_read_count = *my_buf_read_count - re_max;
            //CRCУ��
            if(CRC_check_status == 1)
            {
                *my_pro1_status = my_fun_101check_verify(my_com_buf1, length_long_status);
                return *my_pro1_status;
            }
            else
            {

                *my_pro1_status = 1; //��ʾȡ��һ֡����������
                my_point = 22;
                return 1;
            }
        }

    }

    //===========
    return 0;
}

//����ʹ�ã�16������ʾ�յ��ĵ�����
void my_fun_display_fram_16(uint8_t portnumber)
{
    uint8_t *pt_buf = NULL;
    uint16_t my_length = 0;
    uint16_t ii = 0;
    char *ptchar = NULL;
    if(portnumber == 4)
    {
        pt_buf = my_CC1101_COM_Fram_buf;
        ptchar = "CC1101 re:";

    }

    if(pt_buf[0] == 0X10 && pt_buf[5] == 0X16)
    {
        my_length = 6;

    }
    else if(pt_buf[0] == 0X10 && pt_buf[7] == 0X16)
    {

        my_length = 8;
    }
    else if(pt_buf[0] == 0X68 && pt_buf[3] == 0X68)
    {
        my_length = pt_buf[1] + 6;

    }

    else if(pt_buf[0] == 0X68 && pt_buf[5] == 0X68)
    {
        my_length = pt_buf[2];
        my_length = my_length << 8;
        my_length = my_length + pt_buf[1] + 8;

    }
    printf("%s", ptchar);
    //my_length=8;
    for(ii = 0; ii < my_length; ii++)
    {
        printf("[%XH]-", pt_buf[ii]);

    }
    printf("\r\n");

}

//����ʹ�ã���ʾ���͵�����
//���һ������������Ϊ1������Ϊ0
void my_fun_display_buf_16(uint8_t *pt, uint16_t size, uint8_t TX_status)
{
    uint16_t ii = 0;
    for(ii = 0; ii < size; ii++)
    {
        printf ("[%XH]-", pt[ii]);

    }
    if(TX_status == 1)
        printf("TX\r\n");
    else
        printf("RX\r\n");

}

//��������CC1101����
/*
CC1101��󻺳�����64���ֽڣ������� ����1+��ַ1+....N������+2��crc
�������һ�η���60���ֽ�

���ܣ�����CC1101 ���ͳ�������
������
1-�������ݵĻ�������2-�������ݵ��ֽ���
3-����ģʽ�㲥�����ߵ�ַ
4.Ŀ���ַ
*/
//void CC1101SendPacket_add( INT8U *txbuffer, INT8U size, TX_DATA_MODE mode, INT8U address)
void my_fun_CC1101_send_long_data(INT8U *txbuffer, uint16_t size, TX_DATA_MODE mode, INT8U address)
{


    uint8_t ii = 0;

    uint8_t my_block_size = 0;
    uint8_t my_block_last_data = 0;
    uint8_t *pt = txbuffer;

    my_block_size = size / My_CC1101_send_data_size_MAX;
    my_block_last_data = size % My_CC1101_send_data_size_MAX;

    for(ii = 0; ii < my_block_size; ii++)
    {
        CC1101SendPacket_add(pt + ii * My_CC1101_send_data_size_MAX, My_CC1101_send_data_size_MAX, mode, address);
        //====
        if(ii == 0)
        {
            printf("===CC1101 Send start DB=%d===\n", ii);
            //my_fun_display_buf_16(pt + ii * My_CC1101_send_data_size_MAX,My_CC1101_send_data_size_MAX); //����ʹ�ã���ʾ���͵�����
        }
        //====
        //HAL_Delay(1);
        if(my_block_size > 0)
            HAL_Delay(20);
    }
    if(my_block_last_data != 0)
    {

        CC1101SendPacket_add(pt + ii * My_CC1101_send_data_size_MAX, my_block_last_data, mode, address);
        printf(" CC1101 Send last DB=%d\n", ii);
        //my_fun_display_buf_16(pt + ii * My_CC1101_send_data_size_MAX,my_block_last_data); //����ʹ�ã���ʾ���͵�����
    }
}

//��ʼ��CC1101

void my_fun_CC1101_init_reum(void)
{
    //xSemaphoreTakeFromISR(myMutex01Handle,3000);

    HAL_NVIC_DisableIRQ(EXIT_CC_IRQ_GD0);
	 CC1101_PWR_OFF;
		HAL_Delay(300);
	CC1101_PWR_ON;
	  HAL_Delay(300);
    CC1101Init();
    HAL_Delay(500);
    CC1101WriteCmd(CC1101_SIDLE); //�������״̬
    CC1101WriteReg(CC1101_MCSM2, 0X00); //д0x16 ,RX_TIME��д��0X00�����ý���ʱ�����ƣ�ռ�ձ�����00���12%,���û���յ����ݣ��ͽ���SLEEP��
    CC1101WriteReg(CC1101_MCSM0, 0x18); //0X18 	//
    CC1101WriteReg(CC1101_WOREVT1, 0x87); //0X1E,event0 ���ֽ�
    CC1101WriteReg(CC1101_WOREVT0, 0x6A);  //0X1F event0 ���ֽ�
    CC1101WriteReg(CC1101_WORCTRL, 0X78); //д0X20,0111 100,WOR_RES
    //CC1101WriteCmd(CC1101_SWOR); //����WORģʽ

    //---------�������ģʽ

    CC1101SetIdle();
    CC1101WriteReg(CC1101_MCSM2, 0X07); //д0x16 ,RX_TIME��д��0X07,����û��ʱ�����ƣ�һ�½�������
    CC1101SetTRMode(RX_MODE);           // �������ģʽ

    __HAL_GPIO_EXTI_CLEAR_IT(PIN_CC_IRQ);
    HAL_NVIC_ClearPendingIRQ(EXIT_CC_IRQ_GD0);
    HAL_NVIC_EnableIRQ(EXIT_CC_IRQ_GD0); //�����жϣ�����/�������ݲ���
    //xSemaphoreGiveFromISR(myMutex01Handle);
		printf("\n====reset CC1101!!!!===\n");
}

/*
����У����
��������һ��Ϊָ�����飬�ڶ���Ϊ��֡��飬1��ʾ˫�ֽڳ��ȣ�0Ϊ���ֳ���
*/
uint8_t my_fun_101check_generate(uint8_t *buffer, uint8_t longstatus)
{
    uint16_t k = 0;
    uint8_t status68 = 0;
    uint16_t length = 0;

    uint8_t check_char = 0x00;
    if(longstatus == 0) //���ֽڵ�ַ���
    {
        if(buffer[0] == 0x10)
        {
            status68 = 1;
        }
        else if(buffer[0] == 0x68)
        {
            status68 = 2;
        }


        if(status68 == 1) //�̶�����У��λ���
        {
            check_char = buffer[1] + buffer[2] + buffer[3];
            buffer[4] = check_char;
        }
        else if(status68 == 2) //�ǹ̶�����У��λ���
        {
            for(k = 0; k < buffer[1]; k++)
            {
                check_char = check_char + buffer[k + 4];
            }
            buffer[buffer[1] + 4] = check_char;
        }
    }
    else if(longstatus == 1) //˫�ֽڵ�ַ���
    {

        if(buffer[0] == 0x10)
        {
            status68 = 1;
        }
        else if(buffer[0] == 0x68)
        {
            status68 = 2;
        }


        if(status68 == 1) //�̶�����У��λ���
        {
            check_char = buffer[1] + buffer[2] + buffer[3] + buffer[4] + buffer[5];;
            buffer[6] = check_char;
        }
        else if(status68 == 2) //�ǹ̶�����У��λ���
        {
            length = buffer[2];
            length = buffer [1] + (length << 8);
            for(k = 0; k < length; k++)
            {
                check_char = check_char + buffer[k + 6];
            }
            buffer[length + 6] = check_char;
        }


    }
    return check_char;

}

/*
У���ּ��
����ֵ��ͨ��Ϊ1����ͨ��Ϊ0
*/

uint8_t my_fun_101check_verify(uint8_t *buffer, uint8_t longstatus)
{
    uint16_t k = 0;
    uint8_t status68 = 0;
    uint8_t temp = 0;
    uint8_t check_char = 0x00;
    uint16_t my_length = 0;
    if(longstatus == 0)
    {
        if(buffer[0] == 0x10)
        {
            status68 = 1;
        }
        else if(buffer[0] == 0x68)
        {
            status68 = 2;
        }

        if(status68 == 1) //�̶�����У��λ���
        {
            check_char = buffer[1] + buffer[2] + buffer[3];
            if(check_char == buffer[4])
                temp = 1;
            else temp = 0;


        }
        else if(status68 == 2) //�ǹ̶�����У��λ���
        {
            for(k = 0; k < buffer[1]; k++)
            {
                check_char = check_char + buffer[k + 4];
            }

            if(check_char == buffer[buffer[1] + 4])
                temp = 1;
            else temp = 0;
        }
        temp = 1;

    }
    else if(longstatus == 1)
    {
        if(buffer[0] == 0x10)
        {
            status68 = 1;
        }
        else if(buffer[0] == 0x68)
        {
            status68 = 2;
        }

        if(status68 == 1) //�̶�����У��λ���
        {
            check_char = buffer[1] + buffer[2] + buffer[3] + buffer[4] + buffer[5];
            if(check_char == buffer[6])
                temp = 1;
            else temp = 0;
        }
        else if(status68 == 2) //�ǹ̶�����У��λ���
        {
            my_length = buffer[2];
            my_length = buffer[1] + (my_length << 8);
            for(k = 0; k < my_length; k++)
            {
                check_char = check_char + buffer[k + 6];
            }

            if(check_char == buffer[my_length + 6])
                temp = 1;
            else temp = 0;
        }
        temp = 1;



    }
    //printf("CRC verify is  %d==%d\r\n",temp,status68);
    return temp;

}



//CC1101��������״̬
void CC1101SetSleep( void )
{
    CC1101SetIdle();
    CC1101WriteCmd(CC1101_SPWD);
}


void  CC1101CCA( void )//����Ϊ�ز���������
{
    CC1101WriteReg(CC1101_IOCFG2, 0x0E); //GDO2��������ز���Ӧ��ƽ,���RSSI����������֮��Ϊ�ߵ�ƽ,�ŵ�ռ��Ϊ�ߵ�ƽ
    CC1101WriteReg(CC1101_MCSM1, 0x3F); //�����ź�ǿ�ȵ�������ֵ�ҵ�ǰδ���ձ������ŵ����У���TX��RX������оƬ��������RX״̬��
    CC1101WriteReg(CC1101_AGCCTRL1, 0x40); //�������Ŵ��������ȼ�С���ز����������ֵ����,�ز�����������ֵ��MAGN_TARGET����
    CC1101WriteReg(CC1101_AGCCTRL2, 0x07); //42dB
}

/*
���ܣ����ص�ǰ�ز������ŵ����ź�״̬�����źŷ���1��û���źţ����У�����0
����ֵ�����źŷ���1��û���źţ����У�����0
*/
uint8_t my_fun_CC1101_re_CCA_status(void)
{
    //if(HAL_GPIO_ReadPin(GPIO2_INPUT_GPIO_Port,GPIO2_INPUT_Pin)==1)//��⵽���������ز��ź�

    CC1101SetTRMode( RX_MODE );
    HAL_Delay(10);
    return (uint8_t)(HAL_GPIO_ReadPin(GPIO2_INPUT_GPIO_Port, GPIO2_INPUT_Pin));

}
