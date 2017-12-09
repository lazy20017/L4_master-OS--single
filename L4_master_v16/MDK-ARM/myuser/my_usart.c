#include "stdio.h"
#include "usart.h"
#include "my_usart.h"
#include "stdarg.h"
#include "string.h"




//Retargets the C library printf function to the UART.
int fputc(int ch, FILE *f)
{
    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 0xFFFF);
    return ch;
}






/*
 * ��������itoa
 * ����  ������������ת�����ַ���
 * ����  ��-radix =10 ��ʾ10���ƣ��������Ϊ0
 *         -value Ҫת����������
 *         -buf ת������ַ���
 *         -radix = 10
 * ���  ����
 * ����  ����
 * ����  ����USART1_printf()����
 */
char *itoa(int value, char *string, int radix)
{
    int     i, d;
    int     flag = 0;
    char    *ptr = string;

    /* This implementation only works for decimal numbers. */
    if (radix != 10)
    {
        *ptr = 0;
        return string;
    }

    if (!value)
    {
        *ptr++ = 0x30;
        *ptr = 0;
        return string;
    }

    /* if this is a negative value insert the minus sign. */
    if (value < 0)
    {
        *ptr++ = '-';

        /* Make the value positive. */
        value *= -1;
    }

    for (i = 10000; i > 0; i /= 10)
    {
        d = value / i;

        if (d || flag)
        {
            *ptr++ = (char)(d + 0x30);
            value -= (d * i);
            flag = 1;
        }
    }

    /* Null terminate the string. */
    *ptr = 0;

    return string;

} /* NCL_Itoa */

/*
 * ��������USART_printf
 * ����  ����ʽ�������������C���е�printf��������û���õ�C��
 * ����  ��-USARTx ����ͨ��������ֻ�õ��˴���1����USART1
 *		     -Data   Ҫ���͵����ڵ����ݵ�ָ��
 *			   -...    ��������
 * ���  ����
 * ����  ����
 * ����  ���ⲿ����
 *         ����Ӧ��USART1_printf( USART1, "\r\n this is a demo \r\n" );
 *            		 USART1_printf( USART1, "\r\n %d \r\n", i );
 *            		 USART1_printf( USART1, "\r\n %s \r\n", j );
 */
void USART_printf(UART_HandleTypeDef* USARTx, uint8_t *Data,...)
{
    const char *s=0;
    int d;
    char buf[16];
    int16_t n=0;
    uint8_t status=0;
    uint8_t  my_0A=0X0A;
    uint8_t  my_0D=0X0D;

    va_list ap;
    va_start(ap, Data);
    ////////////


    if(*Data==0x10) {
        n=6;
        status=1;
    }
    else if(*Data==0x68&& *(Data+3)==0x68) {
        n=(*(Data+1)+6);
        status=1;
    }
    else if(*Data==0x67&& *(Data+5)==0x67) {
        n=(*(Data+1)+((uint16_t)(*(Data+2))<<8)+6);
        status=1;
    }

    else {
        status=0;
    }

    //while(HAL_UART_GetState(USARTx) !=HAL_UART_STATE_READY );

    if(status==1)
    {
        HAL_UART_Transmit(USARTx,Data,n,1000);
    }


    /////////////

    //while( HAL_UART_GetState(USARTx) !=HAL_UART_STATE_READY );
    while ( status==0&&*Data != 0)     // �ж��Ƿ񵽴��ַ������������������䣬����101Э���ַ���
    {
        if (*Data == 0x5c )  //'\'
        {
            switch ( *++Data )
            {
            case 'r':							          //�س���
                //USART_SendData(USARTx, 0x0d);
                HAL_UART_Transmit(USARTx,&my_0D,1,1000);
                Data ++;
                break;

            case 'n':							          //���з�
                //USART_SendData(USARTx, 0x0a);
                HAL_UART_Transmit(USARTx,&my_0A,1,1000);
                Data ++;
                break;

            default:
                Data ++;
                break;
            }
        }
        else if ( *Data == '%')
        {   //
            switch ( *++Data )
            {
            case 's':										  //�ַ���
                s = va_arg(ap, const char *);
                HAL_UART_Transmit(USARTx,(uint8_t *)s,strlen(s),1000);
                Data++;
                break;

            case 'd':										//ʮ����
                d = va_arg(ap, int);
                itoa(d, buf, 10);
                s=buf;
                HAL_UART_Transmit(USARTx,(uint8_t *)s,strlen(s),1000);
                Data++;
                break;
            case 'c':										//�ַ�
                HAL_UART_Transmit(USARTx,(uint8_t *)s,1,1000);
                //while( HAL_UART_GetState(USARTx) !=HAL_UART_STATE_READY );
                Data++;
                break;

            default:
                Data++;
                break;
            }
        } /* end of else if */
        else HAL_UART_Transmit(USARTx,Data++,1,1000);

        //while( HAL_UART_GetState(USARTx) !=HAL_UART_STATE_READY );
    }

    //------------
}



/*

���ܣ��Զ���USART_printf���%f������ֻ����һ��%f��
���磺USART_printf1(&huart2,"x=%f\n",1.23456789);
*/
void USART_printf1(UART_HandleTypeDef* USARTx, uint8_t *Data,double my_data)
{
    char my_buf[10];


    sprintf(my_buf,"%lf",my_data);
    while (*Data != '\0')
    {
        if(*Data=='%')
            USART_printf(USARTx,"%s",my_buf);
        else if(*Data=='f') ;
        else	HAL_UART_Transmit(USARTx,(uint8_t *)Data,1,1000);

        Data++;
    }

}
