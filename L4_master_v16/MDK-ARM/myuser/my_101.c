#include "my_101.h"



//typedef enum { TX_MODE, RX_MODE } TRMODE;
//typedef enum { BROAD_ALL, BROAD_NO, BROAD_0, BROAD_0AND255 } ADDR_MODE;  //��ַģʽ
//typedef enum { BROADCAST, ADDRESS_CHECK} TX_DATA_MODE;


extern uint8_t my_Fault_Current_End_Status;  //
extern uint8_t my_Fault_E_Fild_End_Status;
extern uint8_t my_PWR_all_count;

extern uint8_t my_CC1101_COM_Fram_buf[]; //ָ�����
extern uint8_t my_CC1101_Frame_status;

extern double ADC1_Filer_value_buf[ADC1_COLM];
extern uint16_t ADC1_GetValue_Aver[ADC1_COLM];
extern double ADC2_Filer_value_buf_2[ADC2_COLM][3];
extern double ADC2_Filer_value_buf_3[ADC2_COLM][3];
extern uint16_t ADC2_Filer_value_buf_1[ADC2_COLM][3];

//����12�����ڵ���Чֵ,��36������
extern double WAVE_half_ave_Current2[12][3]; //�������д棬ƽ������Ч�����12�����ڵ�ÿ�����ڵ�ֵ
extern double WAVE_all_ave_Current2[12][3]; //�������д棬ƽ������Ч�����
extern double WAVE_all_ave_E_Field2[12][3]; //�������д棬ƽ������Ч�����

extern double WAVE_half_ave_Current3[12][3]; //�������д棬ƽ������Ч�����12�����ڵ�ÿ�����ڵ�ֵ
extern double WAVE_all_ave_Current3[12][3]; //�������д棬ƽ������Ч�����
extern double WAVE_all_ave_E_Field3[12][3]; //�������д棬ƽ������Ч�����

//��12���ڣ�ÿ�����ڲ���ֵ�ã���ʵֵ
extern uint16_t WAVE_half_ave_Current1[12][3]; //�������д棬ƽ������Ч�����12�����ڵ�ÿ�����ڵ�ֵ
extern uint16_t WAVE_all_ave_Current1[12][3]; //�������д棬ƽ������Ч�����
extern uint16_t WAVE_all_ave_E_Field1[12][3];

extern uint16_t my_wave_record_sec2[WAVE_Channel_Num][WAVE_number_sec2];
extern uint16_t my_wave_record_sec3[WAVE_Channel_Num][WAVE_number_sec2];
extern double MY_VDD;
extern double my_i_ratio_value;
extern uint16_t my_all_i_up_value;
extern uint8_t  my_temp8_buf1[];

extern uint8_t my_CC1101_dest_address;
extern double my_I_100A_Radio;
extern double my_E_ratio_value;

extern uint8_t my_CC1101_chip_address; //����Դ��ַ
extern uint8_t my_CC1101_dest_address; //����Ŀ���ַ
extern uint8_t my_sys_start_status;


//extern uint8_t my_protocole1_status;  //��ʾ����֡��״̬��1Ϊ��ʾ�յ�1��������֡��0��ʾû���յ�������֡
//extern uint8_t my_protobole1_buf1[];
#define my_protocole1_status my_CC1101_Frame_status
#define my_protobole1_buf1 my_CC1101_COM_Fram_buf

uint8_t my_point = 0;
uint16_t my_PWR_heart_count = 0;
uint8_t my_101_send_buf[1935] = {0X68};  //101����Э�鷢�ͻ�����

/*

���ܣ����ô��ڣ��������ݻ������е�����
����1�����ں�
����2����������
����3����������
*/


void my_fun_101_send_short_data(UART_HandleTypeDef* USARTx, uint8_t control_bye, uint16_t data, TX_DATA_MODE mode, INT8U desc_address)
{


    my_101_send_buf[0] = 0x10; //֡ͷ
    my_101_send_buf[1] = control_bye; //ID
    my_101_send_buf[2] = my_CC1101_chip_address; //Դ��ַ
    my_101_send_buf[3] = my_CC1101_dest_address; //Ŀ�ĵ�ַ
    my_101_send_buf[4] = data; //���ݵ��ֽ�
    my_101_send_buf[5] = (data >> 8) ; //���ݸ��ֽ�
    my_101_send_buf[6] = my_fun_101check_generate(my_101_send_buf, 1);
    my_101_send_buf[7] = 0x16;

    //USART_printf(USARTx,my_send_buf);
#if Debug_Usart_use_433data==1
    HAL_UART_Transmit(USARTx, my_101_send_buf, 8, 3000);
#endif

#if CC1101_Use==1
    my_fun_CC1101_send_long_data(my_101_send_buf, 8, mode, desc_address); //����CC1101��������
#endif
}
/*

���ܣ����ͳ�֡����
����1�����ں�
����2����������
����3�����ݻ������׵�ַ
����4��������������
*/


void my_fun_101_send_long_data(UART_HandleTypeDef* USARTx, uint8_t control_bye, uint8_t *send_buf, uint16_t send_number, TX_DATA_MODE mode, INT8U desc_address)
{
    uint16_t ii = 0;



    my_101_send_buf[0] = 0x68; //֡ͷ
    my_101_send_buf[1] = (uint8_t)((send_number + 3) & 0x00ff); //����,��ID��ʼ��ĳ���
    my_101_send_buf[2] = (uint8_t)(((send_number + 3) & 0xff00) >> 8); //���ȣ���֡ͷ����ĳ���

    my_101_send_buf[3] = my_101_send_buf[1];
    my_101_send_buf[4] = my_101_send_buf[2];

    my_101_send_buf[5] = 0x68; //֡ͷ
    my_101_send_buf[6] = control_bye;
    my_101_send_buf[7] = my_CC1101_chip_address; //Դ��ַ
    my_101_send_buf[8] = my_CC1101_dest_address; //Ŀ�ĵ�ַ


    for(ii = 0; ii < send_number; ii++)
    {
        my_101_send_buf[9 + ii] = send_buf[ii];

    }
    my_101_send_buf[9 + ii] = my_fun_101check_generate(my_101_send_buf, 1);;
    my_101_send_buf[9 + ii + 1] = 0x16;


#if 	Debug_Usart_use_433data==1 || CC1101_Use==1
    uint16_t my_length = 0;
    my_length = send_number + 11;
#endif


#if Debug_Usart_use_433data==1
    HAL_UART_Transmit(USARTx, my_101_send_buf, my_length, 3000); //���ô��ڴ������ݣ�����ʹ��
#endif

#if CC1101_Use==1
    my_fun_CC1101_send_long_data(my_101_send_buf, my_length, mode, desc_address); //����CC1101��������

#endif


}




/*
���ܣ�����ֱ��ADC�������ݣ�4001��Ϣ�壬��Ӧת��������ݣ������Ŵ󣬱���С�����2λ���Ŵ�100������
˼·�����ú���֮ǰ�������������������ADC1�Ĳ���������

����1��Ϊ0����ʾֻ��ת����Ľ����Ϊ1����ʾ�ȷ�ת���������󷢲���ֵ
����2��Ϊ���͵Ŀ����룬����Ϊֱ�������ڣ�����ֱ���ı���
*/


void my_fun_101send_DC_data(UART_HandleTypeDef* USARTx, uint8_t my_status, uint8_t my_contorl_byte)
{
    uint8_t my_data[ADC1_COLM * 2] = {0};
    uint8_t ii = 0;
    uint16_t temp = 0;



    if(my_status >= 1)
    {
        for(ii = 0; ii < ADC1_COLM; ii++)
        {   
					  if(my_sys_start_status==0)
						{
						temp = ADC1_GetValue_Aver[ii]; //ֱ�����ݣ�����ֵ��δ����ת��
						}
						else
						{
							temp=0x11; //ָʾ������������ʶ						
						}
            my_data[2 * ii] = temp;
            my_data[2 * ii + 1] = (temp >> 8) & 0x00FF;

        }
				my_sys_start_status=0;
        my_fun_101_send_long_data(USARTx, my_contorl_byte + 1, my_data, ADC1_COLM * 2, ADDRESS_CHECK, my_CC1101_dest_address); //����ADC��������ֵ

    }


//����ת���������
    for(ii = 0; ii < ADC1_COLM; ii++)  //1�¶ȡ�2��Դ��3�ο���ѹ��4�ɵ�ء�5���ϵ�ѹ��6̫���ܡ�7﮵��
    {   
			if(my_sys_start_status==0)
			{
					temp = (uint16_t)(ADC1_Filer_value_buf[ii] * 10); //ֱ�����ݣ�ת��������������ŵ�100��������С�����2λ;�Ŵ�10��������С�����1λ
			}
			else
				{
					temp=0x1111; //ָʾ������������ʶ						
				}
				my_data[2 * ii] = temp;
        my_data[2 * ii + 1] = (temp >> 8) & 0x00FF;
			

    }
		my_sys_start_status=0;
    my_fun_101_send_long_data(USARTx, my_contorl_byte, my_data, ADC1_COLM * 2, ADDRESS_CHECK, my_CC1101_dest_address); //����ת����Ľ��
}


//���ͽ�����Чֵ

void my_fun_101send_AC_data(UART_HandleTypeDef* USARTx, uint8_t my_status, uint8_t my_contorl_byte)
{

    uint8_t my_data[ADC2_COLM * 2] = {0};
    uint8_t ii = 0;
    uint16_t temp = 0;
    //����ֵ

    if(my_status >= 1)
    {
        for(ii = 0; ii < ADC2_COLM; ii++)
        {
            temp = ADC2_Filer_value_buf_1[ii][1];  //����ֵ��0ƽ��ֵ��1��Чֵ��2���ֵ
            my_data[2 * ii] = temp;
            my_data[2 * ii + 1] = (temp >> 8) & 0x00FF;

        }
        my_fun_101_send_long_data(USARTx, my_contorl_byte + 1, my_data, ADC2_COLM * 2, ADDRESS_CHECK, my_CC1101_dest_address); //����ADC��������ֵ
    }
    //=============
    if(my_contorl_byte == 0X41)
    {
        for(ii = 0; ii < ADC2_COLM; ii++)
        {   temp = (uint16_t)(ADC2_Filer_value_buf_2[ii][1] * 10); //ת��ֵ
            my_data[2 * ii] = temp;
            my_data[2 * ii + 1] = (temp >> 8) & 0x00FF;

        }
    }
    else if(my_contorl_byte == 0X51)
    {

        for(ii = 0; ii < ADC2_COLM; ii++)
        {   temp = (uint16_t)(ADC2_Filer_value_buf_3[ii][1] * 10); //ת��ֵ
            my_data[2 * ii] = temp;
            my_data[2 * ii + 1] = (temp >> 8) & 0x00FF;

        }
    }



    my_fun_101_send_long_data(USARTx, my_contorl_byte, my_data, ADC2_COLM * 2, ADDRESS_CHECK, my_CC1101_dest_address); //����ת����Ľ��

}


void my_fun_101send_AC12T_Cyc_data(UART_HandleTypeDef* USARTx, uint8_t my_status, uint8_t my_contorl_byte)
{

    uint8_t my_data[ADC2_COLM * 12 * 2] = {0};
    uint8_t ii = 0;
    uint16_t temp = 0;




    if(my_status >= 1)
    {
        for(ii = 0; ii < 12; ii++)
        {
            temp = WAVE_all_ave_Current1[ii][1];
            my_data[2 * ii] = temp;
            my_data[2 * ii + 1] = (temp >> 8) & 0x00FF;

        }

        for(ii = 0; ii < 12; ii++)
        {
            temp = (uint16_t)(WAVE_all_ave_E_Field1[ii][1] );
            my_data[2 * ii + 24] = temp;
            my_data[2 * ii + 1 + 24] = (temp >> 8) & 0x00FF;

        }
        for(ii = 0; ii < 12; ii++)
        {
            temp = (uint16_t)(WAVE_half_ave_Current1[ii][1]);
            my_data[2 * ii + 48] = temp;
            my_data[2 * ii + 1 + 48] = (temp >> 8) & 0x00FF;

        }
        my_fun_101_send_long_data(USARTx, my_contorl_byte + 1, my_data, ADC2_COLM * 12 * 2, ADDRESS_CHECK, my_CC1101_dest_address); //����ADC��������ֵ
    }


    if(my_contorl_byte == 0X42) //����
    {

        for(ii = 0; ii < 12; ii++)
        {   temp = (uint16_t)(WAVE_all_ave_Current2[ii][1] * 10); //����ȫ��
            my_data[2 * ii] = temp;
            my_data[2 * ii + 1] = (temp >> 8) & 0x00FF;

        }
        for(ii = 0; ii < 12; ii++)
        {   temp = (uint16_t)(WAVE_all_ave_E_Field2[ii][1] * 10);//�糡ȫ��
            my_data[2 * ii + 24] = temp;
            my_data[2 * ii + 1 + 24] = (temp >> 8) & 0x00FF;

        }
        for(ii = 0; ii < 12; ii++)
        {   temp = (uint16_t)(WAVE_half_ave_Current2[ii][1] * 10); //�����벨
            my_data[2 * ii + 48] = temp;
            my_data[2 * ii + 1 + 48] = (temp >> 8) & 0x00FF;

        }
    }
    else if(my_contorl_byte == 0X52)
    {

        for(ii = 0; ii < 12; ii++)
        {   temp = (uint16_t)(WAVE_all_ave_Current3[ii][1] * 10);
            my_data[2 * ii] = temp;
            my_data[2 * ii + 1] = (temp >> 8) & 0x00FF;

        }
        for(ii = 0; ii < 12; ii++)
        {   temp = (uint16_t)(WAVE_all_ave_E_Field3[ii][1] * 10);
            my_data[2 * ii + 24] = temp;
            my_data[2 * ii + 1 + 24] = (temp >> 8) & 0x00FF;

        }
        for(ii = 0; ii < 12; ii++)
        {   temp = (uint16_t)(WAVE_half_ave_Current3[ii][1] * 10);
            my_data[2 * ii + 48] = temp;
            my_data[2 * ii + 1 + 48] = (temp >> 8) & 0x00FF;

        }
    }
    my_fun_101_send_long_data(USARTx, my_contorl_byte, my_data, ADC2_COLM * 12 * 2, ADDRESS_CHECK, my_CC1101_dest_address); //����ת����Ľ��


}

/*
���ܣ�//����¼������
*/

void my_fun_101send_AC_Rec_data(UART_HandleTypeDef* USARTx, uint8_t my_status, uint8_t my_contorl_byte)
{
    uint8_t *my_data = my_temp8_buf1;
    uint8_t my_row = 0;
    uint16_t ii = 0;
    uint16_t temp = 0; //�з�������
    int my_cc=0;



    if(my_contorl_byte == 0X43 ) //���ڵ���ȫ��
    {
        my_row = 0;
    }

    else if(my_contorl_byte == 0X44) //���ڵ����糡
    {
        my_row = 1;
    }
    else if(my_contorl_byte == 0X53) //��������ȫ��
    {
        my_row = 10;
    }

    else if(my_contorl_byte == 0X54) //���������糡
    {
        my_row = 11;
    }
   
//--------------------
    if(my_row == 0 ) //ȫ������
    {

        for(ii = 0; ii < WAVE_number_sec2; ii++)
        {
            temp = (my_wave_record_sec2[my_row][ii] - my_all_i_up_value);

            //temp=my_wave_record_sec2[my_row][ii];
            temp = temp * MY_VDD / 4096 * (my_i_ratio_value * my_I_100A_Radio) * 10; //����С�����1λ
            my_data[2 * ii] = temp;
            my_data[2 * ii + 1] = (temp >> 8) & 0x00FF;
        }

    }

    if(my_row == 1 )  //�糡
    {

        for(ii = 0; ii < WAVE_number_sec2; ii++)
        {
            temp=my_wave_record_sec2[my_row][ii];
            temp = temp * MY_VDD / 4096 * (my_E_ratio_value) * 10; //����С�����1λ

//#if				CC1101_SEND_E_Simulation_data_status==1
//					temp=ii;
//#endif
            my_data[2 * ii] = temp;
            my_data[2 * ii + 1] = (temp >> 8) & 0x00FF;


        }

    }



    else if(my_row == 10 )
    {
        my_row = my_row - 10;
        for(ii = 0; ii < WAVE_number_sec3; ii++)
        {
            temp = (my_wave_record_sec3[my_row][ii] - my_all_i_up_value);

            //temp=my_wave_record_sec2[my_row][ii];
            temp = temp * MY_VDD / 4096 * (my_i_ratio_value * my_I_100A_Radio) * 10; //����С�����1λ
            my_data[2 * ii] = temp;
            my_data[2 * ii + 1] = (temp >> 8) & 0x00FF;
        }

    }
    else if(my_row == 11 )
    {
        my_row = my_row - 10;
        for(ii = 0; ii < WAVE_number_sec3; ii++)
        {

            temp=my_wave_record_sec3[my_row][ii];
            temp = temp * MY_VDD / 4096 * (my_E_ratio_value ) * 10; //����С�����1λ
            my_data[2 * ii] = temp;
            my_data[2 * ii + 1] = (temp >> 8) & 0x00FF;
        }

    }

    //����ʹ��
#if Debug_Usart_OUT_WAVE_Chazhi==1
    my_cc = my_wave_record_sec2[my_row][0] - my_wave_record_sec2[my_row][880];
    printf("error chazhi=%d\n", my_cc);
#endif


    my_fun_101_send_long_data(USARTx, my_contorl_byte, my_data, WAVE_number_sec2 * 2, ADDRESS_CHECK, my_CC1101_dest_address); //����ת����Ľ��



}

/*

���ܣ��������,���MCU�ĶԻ�����
*/


extern uint16_t my_pc01_count;
extern uint16_t my_pc02_count;

extern uint16_t my_dianliu_exit_add; //�˱��������������жϻ��߶�ʱ����ʱ�̣���ѯ¼��1�����������õĵ�ַ
extern uint16_t my_Wave_It_add;  //��¼�ж�ʱ�̵�ַ����ͬ��¼��ʹ��

extern uint8_t  my_Current_exit_Status; //��ʾ���������ж�
//extern uint16_t my_Current_Exit_add;

extern uint8_t  my_E_Field_exit_Status; //��ʾ�糡�����ж�
extern uint16_t my_E_Field_exit_add;

extern uint8_t  my_Time_Cyc_exit_Status; //��ʾ�糡�����ж�
extern uint16_t my_Time_Cyc_exit_add;

extern uint16_t my_wave_write_add;

uint8_t my_DC_AC_status = 0; //�������ݵ����ͣ�0Ϊֻ����ת��������ݣ�1Ϊ�ȷ���ת��������ݣ�����ADC�Ĳ���ֵ��
uint16_t my_pro_step = 0;
uint8_t my_step_send_count = 0;

uint16_t my_step_3S_start_count = 0;
uint16_t my_step_3S_end_count = 0;

extern uint16_t my_send_3S_count;
extern uint8_t my_send_ALL_A_960_data_status;  //����¼�����ݱ�־��1Ϊ��ʾ���ͣ�0��ʾ������
extern uint8_t my_send_ALL_E_960_data_status;
extern uint8_t my_send_HALF_A_960_data_status;


/*
���ܣ�����ң��֡
*/
extern uint16_t  my_cyc_delay;
void my_fun_101send_Alarm_status_data(UART_HandleTypeDef* USARTx, uint8_t my_status, uint8_t my_contorl_byte)
{
    uint8_t my_data[4] = {0}; //����һ����ʱ��

    my_data[0] = my_Fault_Current_End_Status;
    my_data[1] = my_cyc_delay;  //��ʱ���ֽ�
    my_data[2] = my_Fault_E_Fild_End_Status;
    my_data[3] = (my_cyc_delay>>8);  //��ʱ���ֽ�
		//my_sys_start_status=0;

    my_fun_101_send_long_data(USARTx, my_contorl_byte, my_data, 4, ADDRESS_CHECK, my_CC1101_dest_address); //����ת����Ľ�������ô��ڽ��з���



}





//���ܣ�����PWR����֡
void my_fun_101send_PWR_heart_data(UART_HandleTypeDef* USARTx, uint8_t my_contorl_byte)
{

    uint8_t my_data[8] = {0};
    if(my_PWR_all_count == 1)
        my_PWR_heart_count++;

    my_data[0] = 0X10;
    my_data[1] = 0X1F;
    my_data[2] = 0X01;
    my_data[3] = 0X00;
    my_data[4] = my_PWR_heart_count;
    my_data[5] = (my_PWR_heart_count >> 8);
    my_data[6] = my_fun_101check_generate(my_data, 1);
    my_data[7] = 0X16;

    HAL_UART_Transmit(USARTx, my_data, 8, 3000);


}
void my_fun_101send_PWR_OK_data(UART_HandleTypeDef* USARTx, uint8_t my_contorl_byte)
{

    uint8_t my_data[8] = {0};


    my_data[0] = 0X10;
    my_data[1] = 0X20;
    my_data[2] = 0X01;
    my_data[3] = 0X00;
    my_data[4] = my_PWR_heart_count;
    my_data[5] = (my_PWR_heart_count >> 8);
    my_data[6] = my_fun_101check_generate(my_data, 1);
    my_data[7] = 0X16;

    HAL_UART_Transmit(USARTx, my_data, 8, 3000);
}



//==========
/*
���ܣ�����CC1101���Ͷ�֡���ݣ�8���ֽ�
����1������ID
����2�����͵�16bit����
����3������ģʽ���㲥���ߵ�ַ���
����4��Ŀ���ַ��CC1101�ĵ�ַ
*/

void my_fun_101_send_short_data_CC1101(uint8_t control_bye, uint16_t data, TX_DATA_MODE mode, INT8U desc_address)
{


    my_101_send_buf[0] = 0x10; //֡ͷ
    my_101_send_buf[1] = control_bye; //ID
    my_101_send_buf[2] = 01; //ADD_L
    my_101_send_buf[3] = 02; //ADD_H
    my_101_send_buf[4] = data; //���ݵ��ֽ�
    my_101_send_buf[5] = (data >> 8) ; //���ݸ��ֽ�
    my_101_send_buf[6] = my_fun_101check_generate(my_101_send_buf, 1);
    my_101_send_buf[7] = 0x16;

    my_fun_CC1101_send_long_data(my_101_send_buf, 8, mode, desc_address); //����CC1101��������
}
/*

���ܣ����ͳ�֡����
*/


void my_fun_101_send_long_data_CC1101(uint8_t control_bye, uint8_t *send_buf, uint16_t send_number, TX_DATA_MODE mode, INT8U desc_address)
{
    uint16_t ii = 0;
    uint16_t my_length = 0;


    my_101_send_buf[0] = 0x68; //֡ͷ
    my_101_send_buf[1] = (uint8_t)((send_number + 3) & 0x00ff); //����,��ID��ʼ��ĳ���
    my_101_send_buf[2] = (uint8_t)(((send_number + 3) & 0xff00) >> 8); //���ȣ���֡ͷ����ĳ���

    my_101_send_buf[3] = my_101_send_buf[1];
    my_101_send_buf[4] = my_101_send_buf[2];

    my_101_send_buf[5] = 0x68; //֡ͷ
    my_101_send_buf[6] = control_bye;
    my_101_send_buf[7] = 01; //ADD_L
    my_101_send_buf[8] = 02; //ADD_H


    for(ii = 0; ii < send_number; ii++)
    {
        my_101_send_buf[9 + ii] = send_buf[ii];

    }
    my_101_send_buf[9 + ii] = my_fun_101check_generate(my_101_send_buf, 1);;
    my_101_send_buf[9 + ii + 1] = 0x16;
    my_length = send_number + 11;

    my_fun_CC1101_send_long_data(my_101_send_buf, my_length, mode, desc_address); //����CC1101��������

}