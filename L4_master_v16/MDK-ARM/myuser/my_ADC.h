#include "adc.h"
#include "my_usart.h"


//#define MY_ADC_NUM 960 //ÿ������80����12��������Ҫ960������


void filter_1(void); //���ݼ��㣬������ƽ��ֵ
void my_adc_1_convert(void);
void my_adc_1_convert_dis(uint8_t); ////����Ϊ1����ʾ��ADC������ת������ʾ��Ϊ0ֻ��ʾ�������в�����ת��


