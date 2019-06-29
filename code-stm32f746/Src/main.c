/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "comm_buffer.h"
#include "debug_trace.h"
#include "timer_sched.h"
#include "neural_network.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define UART_BUFFER_SIZE 256

#define DEBUG_PORT GPIOG
#define DEBUG_PIN GPIO_PIN_7

/* Uncomment to overclock to 295 MHz */
//#define OVERCLOCK

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

UART_HandleTypeDef huart6;

/* USER CODE BEGIN PV */
/* Those weight were calculated by training the model */
double weights[] = {
	9.67299303,
	-0.2078435,
	-4.62963669};

/* Those are the weights for the NN with the hidden layer */
double weights_1[32][3] = {
		{ 1.5519611 ,  0.65860842, -0.85158313},
		{ 1.06265208,  0.31949065, -0.13750623},
		{-2.30455417,  0.30591339,  0.7240955 },
		{ 0.30877321,  0.13609545,  0.06030529},
		{-1.17039621,  0.37650795,  0.68144084},
		{-0.69348733,  0.60220631,  0.28797794},
		{ 0.40589165, -0.18831817,  0.35705378},
		{-0.05932593,  0.59004868, -0.1701374 },
		{ 1.392689  ,  0.70259631, -0.74218012},
		{ 0.48329391,  0.60207043,  0.38431145},
		{-0.95295493,  0.07404055,  0.3302735 },
		{ 2.10166106,  0.39611358, -1.00823685},
		{-0.08448525, -0.10317019,  0.53841852},
		{ 2.540211  ,  0.05323626, -1.06332893},
		{-1.84539917,  0.48765352,  0.5685712 },
		{-0.01802026,  0.18219485,  0.62404852},
		{ 0.92427558,  0.04736742,  0.12561239},
		{ 1.08965851, -0.0801783 , -0.08860555},
		{-1.82176436, -0.20573879,  0.780304  },
		{ 0.45220284,  0.4997675 , -0.18653686},
		{ 2.14035487, -0.03879152, -0.80268326},
		{ 1.82177898, -0.11746983, -0.36502932},
		{ 0.62309348,  0.30696019, -0.13735078},
		{-0.05199235, -0.05282913,  0.91758557},
		{ 0.29694869,  0.54693956,  0.69834386},
		{ 0.39180283,  0.07674308,  0.54732749},
		{-1.72233784,  0.36036508,  0.28150989},
		{-1.70252288,  0.30217376,  0.69514718},
		{ 0.78278019, -0.07366525, -0.16997775},
		{ 1.28860736,  0.17580282, -0.17742198},
		{-0.47323461,  0.48374213,  0.08422625},
		{ 1.49427898,  0.191315  , -0.6043115 }
};

double weights_2[] = {
		  1.45800076,
		  0.70967606,
		 -2.68516009,
		  0.02268129,
		 -1.49619033,
		 -0.91787325,
		  0.09344477,
		 -0.29547836,
		  1.28547469,
		  0.04046954,
		 -1.29961045,
		  2.02908422,
		 -0.42753493,
		  2.49285692,
		 -2.11993655,
		 -0.59068471,
		  0.62114759,
		  0.76446439,
		 -2.22804807,
		  0.25244996,
		  1.9591224 ,
		  1.53351039,
		  0.37624935,
		 -0.74102723,
		 -0.48484892,
		 -0.2683768 ,
		 -2.020174  ,
		 -1.99787033,
		  0.54688533,
		  0.90035289,
		 -0.67413096,
		  1.31408841,
};

/* Those are the inputs to test and benchmark */
double inputs[8][3] = {
		{0, 0, 0},
		{0, 0, 1},
		{0, 1, 0},
		{0, 1, 1},
		{1, 0, 0},
		{1, 0, 1},
		{1, 1, 0},
		{1, 1, 1},
};

DECLARE_COMM_BUFFER(dbg_uart, UART_BUFFER_SIZE, UART_BUFFER_SIZE);

__IO uint32_t glb_tmr_1ms = 0;
__IO uint8_t tmp_rx = 0;
__IO uint32_t rx_timeout = 0;

uint32_t trace_levels;

/* Benchmark timer object */
struct obj_timer_t * benchmark_timer;

/* Create the list head for the timer */
static LIST_HEAD(obj_timer_list);

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART6_UART_Init(void);
/* USER CODE BEGIN PFP */
void dbg_uart_parser(uint8_t *buffer, size_t bufferlen, uint8_t sender);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void test_neural_network()
{

	for (int i=0; i<8; i++) {
		double result = nn_predict(inputs[i], weights, 3);
		TRACE(("result: %f\n", result));
	}
	TRACE(("\n"));
}

static inline void toggle_debug_pin(void)
{
	/* First toggle */
	DEBUG_PORT->ODR |= DEBUG_PIN;
	DEBUG_PORT->ODR &= ~DEBUG_PIN;

	/* Second toggle */
	DEBUG_PORT->ODR |= DEBUG_PIN;
	DEBUG_PORT->ODR &= ~DEBUG_PIN;
}

void benchmark_neural_network()
{
	toggle_debug_pin();
	/* Predict */
	DEBUG_PORT->ODR |= DEBUG_PIN;
  	double output = sigmoid(dot(inputs[0], weights, 3));
	DEBUG_PORT->ODR &= ~DEBUG_PIN;
	TRACE(("DONE: %f\n", output));
}

/**
 *  @brief This NN has a hidden layer with 32 nodes.
 * 	This function will print all the outputs in order
 *  to verify that are valid
 */
void test_neural_network2()
{
	double layer[32];

	for (int k=0; k<8; k++) {
		for (int i=0; i<32; i++) {
			layer[i] = sigmoid(dot(inputs[k], weights_1[i], 3));
		}
		double output = sigmoid(dot(layer, weights_2, 32));
		TRACE(("output[%d]: %f\n", k, output));
	}
}

/**
 * @brief This NN has a hidden layer with 32 nodes.
 * This function will just do a forward run
 */
void benchmark_neural_network2()
{
	double layer[32];

	DEBUG_PORT->ODR |= DEBUG_PIN;
	for (int i=0; i<32; i++) {
		layer[i] = sigmoid(dot(inputs[0], weights_1[i], 3));
	}
	double output = sigmoid(dot(layer, weights_2, 32));
	DEBUG_PORT->ODR &= ~DEBUG_PIN;
	TRACE(("DONE...%f\n", output));
}

void main_loop()
{
	if (glb_tmr_1ms) {
		glb_tmr_1ms = 0;

		mod_timer_polling(&obj_timer_list);

		if (rx_timeout) rx_timeout--;
		if (rx_timeout == 1) {
			rx_timeout = 0;
			dbg_uart.rx_buffer[dbg_uart.rx_ptr_in] = 0; // terminate string
			TRACE(("Received: %s\n", dbg_uart.rx_buffer));
			dbg_uart_parser(dbg_uart.rx_buffer, dbg_uart.rx_ptr_in, 0);
			dbg_uart.rx_ptr_in = 0;
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
  

  /* Enable I-Cache---------------------------------------------------------*/
  SCB_EnableICache();

  /* Enable D-Cache---------------------------------------------------------*/
  SCB_EnableDCache();

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
  MX_USART6_UART_Init();
  /* USER CODE BEGIN 2 */
	trace_levels_set(
			0
			| TRACE_LEVEL_DEFAULT
			,1);

  HAL_UART_Receive_IT(&huart6, (uint8_t*) &tmp_rx, 1);

  TRACE(("Program started...\n"));
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
//	  HAL_GPIO_TogglePin(GPIOG, GPIO_PIN_7);
	  main_loop();
//	  HAL_Delay(200);
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
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Configure the main internal regulator output voltage 
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 216;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;

#ifdef OVERCLOCK
  RCC_OscInitStruct.PLL.PLLN = 295;	// Overclock
#endif

  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Activate the Over-Drive mode 
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

#ifdef OVERCLOCK
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV8;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV4;
#endif

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART6;
  PeriphClkInitStruct.Usart6ClockSelection = RCC_USART6CLKSOURCE_PCLK2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART6_UART_Init(void)
{

  /* USER CODE BEGIN USART6_Init 0 */

  /* USER CODE END USART6_Init 0 */

  /* USER CODE BEGIN USART6_Init 1 */

  /* USER CODE END USART6_Init 1 */
  huart6.Instance = USART6;
  huart6.Init.BaudRate = 115200;
  huart6.Init.WordLength = UART_WORDLENGTH_8B;
  huart6.Init.StopBits = UART_STOPBITS_1;
  huart6.Init.Parity = UART_PARITY_NONE;
  huart6.Init.Mode = UART_MODE_TX_RX;
  huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart6.Init.OverSampling = UART_OVERSAMPLING_16;
  huart6.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart6.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart6) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART6_Init 2 */

  /* USER CODE END USART6_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOG, GPIO_PIN_7, GPIO_PIN_RESET);

  /*Configure GPIO pin : PG7 */
  GPIO_InitStruct.Pin = GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */


void dbg_uart_parser(uint8_t *buffer, size_t bufferlen, uint8_t sender)
{
	buffer[bufferlen] = 0;
	if (!strncmp((char*) buffer, "TEST=", 5)) {
		/* Get mode */
		uint8_t mode = atoi((const char*) &buffer[5]);

		TRACE(("Running nn test mode: %d\n\n", mode));
		switch(mode) {
			case 1:
				test_neural_network();
				break;
			case 2:
				test_neural_network2();
				break;
			default:
				TRACE(("Available modes: 1-2\n"));
		}
		TRACE(("\nFinished.\n----------\n"));
	}
	else if (!strncmp((char*) buffer, "START=", 6)) {
		/* Get mode */
		uint8_t mode = atoi((const char*) &buffer[6]);

		TRACE(("Starting benchmark mode %d...\n\n", mode));
		switch(mode) {
			case 1:
				benchmark_timer = mod_timer_add(NULL, 2000, (void*) &benchmark_neural_network, &obj_timer_list);
				break;
			case 2:
				benchmark_timer = mod_timer_add(NULL, 2000, (void*) &benchmark_neural_network2, &obj_timer_list);
				break;
			default:
				TRACE(("Available modes: 1-2\n"));
		}
	}
	else if (!strncmp((char*) buffer, "STOP", 4)) {
		TRACE(("Stoping benchmark mode...\n\n"));
		mod_timer_del(benchmark_timer, &obj_timer_list);
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle)
{
	/* Receive byte from uart */
	rx_timeout = 10;
	dbg_uart.rx_buffer[0xFF & dbg_uart.rx_ptr_in] = tmp_rx;
	dbg_uart.rx_ptr_in++;
	HAL_UART_Receive_IT(&huart6, (uint8_t*) &tmp_rx, 1);
}


int __io_putchar(int ch)
{
	HAL_UART_Transmit(&huart6, (uint8_t*) &ch, 1, 10);
	return ch;
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
