/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include "NRF.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define X_MIN 2040
#define X_MAX 4040
#define X_MIN_N 0
#define X_MAX_N 1980
#define Y_MIN 1930
#define Y_MAX 4040
#define Y_MIN_N 0
#define Y_MAX_N 1880
#define PWM_MIN 2800
#define PWM_MAX 4200
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi2;

TIM_HandleTypeDef htim1;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
int a = 0;
int TX_EMPTY = 5;
int DataInFifo = 5;
uint8_t turn = 0;
int8_t omega = 0;

uint8_t RxAddress[5] = {0xEE,0xDD,0xCC,0xBB,0xAA};
uint8_t RxData[4];
uint8_t st = 0;

uint16_t point[2] = {1,1};

uint8_t rf;
uint8_t cfg;

uint32_t pwm = 0;

int32_t left_pwm  = 0;
int32_t right_pwm = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI2_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM1_Init(void);
/* USER CODE BEGIN PFP */
uint32_t map_u16_to_u32(uint16_t v, uint16_t in_min, uint16_t in_max, uint32_t out_min, uint32_t out_max);

void drive_arcade(uint16_t x, uint16_t y);
void drive_simple(uint16_t x, uint16_t y);

int in_range_u16(uint16_t v, uint16_t lo, uint16_t hi);

// Motor direction + stop (YOUR wiring on GPIOB)
void motor_left_forward(void);
void motor_left_backward(void);
void motor_left_stop(void);

void motor_right_forward(void);
void motor_right_backward(void);
void motor_right_stop(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_SPI2_Init();
  MX_USART2_UART_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */
	NRF24_Init();
	NRF24_RxMode(RxAddress, 76);
	
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);

//	st = nrf24_ReadReg(STATUS);
//	if (st == 0x00 || st == 0xFF) {
//			// SPI/power/wiring problem
//			while (1) {
//					HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
//					HAL_Delay(100);   // fast blink = SPI dead
//			}
//	} else {
//			while (1) {
//					HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
//					HAL_Delay(1000);   // slow blink = SPI OK
//			}
//	}
	rf = nrf24_ReadReg(RF_SETUP);
	cfg = nrf24_ReadReg(CONFIG);

	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);
	htim1.Instance->CCR2 = 2200;
	
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET);
	htim1.Instance->CCR1 = 2200;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		uint8_t got_packet = 0;

		while ((nrf24_ReadReg(FIFO_STATUS) & (1<<0)) == 0)   // while RX not empty
		{
				NRF24_Receive(RxData);
				got_packet = 1;
		}

		if (got_packet)
		{
				point[0] = (uint16_t)((RxData[1] << 8) | RxData[0]);
				point[1] = (uint16_t)((RxData[3] << 8) | RxData[2]);
		}
		
		drive_arcade(point[0], point[1]);

		HAL_Delay(10);
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

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
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
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

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 4199;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4|LD2_Pin|GPIO_PIN_6, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PA4 LD2_Pin PA6 */
  GPIO_InitStruct.Pin = GPIO_PIN_4|LD2_Pin|GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB1 PB13 PB14 PB15 */
  GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
// Map v from [in_min..in_max] to [out_min..out_max], clamped
uint32_t map_u16_to_u32(uint16_t v,
                        uint16_t in_min, uint16_t in_max,
                        uint32_t out_min, uint32_t out_max)
{
    if (v <= in_min) return out_min;
    if (v >= in_max) return out_max;
    return out_min + ((uint32_t)(v - in_min) * (out_max - out_min)) / (uint32_t)(in_max - in_min);
}

int in_range_u16(uint16_t v, uint16_t lo, uint16_t hi)
{
    return (v >= lo && v <= hi);
}


// ===== LEFT MOTOR (TIM1 CH1 = CCR1), direction pins: PB1, PB15 =====
void motor_left_forward(void)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1,  GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);
}

void motor_left_backward(void)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1,  GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET);
}

void motor_left_stop(void)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1,  GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);
    htim1.Instance->CCR2 = 0;
}


// ===== RIGHT MOTOR (TIM1 CH2 = CCR2), direction pins: PB14, PB13 =====
void motor_right_forward(void)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET);
}

void motor_right_backward(void)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET);
}

void motor_right_stop(void)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET);
    htim1.Instance->CCR1 = 0;
}


// ===== Joystick logic using the motor_* functions =====
void drive_simple(uint16_t x, uint16_t y)
{
    // Stop
    if (in_range_u16(x, X_MAX_N, X_MIN) && in_range_u16(y, Y_MAX_N, Y_MIN))
    {
        motor_left_stop();
        motor_right_stop();
        return;
    }

    // Forward
    if (x > X_MIN)
    {
        uint32_t pwm = map_u16_to_u32(x, X_MIN, X_MAX, PWM_MIN, PWM_MAX);

        motor_left_forward();
        motor_right_forward();
        htim1.Instance->CCR2 = pwm;
        htim1.Instance->CCR1 = pwm;
        return;
    }

    // Backward
    if (x < X_MAX_N)
    {
        uint16_t amount = (uint16_t)(X_MAX_N - x); // 0..2000
        uint32_t pwm = map_u16_to_u32(amount, X_MIN_N, X_MAX_N, PWM_MIN, PWM_MAX);

        motor_left_backward();
        motor_right_backward();
        htim1.Instance->CCR2 = pwm;
        htim1.Instance->CCR1 = pwm;
        return;
    }

    // Turn-in-place only when x is in stop band

    // If y < 1900: RIGHT motor stopped, LEFT motor PWM depends on (1900 - y)
    if (in_range_u16(x, 2000, 2020) && y < Y_MAX_N)
    {
				turn++;
        uint16_t amount = (uint16_t)(Y_MAX_N - y); // 0..1900

				uint32_t pwm = (amount == 0) ? 0 : map_u16_to_u32(amount, Y_MIN_N, Y_MAX_N, PWM_MIN, PWM_MAX);

        motor_left_forward();
        motor_right_stop();
				htim1.Instance->CCR2 = pwm;
        // CCR2 already set to 0 by motor_right_stop()
        return;
    }

    // If y > 1915: LEFT motor stopped, RIGHT motor PWM depends on (y - 1915)
    if (in_range_u16(x, 2000, 2020) && y > Y_MIN)
    {
//        uint16_t amount = (uint16_t)(y - Y_MIN); // 0..(4040-1930)
        uint32_t pwm = map_u16_to_u32(y, Y_MIN, Y_MAX, PWM_MIN, PWM_MAX);

        motor_left_stop();
        motor_right_forward();
        htim1.Instance->CCR1 = pwm;
        // CCR1 already set to 0 by motor_left_stop()
        return;
    }

    // Fallback stop
    motor_left_stop();
    motor_right_stop();
}


void drive_arcade(uint16_t x, uint16_t y)
{
		int forward = 0;
    uint32_t base = 0;
	
    // Stop deadband
    if (in_range_u16(x, X_MAX_N, X_MIN) && in_range_u16(y, Y_MAX_N, Y_MIN)){
        motor_left_stop();
        motor_right_stop();
        return;
    }

    // ---------- 1) BASE SPEED from X ----------

    if (x > X_MIN) {
        forward = 1;
        base = map_u16_to_u32(x, X_MIN, X_MAX, PWM_MIN, PWM_MAX);
    } else if (x < X_MAX_N) {
        forward = 0;
        uint16_t amt = (uint16_t)(X_MAX_N - x); // 0...X_MAX_N
        base = map_u16_to_u32(amt, X_MIN_N, X_MAX_N, PWM_MIN, PWM_MAX);
    } else {
        // x in stop band => base 0 (not moving forward/back)
        base = 0;
    }

    // ---------- 2) TURN DELTA (omega) from Y ----------
    int32_t omega = 0;

    // y < 1900 => turn RIGHT (omega positive)
    if (y < Y_MAX_N) {
        uint16_t amt = (uint16_t)(Y_MAX_N - y);                 // 0..Y_MAX_N
        omega = (int32_t)map_u16_to_u32(amt, Y_MIN_N, Y_MAX_N, 0, 1400);
    }
    // y > 1915 => turn LEFT (omega negative)
    else if (y > Y_MIN) {
				omega = -(int32_t)map_u16_to_u32(y, Y_MIN, Y_MAX, 0, 1400);
    }
		omega = (omega * 700) / 1000;
    // ---------- 3) MIX ----------
    left_pwm  = (int32_t)base + omega;
    right_pwm = (int32_t)base - omega;

    // Clamp to [0..4200]
    if (left_pwm < 0) left_pwm = 0;
    if (right_pwm < 0) right_pwm = 0;
    if (left_pwm > PWM_MAX) left_pwm = PWM_MAX;
    if (right_pwm > PWM_MAX) right_pwm = PWM_MAX;

    // ---------- 4) APPLY DIRECTION + PWM ----------

    if (base == 0 && omega != 0)
    {
				// Minimum spin rule: if nonzero but below 2500 -> bump to 2500
				if (left_pwm > 0 && left_pwm < PWM_MIN) left_pwm = PWM_MIN + omega;
				if (right_pwm > 0 && right_pwm < PWM_MIN) right_pwm = PWM_MIN - omega;
			
        if (omega > 0)
        {
            motor_left_forward();
            motor_right_backward();
        }
        else
        {
            motor_left_backward();
            motor_right_forward();
        }

        htim1.Instance->CCR2 = (uint32_t)left_pwm;   // left
        htim1.Instance->CCR1 = (uint32_t)right_pwm;  // right
        return;
    }

    if (base == 0 && omega == 0)
    {
        motor_left_stop();
        motor_right_stop();
        return;
    }

    if (forward)
    {
        motor_left_forward();
        motor_right_forward();
    }
    else
    {
        motor_left_backward();
        motor_right_backward();
    }

    // CCR2 = LEFT, CCR1 = RIGHT (your corrected wiring)
    htim1.Instance->CCR2 = (uint32_t)left_pwm;
    htim1.Instance->CCR1 = (uint32_t)right_pwm;
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
