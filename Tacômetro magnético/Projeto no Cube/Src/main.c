/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "stm32f4xx.h"
#include "imagem.h"
#include "LCD.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
enum t_bordas{anterior,atual}; //Enumeração para facilitar a visualização [atual]==1 [anterior]==0
enum estado {desligado,ligado}; //Enumeração para facilitar a visualização [ligado]==1 [desligado]==0
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define FATOR_FILTRO 3
#define PWM_Start 40
#define NUM_AMOSTRAS 64
#define TIM_ARR 100-1
#define K 600000//Constante utilizada no calculo do RPM -> Qnt de ARR necessária para um minuto
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

uint32_t borda[2]={1,1};
uint32_t RPM=0;
uint32_t intro1[504]={0};
uint32_t intro2[504]={0};
uint32_t samples[NUM_AMOSTRAS]={};

int aux_samples=0;
int PWM_DutyCycle=0;
int aux=0;
int estado=desligado;

char counter_s[10]="0";
char RPM_s[10]="0";
char PWM_DutyCycle_s[10]="0";
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int MeanSample()
{
	int media=0;
	int div=0;
	if (estado==desligado)return 0;
	for(int i=0;i< NUM_AMOSTRAS;i++)
	{
		if(samples[i]!=0)div++;
		media+=samples[i];
	}
	if(div!=0)return (media/=div);

	return 0;
}
void AtualizaDisplay(void)
{
	itoa(PWM_DutyCycle,PWM_DutyCycle_s,10);
	itoa(MeanSample(),RPM_s,10);
	LCD5110_clear();
	LCD5110_set_XY(0,0);
	LCD5110_write_string("RPM:");
	LCD5110_write_string(RPM_s);
	LCD5110_set_XY(0,10);
	LCD5110_write_string("Duty-Cycle:");
	LCD5110_write_string(PWM_DutyCycle_s);
	LCD5110_set_XY(0,20);
	if (estado==ligado)
		LCD5110_write_string("Motor [ON]");
	else
		LCD5110_write_string("Motor [OFF]");

}
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM10)//display bonitinho
	{
		switch(aux)
		{
			case 1:
				LCD5110_clear();
				LCD5110_set_XY(0,0);
				for (int a=0;a<=500;a++){LCD5110_LCD_write_byte(intro1[a],1);}
				break;
			case 2:
				LCD5110_clear();
				LCD5110_set_XY(0,0);
				for (int a=0;a<=500;a++){LCD5110_LCD_write_byte(intro2[a],1);}
			break;
			case 3:
				__HAL_TIM_SET_PRESCALER(htim,840-1);
				__HAL_TIM_SET_AUTORELOAD(htim,10000-1);
				HAL_TIM_IC_Start_IT(&htim2,TIM_CHANNEL_2); //Iniciando o Canal 2 do Timer 2, como InputCapture
				HAL_TIM_Encoder_Start_IT(&htim1,TIM_CHANNEL_ALL);//Iniciando o Canal 1 e 2 do Timer 1 no modo Encoder
			break;
			default:
				AtualizaDisplay();
			break;
		}
		aux++;
	}
}
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM1)//Encoder
	{
		//Controle de overflow do encoder
		if(__HAL_TIM_GET_COUNTER(htim)>200)
			__HAL_TIM_SET_COUNTER(htim,200);
		if(__HAL_TIM_GET_COUNTER(htim)<2)
			__HAL_TIM_SET_COUNTER(htim,2);
		else
			PWM_DutyCycle= __HAL_TIM_GET_COUNTER(htim)/2; //Lendo dados do Encoder

		//Controle de estado do motor, baseado no duty-cycle, se estiver abaixo de 10% o motor ficará desligado
		if(PWM_DutyCycle<PWM_Start)estado=desligado;
		else estado=ligado;
		if(estado==ligado)
		{
			HAL_TIM_PWM_Start_IT(&htim3,TIM_CHANNEL_1);
			__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_1,((float)PWM_DutyCycle/100)*TIM_ARR);
		}
		else
		{
			HAL_TIM_PWM_Stop_IT(&htim3,TIM_CHANNEL_1);
			__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_1,0);
		}
		AtualizaDisplay();
	}
	if(htim->Instance == TIM2)//HALL sensor reading
	{
		borda[anterior]=borda[atual];//Deslocando a borda no ARRAY
		borda[atual]=__HAL_TIM_GET_COMPARE(htim,TIM_CHANNEL_2); //Lendo a borda e armazenando na posição [atual] do ARRAY

		if( borda[anterior]!=0 && borda[atual]!=borda[anterior])
		{
			RPM=K/abs(borda[atual]-borda[anterior]);// Cálculo do RPM
			if(RPM<(FATOR_FILTRO*samples[aux_samples-1]) || samples[aux_samples]==0)samples[aux_samples]=RPM;
			if(aux_samples++>(NUM_AMOSTRAS-1))aux_samples=0;
		}

	}

}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

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
  MX_USART2_UART_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_TIM10_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start_IT(&htim10);
  LCD5110_init();

  int coluna=0;
  int linha=0;

  for(int i=0; i<504; i++)
  {
	  intro1[i]= nomes[0+(linha*8)][coluna] * 1;
	  intro1[i]+= nomes[1+(linha*8)][coluna] * 2;
	  intro1[i]+= nomes[2+(linha*8)][coluna] * 4;
	  intro1[i]+= nomes[3+(linha*8)][coluna] * 8;
	  intro1[i]+= nomes[4+(linha*8)][coluna] * 16;
	  intro1[i]+= nomes[5+(linha*8)][coluna] * 32;
	  intro1[i]+= nomes[6+(linha*8)][coluna] * 64;
	  intro1[i]+= nomes[7+(linha*8)][coluna] * 128;
	  coluna++;
	  if(coluna>83)
	  {
		  coluna=0;
		  linha++;
	  }
  }
  coluna=0;
  linha=0;
  for(int i=0; i<504; i++)
  {
	  intro2[i]= micros4411[0+(linha*8)][coluna] * 1;
  	  intro2[i]+= micros4411[1+(linha*8)][coluna] * 2;
	  intro2[i]+= micros4411[2+(linha*8)][coluna] * 4;
	  intro2[i]+= micros4411[3+(linha*8)][coluna] * 8;
	  intro2[i]+= micros4411[4+(linha*8)][coluna] * 16;
	  intro2[i]+= micros4411[5+(linha*8)][coluna] * 32;
	  intro2[i]+= micros4411[6+(linha*8)][coluna] * 64;
	  intro2[i]+= micros4411[7+(linha*8)][coluna] * 128;
	  coluna++;
	  if(coluna>83)
	  {
		  coluna=0;
		  linha++;
	  }
  }
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
