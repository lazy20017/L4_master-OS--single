#include "my_dac.h"
#include "dac.h"
#include "my_ADC.h"
#include "my_wave_rec.h"
#include "my_gloabal_val.h"
#include "my_extern_val.h"








/*
���ܣ�����DA�����жϵĲο���ѹ,�糡�жϲο���ѹ
*/

extern uint16_t my_dac1_channel1_data_i; //����ʱΪ�ߵ�ƽ����·Ϊ�����䣬����һ���½���
extern uint16_t my_dac1_channel2_data_e;  //�糡�����Ӧ��һ������ֵ����������һ�������أ�����ʱ�����0��ƽ�����糡����С�����ֵʱ��������������

extern uint16_t my_all_i_up_value;  //��õ�ȫ��̧��1.2v��ѹ��Ӧ�Ĳ���ƽ��ֵ

extern double my_i_ratio_value;
extern double my_E_ratio_value;
extern double MY_VDD;
extern double my_all_a_adjust; //ʵ�����ݣ�301/387��õģ���300A����ʱ�̣�ʵ��ֵ����ADC����ֵ�ıȽ�ֵ
extern double my_adjust_300_a; //y=x*a+b,��С���˷�����ϵ��a��b��xΪADC����ֵ����У�����ֵ�����ö��˷����ж���У��
extern double my_adjust_300_b;
extern double my_adjust_100_a;
extern double my_adjust_100_b;
extern double my_I_100A_Radio;
extern double my_i_5a_radio;//0.857517365; 
extern double my_i_300a_radio;//0.770811922;


//extern int16_t my_dianliu_exit_add;
extern uint16_t my_wave_write_add;
//extern uint16_t my_Time_Cyc_exit_add;
extern double ADC2_Filer_value_buf_2[][3];

uint16_t my_dac1_channel2_data_e=0;
uint16_t my_dac1_channel1_data_i=0;

#define MY_Rise_current 150  //��Ծ��·������Ĭ��ֵ
int my_150A_ref_int = MY_Rise_current; //��·�����жϵĽ�Ծ����ֵ���������󳬹����ֵ���Ͳ����жϣ��½���,��ֵ�����޸ģ�DACʹ�ã������Ƕ�·�ж�����
//uint16_t my_150A_real_int = 0; //У����150A��Ծ��ѹ��Ӧ��DAC������ֵ
int my_5A_ref_int=5; //�ӵ�©���������Ľ�Ծ�������������ж�

uint16_t my_100E_ref_int = 100;  //�糡����Ĳ�ֵ��ֻҪ������������ֵ���Ͳ����жϡ�������
uint16_t my_100E_real_int = 0; //У��������ѹ��Ӧ��DAC������ֵ




/*
���ܣ�DAC���ú��������ϵ���DAC�Ĳο���ѹ��ԭ�����ò�õ����ϵ�����Чֵ�����Ͻ�Ծ������Чֵ���������жϵ�����Чֵ���Ѵ�ֵ����̧����1.2v��Ӧ��ֵ��
����DAC�����
*/

void my_fun_Set_DAC_I_ref(void)
{
    double temp_i = 0;
	volatile	double temp_e = 0;
    uint16_t my_temp_16=0;
    uint16_t my_12v_int=0;
	
		//GPIO_PinState my_pin_status=HAL_GPIO_ReadPin(EXIT_jiedi_GPIO_Port,EXIT_jiedi_Pin);
	  //printf("====EXIT_jiedi_pin=%d Vref=%.2f\n",my_pin_status,HAL_DAC_GetValue(&hdac1,DAC_CHANNEL_2)/4096.0*3.3);
	

    temp_i = ADC2_Filer_value_buf_2[0][1]; //���12�ܲ���������Чֵ,ת�����ֵ,2Ϊ���ֵ��1Ϊ��Чֵ
    temp_e=temp_i;
#if Debug_Usart_out_DAC_normal_data==1
    printf("DAC_line_i=%.2f MAX_i=%.2f e_aver=%.2f\n",ADC2_Filer_value_buf_2[0][1],ADC2_Filer_value_buf_2[0][2],ADC2_Filer_value_buf_2[1][0]);
#endif

    //������� DA ֵ
    temp_i=(temp_i+my_150A_ref_int); //150A�Ķ�·��Ծ����ֵ
    //if(temp_i>600)
    //temp_i=600;
    //else if(temp_i<150)
    //temp_i=150;

    temp_i=temp_i*1.414;

    if(temp_i>100)
        my_temp_16=(temp_i-my_adjust_300_b)/(my_adjust_300_a*my_all_a_adjust)/(my_i_ratio_value*my_I_100A_Radio)/MY_VDD*4096;
    else
        my_temp_16=(temp_i-my_adjust_100_b)/(my_adjust_100_a*my_all_a_adjust)/(my_i_ratio_value*my_I_100A_Radio)/MY_VDD*4096;


    my_12v_int=(1.2)/3.3*4096;
    my_dac1_channel1_data_i =  my_temp_16 +  my_12v_int;//����̧����ֵ

    HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_12B_R, my_dac1_channel1_data_i); //����������
#if Debug_Usart_out_DAC_normal_data==1
    //printf("\r\n",MY_VDD);
    printf("DAC_Rise_i=[%.2f]_A,OUT_V=[%.2f]_V,MY_VDD=%.2f\n",temp_i,my_dac1_channel1_data_i/4096.0*3.3,MY_VDD); //*=*=
#endif

		
		 //�糡ֵ
		 my_fun_DAC_evref_auto_ajust();

//    //����糡DA ֵ
//    temp_e=temp_e*0.01;
//    temp_e=0;
//    temp_e=(temp_e+my_5A_ref_int);

//    temp_e=temp_e*1.414; //���ֵ

//    if(temp_e>100)
//        my_temp_16=(temp_e-my_adjust_300_b)/(my_adjust_300_a*my_all_a_adjust)/(my_i_ratio_value*my_I_100A_Radio)/MY_VDD*4096;
//    else
//        my_temp_16=(temp_e-my_adjust_100_b)/(my_adjust_100_a*my_all_a_adjust)/(my_i_ratio_value*my_I_100A_Radio)/MY_VDD*4096;

//    //my_dac1_channel2_data_e=my_temp_16;
//		my_12v_int=(1.32)/3.3*4096;
//    my_dac1_channel2_data_e =  my_temp_16 +  my_12v_int;//����̧����ֵ
//    //my_dac1_channel2_data_e=62;//@@@

//    HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_2, DAC_ALIGN_12B_R, my_dac1_channel2_data_e); //����������
//#if Debug_Usart_out_DAC_normal_data==1
//    printf("DAC2_Rise_i=[%.2f]_A,OUT_V=[%.2f]_V \n",temp_e,(my_temp_16*1.0/4096*MY_VDD+1.32)); //*=*=
//    //printf("DAC2_Rise_i=[%.2f]_A,OUT_V=[%.2f]_V \n",temp_e,(my_temp_16*1.0/4096*MY_VDD)); //*=*=
//#endif


}

/*
���ܣ��Զ������ӵ��жϵ�DAC�Ƚ��ź�

*/
double my_adjust_value_V=0.00;
double my_counst_value=0.04*1.414;// �糡�ο���ѹ�ĺ㶨ƫ�0.004��Ӧ1A������
void my_fun_DAC_evref_auto_ajust(void)
{
		double temp_i = 0, temp_e = 0;
    uint16_t my_temp_16=0;
    uint16_t my_12v_int=0;
	
 GPIO_PinState my_pin_status=HAL_GPIO_ReadPin(EXIT_jiedi_GPIO_Port,EXIT_jiedi_Pin);
 
 printf("====EXIT_jiedi_pin=%d Vref=%.2f\n",my_pin_status,HAL_DAC_GetValue(&hdac1,DAC_CHANNEL_2)/4096.0*3.3); //Ĭ������1�����ж�����Ϊ0
	
	if(my_pin_status==0)
	{
		my_adjust_value_V=my_adjust_value_V+0.01; //����Ӧ�ĵ�ѹ����ֵ��һ��0.01V,my_adjust_value_VΪ����
		
    temp_i = ADC2_Filer_value_buf_2[0][1]; //���12�ܲ���������Чֵ,ת�����ֵ,2Ϊ���ֵ��1Ϊ��Чֵ
    temp_e=temp_i;  //���ϵĵ���ֵ

    //����糡DA ֵ
    temp_e=temp_e*0.04; //���ϵ���ֵ�����˲���Ĳ�������ֵ
    temp_e=0; //@@@@@
    //temp_e=(temp_e+my_5A_ref_int); //my_5A_ref_int�ӵ�˲�����ֵ�ý�Ծ��

    temp_e=temp_e*1.414; //��Чֵ����Ϊ���ֵ

//    if(temp_e>100)
//        my_temp_16=(temp_e-my_adjust_300_b)/(my_adjust_300_a*my_all_a_adjust)/(my_i_ratio_value*my_I_100A_Radio)/MY_VDD*4096;
//    else
//        my_temp_16=(temp_e-my_adjust_100_b)/(my_adjust_100_a*my_all_a_adjust)/(my_i_ratio_value*my_I_100A_Radio)/MY_VDD*4096;
		
		
		if(temp_e>10)
        my_temp_16=(temp_e-my_adjust_300_b)/(my_adjust_300_a*my_i_300a_radio)/(my_i_ratio_value*my_I_100A_Radio)/MY_VDD*4096; //��ѹֵ
    else
        my_temp_16=(temp_e-my_adjust_100_b)/(my_adjust_100_a*my_i_5a_radio)/(my_i_ratio_value*my_I_100A_Radio)/MY_VDD*4096;

    //my_dac1_channel2_data_e=my_temp_16;
		my_12v_int=(1.20+my_adjust_value_V)/3.3*4096;//������ѹ ��Ծ���ޣ�1.2V�����ϵ���
    my_dac1_channel2_data_e =  my_temp_16 +  my_12v_int;//������ѹ������̧����ֵ
    //my_dac1_channel2_data_e=62;//@@@

    HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_2, DAC_ALIGN_12B_R, my_dac1_channel2_data_e); //����������
#if Debug_Usart_out_DAC_normal_data==1
    printf("DAC2_Rise_i=[%.2f]_A,OUT_V=[%.2f]_V,my_adjust_value_V=%.2f ===going\n",temp_e,my_dac1_channel2_data_e/4096.0*3.3,my_adjust_value_V); //*=*=
    //printf("DAC2_Rise_i=[%.2f]_A,OUT_V=[%.2f]_V \n",temp_e,(my_temp_16*1.0/4096*MY_VDD)); //*=*=
#endif

	}
	else if(my_pin_status==1)
	{
		
		temp_i = ADC2_Filer_value_buf_2[0][1]; //���12�ܲ���������Чֵ,ת�����ֵ,2Ϊ���ֵ��1Ϊ��Чֵ
    temp_e=temp_i;

    //����糡DA ֵ
    temp_e=temp_e*0.04;
    temp_e=0;
    //temp_e=(temp_e+my_5A_ref_int);

    temp_e=temp_e*1.414; //���ֵ

    if(temp_e>10)
        my_temp_16=(temp_e-my_adjust_300_b)/(my_adjust_300_a*my_i_300a_radio)/(my_i_ratio_value*my_I_100A_Radio)/MY_VDD*4096;
    else
        my_temp_16=(temp_e-my_adjust_100_b)/(my_adjust_100_a*my_i_5a_radio)/(my_i_ratio_value*my_I_100A_Radio)/MY_VDD*4096;

    //my_dac1_channel2_data_e=my_temp_16;
		my_12v_int=(1.20+my_adjust_value_V+my_counst_value)/3.3*4096; //�ҵ������ٽ����޺����һ���㶨��ֵ���ޡ�
    my_dac1_channel2_data_e =  my_temp_16 +  my_12v_int;//����̧����ֵ
    //my_dac1_channel2_data_e=62;//@@@

    HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_2, DAC_ALIGN_12B_R, my_dac1_channel2_data_e); //����������
#if Debug_Usart_out_DAC_normal_data==1
    printf("DAC2_Rise_i=[%.2f]_A,OUT_V=[%.2f]_V,my_adjust_value_V=%.2f ===end\n",temp_e,my_dac1_channel2_data_e/4096.0*3.3,my_adjust_value_V); //*=*=
    //printf("DAC2_Rise_i=[%.2f]_A,OUT_V=[%.2f]_V \n",temp_e,(my_temp_16*1.0/4096*MY_VDD)); //*=*=
#endif
		
		
		my_DAC_cyc_time=17;
		//HAL_NVIC_EnableIRQ(EXIT_jiedi_EXTI_IRQn); //�ӵ��жϿ���
		
	}
	

	
}








