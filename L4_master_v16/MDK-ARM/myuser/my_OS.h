#include "stm32l4xx_hal.h"
#include "cmsis_os.h"
//#include "my_gprs.h"
#include "my_cc_TX_RX.h"
#include "my_extern_val.h"

void my_fun_set_group(void);
void my_fun_take_group(void);
void my_fun_give_BinarySem(void);
void my_fun_take_BinarySem(void);
void my_fun_give_Queue1(void);
//void my_fun_give_Queue31(void);
//void my_fun_give_Queue3(uint16_t temp_step);


void my_fun_give_Queue(osMessageQId *my_QHL,uint16_t temp_step);
void my_fun_take_Queue(void);
void my_fun_CC1101_init_resume(void);
void my_fun_usart_init_resume(void);
void my_fun_task_heap_value(void);
//=========================





void my_fun_gprs_time_dialog_rx(
    osMessageQId *QHL_send,
    uint16_t my_get_step,   //
    uint16_t my_before_step,
    uint16_t my_now_step,
    uint16_t my_next_step,
    uint8_t end_status,
    uint8_t (*ptfun)(void)
);

void my_fun_gprs_time_dialog_tx(
    uint16_t my_get_step,
    uint16_t my_before_step,
    uint16_t my_now_step,
    uint8_t end_status,
    void (*ptfun)(void)
);


void my_fun_CC1101_time_dialog_tx(
    uint16_t my_step,
    uint16_t my_first_step,
    uint16_t my_second_step,
    uint8_t end_status,
    void (*ptfun)(void)
);
void my_fun_CC1101_time_dialog_rx(
    osMessageQId *QHL_send,
    uint16_t my_step,
    uint16_t my_first_step,
    uint16_t my_second_step,
    uint8_t end_status,
    void (*ptfun)(void)
);


//=========
void my_fun_CC1101_time_dialog_rx2(
    osMessageQId *QHL_send,
    uint16_t my_get_step,   //
    uint16_t my_before_step,
    uint16_t my_now_step,
    uint16_t my_next_step,
    uint8_t end_status,
    uint8_t (*ptfun)(void)
);
void my_fun_CC1101_time_dialog_tx2(
    uint16_t my_get_step,
    uint16_t my_before_step,
    uint16_t my_now_step,
    uint8_t end_status,
    void (*ptfun)(void)
);
//==============

void my_fun_CC1101_test1(void);

void my_fun_TX_CC1101_test0(void);
void my_fun_TX_CC1101_test1(void);
void my_fun_TX_CC1101_test2(void);
void my_fun_TX_CC1101_test3(void);
void my_fun_TX_CC1101_test4(void);


uint8_t my_fun_RX_CC1101_text0_RX_OK(void);




void my_fun_PWR_time_dialog_rx2(
    osMessageQId *QHL_send,
    uint16_t my_get_step,   //
    uint16_t my_before_step,
    uint16_t my_now_step,
    uint16_t my_next_step,
    uint8_t end_status,
    uint8_t (*ptfun)(void)
);
void my_fun_PWR_time_dialog_tx2(
    uint16_t my_get_step,
    uint16_t my_before_step,
    uint16_t my_now_step,
    uint8_t end_status,
    void (*ptfun)(void)
);
		
		
void my_fun_TX_CC1101_heart(void); //����
