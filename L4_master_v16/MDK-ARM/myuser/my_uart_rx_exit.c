#include "stdio.h"
#include "my_usart.h"
#include "my_cc1101.h"


extern INT8U   COM_TxNeed;
extern INT8U   COM_TimeOut;
extern INT8U   COM_RxCounter;
extern INT8U   COM_TxCounter;
extern INT8U   COM_RxBuffer[];
extern INT8U   COM_TxBuffer[];



/*
���ô���2�������ݣ����Ѵ���������CC1101����
*/

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle)
{
    uint8_t ret = HAL_OK;


    COM_RxBuffer[COM_RxCounter++]=my_usart2_re_buf[my_usart2_re_count];  //�����жϾ��Ѿ����յ���һ���ַ���������ַ�
    if (COM_RxCounter > 64)     {
        COM_RxCounter = 0;    //�ж��Ƿ�Խ�磬RX��������
    }
    COM_TimeOut = 5;  //ͬʱ��ʼ��ʱ���ȴ�������ɣ�TIM3��ʱ������λΪ1ms��5����λΪ5����



    my_usart2_re_count++;    //
    if(my_usart2_re_count>=200) my_usart2_re_count=0; //���ջ���������

    do
    {
        ret = 	HAL_UART_Receive_IT(&huart2,&my_usart2_re_buf[my_usart2_re_count],1);  //��������USART2�������ȴ�������һ���ַ�
    } while(ret != HAL_OK);




}