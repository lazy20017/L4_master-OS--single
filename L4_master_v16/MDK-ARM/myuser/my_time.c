#include "gpio.h"
#include "tim.h"
#include "my_led.h"
#include "my_usart.h"
#include "my_extern_val.h"
#include "my_gloabal_val.h"
#include "cmsis_os.h"
#include "my_OS.h"

#include "my_time.h"
#include "my_ADC.h"
#include "my_wave_rec.h"
#include "my_DAC.h"

extern EventGroupHandle_t xCreatedEventGroup2;
extern osMessageQId myQueue01Handle;

extern uint16_t my_wave_record[WAVE_Channel_Num][WAVE_number]; //ȫ����һ�����棬���ö�ʱ�����в��εĴ洢���ٶ�20ms�ɼ�һ�Σ����ܲɼ������ٸ����ݡ�
extern uint16_t my_wave_write_add;  //ȫ��¼��һ������ָ��
extern uint16_t ADC2_GetValue[ADC2_ROW][ADC2_COLM];

uint8_t my_IT_status = 0; //Ϊ0��ʾ��û���жϣ�Ϊ1��ʾ���������жϣ����Ұ�2�����������Ѿ��ƶ���1��������
uint8_t my_IT_Count = 0;  //�жϴ�����¼
uint8_t my_reset_count=0;

extern uint16_t  my_CC1101_all_step;


void HAL_TIM_PeriodElapsedCallback2(TIM_HandleTypeDef *htim)
{


    if(htim == &htim7)
    {

        //    //ȫ��¼�����������ϵ�¼��250usȡһ���㣬
        my_wave_record[0][my_wave_write_add] = (ADC2_GetValue[0][0]+ 	ADC2_GetValue[1][0]+ ADC2_GetValue[2][0])/3; //ȫ������
        my_wave_record[1][my_wave_write_add] = (ADC2_GetValue[0][1]+		ADC2_GetValue[1][1]+	ADC2_GetValue[2][1])/3; //ȫ���糡
        my_wave_record[2][my_wave_write_add] = (ADC2_GetValue[0][2]+		ADC2_GetValue[1][2]+	ADC2_GetValue[2][2])/3; //ȫ���糡

        my_wave_write_add++;
        if(my_wave_write_add >= WAVE_number)
            my_wave_write_add = 0;

        //�Զ��崦���֣���Ҫ��ϵͳ�Զ����ɵ�����Ĵ������ε�

        __HAL_TIM_CLEAR_IT(&htim7, TIM_IT_UPDATE);
        return ;

    }


    //============
    else if(htim == &htim6) //TIM6Ϊ1sһ�Σ�Ϊ����Ӧ�ж�
    {
        my_tim6_count++;
				my_Line_short_count++;
			 //ι���ź�
			  //LED3_TOGGLE;
			  WDI1_TOGGLE;
        
        BaseType_t xResult;
        BaseType_t xHigherPriorityTaskWoken=pdFAIL;

        if(my_Current_exit_Status==1 || my_E_Field_exit_Status==1 || my_sys_start_status==1)
				{
          printf("==Current_exit_status=%d,e_filed_exit_status=%d,my_sys_start_status=%d  time6=%d,reset_count=%d\n"
					,my_Current_exit_Status,my_E_Field_exit_Status,my_sys_start_status,my_tim6_count,my_reset_count);  
					
					my_reset_count++;
					if(my_reset_count>250)
						HAL_NVIC_SystemReset();  //ϵͳ����
						
					return;
				}
				my_reset_count=0;

       
				//�������ڲ���
				 if(my_tim6_count%my_cyc_time_count==0 && my_tim6_count!=0) 
				{
					  my_Time_Cyc_exit_Status=1;
            my_dianliu_exit_add=my_wave_write_add; //��ǰ¼����ַ
            my_Time_Cyc_exit_add = my_wave_write_add;
					
							switch(my_CC1101_chip_address)
						{
						case 0x01:
								my_cyc_delay=my_tim6_count+3;
								break;
						case 0x02:
								my_cyc_delay=my_tim6_count+10;
								break;
						case 0x03:
								my_cyc_delay=my_tim6_count+17;
								break;
						default:
								my_cyc_delay=0;
						}
					
					
            xResult=	xEventGroupSetBitsFromISR(xCreatedEventGroup2, 0X02,&xHigherPriorityTaskWoken);
            if(xResult!=pdFAIL)
            {
                portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            }
					
				}

        


        //�������ڷ���
        if(my_tim6_count==my_cyc_delay )
        {
          printf("\n------my_tim6_count6=%d -----\n",my_tim6_count);
					my_fun_give_Queue(&myQueue01Handle, 0X0001); //������������
        }

			//���ò�������
			if(my_tim6_count % (123+(my_CC1101_chip_address)*2) ==0 && my_CC1101_all_step==0x00 && my_tim6_count!=my_cyc_delay)
			{
					printf("==send config parameter��my_tim6_count=%d !!!--2\n",my_tim6_count);
					my_fun_give_Queue(&myQueue01Handle, 0X00E1); //���ͱ���
			}
       
				
				//CC1101���³�ʼ��
					if(my_tim6_count%3600==0 && my_tim6_count!=0 && my_tim6_count!=my_cyc_delay)	
					{
						my_fun_CC1101_init_reum();
						
					}
					
					 //��ʱһ��ʱ�����¼�����㣬������ϵĵ����͵糡ֵ
        
        if(my_Time_Cyc_exit_Status==0 && my_tim6_count%17==0 && my_tim6_count!=0 && my_tim6_count%my_cyc_time_count!=0 && my_CC1101_all_step==0) //
        {
         
            my_dianliu_exit_add=my_wave_write_add; //��ǰ¼����ַ
            my_Time_Cyc_exit_add = my_wave_write_add;								
						my_ADC_Count++;
            xResult=	xEventGroupSetBitsFromISR(xCreatedEventGroup2, 0X02,&xHigherPriorityTaskWoken);
            if(xResult!=pdFAIL)
            {
                portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            }																	
        }
					

    }


}





