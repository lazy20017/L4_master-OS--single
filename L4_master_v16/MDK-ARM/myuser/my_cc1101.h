#include "stm32l4xx_hal.h"
#include "my_cc1101REG.h"
#include "my_spi.h"


//#include "bsp_led.h"

#define INT8U uint8_t
#define INT16U uint16_t

/*===========================================================================
------------------------------Internal IMPORT functions----------------------
you must offer the following functions for this module
1. INT8U SPI_ExchangeByte(INT8U input); // SPI Send and Receive function
2. CC_CSN_LOW();                        // Pull down the CSN line
3. CC_CSN_HIGH();                       // Pull up the CSN Line
===========================================================================*/
// CC1101��ؿ������Ŷ���
#define PORT_CC_CSN     GPIOC
#define PIN_CC_CSN      GPIO_PIN_9

#define PORT_CC_IRQ     GPIOC
#define PIN_CC_IRQ      GPIO_PIN_6

#define PORT_CC_GDO2    GPIOC
#define PIN_CC_GDO2     GPIO_PIN_7

#define CC_IRQ_READ()  HAL_GPIO_ReadPin(PORT_CC_IRQ,PIN_CC_IRQ)

#define EXIT_CC_IRQ_GD0    EXTI9_5_IRQn         //EXTI9_5_IRQn



// ��������
#define SEND_MAX        64       // �������ݵ���󳤶�
#define My_CC1101_send_data_size_MAX 40

#define CC1101_CRC_check 1



/*===========================================================================
----------------------------------macro definitions--------------------------
============================================================================*/

//typedef enum { TX_MODE, RX_MODE } TRMODE;
//typedef enum { BROAD_ALL, BROAD_NO, BROAD_0, BROAD_0AND255 } ADDR_MODE;  //��ַģʽ
//typedef enum { BROADCAST, ADDRESS_CHECK} TX_DATA_MODE;



void CC_CSN_LOW(void);
void CC_CSN_HIGH( void);
void CC1101WriteCmd( uint8_t command );
void CC1101Reset( void );
void CC1101Init( void );

void CC1101WriteReg( uint8_t addr, uint8_t value );
void CC1101SetAddress( uint8_t address, ADDR_MODE AddressMode);
uint8_t CC1101ReadReg( uint8_t addr );
void CC1101SetSYNC( INT16U sync );
void CC1101WriteMultiReg( INT8U addr, INT8U *buff, INT8U size );
INT8U CC1101ReadStatus( INT8U addr );

void CC1101SetTRMode( TRMODE mode );
void RF_Initial(void);
//void RF_RecvHandler(void);
void RF_SendPacket(void);
INT8U CC1101RecPacket( INT8U *rxBuffer );
INT8U CC1101GetRXCnt( void );
void CC1101ReadMultiReg( INT8U addr, INT8U *buff, INT8U size);
void CC1101ClrRXBuff( void );
void CC1101SetIdle( void );
//void CC1101SendPacket( INT8U *txbuffer, INT8U size, TX_DATA_MODE mode );
void CC1101ClrTXBuff( void );
void RF_SendPacket2(void);
void RF_RecvHandler_intrrupt(void);
void my_read_all_reg(void);
void  CC1101WORInit( void );
void CC1101SendPacket_add( INT8U *txbuffer, INT8U size, TX_DATA_MODE mode,INT8U address);
void RF_SendPacket3(void);  //����ʹ�ã�����FIFO�����ʹ��
void CC1101SetSleep( void );


int  RF_RecvHandler_intrrupt_get_data_to_buf(void);
int my_101frame_analyse(uint8_t port_number, uint8_t length_long_status,uint8_t CRC_check_status);

void my_fun_CC1101_send_long_data(INT8U *txbuffer, uint16_t size, TX_DATA_MODE mode, INT8U desc_address);


void my_fun_display_fram_16(uint8_t portnumber);
void my_fun_display_buf_16(uint8_t *pt, uint16_t size,uint8_t TX_status);

void my_fun_CC1101_init_reum(void);
uint8_t my_fun_101check_verify(uint8_t *buffer,uint8_t longstatus);
uint8_t my_fun_101check_generate(uint8_t *buffer,uint8_t longstatus);
void  CC1101CCA( void );//����Ϊ�ز���������
uint8_t my_fun_CC1101_re_CCA_status(void);
