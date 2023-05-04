/* USER CODE BEGIN Header */
/**
  **************************
  * @file           : main.c
  * @brief          : Main program body
  **************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  **************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
#include <string.h>
#include "notsure.c"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/*enum modos
{
	LIGADO,
	DESLIGADO
}contagem,estado_ponto;
*/

typedef struct
{
	int atual;
	int anterior;
}info_botoes;

info_botoes botoes[2];
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TIMEOUT 1000
#define TAM_MSG 200
#define TAM_CMD 1
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
int num=0;
int aux=0;
uint8_t msg[TAM_MSG]="Bernardo Rodrigues da Silva - 4411\r\n";
uint8_t CMD[TAM_CMD];
int num_display[5]={ //catodo
//        GFEDCBA
		0b1110111, //caractere A
		0b1111001,
		0b0000110,
		0b0111111,
		0b0111110
};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void LIMPA_STRING()
{
	if (msg[0]!='\0')
	{
	  HAL_UART_Transmit(&huart2, msg, strlen(msg),TIMEOUT);
	  msg[0]='\0';
	}
}
void SET_DISPLAY(int num)
{
	//Limpando o display

	HAL_GPIO_WritePin(GPIOC,0b1111111,0);

	//fazer um switch-case de 0 a 9 para cada número
	switch(num)
	{
		case 0:
			HAL_GPIO_WritePin(GPIOC,num_display[0],1);
			break;
		case 1:
			HAL_GPIO_WritePin(GPIOC,num_display[1],1);
			break;
		case 2:
			HAL_GPIO_WritePin(GPIOC,num_display[2],1);
			break;
		case 3:
			HAL_GPIO_WritePin(GPIOC,num_display[3],1);
			break;
		case 4:
			HAL_GPIO_WritePin(GPIOC,num_display[4],1);
			break;
	}
}
void SET_PONTO(int freq)
{
	//Limpando o display

	//fazer um switch-case de 0 a 9 para cada número
	switch(freq)
	{
		case 0:
			__HAL_TIM_SET_AUTORELOAD(&htim9,(1000/2));
			break;
		case 1:
			__HAL_TIM_SET_AUTORELOAD(&htim9,(143/2));
			break;
		case 2:
			__HAL_TIM_SET_AUTORELOAD(&htim9,(55/2));
			break;
		case 3:
			__HAL_TIM_SET_AUTORELOAD(&htim9,(5/2));
			break;
	}
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	CMD[0]=toupper(CMD[0]);
	switch(CMD[0])
	{
		case 'A':
			strcpy(msg,"\n VOGAL 'A' SENDO EXIBIDA\r\n");
			HAL_UART_Transmit(&huart2, msg, strlen(msg),TIMEOUT);
			num=0;
			break;
		case 'E':
			strcpy(msg,"\n VOGAL 'E' SENDO EXIBIDA\r\n");
			HAL_UART_Transmit(&huart2, msg, strlen(msg),TIMEOUT);
			num=1;
			break;
		case 'I':
			num=2;
			strcpy(msg,"\n VOGAL 'I' SENDO EXIBIDA\r\n");
			HAL_UART_Transmit(&huart2, msg, strlen(msg),TIMEOUT);
			break;
		case 'O':
			strcpy(msg,"\n VOGAL 'O' SENDO EXIBIDA\r\n");
			HAL_UART_Transmit(&huart2, msg, strlen(msg),TIMEOUT);
			num=3;
			break;
		case 'U':
			strcpy(msg,"\n VOGAL 'U' SENDO EXIBIDA\r\n");
			HAL_UART_Transmit(&huart2, msg, strlen(msg),TIMEOUT);
			num=4;
			break;
		case '+':
			strcpy(msg,"\n VOGAL SENDO INCREMENTADA\r\n");
			HAL_UART_Transmit(&huart2, msg, strlen(msg),TIMEOUT);
			num++;
			break;
		case '-':
			strcpy(msg,"\n VOGAL SENDO DECREMENTADA\r\n");
			HAL_UART_Transmit(&huart2, msg, strlen(msg),TIMEOUT);
			num--;
			break;

		default :
		strcpy(msg,"\nParametro invalido\r\n");
	}
		HAL_UART_Receive_IT(&huart2,CMD,TAM_CMD);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM11)//7 segmento
	{
		//nao sei
	}
	if(htim->Instance == TIM10)//debounce
	{

		botoes[0].atual=HAL_GPIO_ReadPin(BOTAO_0_GPIO_Port, BOTAO_0_Pin);
		botoes[1].atual=HAL_GPIO_ReadPin(BOTAO_1_GPIO_Port, BOTAO_1_Pin);

		if(botoes[0].atual!=botoes[0].anterior)
		{
			botoes[0].anterior=botoes[0].atual;
			if(botoes[0].atual)
			{
				num++;
				switch(num)
				{
					case 0:
						strcpy(msg,"\n VOGAL 'A' SENDO EXIBIDA\r\n");
						HAL_UART_Transmit(&huart2, msg, strlen(msg),TIMEOUT);
						break;
					case 1:
						strcpy(msg,"\n VOGAL 'E' SENDO EXIBIDA\r\n");
						HAL_UART_Transmit(&huart2, msg, strlen(msg),TIMEOUT);
						break;
					case 2:
						strcpy(msg,"\n VOGAL 'I' SENDO EXIBIDA\r\n");
						HAL_UART_Transmit(&huart2, msg, strlen(msg),TIMEOUT);
						break;
					case 3:
						strcpy(msg,"\n VOGAL 'O' SENDO EXIBIDA\r\n");
						HAL_UART_Transmit(&huart2, msg, strlen(msg),TIMEOUT);
						break;
					case 4:
						strcpy(msg,"\n VOGAL 'U' SENDO EXIBIDA\r\n");
						HAL_UART_Transmit(&huart2, msg, strlen(msg),TIMEOUT);
						break;
				}
			}
		}
		if(botoes[1].atual!=botoes[1].anterior)
		{
			botoes[1].anterior=botoes[1].atual;
			if(botoes[1].atual)
			{
				aux++;
				switch(aux)
				{
					case 0:
						strcpy(msg,"\n FREQ '1Hz' SENDO EXIBIDA\r\n");
						HAL_UART_Transmit(&huart2, msg, strlen(msg),TIMEOUT);
						break;
					case 1:
						strcpy(msg,"\n FREQ '7Hz' SENDO EXIBIDA\r\n");
						HAL_UART_Transmit(&huart2, msg, strlen(msg),TIMEOUT);
						break;
					case 2:
						strcpy(msg,"\n FREQ '18Hz' SENDO EXIBIDA\r\n");
						HAL_UART_Transmit(&huart2, msg, strlen(msg),TIMEOUT);
						break;
					case 3:
						strcpy(msg,"\n PONTO DESLIGADO  SENDO EXIBIDo\r\n");
						HAL_UART_Transmit(&huart2, msg, strlen(msg),TIMEOUT);
						break;
				}
			}
		}
	}
	if(htim->Instance == TIM9)//ponto
	{
		if(aux==3)
		{
			HAL_GPIO_WritePin(SEG_PONTO_GPIO_Port, SEG_PONTO_Pin,GPIO_PIN_RESET);
		}
		else
		{
			HAL_GPIO_TogglePin(SEG_PONTO_GPIO_Port, SEG_PONTO_Pin);
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
  MX_TIM9_Init();
  MX_TIM10_Init();
  MX_TIM11_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start_IT(&htim11);
  HAL_TIM_Base_Start_IT(&htim10);
  HAL_TIM_Base_Start_IT(&htim9);

  HAL_UART_Transmit(&huart2, msg, strlen(msg),TIMEOUT);
  msg[0]='\0';
  HAL_UART_Receive_IT(&huart2,CMD,TAM_CMD);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	  SET_DISPLAY(num);
	  LIMPA_STRING();

	  if(num==5)
		  num=0;
	  if(num==-1)
		  num=4;
	  if(aux==4)
		  aux=0;

	  switch(aux)
	  {
	  case 0:
		  __HAL_TIM_SET_AUTORELOAD(&htim9,(1000/2));
		  break;
	  case 1:
		  __HAL_TIM_SET_AUTORELOAD(&htim9,(143/2));
		  break;
	  case 2:
		  __HAL_TIM_SET_AUTORELOAD(&htim9,(55/2));
		  break;
	  case 3:
		  __HAL_TIM_SET_AUTORELOAD(&htim9,(1000/2));
		  break;
	  }
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
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 64;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
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
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV4;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
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
