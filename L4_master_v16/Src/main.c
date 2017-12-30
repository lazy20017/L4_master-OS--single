/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2017 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32l4xx_hal.h"
#include "cmsis_os.h"
#include "adc.h"
#include "dac.h"
#include "dma.h"
#include "rtc.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* USER CODE BEGIN Includes */
#include "my_usart.h"
#include "my_extern_val.h"
#include "my_gloabal_val.h"
#include "my_ADC.h"
#include "my_DAC.h"
//#include "my_led.h"
#include "my_101.h"
#include "my_time.h"
#include "my_cc1101.h"
#include "bsp_stm_flash.h"
#include "bsp_iap.h"

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
#include "my_OS.h"
/*
�汾��20171227E
*/
extern uint8_t my_nmi_status;
extern uint16_t ADC2_GetValue[ADC2_ROW][ADC2_COLM];//���ڱ���ɼ���ֵ,M��ͨ��,N��
extern uint16_t ADC1_GetValue[ADC1_ROW][ADC1_COLM];//���ڱ���ɼ���ֵ,M��ͨ��,N��

extern	uint32_t my_dac_tr_data;

extern uint8_t  rsbuf2[200];
extern uint16_t my_usart2_re_count;
extern uint16_t VREFINT_CAL;
extern uint16_t MY_TS_CAL1;
extern uint16_t MY_TS_CAL2;

//DAC1
extern uint16_t my_dac1_channel1_data_i;
extern uint16_t my_dac1_channel2_data_e;

//USART3


extern uint8_t  my_Time_Cyc_exit_Status;
extern uint16_t my_pro_step;



/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

uint8_t my_sys_start_status=0;  //ϵͳ������ʶ

/* USER CODE END 0 */

int main(void)
{

  /* USER CODE BEGIN 1 */
    __IO uint8_t xi=0,yi=0;

    uint8_t my_usart_tx_data[20]= {0x31,0x32,0x033,0x34,0X35,0X36,0X37,0X38,0X39,0X3A};
    uint32_t my_fr=0;
  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_DAC1_Init();
  MX_RTC_Init();
  MX_ADC2_Init();
  MX_USART2_UART_Init();
  MX_SPI3_Init();
  MX_TIM7_Init();
  MX_TIM6_Init();

  /* USER CODE BEGIN 2 */

    HAL_NVIC_DisableIRQ(EXTI0_IRQn); //�ӵ��ж�
    HAL_NVIC_DisableIRQ(EXTI1_IRQn); //��·�ж�
		HAL_NVIC_DisableIRQ(EXTI9_5_IRQn); //CC1101�����ж�
	
    HAL_NVIC_ClearPendingIRQ(EXTI0_IRQn);
    HAL_NVIC_ClearPendingIRQ(EXTI1_IRQn);
		HAL_NVIC_ClearPendingIRQ(EXTI9_5_IRQn);
  //ϵͳ������ʶ
  my_sys_start_status=1;
	HAL_TIM_Base_Start_IT(&htim6);  //����tim6��ʱ��,1s
    //__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_1); //�����������ⲿ�жϣ�ÿ���жϵ������������һ������Ҫ�ֱ����
    //__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_2); //�����������ⲿ�жϣ�ÿ���жϵ������������һ������Ҫ�ֱ����
   // HAL_TIM_Base_Start_IT(&htim6);  //����tim6��ʱ��,1s
   

   
		
		
		HAL_Delay(200);
    //LED2_ON;		
		HAL_Delay(200);
		LED2_OFF;
		

		 HAL_Delay(200);
    //LED4_ON;		
		HAL_Delay(200);
		LED4_OFF;
		
    //����UART2
    HAL_UART_Transmit(&huart2,my_usart_tx_data,4,1000);
    printf("\nPrintf MCU1-UART2 IS OK!!\n");
    USART_printf(&huart2,"usart_printf MCU1-UART2 IS OK!!");
	
    my_fr=HAL_RCC_GetSysClockFreq();  //��ȡϵͳʱ��
    printf("\n ================SysClockFreq:%dMHZ========\n",my_fr/1000000);

    //---ADC1,ADC2---DMA��ʽ����������
    HAL_ADC_Start_DMA(&hadc1, (uint32_t *)&ADC1_GetValue[0][0],ADC1_ROW*ADC1_COLM);//&ADC2_GetValue[0][0],ADC2_ROW*ADC2_COLM
    HAL_ADC_Start_DMA(&hadc2, (uint32_t *)&ADC2_GetValue[0][0],ADC2_ROW*ADC2_COLM);//&ADC1_GetValue[0][0],ADC1_ROW*ADC1_COLM


    //--------��ʱ������
    HAL_TIM_Base_Start_IT(&htim7);  //����tim7��ʱ����250us
    //HAL_TIM_Base_Start_IT(&htim6);  //����tim6��ʱ��,1s


//	HAL_TIM_Base_Stop_IT(&htim6);
//	HAL_TIM_Base_Stop_IT(&htim7);



    /* �õ���׼��ѹУ׼ֵ */
    VREFINT_CAL = *(__IO uint16_t *)(0X1FFF75AA);
    MY_TS_CAL1 = *(__IO uint16_t *)(0X1FFF75A8);
    MY_TS_CAL2 = *(__IO uint16_t *)(0X1FFF75CA);
    printf("\n VREFINT_CAL=[%d],TS_CAL1 =%d,TS_CAL2 =%d\n",VREFINT_CAL,MY_TS_CAL1,MY_TS_CAL2);
    my_adc_1_convert_dis(1);

//��EXIT�ж�
//    HAL_NVIC_DisableIRQ(EXTI1_IRQn);
//    HAL_NVIC_DisableIRQ(EXTI2_IRQn);
//    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_1); //�����������ⲿ�жϣ�ÿ���жϵ������������һ������Ҫ�ֱ����
//    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_2); //�����������ⲿ�жϣ�ÿ���жϵ������������һ������Ҫ�ֱ����


//DAC1
    my_dac1_channel1_data_i=0X0FFF/2;
    my_dac1_channel2_data_e=0X0FFF/4;
    HAL_DAC_SetValue(&hdac1,DAC_CHANNEL_1,DAC_ALIGN_12B_R,my_dac1_channel1_data_i); //����ģ����
    HAL_DAC_Start(&hdac1,DAC_CHANNEL_1); //����DAC1��ͨ��1  ����ͨ��

    HAL_DAC_SetValue(&hdac1,DAC_CHANNEL_2,DAC_ALIGN_12B_R,my_dac1_channel2_data_e);//����ģ����
    HAL_DAC_Start(&hdac1,DAC_CHANNEL_2);//����DAC1��ͨ��2  �糡ͨ��

    my_dac1_channel1_data_i=0X0FFF/11*10;
    HAL_DAC_SetValue(&hdac1,DAC_CHANNEL_1,DAC_ALIGN_12B_R,my_dac1_channel1_data_i);//����ģ���������Զ����
    my_dac1_channel2_data_e=0X0FFF/3.0;
    HAL_DAC_SetValue(&hdac1,DAC_CHANNEL_2,DAC_ALIGN_12B_R,my_dac1_channel2_data_e);//����ģ���������Զ����


//��EXIT�ж�

    HAL_NVIC_ClearPendingIRQ(EXTI0_IRQn);
    HAL_NVIC_ClearPendingIRQ(EXTI1_IRQn);
		HAL_NVIC_ClearPendingIRQ(EXTI9_5_IRQn);
	

    //HAL_NVIC_EnableIRQ(EXTI0_IRQn);
   //HAL_NVIC_EnableIRQ(EXTI1_IRQn); //��·�жϿ���
   // HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
  
  //====�ڲ�FLASH
	  uint64_t passsword=0;
		uint64_t length_L=0;
						
		passsword=APP_update_password;
		length_L=0X1234567890ABCDEF;
			
			
			
//			STMFLASH_Write(APP_update_status1_add,&passsword,1);			
//			STMFLASH_Write(APP_update_status2_add,&passsword,1);					
//			STMFLASH_Write(APP_update_length1_add,&length_L,1);	   
//			STMFLASH_Write(APP_update_length2_add,&length_L,1);
			
		STMFLASH_Read(APP_update_status1_add,&passsword,1);
		printf("status=[%X]\n",(uint32_t)passsword);
		STMFLASH_Read(APP_update_status2_add,&passsword,1);
		printf("status=[%X]\n",(uint32_t)passsword);
		STMFLASH_Read(APP_update_length1_add,&length_L,1);
		printf("status=[%X]\n",(uint32_t)length_L);
		STMFLASH_Read(APP_update_length2_add,&length_L,1);
		printf("status=[%X]\n",(uint32_t)length_L);

		//�������ڽ����ж�
		HAL_UART_Receive_IT(&huart2,&rsbuf2[rsbuf2pt_write],1);  //
		
 	
		
 
//PWR�ضϹ���
GANdianchi_OFF; //�ضϸɵ��
WDI1_TOGGLE;
HAL_Delay(500);
GANdianchi_ON; //�򿪸ɵ�أ�Ĭ�ϣ�
//HAL_Delay(500);
//GANdianchi_OFF; //�ضϸɵ��
  
EN25505_OFF;
WDI1_TOGGLE;
HAL_Delay(500);
EN25505_ON;//BQ25505������Ĭ�ϣ�
 

CT_Source_short_ON;  //CT��·�ŵ�
WDI1_TOGGLE;
HAL_Delay(500);
CT_Source_short_OFF;  //CT��·�����ŵ磨Ĭ�ϣ�

WDI1_TOGGLE;
HAL_Delay(500);
  
CT_to_BQ25505_ON;  //CT��BQ25���
WDI1_TOGGLE;
HAL_Delay(500);
printf("\n 999999 \n"); 

CT_to_BQ25505_OFF;

//==CC1101  START  INIT
#if USE_CC1101_PWR_ON_STATUS==1

  CC1101_PWR_ON; //Ӳ��Ĭ���ǶϿ�״̬	
	WDI1_TOGGLE;
	printf("\n 7777 \n"); 

	HAL_Delay(200);	
	printf("\n 88888 \n"); 

	
#else
	CC1101_PWR_OFF;//CC1101�ϵ�
	
	
	HAL_Delay(200);	
#endif
 printf("\n 66666 \n"); 

	
	WDI1_TOGGLE;
	printf("GDO2_CONFIG-%XH\n",CC1101ReadReg(CC1101_IOCFG2));
	printf("GDO1_CONFIG-%XH\n",CC1101ReadReg(CC1101_IOCFG1));
	printf("GDO0_CONFIG-%XH\n",CC1101ReadReg(CC1101_IOCFG0));
	WDI1_TOGGLE;
	 
	CC1101Init();
	HAL_Delay(1000);
	
	printf("GDO2_CONFIG-%XH\n",CC1101ReadReg(CC1101_IOCFG2));
	printf("GDO1_CONFIG-%XH\n",CC1101ReadReg(CC1101_IOCFG1));
	printf("GDO0_CONFIG-%XH\n",CC1101ReadReg(CC1101_IOCFG0));
	
	my_read_all_reg();
	

	
	//--------����WORģʽ,��ǰ��ʽ�����ð�ť�жϣ�����WORģʽ
   //EVE0=0X876A T0Ϊ1�� ����T0Ϊ0.5���ʱ��Ϊ0X876A��һ�룬0X43B5H  0.25��0X21DA
	
			CC1101WriteCmd(CC1101_SIDLE); //�������״̬	
			CC1101WriteReg(CC1101_MCSM2,0X00); //д0x16 ,RX_TIME��д��0X00�����ý���ʱ�����ƣ�ռ�ձ�����00���12%,���û���յ����ݣ��ͽ���SLEEP��			
			CC1101WriteReg(CC1101_MCSM0,0x18);  //0X18 	//
			CC1101WriteReg(CC1101_WOREVT1,0x87);  //0X1E,event0 ���ֽ�
			CC1101WriteReg(CC1101_WOREVT0,0x6A);   //0X1F event0 ���ֽ�
			CC1101WriteReg(CC1101_WORCTRL,0X78); //д0X20,0111 100,WOR_RES		
			//CC1101WriteCmd(CC1101_SWOR); //����WORģʽ  
	
	//---------�������ģʽ
	
	CC1101SetIdle();
	CC1101WriteReg(CC1101_MCSM2,0X07); //д0x16 ,RX_TIME��д��0X07,����û��ʱ�����ƣ�һ�½�������	
	//CC1101SetTRMode(RX_MODE);           // �������ģʽ
	HAL_Delay(500);
	printf("after rx CC1101 status=[%XH] \n",CC1101ReadStatus(CC1101_MARCSTATE)); //0X01���У�0X0D���գ�0X13����
if(my_CC1101_Sleep_status==1)  //CC1101sleep״̬����
{
  CC1101SetSleep();
}
else
{
	CC1101SetTRMode(RX_MODE);           // �������ģʽ
	HAL_Delay(500);
	printf("after rx CC1101 status=[%XH] \n",CC1101ReadStatus(CC1101_MARCSTATE)); //0X01���У�0X0D���գ�0X13����
}

	
	HAL_Delay(500);
	//---------��ʼ����ɣ����������ж�
	
	//__HAL_GPIO_EXTI_CLEAR_IT(PIN_CC_IRQ);
	//HAL_NVIC_EnableIRQ(EXIT_CC_IRQ_GD0); //�����жϣ�����CC1101���ݲ���
	
	//===CC1101 END
	printf("sysclk=%dMHZ, hclk=%dMHZ\r\n",HAL_RCC_GetSysClockFreq()/1000000,HAL_RCC_GetHCLKFreq()/1000000);


	
  


  //ϵͳ������ʶ
  my_sys_start_status=1;

  /* USER CODE END 2 */

  /* Call init function for freertos objects (in freertos.c) */
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();
  
  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	xi=0;
    while (1)
    {
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */
      
				HAL_Delay(1000);
			  printf("%d\n",++xi);
			  //DisableGPIO();
      //HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON,PWR_STOPENTRY_WFI);//STOP2--2.42MA-��CC1101
      //�ر����ж˿ں�����1.95��ֻ�رն˿�2.18,���ر�2.72
			
			 //HAL_PWR_EnterSTOPMode(PWR_MAINREGULATOR_ON,PWR_STOPENTRY_WFI); //STOP1,?TCC1101--2.42ma��?��DCC1101--3.8ma
			//�ر����ж˿ں�����1.95,ֻ�رն˿�2.18,���ر�2.72
			
			//HAL_PWR_EnterSLEEPMode(PWR_LOWPOWERREGULATOR_ON,PWR_STOPENTRY_WFI);			
			//�ر����ж˿ڣ�����1.84,--ֻ�رն˿�2.08�����ر�2.62
			
			//HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON,PWR_STOPENTRY_WFI);//sleep1--2.42��CC1101����CC1101˯��-2.53MA
			//�ر����ж˿ڡ�����1.95ma--2.18�ر�����GPIO�˿ڣ����ر�2.72,
      
			//HAL_PWR_EnterSTANDBYMode();  //��/��CC1101--0.41ma
			//HAL_PWREx_EnterSHUTDOWNMode(); //0.41ma
			
			//û�е͹���  2.72ma
    }
  /* USER CODE END 3 */

}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_7;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_USART2
                              |RCC_PERIPHCLK_ADC;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_SYSCLK;
  PeriphClkInit.AdcClockSelection = RCC_ADCCLKSOURCE_SYSCLK;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the main internal regulator output voltage 
    */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 15, 0);
}

/* USER CODE BEGIN 4 */






/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM17 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
/* USER CODE BEGIN Callback 0 */

/* USER CODE END Callback 0 */
  if (htim->Instance == TIM17) {
    HAL_IncTick();
  }
/* USER CODE BEGIN Callback 1 */
HAL_TIM_PeriodElapsedCallback2(htim);
	
	
/* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void _Error_Handler(char * file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1) 
  {
  }
  /* USER CODE END Error_Handler_Debug */ 
}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
      ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
