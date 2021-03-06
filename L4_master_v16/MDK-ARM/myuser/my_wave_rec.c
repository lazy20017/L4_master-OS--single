#include "gpio.h"
#include "my_wave_rec.h"
#include "my_ADC.h"
#include "my_usart.h"
#include "math.h"
#include "my_gloabal_val.h"
#include "cmsis_os.h"
#include "my_extern_val.h"

//======================

//====================
//判据用
volatile uint8_t my_Fault_Current_End_Status = 0x00; //
volatile uint8_t my_Fault_E_Fild_End_Status = 0x00; //

uint8_t my_befor_500ms_normal_count = 0; //中断前500ms停电状态记录
uint8_t my_befor_short_stop_count = 0; //中断前的状态，只进行一次采集，利用3级缓存计算
uint8_t my_short_circuit_count = 0; //短路状态，电流产生阶跃性上升,3级缓冲中
uint8_t my_short_circuit_count2 = 0; //短路状态，电流产生阶跃性上升，2级缓冲用

uint8_t my_after_short_stop_count1 = 0; //中断后，第一次停电状态记录
uint8_t my_after_short_stop_count2 = 0; //中断后，第一次停电后来电，第二次停电的次数记录

uint8_t my_after_stop1_normal_count = 0; //第一次停电后，来电状态记录

uint8_t my_E_fild_change_count = 0; //接地状态，电场产生阶跃性下降
uint8_t my_E_fild_min_count = 0;   //接地，电场最小值


uint8_t my_Line_Current_stop_status = 0; //停电状态标识，1为停电，2为接地,3为正常,
uint8_t my_Line_Current_stop_last_status = 0xff; //上一次的状态，利用这个变量，和最新的状态比较，停电了就上传,3为正常,1为停电，2为接地
uint16_t my_Line_Efild_valu = 0; //对地电场的值
uint16_t my_line_Current_value = 0; //线上电流的有效值，放到10倍取整数


uint16_t MY_Efile_Zero_data = 10; //电场小于此值，表示为0
uint16_t MY_Efile_floor_data = 60; //电场下限，小于此值，表示接地，默认40
float my_HA_Zero_data = 2; //半波电流判读最小值，小于这个值就认为停电了。0.2
double my_A_Zero_data = 0; //电流0值条件






#define E_cell 1.0        //电场对应的校正值,利用ADC采样得到的数据进行等比变化
uint16_t Current_D_value = 100; //电流上升突变阀值,判断条件，大于此值表示，产生短路电流,这个值是判断短路条件，ADC使用，默认150
uint16_t E_fild_D_value = 80; //电场下跌突变阀值
uint16_t E_fild_threshold_value = 20; //电场最小值，小于这个值就认为接的了
//========
double my_all_a_adjust = 0.76560196; //半波1.42655781;//全波0.798980075;    //1.42655781; // 0.798980075=301/368          301/387=0.7838541670; //实验数据，301/387获得的，在300A电流时刻，实际值，与ADC测量值的比较值
//295、395.7815=0.745360761
double my_adjust_300_a = 1.054171799;//1.009700101;//1.007328; //y=x*a+b,最小二乘法，的系数a，b，x为ADC测量值经过校正后的值，利用二乘法进行二次校正
double my_adjust_300_b = -12.56341994;//-1.587214894;//-2.42403;

double my_adjust_50_a = 0.986329196; //0.994672291; //1.00022;
double my_adjust_50_b = 0.686363636; //-0.144475163;// -2.19103;

double my_adjust_5_a = 1.059577357; //0.994672291; //1.00022;
double my_adjust_5_b = -0.272350023; //-0.144475163;// -2.19103;

double my_I_100A_Radio = 1.0; //标准线上电流为100A时的校正系数，算法，默认值为1，100A/测得数据的值
double my_i_ratio_value = 1000; //1/2.0*1500; //电流变比系数//采样电阻为2欧姆，变比为1500:1,最终用的1000
double my_value_daya = 2.0; //加法校正系数，默认2.0

//+++++++++++++++++++++++
//double my_i_5a_radio = 5.02 / 7.61; //5.08/6.26;//0.662251656;//0.857517365;   //5.08--6.26  303---393
//double my_i_50a_radio = 50.9 / 71.06;
//double my_i_300a_radio = 306.1 / 412.6; //293.4/377.354;//0.748091603;//0.770811922;

//double my_10A_gatedata = 15.14;
//double my_100A_gatedata = 139.92;

extern double my_i_5a_radio ; //5.08/6.26;//0.662251656;//0.857517365;   //5.08--6.26  303---393
extern double my_i_50a_radio ;
extern double my_i_300a_radio; //293.4/377.354;//0.748091603;//0.770811922;

extern double my_10A_gatedata ;
extern double my_100A_gatedata ;

//+++++++++++++++++++++++++=
//在乘上一个100A时得到的校正系数。

double my_E_ratio_value = 1000; //电场变比系数

//存12周期，每个周期采样值
uint16_t WAVE_half_ave_Current1[12][3] = {0}; //利用三列存，平均，有效，最大，12个周期的每个周期的值
uint16_t WAVE_all_ave_Current1[12][3] = {0}; //利用三列存，平均，有效，最大
uint16_t WAVE_all_ave_E_Field1[12][3] = {0};

//存转换后12周期的结果，物理量，线上真实电流值
double WAVE_half_ave_Current2[12][3] = {0}; //利用三列存，平均，有效，最大，12个周期的每个周期的值
double WAVE_all_ave_Current2[12][3] = {0}; //利用三列存，平均，有效，最大
double WAVE_all_ave_E_Field2[12][3] = {0}; //利用三列存，平均，有效，最大

//3级缓冲，线上真实电流值
double WAVE_half_ave_Current3[12][3] = {0}; //利用三列存，平均，有效，最大，12个周期的每个周期的值
double WAVE_all_ave_Current3[12][3] = {0}; //利用三列存，平均，有效，最大
double WAVE_all_ave_E_Field3[12][3] = {0}; //利用三列存，平均，有效，最大

//================
extern double MY_VDD;
extern uint16_t ADC2_GetValue[ADC2_ROW][ADC2_COLM];//用于保存采集的值,M个通道,N次,录波
extern double ADC2_Filer_value_buf_2[ADC2_COLM][3];
extern double ADC2_Filer_value_buf_3[ADC2_COLM][3];
extern uint16_t ADC1_GetValue[ADC1_ROW][ADC1_COLM];//用于保存采集的值,M个通道,N次，直流信号
extern uint8_t  my_Current_exit_Status; //表示电流产生中断
extern uint16_t my_dianliu_exit_add;
extern uint16_t my_wave_record[WAVE_Channel_Num][WAVE_number]; //全波的一级缓存，利用定时器进行波形的存储，假定20ms采集一次，看能采集到多少个数据。
extern uint16_t my_wave_write_add;  //全波录波一级缓存指针
extern uint16_t my_wave_record_sec2[WAVE_Channel_Num][WAVE_number_sec2]; //全波的二级缓存
extern uint16_t my_wave_record_sec3[WAVE_Channel_Num][WAVE_number_sec3]; //全波的二级缓存
extern uint8_t  my_wave_re_status;  //录波状态，0表示无数据，1，2,3,4,5表示二级缓存有数据，10表示发送数据
extern uint8_t my_IT_Count;
extern int8_t my_IT_status;
extern uint8_t  my_E_Field_exit_Status;
extern uint16_t my_Wave_It_add;
//extern uint16_t my_Current_Exit_add;
extern uint16_t my_E_Field_exit_add;
extern uint16_t ADC2_Filer_value_buf_1[][3];


extern osMessageQId myQueue01Handle;

extern double my_DAC_Line_I; //在DAC设置期间获得的电流值
extern double my_DAC_Line_Efild; //同上，电场值

uint16_t my_all_i_up_value = 0; //测得的全波抬升1.2v电压对应的采样平均值，ADC采样的值，未经过转换


#define rec_T_Count 12  //记录周期的数量
uint16_t my_E_fild_time_add = 0; //利用周期查询法时，my_E_fild_time_add存储查询的地址

extern uint8_t  my_Time_Cyc_exit_Status;



//====
/*
功能：此函数只是显示了1个周期的平均值的换算的有效值，显示一级缓存中0点开始的320个数据
调试使用，意义不大。
参数：xx为1的时候，表示利用取1个周期的方法，利用2表示利用取12个周期的方法
*/
//1表示存入数据到2级缓存
void my_adc2_convert_dis(uint8_t convet_status)
{

    if(convet_status == 1) //为1，则进行1级录波数据，存到2级缓存，然后计算二级缓存中的数据
        my_adc2_convert2(1);

    //有效值=1.1*平均值，最大值=平均值/0.637
    printf("1All_A:AVR=%.2f, RMS=%.2f, MAX=%.2f, \n", ADC2_Filer_value_buf_2[0][0], ADC2_Filer_value_buf_2[0][1], ADC2_Filer_value_buf_2[0][2]);
    printf("1ALL_E:AVR=%.2f, RMS=%.2f, MAX=%.2f, \n", ADC2_Filer_value_buf_2[1][0], ADC2_Filer_value_buf_2[1][1], ADC2_Filer_value_buf_2[1][2]);
    printf("1Hal_A:AVR=%.2f, RMS=%.2f, MAX=%.2f, \n", ADC2_Filer_value_buf_2[2][0], ADC2_Filer_value_buf_2[2][1], ADC2_Filer_value_buf_2[2][2]);

    //二级缓存录波数据输出，采样录波数据发送
//    printf("\n==========\n");
//    for(xj=0; xj<960; xj++) //串口发送录波的的数据，从0地址开始
//    {

//        //printf("%d\n",my_wave_record_sec2[0][xj]); //电流，全波
//        //printf("%d\n",my_wave_record_sec2[1][xj]); //电场，全波
//        //printf("%d\n",my_wave_record_sec2[2][xj]); //电流半波
//    }
//    printf("\n==========\n");


    //发送转换后的结果，录波数据发送
#if Debug_Usart_out_wavedata_960Data_2cach ==1  //全波电流
    int xj = 0;
    double temp = 0; //有符号数据
    printf("\n**************\n");
    for(xj = 0; xj < WAVE_number_sec2; xj++)
    {
        temp = (my_wave_record_sec2[0][xj] - my_all_i_up_value);
        temp = temp * MY_VDD / 4096 * (my_i_ratio_value * my_I_100A_Radio); //保留小数点后1位
        printf("%.1f\n", temp);
    }
    printf("\n**************\n");
#elif Debug_Usart_out_wavedata_960Data_2cach ==2  //全波电场
    int xj = 0;
    double temp = 0; //有符号数据
    printf("\n**************\n");
    for(xj = 0; xj < WAVE_number_sec2; xj++)
    {
        temp = (my_wave_record_sec2[1][xj]);
        temp = temp * MY_VDD / 4096 * my_E_ratio_value; //保留小数点后1位
        printf("%.1f\n", temp);
    }
    printf("\n**************\n");
#elif Debug_Usart_out_wavedata_960Data_2cach ==3 //半波电流
    int xj = 0;
    double temp = 0; //有符号数据
    printf("\n**************\n");
    for(xj = 0; xj < WAVE_number_sec2; xj++)
    {
        temp = (my_wave_record_sec2[2][xj] - my_all_i_up_value);
        temp = temp * MY_VDD / 4096 * (my_i_ratio_value * my_I_100A_Radio); //保留小数点后1位
        printf("%.1f\n", temp);
    }
    printf("\n**************\n");
#endif


}





//此部分为自定义函数
/*
功能：利用半波计算电流的有效值 12个周期，的每个周期AVR RMS MAX
*/




void fun_real_half_Current(void)
{
    uint32_t sum = 0, sum_pwr = 0, count = 0;
    int xi = 0, xj = 0, max = 0;
    for(xi = 0; xi < 12; xi++)
    {   sum = 0;
        max = 0;
        count = 0;
        sum_pwr = 0;
        //为计算80个点的平均值
        for(xj = 0; xj < 80; xj++)
        {
            if(my_wave_record_sec2[2][xi * 80 + xj] > 0)
            {
                count++;
                sum = sum + my_wave_record_sec2[2][xi * 80 + xj]; //为计算80个点的和
                sum_pwr = sum_pwr + my_wave_record_sec2[2][xi * 80 + xj] * my_wave_record_sec2[2][xi * 80 + xj]; //计算80个点的平方和
                if(my_wave_record_sec2[2][xi * 80 + xj] > max)
                    max = my_wave_record_sec2[2][xi * 80 + xj];
            }

            //@@@@@@测试使用
            //printf("\n %d",my_wave_record_sec2[2][xi * 80 + xj]);

        }
        //2017-04-18

        //测量测量值
        WAVE_half_ave_Current1[xi][0] = sum*1.0 / count;
        WAVE_half_ave_Current1[xi][1] = sqrt(sum_pwr * 1.0 / count);
        WAVE_half_ave_Current1[xi][2] = max;
        //printf("\n @@max=%d",max);

        //转换值
        WAVE_half_ave_Current2[xi][0] = sum * MY_VDD / 4096 / count; //平均值
        WAVE_half_ave_Current2[xi][1] = sqrt(sum_pwr * 1.0 / count) * MY_VDD / 4096; //利用均方根法计算有效值
        WAVE_half_ave_Current2[xi][2] = max * MY_VDD / 4096;
#if USE_half_adjust_zero==1
        if(WAVE_half_ave_Current1[xi][0] == WAVE_half_ave_Current1[xi][2] && WAVE_half_ave_Current1[xi][2] != 0)
        {
            //测量测量值
            WAVE_half_ave_Current1[xi][0] = 0;
            WAVE_half_ave_Current1[xi][1] = 0;
            WAVE_half_ave_Current1[xi][2] = 0;
            //printf("\n @@max=%d",max);
            //转换值
            WAVE_half_ave_Current2[xi][0] = 0; //平均值
            WAVE_half_ave_Current2[xi][1] = 0; //利用均方根法计算有效值
            WAVE_half_ave_Current2[xi][2] = 0;
        }
#endif
        //调试使用
        //printf("half no zero count=%d\n",count);
    }

}
/*
功能：利用利用二级缓存全波，计算12个周期的电流，  每个周期平均值，有效值，最大值
*/

//
double temp_double = 0;
void fun_real_all_Current(void)
{
    uint32_t sum = 0, sum_pwr = 0, avr = 0, sum_avr_up = 0;

    int temp = 0;
    int xi = 0, xj = 0, max = 0;
    for(xi = 0; xi < 12; xi++) //12个周期
    {   sum = 0;
        max = 0;
        sum_pwr = 0;

        for(xj = 0; xj < 80; xj++) //为计算80个点的平均值
        {
            sum = sum + my_wave_record_sec2[0][xi * 80 + xj]; //为计算80个点的和

        }
        //
        avr = sum / 80;
        sum_avr_up = sum_avr_up + avr;
        sum_pwr = 0;
        sum = 0;
        max = 0;

        //2017-04-16 删除抬升电压1.2v
        for(xj = 0; xj < 80; xj++)
        {

            temp = avr - my_wave_record_sec2[0][xi * 80 + xj];
            //temp=my_wave_record_sec2[0][xi*80+xj];
            if(temp < 0)
                temp = -temp;
            //有效值
            sum_pwr = sum_pwr + temp * temp;

            //平均值
            sum = sum + temp;
            //最大值
            if(temp > max)
                max = temp;

        }
        //采样值
        WAVE_all_ave_Current1[xi][0] = sum / 80.0; //平均值
        WAVE_all_ave_Current1[xi][1] = sqrt(sum_pwr * 1.0 / 80.0); //利用均方根法计算有效值,先矫正，后最小二乘法拟合
        WAVE_all_ave_Current1[xi][2] = max;



        //转换后的值
        //平均值
        WAVE_all_ave_Current2[xi][0] = sum * MY_VDD / 4096 / 80 * (my_i_ratio_value * my_I_100A_Radio);

        //有效值
        temp_double = sqrt(sum_pwr * 1.0 / 80) * MY_VDD / 4096 * (my_i_ratio_value * my_I_100A_Radio);


#if USE_Adjust_suanfa==1

//				if(temp_double > 100) //最小二乘法拟合
//            WAVE_all_ave_Current2[xi][1] = temp_double * my_all_a_adjust * my_adjust_300_a + my_adjust_300_b; //利用均方根法计算有效值,先矫正，后最小二乘法拟合
//        else
//            WAVE_all_ave_Current2[xi][1] = temp_double * my_all_a_adjust * my_adjust_100_a + my_adjust_100_b;
//        //WAVE_all_ave_Current2[xi][1] = temp_double * my_all_a_adjust * my_adjust_300_a + my_adjust_300_b;
//        //有效值负值校正
//        if(WAVE_all_ave_Current2[xi][1] <50)
//            WAVE_all_ave_Current2[xi][1] =WAVE_all_ave_Current2[xi][1] +my_value_daya; //绝对校正系统

#elif USE_Adjust_suanfa==2
        if(temp_double > my_100A_gatedata) //系数校正法
            if(temp_double > 138 && temp_double < 230)
                WAVE_all_ave_Current2[xi][1] = temp_double * my_i_300a_radio ;
            else
                WAVE_all_ave_Current2[xi][1] = temp_double * my_i_300a_radio * my_adjust_300_a + my_adjust_300_b;
        else if(temp_double > my_10A_gatedata && temp_double <= my_100A_gatedata )
            WAVE_all_ave_Current2[xi][1] = temp_double * my_i_50a_radio * my_adjust_50_a + my_adjust_50_b;
        else
            WAVE_all_ave_Current2[xi][1] = temp_double * my_i_5a_radio * my_adjust_5_a + my_adjust_5_b;

#else

        WAVE_all_ave_Current2[xi][1] = temp_double;
#endif


        //最大值
        WAVE_all_ave_Current2[xi][2] = max * MY_VDD / 4096 * (my_i_ratio_value * my_I_100A_Radio);
    }

    my_all_i_up_value = sum_avr_up / 12; //ADC采样的值，未经过转换


}
//利用二级缓存全波，计算12个周期，每个周期的评价，有效值，最大值

void fun_real_all_dianchang(void)
{
    uint32_t sum = 0, sum_pwr = 0, count = 0;
    int xi = 0, xj = 0, max = 0;
    for(xi = 0; xi < 12; xi++)
    {   sum = 0;
        max = 0;
        count = 0;
        sum_pwr = 0;
        for(xj = 0; xj < 80; xj++)
        {
            if(my_wave_record_sec2[1][xi * 80 + xj] > 0)
            {
                count++;
                sum = sum + my_wave_record_sec2[1][xi * 80 + xj];
                sum_pwr = sum_pwr + my_wave_record_sec2[1][xi * 80 + xj] * my_wave_record_sec2[1][xi * 80 + xj];
                if(my_wave_record_sec2[1][xi * 80 + xj] > max)
                    max = my_wave_record_sec2[1][xi * 80 + xj];

            }

        }
        //采样值
				count=80;
        WAVE_all_ave_E_Field1[xi][0] = sum*1.0 / count;
        WAVE_all_ave_E_Field1[xi][1] = sum *1.0/ count;//sqrt(sum_pwr * 1.0 / 80); //均方根法
        WAVE_all_ave_E_Field1[xi][2] = max;

        //转换值
        WAVE_all_ave_E_Field2[xi][0] = sum * MY_VDD / 4096 / count * my_E_ratio_value;

        //WAVE_all_ave_E_Field2[xi][1] = sqrt(sum_pwr * 1.0 / count) * MY_VDD / 4096 * my_E_ratio_value; //电场，均方根法
        WAVE_all_ave_E_Field2[xi][1] = WAVE_all_ave_E_Field2[xi][0] ; //电场，平均值方法
        WAVE_all_ave_E_Field2[xi][2] = max * MY_VDD / 4096 * my_E_ratio_value;
				//零飘正定
				if( WAVE_all_ave_E_Field1[xi][0]==WAVE_all_ave_E_Field1[xi][2] && WAVE_all_ave_E_Field1[xi][0]!=0)
				{
					//采样值

        WAVE_all_ave_E_Field1[xi][0] = 0;
        WAVE_all_ave_E_Field1[xi][1] = 0;
        WAVE_all_ave_E_Field1[xi][2] = 0;

        //转换值
        WAVE_all_ave_E_Field2[xi][0] = 0;     
        WAVE_all_ave_E_Field2[xi][1] = 0 ; //电场，平均值方法
        WAVE_all_ave_E_Field2[xi][2] = 0;
					
				}
    }

}

/*
功能：录波数据，从一级缓存中取12个周期的数据，存入二级缓存中
返回值，如果取得了12个周期的录波数据，就返回1,如果没有取得就返回0


*/
uint16_t  number_before_pp = 320; //录波数据，中断时刻前的点数，320个点，4个周期，就是4*80=320

int fun_my_wave1_to_wave2(void)
{
    int xi = 0, xj = 0;
    volatile	int xk = 0;
    //int ii=0,temp=0;
    uint16_t my_wav2_add = 0;
    uint8_t  my_re = 0;
    int my_stop_add1 = 0;
    int my_stop_add2 = 0;


    //第1种情况，录波数据包含在写地址前，最简单情况
    if((my_wave_write_add > my_dianliu_exit_add) && (my_wave_write_add - my_dianliu_exit_add) >= (WAVE_number_sec2 - number_before_pp))
    {   my_wave_re_status = 1; //进入存数据到二级缓存
        my_wav2_add = my_dianliu_exit_add;
        if(my_wav2_add >= number_before_pp) //1.1前320个点在中断点前
        {
            for(xi = 0; xi < WAVE_number_sec2; xi++)
            {
                my_wave_record_sec2[0][xi] = my_wave_record[0][my_wav2_add - number_before_pp + xi];
                my_wave_record_sec2[1][xi] = my_wave_record[1][my_wav2_add - number_before_pp + xi];
                my_wave_record_sec2[2][xi] = my_wave_record[2][my_wav2_add - number_before_pp + xi];
                //my_wav2_add++;
                xk = 1;
            }
        }
        else //1.2前320点，有一部分没在中断点前
        {
            for(xi = 0; xi < number_before_pp - my_wav2_add; xi++) //1.2.1取中断点后的320个数据中部分，数组的末尾
            {
                my_wave_record_sec2[0][xi] = my_wave_record[0][WAVE_number - (number_before_pp - my_wav2_add) + xi];
                my_wave_record_sec2[1][xi] = my_wave_record[1][WAVE_number - (number_before_pp - my_wav2_add) + xi];
                my_wave_record_sec2[2][xi] = my_wave_record[2][WAVE_number - (number_before_pp - my_wav2_add) + xi];
                xk = 2;
            }
            for(xj = 0; xi < WAVE_number_sec2; xi++, xj++) //1.2.2取中断点前的320个数据中的部分，数组的头
            {
                my_wave_record_sec2[0][xi] = my_wave_record[0][xj];
                my_wave_record_sec2[1][xi] = my_wave_record[1][xj];
                my_wave_record_sec2[2][xi] = my_wave_record[2][xj];
                xk = 3;
            }

        }
        //@@@，测试使用，计算录波数据第1个数据和最后一个周期的第一个数据的差值
        //temp=(my_wave_record_sec2[0][0]-my_wave_record_sec2[0][881]);



#if Debug_Usart_out_chazhidata==1
        if(xk == 1 || xk == 2 || xk == 3) //XK1表示，录波数据在写地址前，3表示录波数据在写地址前，但是320个数据部分在数组末尾
            printf("==XK%d-- error chazhi=%d\n", xk, my_wave_record_sec2[0][0] - my_wave_record_sec2[0][881]);
#endif

        //串口发送录波数据，二级缓存中的数据，没有经过处理的
//					if(temp>100 ||temp<=-100)
//				{   printf("====start================\n");
//					  for(ii=0;ii<960;ii++)
//					   	printf("%d\n",my_wave_record_sec2[0][ii]);
//					printf("====end=================\n");
//				}

        my_re = 1;


    }
    //2录波数据部分在写地址前，数组末尾还有部分数据
    else if((my_wave_write_add < my_dianliu_exit_add) && ((WAVE_number - my_dianliu_exit_add + my_wave_write_add) >= (WAVE_number_sec2 - number_before_pp)))
    {   my_wave_re_status = 1; //进入存数据到二级缓存
        my_wav2_add = my_dianliu_exit_add;

        for(xi = 0; (xi < WAVE_number - my_dianliu_exit_add + number_before_pp) && (xi < WAVE_number_sec2); xi++)
        {
            my_wave_record_sec2[0][xi] = my_wave_record[0][my_dianliu_exit_add - number_before_pp + xi];
            my_wave_record_sec2[1][xi] = my_wave_record[1][my_dianliu_exit_add - number_before_pp + xi];
            my_wave_record_sec2[2][xi] = my_wave_record[2][my_dianliu_exit_add - number_before_pp + xi];
            xk = 4;
        }
        for(xj = 0; xi < WAVE_number_sec2; xi++, xj++)
        {
            my_wave_record_sec2[0][xi] = my_wave_record[0][xj];
            my_wave_record_sec2[1][xi] = my_wave_record[1][xj];
            my_wave_record_sec2[2][xi] = my_wave_record[2][xj];
            xk = 5;
        }
#if Debug_Usart_out_chazhidata==1
        if(xk == 4 || xk == 5) //5代表录波数据部分在数组末尾
            printf("==XK%d-- error chazhi=%d\n", xk, my_wave_record_sec2[0][0] - my_wave_record_sec2[0][881]);
#endif
        my_re = 1;

    }
    else
    {
        my_re = 0;
    }
    //计算前500ms周期前，是否停电
    if(my_re == 1 && my_IT_status == 0)
    {
        my_befor_500ms_normal_count = 0;
        my_stop_add1 = my_dianliu_exit_add - 2400;
        my_stop_add2 = my_dianliu_exit_add - 2480;
        if(my_stop_add1 >= 0 && my_stop_add2 >= 0)
        {
            for(xi = my_stop_add2; xi <= my_stop_add1; xi++)
            {
                if(my_wave_record[2][xi] >my_HA_Zero_data)
                    my_befor_500ms_normal_count++;
            }
        }
        else if(my_stop_add1 >= 0 && my_stop_add2 < 0)
        {
            my_stop_add2 = WAVE_number + my_stop_add2;
            for(xi = my_stop_add2; xi < WAVE_number; xi++)
            {
                if(my_wave_record[2][xi] >my_HA_Zero_data)
                    my_befor_500ms_normal_count++;
            }
            for(xi = 0; xi <= my_stop_add1; xi++)
            {
                if(my_wave_record[2][xi] >my_HA_Zero_data)
                    my_befor_500ms_normal_count++;

            }

        }
        else if(my_stop_add1 < 0 && my_stop_add2 < 0)
        {
            my_stop_add2 = WAVE_number + my_stop_add2;
            my_stop_add1 = WAVE_number + my_stop_add1;
            for(xi = my_stop_add2; xi <= my_stop_add1; xi++)
            {
                if(my_wave_record[2][xi] >my_HA_Zero_data)
                    my_befor_500ms_normal_count++;
            }

        }
    }

//----测试使用
#if				CC1101_SEND_E_Simulation_data_status==1
    int ii = 0, my_temp = 0;
    int16_t xx = 0;
    for(ii = 0; ii < 960; ii++)
    {
        my_temp = ii % 80;
        xx = (int16_t)(sinf((float)((my_temp / 79.0 * 2 * 3.1415926) + 1)) / 2.0 * 4096);
        my_wave_record_sec2[1][ii] = xx;

    }


#endif


    return my_re;
}

/*
测试使用：
功能：把二次缓存中的数据，12个周期计算的平均值，有效值，最大值输出到串口，

*/
void funT_display_wave2(void)
{
    uint8_t xi = 0;
    printf("================\n");
    for(xi = 0; xi < 12; xi++)
        printf("half_A: AVR=%f,RMP=%f,MAX=%f\n", WAVE_half_ave_Current2[xi][0], WAVE_half_ave_Current2[xi][1], WAVE_half_ave_Current2[xi][2]);
    printf("-------------\n");
    for(xi = 0; xi < 12; xi++)
        printf("all__A: AVR=%f,RMP=%f,MAX=%f\n", WAVE_all_ave_Current2[xi][0], WAVE_all_ave_Current2[xi][1], WAVE_all_ave_Current2[xi][2]);
    printf("-------------\n");
    for(xi = 0; xi < 12; xi++)
        printf("all__E: AVR=%f,RMP=%f,MAX=%f\n", WAVE_all_ave_E_Field2[xi][0], WAVE_all_ave_E_Field2[xi][1], WAVE_all_ave_E_Field2[xi][2]);
    printf("================\n");

    //发送给433模块
    for(xi = 0; xi < 12; xi++)
    {
        USART_printf1(&huart2, "half_A: AVR=%f, ", WAVE_half_ave_Current2[xi][0]);
        USART_printf1(&huart2, "RMP=%f, ", WAVE_half_ave_Current2[xi][1]);
        USART_printf1(&huart2, "MAX=%f \n", WAVE_half_ave_Current2[xi][2]);
    }

    for(xi = 0; xi < 12; xi++)
    {
        USART_printf1(&huart2, "all__A: AVR=%f, ", WAVE_all_ave_Current2[xi][0]);
        USART_printf1(&huart2, "RMP=%f, ", WAVE_all_ave_Current2[xi][1]);
        USART_printf1(&huart2, "MAX=%f \n", WAVE_all_ave_Current2[xi][2]);
    }

    for(xi = 0; xi < 12; xi++)
    {
        USART_printf1(&huart2, "all__E: AVR=%f, ", WAVE_all_ave_E_Field2[xi][0]);
        USART_printf1(&huart2, "RMP=%f, ", WAVE_all_ave_E_Field2[xi][1]);
        USART_printf1(&huart2, "MAX=%f \n", WAVE_all_ave_E_Field2[xi][2]);
    }



}



/*
功能：
当前时刻，1级缓冲数据放入12个周期到2级缓冲区
利用二级缓存中数据，12个周期，计算出每个周期平均值，有效值，最大值,放入到WAVE_all_ave_Current2数组中
利用12个每个周期的计算后的值，计算12个周期的评价值（平均值，有效值，最大值），放到ADC2_Filer_value_buf_2数组中

输出：WAVE_all_ave_Current2数组，12个周期每个周期的值（3个量，平均值，有效值，最大值）
ADC2_Filer_value_buf_2数组，每个通道12个周期的平均值，利用上面的数组计算的，3个量（平均值，有效值，最大值）

参数说明：1表示，内部启用录波，1级缓冲到2级缓存，0表示不用，外部已经进行了2级缓冲
*/


void my_adc2_convert2(uint8_t my_status)
{

    double temp1 = 0, temp2 = 0, temp3 = 0,max3=0;
    uint16_t temp11 = 0, temp12 = 0, temp13 = 0,max13=0;
    int xi = 0;

    if(my_status == 1)
    {
        my_fun_wave_rec();
    }

    fun_real_half_Current();  //计算二级缓存中12个周期的，每个周期的3个数据，共计3*12*3=108
    fun_real_all_Current();
    fun_real_all_dianchang();

    //计算12个周期的累计平均值
    //===全波电流
    temp1 = temp2 = temp3 =max3= 0;
    temp11 = temp12 = temp13 =max13= 0;
    for(xi = 0; xi < 12; xi++) //12个周期，每个周期的值
    {
        //采样值
        temp11 = temp11 + WAVE_all_ave_Current1[xi][0];
        temp12 = temp12 + WAVE_all_ave_Current1[xi][1];
        temp13 = temp13 + WAVE_all_ave_Current1[xi][2];
				if(WAVE_all_ave_Current1[xi][1]>max13) //计算有效值的最大值
					max13=WAVE_all_ave_Current1[xi][1];

        //转换值
        temp1 = temp1 + WAVE_all_ave_Current2[xi][0]; //平均值累计求和
        temp2 = temp2 + WAVE_all_ave_Current2[xi][1];
        temp3 = temp3 + WAVE_all_ave_Current2[xi][2];
				if( WAVE_all_ave_Current2[xi][1]>max3)  //计算有效值的最大值
					max3= WAVE_all_ave_Current2[xi][1];
    }

    //得到12周期平均后AC的一个结果，
    //测量值
    ADC2_Filer_value_buf_1[0][0] = temp11 / 12; //平均
    ADC2_Filer_value_buf_1[0][1] = temp12 / 12; //有效值
    //ADC2_Filer_value_buf_1[0][2] = temp13 / 12; //最大值
		ADC2_Filer_value_buf_1[0][2] = max13; //最大值（有效值）

    //转换值
    ADC2_Filer_value_buf_2[0][0] = temp1 / 12; //平均
    ADC2_Filer_value_buf_2[0][1] = temp2 / 12; //有效值
    //ADC2_Filer_value_buf_2[0][2] = temp3 / 12; //最大值
		ADC2_Filer_value_buf_2[0][2] = max3; //最大值（有效值）


    //====电场
    temp1 = temp2 = temp3 =max3= 0;
    temp11 = temp12 = temp13 =max13=0;
    for(xi = 0; xi < 12; xi++)
    {
        //采样值
        temp11 = temp11 + WAVE_all_ave_E_Field1[xi][0];
        temp12 = temp12 + WAVE_all_ave_E_Field1[xi][1];
        temp13 = temp13 + WAVE_all_ave_E_Field1[xi][2];
				if(WAVE_all_ave_E_Field1[xi][1]>max13)  //有效值的最大值
					max13=WAVE_all_ave_E_Field1[xi][1];

        temp1 = temp1 + WAVE_all_ave_E_Field2[xi][0]; //平均值累计求和
        temp2 = temp2 + WAVE_all_ave_E_Field2[xi][1];
        temp3 = temp3 + WAVE_all_ave_E_Field2[xi][2];
				if( WAVE_all_ave_E_Field2[xi][1]>max3)  //有效值的最大值
					max3= WAVE_all_ave_E_Field2[xi][1];
    }

    //测量值
    ADC2_Filer_value_buf_1[1][0] = temp11 / 12; //平均
    ADC2_Filer_value_buf_1[1][1] = temp12 / 12; //有效值
    //ADC2_Filer_value_buf_1[1][2] = temp13 / 12; //最大值
		ADC2_Filer_value_buf_1[1][2] = max13; //最大值

    //转换值
    ADC2_Filer_value_buf_2[1][0] = temp1 / 12; //平均
    ADC2_Filer_value_buf_2[1][1] = temp2 / 12; //有效值
    //ADC2_Filer_value_buf_2[1][2] = temp3 / 12; //最大值
		ADC2_Filer_value_buf_2[1][2] = max3; //最大值
    //====半波电流
    temp1 = temp2 = temp3 =max3= 0;
    temp11 = temp12 = temp13 =max13= 0;
    for(xi = 0; xi < 12; xi++)
    {
        //采样值
        temp11 = temp11 + WAVE_half_ave_Current1[xi][0];
        temp12 = temp12 + WAVE_half_ave_Current1[xi][1];
        temp13 = temp13 + WAVE_half_ave_Current1[xi][2];
				if(WAVE_half_ave_Current1[xi][2]>max13)
					max13=WAVE_half_ave_Current1[xi][2];

        temp1 = temp1 + WAVE_half_ave_Current2[xi][0]; //平均值累计求和
        temp2 = temp2 + WAVE_half_ave_Current2[xi][1];
        temp3 = temp3 + WAVE_half_ave_Current2[xi][2];
				if( WAVE_half_ave_Current2[xi][2]>max3)
					max3= WAVE_half_ave_Current2[xi][2];
    }
    //测量值
    ADC2_Filer_value_buf_1[2][0] = temp11 / 12; //平均
    ADC2_Filer_value_buf_1[2][1] = temp12 / 12; //有效值
    //ADC2_Filer_value_buf_1[2][2] = temp13 / 12; //最大值
		ADC2_Filer_value_buf_1[2][2] = max13; //最大值


    ADC2_Filer_value_buf_2[2][0] = temp1 / 12; //平均
    ADC2_Filer_value_buf_2[2][1] = temp2 / 12; //有效值
    //ADC2_Filer_value_buf_2[2][2] = temp3 / 12; //最大值
		ADC2_Filer_value_buf_2[2][2] = max3; //最大值


//===
#if Debug_Usart_OUT_WAVE_12T_CYC==1
    //if(my_Current_exit_Status==1 || my_E_Field_exit_Status==1 || my_Time_Cyc_exit_Status==1 )
    if(my_Current_exit_Status == 1 || my_E_Field_exit_Status == 1  )
        return;



    printf("***cache2 WAVE0 12T DATA --ALL_Current2****\r\n");
    int ii = 0;
    for(ii = 0; ii < 12; ii++)
        printf("%.2f MAX=%.2f\r\n", WAVE_all_ave_Current2[ii][1], WAVE_all_ave_Current2[ii][2]); //12T,全波电流

    printf("**cache2 *WAVE0 12T DATA --Half_Current2-MAX****\r\n");
    for(ii = 0; ii < 12; ii++)
    {
				 printf("%.2f MAX=%.2f\r\n", WAVE_half_ave_Current2[ii][1], WAVE_half_ave_Current2[ii][2]); //12T,全波电流
        //printf("%d\r\n", WAVE_half_ave_Current1[ii][2]); //12T,全波电流
    }
    printf("**cache2 *WAVE0 12T DATA --E_fild2-MAX****\r\n");
    for(ii = 0; ii < 12; ii++)
    {
				 printf("%.2f MAX=%.2f\r\n", WAVE_all_ave_E_Field2[ii][1], WAVE_all_ave_E_Field2[ii][2]); //12T,全波电流
        //printf("%d\r\n", WAVE_half_ave_Current1[ii][1]); //12T,电池
    }

    printf("**cache2*WAVE0 12T DATA --END****\r\n");
#endif
#if Debug_Usart_OUT_WAVE_VALUE==1
    printf("\n ==my_cc1101_all_step=[%x]\n", my_CC1101_all_step);
    my_adc2_convert_dis(0);
#endif


}





/*
功能：把2级缓存中的录波数据放入到3级缓存中，同时把2级缓存计算平均值放入到3级中
*/
void fun_wave2_to_wave3(void)
{
    uint16_t ii = 0, jj = 0;

    //12个周期的960个点
    for(ii = 0; ii < WAVE_number_sec2; ii++)
    {
        for(jj = 0; jj < 3; jj++)
            my_wave_record_sec3[jj][ii] = my_wave_record_sec2[jj][ii];

    }
    //12周期每个周的 平均、有效、最大
    for(ii = 0; ii < 12; ii++)
    {
        for(jj = 0; jj < 3; jj++)
        {
            WAVE_half_ave_Current3[ii][jj] = WAVE_half_ave_Current2[ii][jj];
            WAVE_all_ave_Current3[ii][jj] = WAVE_all_ave_Current2[ii][jj];
            WAVE_all_ave_E_Field3[ii][jj] = WAVE_all_ave_E_Field2[ii][jj];

        }
    }
    //12个周期3个通道，各自累计平均值
    for(ii = 0; ii < 3; ii++)
    {
        for(jj = 0; jj < 3; jj++)
        {
            ADC2_Filer_value_buf_3[jj][ii] = ADC2_Filer_value_buf_2[jj][ii];  //


        }

    }


}



//================
/*

功能：故障判断函数，判断短路、接地
返回值：0为正常，0X01为短路，0x10为接地
高四位为接地,最低位为1表示接地故障，高3位代表不同的接地类型，
低四位为短路，最低位为1表示短路故障，高3位代表不同的短路类型

*/


//////////////============================
double my_all_current_aver0 = 0;
uint8_t fun_Judege_It_cache3(void)
{
    //double my_all_current_aver0 = 0;

    double my_all_current_aver1[4] = {0};
    double my_all_current_aver2[8] = {0};

    double my_half_current_aver1[4] = {0}; //故障前4个半波
    double my_half_current_aver2[8] = {0}; //半波ADC采样后转换后得到的电流值
    double my_E_fild_aver0 = 0;
    double my_E_fild_aver1[4] = {0};
    double my_E_fild_aver2[8] = {0};
    double my_cmpar_value[3][12] = {0}; //0行全波电流，1行电场，2行半波电流

    uint16_t ii = 0, jj = 0;
    //全波对应有效电流值
    //my_all_current_aver0 = WAVE_all_ave_Current3[0][1]; //故障前，第一个全波的值
		my_all_current_aver0 = 0;
    my_all_current_aver0 = my_DAC_Line_I;

    //中断前电场的平均值
    //my_E_fild_aver0 = (WAVE_all_ave_E_Field3[0][1] + WAVE_all_ave_E_Field3[1][1] + WAVE_all_ave_E_Field3[2][1] + WAVE_all_ave_E_Field3[3][1]) / 4.0 / E_cell;
    my_E_fild_aver0 = my_DAC_Line_Efild;


    for(ii = 0; ii < 4; ii++)
    {
        my_half_current_aver1[ii] = WAVE_half_ave_Current3[ii][1]; //故障前4个周期
        my_all_current_aver1[ii] = WAVE_all_ave_Current3[ii][1];
        my_E_fild_aver1[ii] == WAVE_all_ave_Current3[ii][1];
    }



    for(ii = 0; ii < 8; ii++)
    {

        my_all_current_aver2[ii] = WAVE_all_ave_Current3[4 + ii][1]; //求出后8个周期对应的有效电流值
        my_half_current_aver2[ii] = WAVE_half_ave_Current3[4 + ii][1];
        my_E_fild_aver2[ii] = WAVE_all_ave_E_Field3[4 + ii][1] / E_cell; //中断后8个周期的每个周期的电场值
    }


    //==========================




    //中断后8个周期，对中断前的值的差值计算
    for(jj = 0; jj < 8; jj++)
    {
        my_cmpar_value[0][jj] = (my_all_current_aver2[jj] - my_all_current_aver0); //后8个周期的每个周期的电流值，减去故障前的平均值
        if(my_cmpar_value[0][jj] < 0) my_cmpar_value[0][jj] = 0;

        //my_cmpar_value[1][jj] = (my_E_fild_aver2[jj] - my_E_fild_aver0); //后8个周期，减去前4个周期的平均值，电场值
        my_cmpar_value[1][jj] = fabs(my_E_fild_aver0 - my_E_fild_aver2[jj]);


#if Debug_usart_out_wave_cmpare_data==1
        printf("cmpar=%.2f\r\n", my_cmpar_value[0][jj]); //电流差值
#endif
    }

    //电流和电场变化次数记录
    my_short_circuit_count = 0; //短路电流大于门限，次数
		my_short_circuit_count2=0;
    my_after_short_stop_count1 = 0;
    my_after_stop1_normal_count = 0;
    my_after_short_stop_count2 = 0;
    my_E_fild_change_count = 0;
    my_E_fild_min_count = 0;

    //开始判断
    for(ii = 0; ii < 4; ii++)
    {
        if((my_all_current_aver1[ii] - my_all_current_aver0) >= Current_D_value)
            my_short_circuit_count++;

        if(fabs(my_E_fild_aver0 - my_E_fild_aver1[ii]) >= E_fild_D_value)
            my_E_fild_change_count++;
    }

    //中断后的判断

    for(ii = 0; ii < 8; ii++)
    {
        if(my_cmpar_value[0][ii] >= Current_D_value)
            my_short_circuit_count++;  //中断时刻后，短路周期计数，利用大于阶跃值进行判断 1

        if(my_half_current_aver2[ii] <= my_HA_Zero_data && my_after_stop1_normal_count == 0)
            my_after_short_stop_count1++;  //中断时刻后，第一次停电周期计数，条件是停电，并且没有来电 2

        if(my_half_current_aver2[ii] > my_HA_Zero_data && my_after_short_stop_count1 > 0)
            my_after_stop1_normal_count++; //第一次停电后，来电状态统计,是否需要考虑半波测量的残余电流？？ 3

        if(my_half_current_aver2[ii] <= my_HA_Zero_data && my_after_stop1_normal_count > 0)
            my_after_short_stop_count2++;  //二次停电的统计 4

        if(fabs(my_cmpar_value[1][ii]) >= E_fild_D_value) //
            my_E_fild_change_count++;//中断时候后，，电场跌落超过阀值周期计数 5


        if(fabs(my_E_fild_aver2[ii]) <= E_fild_threshold_value)
            my_E_fild_min_count++;  //中断后，记录电场静态值小于阀值的次数 6


    }

    //中断时刻前，停电周期的次数 7
    my_befor_short_stop_count = 0;
    for(ii = 0; ii < 4; ii++)
    {
        if(my_half_current_aver1[ii] <= my_HA_Zero_data)
            my_befor_short_stop_count++;

    }

    printf("my_short_circuit_count=%d\r\n", my_short_circuit_count);
    //======12T======
#if Debug_Usart_OUT_WAVE_12T_Interupt==1
    printf("***WAVE0 12T DATA --ALL_Current3****\r\n");
    for(ii = 0; ii < 12; ii++)
        printf("%.2f MAX=%.2f\r\n", WAVE_all_ave_Current3[ii][1], WAVE_all_ave_Current3[ii][2]); //12T,全波电流

    printf("***WAVE0 12T DATA --Half_Current3****\r\n");
    for(ii = 0; ii < 12; ii++)
        printf("%.2f\r\n", WAVE_half_ave_Current3[ii][1]); //12T,全波电流

    printf("***WAVE0 12T DATA --END****\r\n");
#endif
    //=======960DATA======
#if Debug_Usart_OUT_WAVE_960Data_Interupt==1
    for(ii = 0; ii < 960; ii++)
        printf("%.2f\n", my_wave_record_sec3[0][ii]*MY_VDD / 4096.0 * (my_i_ratio_value * my_I_100A_Radio)); //960data,全波电流
#elif Debug_Usart_OUT_WAVE_960Data_Interupt==2
    for(ii = 0; ii < 960; ii++)
        printf("%.2f\r\n", my_wave_record_sec3[1][ii]*MY_VDD / 4096.0 * (my_i_ratio_value * my_I_100A_Radio)); //960data,全波电场
#elif Debug_Usart_OUT_WAVE_960Data_Interupt==3
    for(ii = 0; ii < 960; ii++)
        printf("%.2f\r\n", my_wave_record_sec3[2][ii]*MY_VDD / 4096.0 * (my_i_ratio_value * my_I_100A_Radio)); //960data,全波电场
#endif

    return 0;
}




//////////////////////////=====================
/*
功能：对2级缓存进行计算，统计出，
（1）中断后 短路周期数量，
（2）中断后 停电周期数量，
（3）中断前，停电数量，
（4）中断后电场跌落超过阀值数量
（5）中断后电场静态值小于门限值的数据

*/

uint8_t fun_Judege_It_cache2(void)
{
    //double  my_all_current_aver0 = 0;
    double	my_all_current_aver2[12] = {0};
    double  my_half_current_aver2[12] = {0}; //半波ADC采样后转换后得到的电流值
    double  my_E_fild_aver0 = 0;
    double  my_E_fild_aver2[12] = {0};
    double  my_cmpar_value[3][12] = {0}; //0行全波电流，1行电场，2行半波电流

    uint8_t ii = 0, jj = 0;
    //全波对应有效电流值
    // my_all_current_aver0=(WAVE_all_ave_Current2[0][1]+WAVE_all_ave_Current2[1][1]+WAVE_all_ave_Current2[2][1]+WAVE_all_ave_Current2[3][1])/4.0;
    //my_all_current_aver0 = WAVE_all_ave_Current3[0][1]; //故障时刻前的4个周期的平均有效电流值


    for(ii = 0; ii < 12; ii++)
    {
        //my_all_current_aver1[ii]=(WAVE_all_ave_Current2[4+ii][1]-1.2)/V_cell;  //求出后8个周期对应的有效电流值
        my_all_current_aver2[ii] = WAVE_all_ave_Current2[ii][1];
        my_half_current_aver2[ii] = WAVE_half_ave_Current2[ii][1]; //有效值的转换
        my_E_fild_aver2[ii] = WAVE_all_ave_E_Field2[ii][1] / E_cell;
    }

    //电场校正成相对量
    my_E_fild_aver0 = WAVE_all_ave_E_Field3[0][1] / E_cell;

    //计算全波电流和电场的变化阀值
    for(jj = 0; jj < 12; jj++)
    {
        my_cmpar_value[0][jj] = my_all_current_aver2[jj] - my_all_current_aver0;
        if(my_cmpar_value[0][jj] < 0) my_cmpar_value[0][jj] = 0;
        my_cmpar_value[1][jj] = my_E_fild_aver2[jj] - my_E_fild_aver0;
    }

    //电流和电场变化次数记录
    for(ii = 0; ii < 12; ii++)
    {


        if(my_cmpar_value[0][ii] >= Current_D_value)
            my_short_circuit_count2++;  //中断时刻后，短路周期计数
        if(my_half_current_aver2[ii] <= my_HA_Zero_data && my_after_stop1_normal_count == 0)
            my_after_short_stop_count1++;  //中断时刻后，第一次停电周期计数，条件是停电，并且未来电
        if(my_half_current_aver2[ii] > my_HA_Zero_data && my_after_short_stop_count1 > 0)
            my_after_stop1_normal_count++; //第一次停电后，来电状态统计,是否需要考虑半波测量的残余电流？？
        if(my_half_current_aver2[ii] <=my_HA_Zero_data && my_after_stop1_normal_count > 0)
            my_after_short_stop_count2++;  //二次停电的统计



        if((my_cmpar_value[1][ii]) >= E_fild_D_value) //
            my_E_fild_change_count++;//中断时候后，，电场跌落超过阀值周期计数
        if((my_E_fild_aver2[ii]) <= E_fild_threshold_value)
            my_E_fild_min_count++;  //中断后，记录电场静态值小于阀值的次数


    }
    //======12T======
#if Debug_Usart_OUT_WAVE_Last_12T_Interupt==1
    printf("***interrupt WAVE-%d 12T DATA --ALL_Current****\r\n", my_IT_Count);
    for(ii = 0; ii < 12; ii++)
        printf("%.2f\r\n", WAVE_all_ave_Current2[ii][1]); //12T,全波电流

    printf("***interrupt WAVE-%d 12T DATA --Half_Current****\r\n", my_IT_Count);
    for(ii = 0; ii < 12; ii++)
        printf("%.2f\r\n", WAVE_half_ave_Current2[ii][1]); //12T,全波电流

    printf("***interrupt WAVE-%d 12T DATA --END****\r\n", my_IT_Count);
#endif



    return 0;
}
/*

功能：利用中断后，3秒的录波数据的分析，对3级缓存和2级缓存的分析后各种状态的计数值的分析

	my_Fault_End_Status=0x00;  //高4位为接地，低4位为短路
  0x01,短路，0X03重合闸，0X0

	my_befor_short_stop_count=0;
	my_short_circuit_count=0;
	my_after_short_stop_count=0;

	my_E_fild_change_count=0;
	my_E_fild_min_count=0;

*/



uint8_t fun_Judege_It_end(void)
{
    //短路
    my_Fault_Current_End_Status = 0x00;

    if(	my_befor_short_stop_count == 0 && my_short_circuit_count > 0 && my_short_circuit_count <= 12 && my_short_circuit_count2 <20&& my_after_short_stop_count1 > 0  && my_after_stop1_normal_count == 0 && my_after_short_stop_count2 == 0 )
        my_Fault_Current_End_Status = (0X01 | my_Fault_Current_End_Status); //（1）--短路1

    if(my_befor_short_stop_count > 0 && my_short_circuit_count > 0 && my_after_short_stop_count1 < 4  && my_after_stop1_normal_count == 0  && my_after_short_stop_count2 == 0)
        my_Fault_Current_End_Status = 0X02 | my_Fault_Current_End_Status; //（2）--突合负载涌流,不报警

    if(my_befor_500ms_normal_count > 0 && my_befor_short_stop_count > 0 && my_short_circuit_count > 0 && my_after_short_stop_count1 > 0  && my_after_stop1_normal_count == 0  && my_after_short_stop_count2 == 0)
        my_Fault_Current_End_Status = 0X04 | my_Fault_Current_End_Status; //（3）非故障相重合闸涌流（不成功）,不报警--短路中断
		if(my_befor_500ms_normal_count > 0 && my_befor_short_stop_count == 0 && my_short_circuit_count ==0 && my_after_short_stop_count1 > 0  && my_after_stop1_normal_count > 0  && my_after_short_stop_count2 > 0)
        my_Fault_Current_End_Status = 0X04 | my_Fault_Current_End_Status; //（3）非故障相重合闸涌流（不成功）,不报警--电场中断
		
		if(my_befor_500ms_normal_count > 0 && my_befor_short_stop_count > 0 && my_short_circuit_count > 0 && my_after_short_stop_count1 == 0  && my_after_stop1_normal_count == 0  && my_after_short_stop_count2 == 0)
        my_Fault_Current_End_Status = 0X0E; //非故障相合闸成功,不报警--短路中断
		if(my_befor_500ms_normal_count > 0 && my_befor_short_stop_count ==0 && my_short_circuit_count == 0 && my_after_short_stop_count1 > 0  && my_after_stop1_normal_count > 0  && my_after_short_stop_count2 > 0)
        my_Fault_Current_End_Status = 0X0E; //非故障相合闸成功,不报警--电场中断

    if(my_befor_short_stop_count == 0 && my_short_circuit_count > 0 && my_after_short_stop_count1 == 0  && my_after_stop1_normal_count == 0  && my_after_short_stop_count2 == 0)
        my_Fault_Current_End_Status = 0X08 | my_Fault_Current_End_Status; //（4）--负荷瞬间突变，不报警

    if(my_befor_short_stop_count == 0 && my_short_circuit_count > 0 && my_short_circuit_count2 >=20 && my_after_short_stop_count1 > 0 && my_after_stop1_normal_count == 0  && my_after_short_stop_count2 == 0)
        my_Fault_Current_End_Status = 0X10 | my_Fault_Current_End_Status; //（5）--人工投切大负荷,不报警

    if(my_befor_500ms_normal_count == 0 && my_befor_short_stop_count > 0 && my_short_circuit_count > 0 && my_after_short_stop_count1 > 0  && my_after_stop1_normal_count == 0  && my_after_short_stop_count2 == 0)
        my_Fault_Current_End_Status = 0X20 | my_Fault_Current_End_Status; //（6）---空载合闸,不报警


    if(my_befor_short_stop_count == 0 && my_short_circuit_count > 0 && my_after_short_stop_count1 > 0  && my_after_stop1_normal_count > 0  && my_after_short_stop_count2 == 0)
        my_Fault_Current_End_Status = 0X41 | my_Fault_Current_End_Status; //短路2，重合闸成功--短路中断
    if(my_befor_short_stop_count == 0 && my_short_circuit_count > 0 && my_after_short_stop_count1 > 0  && my_after_stop1_normal_count > 0  && my_after_short_stop_count2 > 0)
        my_Fault_Current_End_Status = 0X81 | my_Fault_Current_End_Status; //短路3，重合闸不成功--短路中断




    




    //else
    //my_Fault_Current_End_Status=0X0E; //未知状态
    //电场判断
    if(my_E_fild_change_count > 0 && my_E_fild_min_count > 0 && my_after_short_stop_count1 == 0) 
        my_Fault_E_Fild_End_Status = 0x01; //电场跌落，电场小于特定值，电流正常，报接地故障
    else if(my_E_fild_change_count > 0 && my_E_fild_min_count > 0 && my_after_short_stop_count1 > 0)
        my_Fault_E_Fild_End_Status = 0x02; //电场跌落，但是没有电流了，停电


#if Debug_Usart_OUT_WAVE_End_Just_Interupt==1
    printf("  I=[%XH],  E=[%XH]\r\n", my_Fault_Current_End_Status, my_Fault_E_Fild_End_Status);
    //printf("500ms  短路前有电=%d, 前停电=%d, 短路=%d, 短路后停电=%d, 停电后正常=%d, 正常后停电=%d\r\n",
    printf("500ms  frontX_A_A=%d, frontX_NOA_B=%d, short_circurt_C=%d, afterX_NOA_D=%d, afterNOA_normal_E=%d, afternorm_NOA_F=%d,short_circurt2-G=%d\r\n",
           my_befor_500ms_normal_count,
           my_befor_short_stop_count,
           my_short_circuit_count,
           my_after_short_stop_count1,
           my_after_stop1_normal_count,
           my_after_short_stop_count2,
						my_short_circuit_count2);
    printf("短路前有电-A, 短路前停电-B, 短路-C, 短路后停电-D, 停电后正常-E, 正常后停电-F\n");

#endif

    return 0;
}

/*
功能：录波数据存储

*/
void my_fun_wave_rec(void)
{
    uint32_t ii = 0;
    ii++;
    while(fun_my_wave1_to_wave2() != 1)
    {
        ii++;
        if(ii > 0xFFFFF)
            break;

    }

}

//==========短路中断=============================
/*
功能：
（1）先进行首次录波，1级到2级，2级到3三级，进行初步判断
（2）200ms后再次进行，进行二次录波，1级到2级，重复10次，总共2秒，每次都对2级缓存进行判断
（3）第10次录波，进行综合判断，得出结论
  思路：故障录波后，每间隔200ms录波一次，总共2s后分析波形，得出结论。

*/


uint8_t my_fun_current_exit_just(void)
{
    my_IT_Count++; //调用中断故障判断程序次数统计
    //USART_printf(&huart2, "---just interrupt count=%d-----\n", my_IT_Count);
    uint8_t temp8 = 0;
    //1.中断事件处理
    if(my_IT_status == 0 && (my_Current_exit_Status == 1 || my_E_Field_exit_Status == 1))
    {
        if(my_Current_exit_Status == 1)
        {
            //my_dianliu_exit_add = my_wave_write_add;
            //my_Current_Exit_add = my_dianliu_exit_add;
            my_Wave_It_add = my_dianliu_exit_add;

        }
        else if(my_E_Field_exit_Status == 1)
        {
            //my_dianliu_exit_add = my_wave_write_add;
            //my_E_Field_exit_add = my_dianliu_exit_add;
            my_Wave_It_add = my_E_Field_exit_add;

        }

        my_adc_1_convert();
        my_fun_wave_rec();
        my_adc2_convert2(0); //计算12个周期的每个周期的3个分量，和12个周期的累计平均值,把2级缓冲中的数据转换完了。
        my_IT_status = 1; //为1表示，中断事件已经响应了，需要进入下一步，等待3秒后在进行采样，判断停电状态
        my_IT_Count = 0; //判断程序统计清零，下次进入为第一次


        fun_wave2_to_wave3(); //二级缓存中的所有数据放入到三级缓存中，处理后值
        //对3级缓存继续故障判断
        fun_Judege_It_cache3();//对首次录波，利用3级缓冲进行，给全局故障状态记录变量赋值

    }

    //============
    if(my_IT_Count >= 1) //进行多次二级采样计算，然后进行各种状态，中断前停电/短路,中断后停电/短路的统计
    {
        my_dianliu_exit_add = my_wave_write_add;
        my_fun_wave_rec();
        my_adc2_convert2(0); //计算12个周期的每个周期的3个分量，和12个周期的累计平均值,数据放在二级缓存中
        fun_Judege_It_cache2();//利用2级缓冲，给全局故障状态记录变量赋值,可以进行后8个周期160ms，任意多次，10次就是1.6s
    }


    if(my_IT_Count >= (5 * 2)) //2秒钟以后，进行最终状态的判断
    {
        //进行中断后2秒钟的录波数据分析后，进行故障类型判断
        fun_Judege_It_end();

        //结束处理，撤销中断事件
        my_IT_Count = 0;  //中断事件处理结束，本次判断结束
        my_IT_status = 0;
        my_Current_exit_Status = 0;
        my_E_Field_exit_Status = 0;

        temp8 = 1;

    }
    //1.处理结束
    return temp8; //1为表示结束了，0为没有结束




}



/*
功能：判断线路停电状态、对地电场状态
思路：此函数不进行录波数据1级到2级的处理，也不进行转换。直接取12T的数据2级缓冲值（转换后的值），利用半波数据判断是否停电。
用法：此函数，需要跟在DAC设置函数，或者周期采样函数后边，这样，利用上一个函数的录波结果就可以判断停电，和对地电场的值。
*/

double my_E_Fild_old=0;
void my_fun_get_Line_stop_Efild(void)
{

    uint16_t  my_step = 00;

    //return ;
//取当前数据
    my_fun_wave1_to_wave2_old_data();
    my_adc2_convert2(0);


//静态数据判断
    if( ADC2_Filer_value_buf_2[1][0] <= MY_Efile_Zero_data &&  ADC2_Filer_value_buf_2[2][1] <= my_HA_Zero_data) //电场为0，半波为0
        my_Line_Current_stop_status = 1; //表示停电,  没有电场，没有电流

    else if(ADC2_Filer_value_buf_2[0][1] >= my_A_Zero_data &&  ADC2_Filer_value_buf_2[1][1] > MY_Efile_floor_data   && ADC2_Filer_value_buf_2[2][1] > my_HA_Zero_data)
        my_Line_Current_stop_status = 3; //表示正常，  线路 有电流，有电场。

    else if(ADC2_Filer_value_buf_2[0][1] >= my_A_Zero_data &&  ADC2_Filer_value_buf_2[1][1] <= MY_Efile_floor_data 	&& ADC2_Filer_value_buf_2[2][1] > my_HA_Zero_data)
        my_Line_Current_stop_status = 2; //表示接地，  线路电场很小，有电流
		
//电场跌落百分比
		if((my_E_Fild_old-ADC2_Filer_value_buf_2[1][1])*1.0/my_E_Fild_old>0.45 && my_E_Fild_old>ADC2_Filer_value_buf_2[1][1] && my_E_Fild_old>MY_Efile_floor_data  && ADC2_Filer_value_buf_2[2][1] > my_HA_Zero_data && ADC2_Filer_value_buf_2[0][1] >= my_A_Zero_data)
		{
			my_Line_Current_stop_status = 2;  //利用百分比方法判断，接地
		}
		my_E_Fild_old=ADC2_Filer_value_buf_2[1][1];
//静态电流大于400A判据
		if(ADC2_Filer_value_buf_2[0][1]>400 && ADC2_Filer_value_buf_2[1][1]>MY_Efile_Zero_data)
		{
				fun_wave2_to_wave3();
        my_Fault_Current_End_Status = 0XF3;
        my_Fault_E_Fild_End_Status = 00;
        printf("==return Short over 400--1 A_S=%d  E_S=%d\n",my_Fault_Current_End_Status,my_Fault_E_Fild_End_Status);
        my_zsq_ALarm_send_status = 1;
        my_step = 0x0002; //发送报警，消息任务
        xQueueSend(myQueue01Handle, &my_step, 100);
			
		}
		
		
		

//报警状态恢复
    if(	my_Line_Current_stop_status == 3 && (my_Fault_Current_End_Status != 0 || my_Fault_E_Fild_End_Status != 0))
    {
        fun_wave2_to_wave3();
				printf("==all return normal--1 A=%d  E=%d\n",my_Fault_Current_End_Status,my_Fault_E_Fild_End_Status);
        my_Fault_Current_End_Status = 00;
        my_Fault_E_Fild_End_Status = 00;
       
        my_zsq_ALarm_send_status = 1;
        my_step = 0x0002; //发送报警，消息任务
        xQueueSend(myQueue01Handle, &my_step, 100);
    }
//短路报警恢复,只考虑电流，不考虑电场
		else if(my_Line_Current_stop_status==2 && my_Fault_Current_End_Status != 0)
		{
				fun_wave2_to_wave3();
				printf("==current return normal--1 A=%d  E=%d\n",my_Fault_Current_End_Status,my_Fault_E_Fild_End_Status);
        my_Fault_Current_End_Status = 00;
        //my_Fault_E_Fild_End_Status = 00;
        
        my_zsq_ALarm_send_status = 1;
        my_step = 0x0002; //发送报警，消息任务
        xQueueSend(myQueue01Handle, &my_step, 100);
			
		}

//停电状态上传
    if(my_Line_Current_stop_status == 1 && my_Line_Current_stop_last_status == 3 ) //上次正常，现在停电，上传
    {
        fun_wave2_to_wave3();
        my_Fault_Current_End_Status = 0xFE;
        my_Fault_E_Fild_End_Status = 0xFE;
        my_Line_Current_stop_last_status = 1; //停电
        printf("==return normal--2---stop A=%d  E=%d\n",my_Fault_Current_End_Status,my_Fault_E_Fild_End_Status);
        my_zsq_ALarm_send_status = 1;
        my_step = 0x0002; //发送报警，消息任务
        xQueueSend(myQueue01Handle, &my_step, 100);
    }
//接地状态上传
    else if(my_Line_Current_stop_status == 2 && my_Line_Current_stop_last_status == 3)
    {
        my_E_Field_exit_add = my_E_fild_time_add;
        fun_wave2_to_wave3();

        my_Fault_E_Fild_End_Status = 0x01;
        my_Line_Current_stop_last_status = my_Line_Current_stop_status;
        printf("==return normal--3---jiedi= A=%d  E=%d\n",my_Fault_Current_End_Status,my_Fault_E_Fild_End_Status);
        my_zsq_ALarm_send_status = 1;
        my_step = 0x0002; //发送报警，消息任务
        xQueueSend(myQueue01Handle, &my_step, 100);

    }
//线路正常，历史数据变化
    else if(my_Line_Current_stop_status == 3 && my_Line_Current_stop_last_status != 3)
    {

        my_Line_Current_stop_last_status = 3; //把历史状态，恢复为最新状态
    }


#if Debug_Usart_OUT_LINE_STOP_STATUS==1
    printf("--Line stop staus=[%d]--A=%d,E=%d \n", my_Line_Current_stop_status,my_Fault_Current_End_Status,my_Fault_E_Fild_End_Status);
#endif

//电场值获得

    if(my_Line_Current_stop_status == 1)
        my_line_Current_value = 0; //把全波的零飘值变为0
    else
        ;  //空语句，目的是显示停电的时候的零飘值，如果想屏蔽零飘，就屏蔽此空语句
    my_line_Current_value = ADC2_Filer_value_buf_2[0][1] * 10; //电流的有效值
    my_Line_Efild_valu = ADC2_Filer_value_buf_2[1][1]; //测得的电场的有效值
#if Debug_Usart_OUT_LINE_Efield_STATUS==1
    printf("--Line current=%.2f  Efild=%d--\r\n", my_line_Current_value / 10.0, my_Line_Efild_valu);

#endif
}

//=============
//数据存储方案2,12周期利用旧数据

void my_fun_wave1_to_wave2_old_data(void)
{
    int ii = 0;
    uint16_t my_end_add = my_wave_write_add;
    uint16_t my_start_add = 0;
    uint16_t temp_add = 0;
    uint16_t count_number = rec_T_Count * 80;
    my_E_fild_time_add = my_wave_write_add;
    if(my_end_add >= count_number)
    {
        my_start_add = my_end_add - count_number;
    }
    else
    {
        my_start_add = WAVE_number - (count_number - my_end_add);
    }

    //计算数据
    temp_add = my_start_add;
    for(ii = 0; ii < count_number; ii++)
    {
        my_wave_record_sec2[0][ii] = my_wave_record[0][temp_add];
        my_wave_record_sec2[1][ii] = my_wave_record[1][temp_add];
        my_wave_record_sec2[2][ii] = my_wave_record[2][temp_add];

        temp_add++;
        if(temp_add >= WAVE_number)
            temp_add = 0;
    }
    if(count_number < WAVE_number_sec2)
    {
        for(; ii < WAVE_number_sec2; ii++)
        {
            my_wave_record_sec2[0][ii] = 0;
            my_wave_record_sec2[1][ii] = 0;
            my_wave_record_sec2[2][ii] = 0;
        }

    }
}

/*
功能：查询法获得，电场状态
*/
//void my_fun_query_Efild(void)
//{

//    uint16_t my_step = 0;
//    my_fun_wave1_to_wave2_old_data();
//    my_adc2_convert2(0);

//    //停电判断
//    if( ADC2_Filer_value_buf_2[1][2] <= MY_Efile_Zero_data &&  ADC2_Filer_value_buf_2[2][2] <= my_HA_Zero_data) //电场为0，半波为0
//        my_Line_Current_stop_status = 1; //表示停电,没有电场，没有电流
//    else if(ADC2_Filer_value_buf_2[0][1] >= my_A_Zero_data &&  ADC2_Filer_value_buf_2[1][1] > MY_Efile_Zero_data)
//        my_Line_Current_stop_status = 0; //表示正常，线路 有电流，有电场。
//    else if(ADC2_Filer_value_buf_2[0][1] >= my_A_Zero_data &&  ADC2_Filer_value_buf_2[1][1] <= MY_Efile_floor_data)
//        my_Line_Current_stop_status = 2; //表示接地，线路电场很小，有电流

//    //接地波形
//    if(my_Line_Current_stop_status == 2  && my_Line_Current_stop_last_status == 0)
//    {
//        my_E_Field_exit_add = my_E_fild_time_add;
//        fun_wave2_to_wave3();

//        my_Fault_E_Fild_End_Status = 0x01;
//        my_Line_Current_stop_last_status = my_Line_Current_stop_status;
//        my_zsq_ALarm_send_status = 1;
//        my_step = 0x0002; //发送报警，消息任务
//        xQueueSend(myQueue01Handle, &my_step, 100);

//    }

//}

