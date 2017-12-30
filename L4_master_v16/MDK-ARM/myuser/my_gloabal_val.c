#include "stm32l4xx_hal.h"
#include "my_gloabal_val.h"




//����
uint8_t rsbuf2[rsbuf2_max];
uint8_t USART2_FRAME_status;
uint8_t USART2_my_frame[100];
uint16_t rsbuf2pt_write;
uint16_t rsbuf2pt_read;

uint16_t my_wave_record[WAVE_Channel_Num][WAVE_number]= {0}; //ȫ����һ�����棬���ö�ʱ�����в��εĴ洢���ٶ�20ms�ɼ�һ�Σ����ܲɼ������ٸ����ݡ�
uint16_t my_wave_write_add=0;  //ȫ��¼��һ������ָ��

uint16_t my_wave_record_sec2[WAVE_Channel_Num][WAVE_number_sec2]= {0}; //ȫ���Ķ�������
uint16_t my_wave_record_sec3[WAVE_Channel_Num][WAVE_number_sec3]= {0}; //ȫ����3������
uint8_t  my_wave_re_status=0;  //¼��״̬��0��ʾ�����ݣ�1��2,3,4,5��ʾ�������������ݣ�10��ʾ��������



uint16_t ADC2_GetValue[ADC2_ROW][ADC2_COLM]= {0}; //���ڱ���ɼ���ֵ,M��ͨ��,N�Σ�ԭʼ����
uint16_t ADC2_Filer_value_buf_1[ADC2_COLM][3]= {0}; //���ڱ���3��ͨ����12�����ڲ�����ƽ��ֵ,1��Ϊ������2��Ϊ�糡��3��Ϊ�벨
double ADC2_Filer_value_buf_2[ADC2_COLM][3]= {0};  //ADת���������ֵ��12�����ڵ�ƽ��ֵ����Чֵ�����ֵ����1��Ϊȫ����������2��Ϊ�糡����3��Ϊ�벨����
double ADC2_Filer_value_buf_3[ADC2_COLM][3]= {0};  //�������壬����˲��Ļ��ɺ������


uint16_t ADC1_GetValue[ADC1_ROW][ADC1_COLM]= {0}; //���ڱ���ɼ���ֵ,M��ͨ��,N��
uint16_t ADC1_GetValue_Aver[ADC1_COLM]= {0}; //���ڱ���ƽ��ֵ
double ADC1_Filer_value_buf[ADC1_COLM]= {0};  //�����洢ADC1�������ĵ�DCֱ�����ݣ���7��ͨ������ʵֵ��ת�����
//1�¶ȡ�2��Դ��3�ο���ѹ��4�ɵ�ء�5���ϵ�ѹ��6̫���ܡ�7﮵��,����С�������λ
//�ж�
uint16_t my_PA01_count=0;
uint16_t my_PA00_count=0;
uint16_t my_PC06_count=0;

uint16_t my_dianliu_exit_add=0; //�˱��������������жϻ��߶�ʱ����ʱ�̣���ѯ¼��1�����������õĵ�ַ
uint16_t my_Wave_It_add=0;  //��¼�ж�ʱ�̵�ַ����ͬ��¼��ʹ��

uint8_t  my_Current_exit_Status=0; //��ʾ���������ж�
//uint16_t my_Current_Exit_add=0;

uint8_t  my_E_Field_exit_Status=0; //��ʾ�糡�����ж�
uint16_t my_E_Field_exit_add=0;

uint8_t  my_Time_Cyc_exit_Status=0; //���ڶ�ʱ�ж�
uint16_t my_Time_Cyc_exit_add=0;

//
uint16_t my_send_3S_count=0;

uint8_t my_send_ALL_A_960_data_status=1;  //Ϊ1��ʾ��������¼�����ݣ�0��ʾ������
uint8_t my_send_ALL_E_960_data_status=0;
uint8_t my_send_HALF_A_960_data_status=0;

//����3״̬��ʶ
uint8_t my_usart3_error_status=0;  //1��ʶ����������

//===============
uint8_t  my_temp8_buf1[2000];

uint8_t my_CC1101_Sleep_status=1;  //CC1101��SLEEP��ʶ��Ϊ1��ʶCC1101˯��

//TIM6
uint16_t my_tim6_count=0;
uint16_t my_cyc_time_count=337;  //���ڷ�������ʱ�䣬��Ϊ��λ
uint16_t  my_cyc_delay=0;

uint8_t my_cc_Efied_count=0;  //��¼CC1101���ͺ�ֹͣ��ʱ�䣬Ȼ�����ӵ��ж�
uint16_t  my_CC1101_all_step = 0;

//CC1101
uint8_t my_DTU_status=1;  //1Ϊ���ߣ�0Ϊ�����ߡ�DTU����״̬���������10�ζ�������ϵ��DTU��DTU������Ϊ0��Ϊ1��ʾ���ߡ�
uint16_t my_DTU_send_faile_count=0;  //��DTU�������ݣ�ʧ�ܼ�����

//DAC
uint8_t my_DAC_cyc_time=1;  //DAC����ʱ�䣬Ĭ��Ϊ5

//CC1101��������
uint8_t my_zsq_ALarm_send_status=0;  //����״̬�Ƿ��ͳ�ȥ��1Ϊ���ͳ�ȥ��0Ϊû�з��ͳ�ȥ��

uint8_t my_cyc_alarm_status=0;   //���ڱ���״̬��1Ϊ���ڲ����ı�����0Ϊû�����ڱ���

