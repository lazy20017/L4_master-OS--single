#include "my_OS.h"
#include "my_usart.h"
#include "my_ADC.h"
#include "my_wave_rec.h"
#include "my_101.h"
#include "my_extern_val.h"

extern EventGroupHandle_t xCreatedEventGroup;
extern osMessageQId myQueue01Handle;
extern osMessageQId myQueue02Handle;
extern osMutexId myMutex01Handle;

extern osThreadId defaultTaskHandle;
extern osThreadId myTask02Handle;
extern osThreadId myTask03Handle;
extern osThreadId myTask04Handle;

extern osThreadId myTask08Handle;



extern uint8_t my_CC1101_dest_address;
extern uint8_t my_cc1101_tx_buf[];
extern uint8_t my_CC1101_COM_Fram_buf[];


extern uint16_t my_wave_write_add;
extern uint16_t my_dianliu_exit_add;
//extern uint16_t my_Time_Cyc_exit_add;
extern uint8_t my_DC_AC_status;
extern uint8_t my_CC1101_Frame_status;
extern uint8_t my_CC1101_COM_Fram_buf[];
extern uint16_t  my_CC1101_all_step;


extern uint8_t my_UART1_Status;
extern uint8_t my_UART2_Status;
extern uint8_t my_UART3_Status;

extern uint8_t my_use_alarm_rec_data_status_Efild;

uint16_t  my_GPRS_all_step = 0;
uint8_t my_GPRS_all_count = 0;

uint8_t  my_CC1101_all_count = 0; //�ظ����ͼ���
uint16_t my_PWR_all_step = 0;
uint8_t my_PWR_all_count = 0;

uint8_t temp8 = 0;

void my_fun_give_Queue(osMessageQId *my_QHL, uint16_t temp_step)
{
    BaseType_t pt = NULL;
    BaseType_t xResult;
    xResult = xQueueSendFromISR(*my_QHL, &temp_step, &pt);

    if(xResult != pdFAIL)
    {
        //printf(" send QH OK--[%XH]\r\n",temp_step);
        portYIELD_FROM_ISR(pt);

    }
    else
    {
        printf(" send QH ERROR--[%XH],xRseult=[%XH]--\r\n", temp_step, xResult);
        
        NVIC_SystemReset(); //Ӧ�ô�ʹ��20171025@@@@@@



    }
    //printf("queue3 is %d\r\n",temp8);
}


//=====CC1101����ʹ��

//============
void my_fun_CC1101_time_dialog_tx2(
    uint16_t my_get_step,
    uint16_t my_before_step,
    uint16_t my_now_step,
    uint8_t end_status,
    void (*ptfun)(void)
)
{
    //===�жϲ���
    if(my_get_step == my_now_step && my_before_step == 0X00)
    {
        //printf("CC1101 TX--step = [%XH]\r\n",my_now_step);
        my_CC1101_all_count = 0;
        my_CC1101_all_step = my_now_step;
        my_get_step = 0;
    }
    else if(my_get_step == my_now_step && my_before_step == my_CC1101_all_step)
    {
        // printf("CC1101 TX--step = [%XH]\r\n",my_now_step);
        my_CC1101_all_count = 0;
        my_CC1101_all_step = my_now_step;
        my_get_step = 0;
    }

    //===�ظ����Ͳ���
    if( my_now_step == my_CC1101_all_step && my_CC1101_all_count < 3 )
    {
        my_CC1101_all_count++;
        printf("CC1101 TX-Fun= [%XH]--%d\r\n", my_now_step, my_CC1101_all_count);
        //osDelay(1);
        //HAL_Delay(1);//CC1101�������ݣ�Ҫ��ʱһ�£���Ϊ����ϵͳ�����л���Ҫ1ms
        ptfun();		//���ö�Ӧ�ĺ���
    }
    else if(my_CC1101_all_count >= 3)
    {

        my_CC1101_all_count = 0;
        my_CC1101_all_step = 0x00;
        my_Time_Cyc_exit_Status = 0;


        if(my_CC1101_Sleep_status == 1 || my_DTU_send_faile_count>=2)
        {
            CC1101SetSleep();
        }
        //DTUʧ�ܴ�����ʶ
        my_DTU_send_faile_count++;
        if(my_DTU_send_faile_count >= 0XFFFF)
            my_DTU_send_faile_count = 3;

    }

    //====ֻ����һ�ξͽ���
    if(end_status == 1 && my_CC1101_all_count > 0 && my_CC1101_all_step == my_now_step )
    {
        my_CC1101_all_count = 0;
        my_CC1101_all_step = 0x00;

        //my_Time_Cyc_exit_Status = 0;



        if(my_CC1101_Sleep_status == 1)
        {
            CC1101SetSleep();
        }

    }

}


//==========
void my_fun_CC1101_time_dialog_rx2(
    osMessageQId *QHL_send,
    uint16_t my_get_step,   //
    uint16_t my_before_step,
    uint16_t my_now_step,
    uint16_t my_next_step,
    uint8_t end_status,
    uint8_t (*ptfun)(void)
)
{
    uint8_t my_status = 0;
    uint8_t my_temp = 0;

    //=====0====
    if(my_get_step == my_now_step && my_before_step == 0x00) //������������յ�������
    {
        my_status = 1;
        my_DTU_send_faile_count = 0;
    }
    else if(my_get_step == my_now_step && my_before_step == my_CC1101_all_step) //�����󶨣���ǰ������
    {
        my_status = 1;
        my_DTU_send_faile_count = 0; //DTU�ڷ�������ʧ�ܼ���
    }
    else
    {
        return;

    }
    //======1=====
    if(my_status == 1 && end_status == 0)
    {
        //printf("CC1101 RX-step = [%XH]\r\n",my_now_step);


        my_temp = ptfun();
        if(my_temp == 1)
        {
            my_CC1101_all_step = my_now_step;	//��ǰ״̬
            xQueueSend(*QHL_send, &my_next_step, 100);	//��ʶ��һ��״̬
        }
        else
            printf("�������ݴ��󣬲�����״̬ת��\r\n");
    }
    else if(my_status == 1 && end_status == 1)
    {
        // printf("CC1101 RX-step = [%XH]\r\n",my_now_step);
        ptfun();
        my_CC1101_all_step = 0X00; //����״̬

        my_Time_Cyc_exit_Status = 0;

        if(my_CC1101_Sleep_status == 1)
        {
            CC1101SetSleep();
        }

    }

    //

}


//=========
//#define com_long_10  "\x10\x20\x3\x4\x5\x6\x7\x16"
//#define com_short_68 "\x68\x04\x04\x68\x1\x2\x3\x4\x5\x16"
//#define com_long_68 "\x68\x05\x00\x05\x00\x68\x1\x2\x3\x4\x5\x6\x16"
//uint8_t my_buf2[]=com_long_10;
//uint8_t my_buf3[]=com_short_68;
//uint8_t my_buf4[]=com_long_68;
//uint8_t my_cc1101_tx_buf[64]={0x10,0x20,0x13,0x14,0x15,0x16};


uint8_t my_heart_count = 0;
void my_fun_CC1101_test2(void)
{
    uint8_t temp_status = 0;
    if(my_CC1101_COM_Fram_buf[0] == 0x10)
    {
        temp_status = my_CC1101_COM_Fram_buf[1];
        my_fun_display_buf_16(my_CC1101_COM_Fram_buf, 8, 0);
    }
    else if (my_CC1101_COM_Fram_buf[0] == 0x68)
    {
        temp_status = my_CC1101_COM_Fram_buf[6];
        my_fun_display_buf_16(my_CC1101_COM_Fram_buf, 8, 0);
    }

    if(temp_status == 0x75)
        printf("====CC heart count %d=======\r\n", my_heart_count++);
    else if(temp_status == 0xF0)
    {
        printf("�����������ݿ�ʼ  F0\r\n");
    }
    else if(temp_status == 0xF2)
    {
        printf("�����������ݿ�ʼ  block=%d   F2\r\n", my_CC1101_COM_Fram_buf[7]);
    }
    else if(temp_status == 0xF4)
    {
        printf("�����������ݰ�   F4\r\n");
    }
    else if(temp_status == 0xF6)
    {
        printf("�����������ݰ�   block=%d  F6\r\n", my_CC1101_COM_Fram_buf[7]);
    }
    else if(temp_status == 0xF8)
    {
        printf("�������ݴ������   F8\r\n");
    }


}
//=====================
//ָʾ��ģ����򲿷֣�����ģ��ָʾ�������ݷ��͸�DTU
//1A--1,1B--2,1C--3    2A--1,2B--2,2C--3    3A--1,3B--2,3C--3
void my_fun_indicator_heart_awaken_send(void)
{
    //10-71-LL-YY-01-00-CRC-16
    static uint8_t count = 0;
    uint8_t indicator_ABC = 1;
    count++;
    my_cc1101_tx_buf[0] = 0x10;
    my_cc1101_tx_buf[1] = 0X71;
    my_cc1101_tx_buf[2] = indicator_ABC;
    my_cc1101_tx_buf[3] = 00;
    my_cc1101_tx_buf[4] = count;
    my_cc1101_tx_buf[5] = 00;
    my_cc1101_tx_buf[6] = count;
    my_cc1101_tx_buf[7] = 0X16;

    CC1101SendPacket_add(my_cc1101_tx_buf, 8, ADDRESS_CHECK, my_CC1101_dest_address);
    my_fun_display_buf_16(my_cc1101_tx_buf, 8, 1); //����ʹ��
}
//1A--1,1B--2,1C--3    2A--1,2B--2,2C--3    3A--1,3B--2,3C--3
void my_fun_indicator_heart_data_send(void)
{
    //10-71-LL-YY-01-00-CRC-16
    static uint8_t count = 0;
    uint8_t indicator_ABC = 1;
    count++;

    my_cc1101_tx_buf[0] = 0x10;
    my_cc1101_tx_buf[1] = 0X73;
    my_cc1101_tx_buf[2] = indicator_ABC;
    my_cc1101_tx_buf[3] = 00;
    my_cc1101_tx_buf[4] = count;
    my_cc1101_tx_buf[5] = 00;
    my_cc1101_tx_buf[6] = count;
    my_cc1101_tx_buf[7] = 0X16;

    CC1101SendPacket_add(my_cc1101_tx_buf, 8, ADDRESS_CHECK, my_CC1101_dest_address);
    my_fun_display_buf_16(my_cc1101_tx_buf, 8, 1); //����ʹ��
}

void my_fun_indicator_heart_data_send2(void)
{
    //10-71-LL-YY-01-00-CRC-16
    static uint8_t count = 0;
    uint8_t indicator_ABC = 1;
    count++;

    my_cc1101_tx_buf[0] = 0x10;
    my_cc1101_tx_buf[1] = 0X75;
    my_cc1101_tx_buf[2] = indicator_ABC;
    my_cc1101_tx_buf[3] = 00;
    my_cc1101_tx_buf[4] = count;
    my_cc1101_tx_buf[5] = 00;
    my_cc1101_tx_buf[6] = count;
    my_cc1101_tx_buf[7] = 0X16;

    CC1101SendPacket_add(my_cc1101_tx_buf, 8, ADDRESS_CHECK, my_CC1101_dest_address);
    my_fun_display_buf_16(my_cc1101_tx_buf, 8, 1); //����ʹ��
}


//=========���մ�����==========
uint8_t my_fun_GPRS_RX_test1(void) //�˺���Ϊ�����������յ�OK֡�󣬽����Ի�����
{

    printf("GPRS dialog is Finish!---[%XH]\r\n", my_GPRS_all_step);
    return 1;

}

//ң�Ž��յ�OK֡
uint8_t my_fun_GPRS_RX_test2(void)  //���յ�����֡����������
{

    printf("GPRS dialog get OK frame---[%XH]\r\n", my_GPRS_all_step);
    return 1;
}
//===CC1101,���ʹ�����
void my_fun_TX_CC1101_test0(void)  //ң��
{

    //=====0 ����ң�����ݰ�
    if(my_CC1101_all_step == 0x0001)
    {
        //==����
        my_fun_101send_Alarm_status_data(&huart2, my_DC_AC_status, 0X01);
    }
    else if(my_CC1101_all_step == 0x0002)
    {
        //==����
        my_fun_101send_Alarm_status_data(&huart2, my_DC_AC_status, 0X02);
    }



#if OS_CC1101_auto_reveive_OK==1
    my_fun_give_Queue(&myQueue02Handle, 0x2000); //@@@@@
#endif


}

void my_fun_TX_CC1101_test1(void)  //ң�� ֱ��
{
    if(my_CC1101_all_step == 0x0040) //����
    {
        //=====1 ����ֱ�����ݰ�
        my_fun_101send_DC_data(&huart2, my_DC_AC_status, 0X40); //����ֱ������
    }
    else if(my_CC1101_all_step == 0x0050)
    {
        //====����
        my_fun_101send_DC_data(&huart2, my_DC_AC_status, 0X50); //����ֱ������

    }

#if Debug_Usart_OUT_DC_DATA_status==1

    my_adc_1_convert_dis(0);
#endif

#if OS_CC1101_auto_reveive_OK==1
    my_fun_give_Queue(&myQueue02Handle, 0x2000); //@@@@@
#endif




}
void my_fun_TX_CC1101_test2(void)  //ң�� ������Чֵ
{

    if(my_CC1101_all_step == 0x0041) //������Чֵ
    {
        //===2����AC
        my_fun_101send_AC_data(&huart2, my_DC_AC_status, 0X41); //
    }
    else if(my_CC1101_all_step == 0x0051)
    {
        //����
        my_fun_101send_AC_data(&huart2, my_DC_AC_status, 0X51); //
    }
    //====	2 ���ͽ������ݰ�


#if OS_CC1101_auto_reveive_OK==1
    my_fun_give_Queue(&myQueue02Handle, 0x2000); //@@@@@
#endif

}

void my_fun_TX_CC1101_test3(void)
{


    if(my_CC1101_all_step == 0x0042)
    {
        //=====3  ����12�����ڵ���Чֵ12TAC
        my_fun_101send_AC12T_Cyc_data(&huart2, my_DC_AC_status, 0X42); //
    }
    else if(my_CC1101_all_step == 0x0052)
    {
        //==����
        my_fun_101send_AC12T_Cyc_data(&huart2, my_DC_AC_status, 0X52); //

    }


#if OS_CC1101_auto_reveive_OK==1
    my_fun_give_Queue(&myQueue02Handle, 0x2000); //@@@@@
#endif

}
uint8_t CC1101_960data_Efield_STATUS = 0;
void my_fun_TX_CC1101_test4(void)
{
    if(my_CC1101_all_step == 0x0043)
    {
        //=====4  ����¼�����ݣ�����
        if (CC1101_960data_Efield_STATUS == 1) //���Ͳ��ε�ѡ��1Ϊ�糡��0Ϊ����
            my_fun_101send_AC_Rec_data(&huart2, my_DC_AC_status, 0X44); //960�糡
        else
            my_fun_101send_AC_Rec_data(&huart2, my_DC_AC_status, 0X43); //960ȫ������

    }
    else if(my_CC1101_all_step == 0x0053)
    {
        //====¼�����ݣ�����
        if (CC1101_960data_Efield_STATUS == 1)
            my_fun_101send_AC_Rec_data(&huart2, my_DC_AC_status, 0X54); //�糡
        else
            my_fun_101send_AC_Rec_data(&huart2, my_DC_AC_status, 0X53); //����
    }

#if CC1101_TX_Delay==1
    HAL_Delay(2000);
#endif

#if Debug_Usart_out_ADCdata==1
    //my_adc2_convert_dis(0); //@@@ ������ʾ����AC�������Դ���
#endif
#if OS_CC1101_auto_reveive_OK==1
    my_fun_give_Queue(&myQueue02Handle, 0x2000); //@@@@@
#endif

}
//�糡
void my_fun_TX_CC1101_test5(void)
{
    if(my_CC1101_all_step == 0x0044)
    {
        //=====4  ����¼�����ݣ�����
       
            my_fun_101send_AC_Rec_data(&huart2, my_DC_AC_status, 0X44); //960�糡
        

    }
    else if(my_CC1101_all_step == 0x0054)
    {
        //====¼�����ݣ�����
        
            my_fun_101send_AC_Rec_data(&huart2, my_DC_AC_status, 0X54); //�糡
        
    }

#if CC1101_TX_Delay==1
    HAL_Delay(2000);
#endif

#if Debug_Usart_out_ADCdata==1
    //my_adc2_convert_dis(0); //@@@ ������ʾ����AC�������Դ���
#endif
#if OS_CC1101_auto_reveive_OK==1
    my_fun_give_Queue(&myQueue02Handle, 0x2000); //@@@@@
#endif

}

//�糡 over



void my_fun_CC1101_test1(void)
{
    uint8_t *pt;
    uint8_t my_step = 0;
    uint8_t indicator_ABC = 1;
    if(my_step == 0)
    {
        my_cc1101_tx_buf[0] = 0x10;
        my_cc1101_tx_buf[1] = 0X20;
        my_cc1101_tx_buf[2] = indicator_ABC;
        my_cc1101_tx_buf[3] = 00;
        my_cc1101_tx_buf[4] = 00;
        my_cc1101_tx_buf[5] = 00;
        my_cc1101_tx_buf[6] = 00;
        my_cc1101_tx_buf[7] = 0X16;
        pt = my_cc1101_tx_buf;
        pt[6] = my_fun_101check_generate(pt, 1);
        HAL_Delay(100);
        CC1101SendPacket_add( pt, 8,  ADDRESS_CHECK, my_CC1101_dest_address);
        my_fun_display_buf_16(pt, 8, 1); //����ʹ��

    }


}

//===CC1101  ���նԻ�
extern uint16_t my_que1_wait_time;
uint8_t my_fun_RX_CC1101_text0_RX_OK(void)
{
    uint16_t my_temp = 0;
    if(my_CC1101_COM_Fram_buf[1] == 0x20) //����tim6��Уʱ
    {
        my_temp = my_CC1101_COM_Fram_buf[5];
        my_temp = (my_temp << 8) + my_CC1101_COM_Fram_buf[4];
        my_tim6_count = my_temp;
    }
    if(my_CC1101_COM_Fram_buf[1] == 0x20 && my_CC1101_all_step == 0x0042)
    {
				my_Time_Cyc_exit_Status = 0;
				if(my_CC1101_Sleep_status==1)
					CC1101SetSleep();
				
        printf("====CC1101 CYC TIME FINISH!!===\n\n");
    }
    else if(my_CC1101_COM_Fram_buf[1] == 0x20 && my_CC1101_all_step == 0x00E0)
    {
				my_Time_Cyc_exit_Status = 0;
				if(my_CC1101_Sleep_status==1)
					CC1101SetSleep();
        printf("====CC1101 Heart TIME FINISH!!===\n\n");
    }
		else if(my_CC1101_COM_Fram_buf[1] == 0x20 && my_CC1101_all_step == 0x0053)
    {  
			if(my_use_alarm_rec_data_status_Efild==0)
			{
				//my_zsq_ALarm_send_status=0;
				if(my_CC1101_Sleep_status==1)
				 	CC1101SetSleep();
			}
        printf("====@@@@ CC1101 ALarm TIME FINISH!!==dianliu=\n\n");
    }
		
		else if(my_CC1101_COM_Fram_buf[1] == 0x20 && my_CC1101_all_step == 0x0054)
    {
				my_zsq_ALarm_send_status=0;
				if(my_CC1101_Sleep_status==1)
					CC1101SetSleep();
        printf("====@@@@ CC1101 ALarm TIME FINISH!!==jiedi=\n\n");
    }
		
		//
		if(my_CC1101_COM_Fram_buf[1] == 0x20 && my_CC1101_all_step == 0x0052)
			my_que1_wait_time=10000;
		else if(my_CC1101_COM_Fram_buf[1] == 0x20 && my_CC1101_all_step == 0x0053)
			my_que1_wait_time=10000;
		else
			my_que1_wait_time=2000;
    return 1;
}






void my_fun_PWR_time_dialog_tx2(
    uint16_t my_get_step,
    uint16_t my_before_step,
    uint16_t my_now_step,
    uint8_t end_status,
    void (*ptfun)(void)
)
{
    //===�жϲ���
    if(my_get_step == my_now_step && my_before_step == 0X00)
    {
        //printf("CC1101 TX--step = [%XH]\r\n",my_now_step);
        my_PWR_all_count = 0;
        my_PWR_all_step = my_now_step;
        my_get_step = 0;
    }
    else if(my_get_step == my_now_step && my_before_step == my_PWR_all_step)
    {
        // printf("CC1101 TX--step = [%XH]\r\n",my_now_step);
        my_PWR_all_count = 0;
        my_PWR_all_step = my_now_step;
        my_get_step = 0;
    }

    //===�ظ����Ͳ���
    if( my_now_step == my_PWR_all_step && my_PWR_all_count < 3 && my_get_step == 0)
    {
        my_PWR_all_count++;
        printf("PWR TX-Fun= [%XH]--%d\r\n", my_now_step, my_PWR_all_count);
        //osDelay(1);
        //HAL_Delay(1);//CC1101�������ݣ�Ҫ��ʱһ�£���Ϊ����ϵͳ�����л���Ҫ1ms
        ptfun();		//���ö�Ӧ�ĺ���
    }
    else if(my_PWR_all_count >= 3)
    {
        my_PWR_all_count = 0;
        my_PWR_all_step = 0x00;
    }

    //====ֻ����һ�ξͽ���
    if(end_status == 1 && my_PWR_all_count > 0 && my_PWR_all_step == my_now_step && my_get_step == 0)
    {
        my_CC1101_all_count = 0;
        my_PWR_all_step = 0x00;
    }

}


//==========
void my_fun_PWR_time_dialog_rx2(
    osMessageQId *QHL_send,
    uint16_t my_get_step,   //
    uint16_t my_before_step,
    uint16_t my_now_step,
    uint16_t my_next_step,
    uint8_t end_status,
    uint8_t (*ptfun)(void)
)
{
    uint8_t my_status = 0;
    uint8_t my_temp = 0;
    //=====0====
    if(my_get_step == my_now_step && my_before_step == 0x00) //������������յ�������
    {
        my_status = 1;
    }
    else if(my_get_step == my_now_step && my_before_step == my_PWR_all_step) //�����󶨣���ǰ������
    {
        my_status = 1;
    }
    else
    {
        return;
    }
    //======1=====
    if(my_status == 1 && end_status == 0)
    {
        //printf("CC1101 RX-step = [%XH]\r\n",my_now_step);
        my_temp = ptfun();
        if(my_temp == 1)
        {
            my_PWR_all_step = my_now_step;	//��ǰ״̬
            xQueueSend(*QHL_send, &my_next_step, 100);	//��ʶ��һ��״̬
        }
        else
            printf("�������ݴ��󣬲�����״̬ת��\r\n");
    }
    else if(my_status == 1 && end_status == 1)
    {
        // printf("CC1101 RX-step = [%XH]\r\n",my_now_step);
        ptfun();
        my_PWR_all_step = 0X00; //����״̬

    }

    //

}

/*
���ܣ�CC1101���³�ʼ��
*/
void my_fun_CC1101_init_resume(void)
{
    uint8_t my_status = 0;
    uint8_t my_rx_count = 0;
    //xSemaphoreTake(myMutex01Handle,1000);
    CC1101SetIdle();
    HAL_Delay(10);
    my_status = CC1101ReadStatus(CC1101_MARCSTATE);
    my_rx_count = CC1101GetRXCnt();
    //xSemaphoreGive(myMutex01Handle);

    if(my_status == 0x01 && my_rx_count > 0)
    {
        printf("------ CC1101 status=[%XH] RXBUF=%d \n", my_status, my_rx_count);
        my_fun_CC1101_init_reum();
    }

    if(my_status != 0x01 && my_status != 0x0D &&  my_status != 0x13  ) //0X01���У�0X0D���գ�0X13����,0x11�������
    {
        printf("--error CC_status=[%XH] \n", my_status);
        //my_fun_CC1101_init_reum();
        printf("--inint after CC_status=[%XH] \n", my_status);
    }

}

void my_fun_usart_init_resume(void)
{

    if(my_UART2_Status == 0X01)
    {
        MX_USART2_UART_Init();
        my_UART2_Status = 0;
        HAL_UART_Receive_IT(&huart2, &rsbuf2[rsbuf2pt_write], 1); //��������USART3����
    }
}

void my_fun_task_heap_value(void)
{

    portBASE_TYPE uxHighWaterMark;

    uxHighWaterMark = uxTaskGetStackHighWaterMark( defaultTaskHandle );
    printf("task01 heap value=%d\r\n", uxHighWaterMark);

    uxHighWaterMark = uxTaskGetStackHighWaterMark( myTask02Handle );
    printf("task02 heap value=%d\r\n", uxHighWaterMark);
    uxHighWaterMark = uxTaskGetStackHighWaterMark( myTask03Handle );
    printf("task03 heap value=%d\r\n", uxHighWaterMark);
    uxHighWaterMark = uxTaskGetStackHighWaterMark( myTask04Handle );
    printf("task04 heap value=%d\r\n", uxHighWaterMark);
    //uxHighWaterMark = uxTaskGetStackHighWaterMark( myTask05Handle );
    //printf("task05 heap value=%d\r\n", uxHighWaterMark);
    //uxHighWaterMark = uxTaskGetStackHighWaterMark( myTask06Handle );
    //printf("task06 heap value=%d\r\n", uxHighWaterMark);
    //uxHighWaterMark = uxTaskGetStackHighWaterMark( myTask07Handle );
    //printf("task07 heap value=%d\r\n", uxHighWaterMark);
    uxHighWaterMark = uxTaskGetStackHighWaterMark( myTask08Handle );
    printf("task08 heap value=%d\r\n", uxHighWaterMark);


}



void my_fun_TX_CC1101_heart(void)  //����
{

    my_fun_101_send_short_data(&huart2, 0x1F, my_tim6_count, ADDRESS_CHECK, my_CC1101_dest_address); //���Ͷ�֡



#if OS_CC1101_auto_reveive_OK==1
    my_fun_give_Queue(&myQueue02Handle, 0x2000); //@@@@@
#endif


}

