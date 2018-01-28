#include "gpio.h"
#include "my_usart.h"
#include "my_rtc.h"
#include "my_adc.h"
#include "my_time.h"
#include "my_wave_rec.h"
#include "my_gloabal_val.h"
//#include "my_led.h"
#include "my_cc1101.h"
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "event_groups.h"
#include "my_extern_val.h"

extern EventGroupHandle_t xCreatedEventGroup;
extern EventGroupHandle_t xCreatedEventGroup2;

extern uint16_t ADC2_GetValue[ADC2_ROW][ADC2_COLM];//���ڱ���ɼ���ֵ,M��ͨ��,N��
extern uint16_t my_wave_record[2][WAVE_number];
extern uint16_t my_wave_write_add;  //ȫ��¼��һ������

extern uint16_t my_PA01_count;
extern uint16_t my_PA00_count;
extern uint16_t my_PC06_count;

extern uint16_t my_dianliu_exit_add; //�˱��������������жϻ��߶�ʱ����ʱ�̣���ѯ¼��1�����������õĵ�ַ
extern uint16_t my_Wave_It_add;  //��¼�ж�ʱ�̵�ַ����ͬ��¼��ʹ��

extern uint8_t  my_Current_exit_Status; //��ʾ���������ж�
//extern uint16_t my_Current_Exit_add;

extern uint8_t  my_E_Field_exit_Status; //��ʾ�糡�����ж�
extern uint16_t my_E_Field_exit_add;

extern uint8_t  my_Time_Cyc_exit_Status; //��ʾ���ڲ����ж�



uint8_t my_get_count=0; //��õ��ַ�����

void HAL_GPIO_EXTI_Callback ( uint16_t GPIO_Pin)
{
    //int xi=0;
    //int xj=0;
    //PA1=������·�жϣ�PA2=�ӵ��ж�
    uint8_t temp_status=0;
    BaseType_t xResult;
    BaseType_t xHigherPriorityTaskWoken=pdFAIL;
    int my_add1=0;


    if(GPIO_Pin==EXIT_dianliu_Pin)
    {
#if ADC_interrupt_data==1
        my_PA00_count++; //��¼�жϴ���
			  if(my_sys_start_status==1) return;
      
        if(my_Current_exit_Status==0) //����Ѿ������1���жϣ��ڴ������֮ǰ����ֹ�ظ�����
        {
            my_Current_exit_Status=1;  //�����ж�
            my_dianliu_exit_add=my_wave_write_add; //��ǰ¼����ַ
						my_E_Field_exit_add=my_wave_write_add;
            //������ϵ�

            my_add1=my_dianliu_exit_add-40;
            if(my_add1<0)
            {
                my_add1=WAVE_number+my_add1;
            }
            my_dianliu_exit_add=my_add1;
						my_dianliu_exit_add=my_add1;


            printf("\n ==�����ж�==exit PA0,I_interrup=%d====\n",my_PA00_count);
						printf("dianliu_add=%d,E_fild_add=%d",my_dianliu_exit_add,my_E_Field_exit_add);

            //�رյ糡�ж�					
						HAL_NVIC_DisableIRQ(EXIT_jiedi_EXTI_IRQn);
						HAL_NVIC_DisableIRQ(EXIT_dianliu_EXTI_IRQn);
						
						
            //����״̬��ʶ0X01����״̬��ʶ����,PA1
            xResult=	xEventGroupSetBitsFromISR(xCreatedEventGroup2, 0X01,&xHigherPriorityTaskWoken);
            if(xResult!=pdFAIL)
            {
                portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            }



        }
#endif
        //__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_1); //�����������ⲿ�жϣ�ÿ���жϵ������������һ������Ҫ�ֱ����


    }

    else if(GPIO_Pin==EXIT_jiedi_Pin)
    {

        //=============
#if ADC_interrupt_data==1
        my_PA01_count++; //��¼�жϴ���
			  if(my_sys_start_status==1) return;
			  printf("==enter E_Interrupt==%d\n",my_PA01_count);
				printf("==efild exit==CC1101_all_step=[%XH],A_EXIT_status=%d,E_exit_status=%d,cyc_exit_status=%d\n",
			  my_CC1101_all_step,my_E_Field_exit_Status,my_Current_exit_Status,my_Time_Cyc_exit_Status);
			 
        if(my_E_Field_exit_Status==0) //����Ѿ������1���жϣ��ڴ������֮ǰ����ֹ�ظ�����
        {
            my_E_Field_exit_Status=1;  //�����ж�
            my_E_Field_exit_add=my_wave_write_add;
            my_dianliu_exit_add=my_wave_write_add; //��ǰ¼����ַ
            //������ϵ�

            //my_add1=my_dianliu_exit_add-40;
						my_add1=my_dianliu_exit_add-40-320;
            if(my_add1<0)
            {
                my_add1=WAVE_number+my_add1;
            }
            my_dianliu_exit_add=my_add1;
            my_E_Field_exit_add=my_dianliu_exit_add;

            printf("\n ==�糡�ж�==exit PA1,E_interrup=%d====\n",my_PA01_count);
						printf("dianliu_add=%d,E_fild_add=%d",my_dianliu_exit_add,my_E_Field_exit_add);
						
						 //�رյ糡�ж�				
						HAL_NVIC_DisableIRQ(EXIT_jiedi_EXTI_IRQn);
						HAL_NVIC_DisableIRQ(EXIT_dianliu_EXTI_IRQn);

            //����״̬��ʶ0X01����״̬��ʶ����,PA1
            xResult=	xEventGroupSetBitsFromISR(xCreatedEventGroup2, 0X01,&xHigherPriorityTaskWoken);
            if(xResult!=pdFAIL)
            {
                portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            }



        }
#endif




    }

    //CC1101

    //--------   CC1101�ж� ������������--


    else 	if(GPIO_Pin==PIN_CC_IRQ)  //PC6
    {   uint32_t my_temp32=0;
        while (CC_IRQ_READ() == 1)
				{
					my_temp32++;
					if(my_temp32>=0x003FFFFFF)
					{
						printf("\n*****EXIT CC_IRQ_READ() == 1 ******\n");
						CC1101ClrRXBuff( );
						return;
					}
					
				}
        // printf("before rx_count=%d\n",	my_get_count);//����ʹ�ã����յ����ַ�����
        temp_status=CC1101GetRXCnt();  //���յ����ַ�����
        my_get_count=temp_status;
        if(temp_status>0 && temp_status<64 )
        {
            //����״̬��ʶ0X08����״̬��ʶ����
            xResult=	xEventGroupSetBitsFromISR(xCreatedEventGroup, 0X08,&xHigherPriorityTaskWoken);
            if(xResult!=pdFAIL)
            {
                portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            }
            my_PC06_count++;

        }
        else
        {
            //my_pc06_count++;

        }
        //printf("CC_IRQ=%d -- rx_count=%d\r\n",my_pb0_count,temp_status);//��ʾ���յ���������


    }

}
