#include "stm32l4xx_hal.h"

//#define rsbuf_max 100
//#define rsbuf_min	100
#define rsbuf3_max    100//51K,  51*1024=52224  41K=41984
#define rsbuf2_max    200


#define ADC1_ROW 3 //N������ƽ��ֵ,��
#define ADC1_COLM 8  //M�ɼ�M��ͨ������

#define ADC2_ROW 3 //N������ƽ��ֵ,��
#define ADC2_COLM 3  //M�ɼ�M��ͨ������



#define WAVE_number 8000  //1������80���㣬12������960���㣬1s��50������80*50=4000���㣬2����8000����
#define WAVE_Channel_Num 3  //¼�����ŵ���������1��Ϊȫ����������2��Ϊȫ���糡����3��Ϊ�벨����

#define WAVE_number_sec2 960  //���������е�¼�����ݣ�960����Ϊ12�����ڵ����ݣ�12*80=960
#define WAVE_number_sec3 WAVE_number_sec2

//=======================
//��������
//********���ڲ���**************************

#define USART_DEBUG &huart2   //������Զ˿�


//===STM FLASH
//=========��������ʹ�õ�����==========
//CC1101
#define Debug_Usart_use_433data 0  //1����ת��CC1101�������ݣ�0���ڲ�ת��CC1101�ķ�������
#define Debug_Usart_out_CC1101_Get_cmd 0 //1��ʾCC1101�յ���101����
#define CC1101_Use 1       //����cc1101�������ݣ�1���ͣ�0������

//ADC2
#define Debug_Usart_out_chazhidata 0  //��ʾ¼�����ݵ�1�����ڣ������1�����ڵĵ�һ�����ݲ�ֵ
#define ADC_CYC_data 1 //CC1101 �������ڲ���
#define ADC_interrupt_data 1 //CC1101 �����жϲ���
//DAC
#define Debug_Usart_out_DAC_normal_data 1 //������ʾDAC��������ز���
#define DAC_auto_change_on 1 //DAC�Ĳο���ѹ�Զ��仯��1��ʾ������0�ر�

//OS_CC1101
#define Debug_Usart_out_ADCdata  0 //Ϊ1����ʾ��CC1101�ĶԻ������У������ʾADC�Ĳ�������
#define Debug_Usart_out_wavedata_960Data_2cach 0  //**2������**ת��ADC��¼������,1Ϊ����ȫ����2Ϊ�糡ȫ����3Ϊ�����벨��ǰ��������Debug_Usart_out_ADCdata==1
#define Debug_Usart_OUT_WAVE_12T_CYC  0  //2�����壬1��ʾ2��������¼����12T����Чֵ
#define Debug_Usart_OUT_WAVE_VALUE 0  //1�ж��У���ʾ12�����ڵĵ������糡��


//OS_CC1101_Test

#define OS_CC1101_auto_reveive_OK 0 //OS����ʹ�ã�CC1101�������ݺ�ģ���յ�OK֡
#define OS_CC1101_ZSQ_Monidata 1  //ָʾ������ģ�����ݸ�DTU
#define OS_heap_high_water_data 1 //OS���Դ�����ʾ��ÿ������ʣ��Ķ�ջ������@@@��1Ϊ��ʾ��0Ϊ����ʾ
#define USE_CC1101_PWR_ON_STATUS  1  //1��CC1101���磬0Ϊ������

//CC1101����¼������
//#define CC1101_960data_Efield_STATUS 0  //1 CC1101����¼���糡���ݣ�0Ϊ�����͵糡�����͵���
#define CC1101_SEND_I_E_Simulation_data_status 1  //CC1101���� �������糡ģ������



//¼��������ʾ
#define Debug_Usart_OUT_WAVE_12T_Interupt 1 // 3�����壬1Ϊ��ʾ������¼��ʱ��4+8=12���жϹ���¼����,12�����ڵ����ݵ���Чֵ
#define Debug_Usart_OUT_WAVE_960Data_Interupt 0 //3������,1λȫ����������¼�����ݣ�2Ϊ�糡��3Ϊ�벨����
#define Debug_usart_out_wave_cmpare_data 0  //3�������У�1��ʾ�����ڵ���β��ֵ

#define Debug_Usart_OUT_WAVE_Last_12T_Interupt 0 //��ʾ�����жϺ�ÿ��200ms¼����12�����ڵ�����
#define Debug_Usart_OUT_WAVE_End_Just_Interupt  1 //������յ��жϽ���������ݡ�


//��ʾ¼�����ݵ���λ��ֵ
#define Debug_Usart_OUT_WAVE_Chazhi 1 //��ʾ¼������  ��ĩ���ڣ��ĵ�һ�����ݵĲ�ֵ

//��ʾͣ��״̬
#define Debug_Usart_OUT_LINE_STOP_STATUS 1  //1Ϊ��ʾ��0Ϊ����ʾ

//��ʾ�糡��ʵʱ״̬
#define Debug_Usart_OUT_LINE_Efield_STATUS 1  //���ڣ�ͣ��״̬�ɼ���1Ϊ��ʾ��0Ϊ����ʾ




//��ʾDCֱ������
#define Debug_Usart_OUT_DC_DATA_status 1 //��ʾֱ������

#define CC1101_TX_Delay 0 //cc1101������ʱһ��

//���ں�DAC������������ʷ����
#define USE_olde_12T_data_cyc 1  //���ڣ�DAC���þɵ�12T���ݣ�����ٶ�
#define USE_olde_12T_data_DAC 1  //DAC��DAC���þɵ�12T���ݣ�����ٶ�

//LED�ƿ���
#define USE_LED14_STATUS 1  //1Ϊʹ��LED��ʾ��0Ϊ��ʹ��

//У���㷨�Ƿ�ʹ��
#define USE_Adjust_suanfa 2  //1Ϊʹ����С���˷�У���㷨��2Ϊ�ֶ�ϵ����С���˷�У������0λ��ʹ��У���㷨

//ADC�벨������ȥ��Ʈ�ķ���
#define USE_half_adjust_zero 1  //1Ϊʹ�ð벨ȥ��Ʈ�������벨������ֱ��ֵ����С0.1���ң�����Ϊ0
