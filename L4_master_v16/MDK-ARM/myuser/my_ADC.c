#include "my_ADC.h"
#include "my_gloabal_val.h"



//extern uint16_t ADC2_GetValue[ADC2_ROW][ADC2_COLM]; //���ڱ���ɼ���ֵ,M��ͨ��,N�Σ�ԭʼ����
//extern double ADC2_Filer_value_buf_2[ADC2_COLM][3];  //ADת���������ֵ��12�����ڵ�ƽ��ֵ����Чֵ�����ֵ����1��Ϊȫ����������2��Ϊ�糡����3��Ϊ�벨����
//extern double ADC2_Filer_value_buf_3[ADC2_COLM][3];  //��������


extern uint16_t ADC1_GetValue[ADC1_ROW][ADC1_COLM]; //���ڱ���ɼ���ֵ,M��ͨ��,N��
extern uint16_t ADC1_GetValue_Aver[ADC1_COLM]; //���ڱ���ƽ��ֵ
extern double ADC1_Filer_value_buf[ADC1_COLM];  //�����洢ADC1�������ĵ�DCֱ�����ݣ���7��ͨ������ʵֵ��ת�����



__IO uint16_t VREFINT_CAL;
__IO uint16_t MY_TS_CAL1;
__IO uint16_t MY_TS_CAL2;

/*
���ܣ�����ADC1��6��ͨ��DMA����ֵ��ƽ��ֵ
*/
void filter_1(void)  //���ݼ��㣬������ƽ��ֵ,��ADC1�е�4·ֱ����ѹ���ݽ��м���ƽ��ֵ
{
    uint32_t  sum = 0;
    uint16_t	 i=0;
    uint8_t  count=0;
    for(i=0; i<ADC1_COLM; i++)

    {

        for (count=0; count<ADC1_ROW; count++)

        {

            sum += ADC1_GetValue[count][i];

        }

        ADC1_GetValue_Aver[i]=sum/ADC1_ROW;

        sum=0;
    }

}

/*
���ܣ�/��ADC1��6��ͨ������ƽ��ֵ��ת��������ֵ���浽������
*/
double MY_VDD=0.0;
void my_adc_1_convert(void)
{
    double x=0;
    filter_1(); //�����3������ֵ��ƽ��ֵ��ֻ��1������

    //��ƽ��ֵת���ɶ�Ӧ������ֵ
    MY_VDD=3.0*VREFINT_CAL/ADC1_GetValue_Aver[2];

    //ADC1_Filer_value_buf[0]=(ADC1_GetValue_Aver[0]-760)/2500.0 + 25;  //�¶�

    x=ADC1_GetValue_Aver[0]*MY_VDD/4096;
    ADC1_Filer_value_buf[0]=(x-0.76)*1000/2.5+30;


    //ADC1_Filer_value_buf[0]=(110.0-30.0)/(MY_TS_CAL2-MY_TS_CAL1)*(int16_t)(ADC1_GetValue_Aver[0]-MY_TS_CAL1)+30;


    ADC1_Filer_value_buf[1]=ADC1_GetValue_Aver[1]*MY_VDD/4096*3;//�����ѹ
    ADC1_Filer_value_buf[2]=MY_VDD; //�ο���ѹ
    ADC1_Filer_value_buf[3]=ADC1_GetValue_Aver[3]*MY_VDD/4096*(2); //�ɵ��
    ADC1_Filer_value_buf[4]=ADC1_GetValue_Aver[4]*MY_VDD/4096*(2); //���ϵ�ѹ
    ADC1_Filer_value_buf[5]=ADC1_GetValue_Aver[5]*MY_VDD/4096*(2); //̫����
    ADC1_Filer_value_buf[6]=ADC1_GetValue_Aver[6]*MY_VDD/4096*(2); //﮵��
		ADC1_Filer_value_buf[7]=ADC1_GetValue_Aver[7]*MY_VDD/4096; //1.2V

}



/*
���ܣ���ADC1��Ӧ��6��ͨ��������ֵ�������
*/
void my_adc_1_convert_dis(uint8_t convert_status)
{

    if(convert_status==1)  //���Ϊ1�������ADC������ת��
        my_adc_1_convert();
    //=============USART2==debug
    printf(" DC:GANbat=%.2f, Zaixian=% .2f, sunbat=%.2f, Libat=%.2f, V1V2=%.2f\n",ADC1_Filer_value_buf[3],ADC1_Filer_value_buf[4],ADC1_Filer_value_buf[5],ADC1_Filer_value_buf[6],ADC1_Filer_value_buf[7]);
    printf(" DC:Temp=%.2f,vbat=%.2f,vref=%.2f\n",ADC1_Filer_value_buf[0],ADC1_Filer_value_buf[1],ADC1_Filer_value_buf[2]);  
    printf("\r\n");
}








