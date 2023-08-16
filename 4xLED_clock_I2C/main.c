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
#include "i2c.h"
#include "spi.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
const unsigned int DS3231_address = 0xd0;

const unsigned char big_digit_shapes[10][8]={
		{0,14,17,17,17,17,17,14},
		{0,4,12,4,4,4,4,14},
		{0,14,17,1,2,4,8,31},
		{0,31,2,4,2,1,17,14},
		{0,2,6,10,18,31,2,2},
		{0,31,16,30,1,1,17,14},
		{0,6,8,16,30,17,17,14},
		{0,31,1,2,4,8,8,8},
		{0,14,17,17,14,17,17,14},
		{0,14,17,17,15,1,2,12}
};
//adapted to feature a half of a colon
const unsigned char middle_left_digit_shapes[10][8]={
		{0,56,69,69,68,69,69,56},
		{0,16,49,17,16,17,17,56},
		{0,56,69,5,8,17,33,124},
		{0,124,9,17,8,5,69,56},
		{0,8,25,41,72,125,9,8},
		{0,124,65,121,4,5,69,56},
		{0,24,33,65,120,69,69,56},
		{0,124,5,9,16,33,33,32},
		{0,56,69,69,56,69,69,56},
		{0,56,69,69,60,5,9,48}
};
//adapted to feature a half of a colon
const unsigned char middle_right_digit_shapes[10][8]={
		{0,14,145,145,17,145,145,14},
		{0,4,140,132,4,132,132,14},
		{0,14,145,129,2,132,136,31},
		{0,31,130,132,2,129,145,14},
		{0,2,134,138,18,159,130,2},
		{0,31,144,158,1,129,145,14},
		{0,6,136,144,30,145,145,14},
		{0,31,129,130,4,136,136,8},
		{0,14,145,145,14,145,145,14},
		{0,14,145,145,15,129,130,12}
};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void send_eight_bytes_2(unsigned char eight[]); //takes 8-element array
void send_initialisation_signals();
void clear_all_screens();
void show_four_digits(unsigned int d1, unsigned int d2, unsigned int d3, unsigned int d4);
void show_4_digits_like_clock(unsigned int d1, unsigned int d2, unsigned int d3, unsigned int d4, bool ShouldShowColon);
uint8_t BinToBcd(int val);
int BcdToDec(uint8_t val);
void decode_BCD_array(uint8_t arr[]);
void encode_array_to_BCD(uint8_t arr[]);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void send_eight_bytes_2(unsigned char eight[])
{
	HAL_GPIO_WritePin(LED4_CS_GPIO_Port, LED4_CS_Pin, 0);
	HAL_SPI_Transmit(&hspi1, eight, 8, 100);
	HAL_GPIO_WritePin(LED4_CS_GPIO_Port, LED4_CS_Pin, 1);
	return;
}

void send_initialisation_signals()
{
	unsigned char sig[5][2] = {{12,0},{9,0},{11,7},{10,0},{12,1}};
	unsigned char for_them[8];
	int i, j;
	for(i=0; i<5; i++)
	{
		for(j=0; j<8; j+=2)
		{
			for_them[j] = sig[i][0];
			for_them[j+1] = sig[i][1];
		}
		send_eight_bytes_2(for_them);
	}
	return;
}

void clear_all_screens()
{
	int i;
	unsigned char clearing_array[8] = {0,0,0,0,0,0,0,0};
	for(i=1; i<=8; i++)
	{
		clearing_array[0] = i;
		clearing_array[2] = i;
		clearing_array[4] = i;
		clearing_array[6] = i;
		HAL_GPIO_WritePin(LED4_CS_GPIO_Port, LED4_CS_Pin, 0);
		send_eight_bytes_2(clearing_array);
		HAL_GPIO_WritePin(LED4_CS_GPIO_Port, LED4_CS_Pin, 1);
	}
	return;
}

void show_four_digits(unsigned int d1, unsigned int d2, unsigned int d3, unsigned int d4)
{
	clear_all_screens();
	uint8_t array_for_row[8];
	int i;
	for(i=1; i<=8; i++)
	{
		array_for_row[0] = i;
		array_for_row[2] = i;
		array_for_row[4] = i;
		array_for_row[6] = i;
		array_for_row[1] = big_digit_shapes[d1][i-1];
		array_for_row[3] = big_digit_shapes[d2][i-1];
		array_for_row[5] = big_digit_shapes[d3][i-1];
		array_for_row[7] = big_digit_shapes[d4][i-1];
		send_eight_bytes_2(array_for_row);
	}
	return;
}

void show_4_digits_like_clock(unsigned int d1, unsigned int d2, unsigned int d3, unsigned int d4, bool ShouldShowColon)
{
	clear_all_screens();
	uint8_t array_for_row[8];
	int i;
	for(i=1; i<=8; i++)
	{
		array_for_row[0] = i;
		array_for_row[2] = i;
		array_for_row[4] = i;
		array_for_row[6] = i;
		array_for_row[1] = big_digit_shapes[d1][i-1];
		array_for_row[7] = big_digit_shapes[d4][i-1];
		if(ShouldShowColon)
		{
			array_for_row[3] = middle_left_digit_shapes[d2][i-1];
			array_for_row[5] = middle_right_digit_shapes[d3][i-1];
		}
		else
		{
			array_for_row[3] = big_digit_shapes[d2][i-1];
			array_for_row[5] = big_digit_shapes[d3][i-1];
		}
		send_eight_bytes_2(array_for_row);
	}
	return;
}

uint8_t BinToBcd(int val)
{
	return (uint8_t)( (val/10*16) + (val%10) );
}

int BcdToDec(uint8_t val)
{
	return (int)( (val/16*10) + (val%16) );
}

void decode_BCD_array(uint8_t arr[])
{
	arr[0] = BcdToDec(arr[0]);
	arr[1] = BcdToDec(arr[1]);
	arr[2] = BcdToDec(arr[2]);
	return;
}

void encode_array_to_BCD(uint8_t arr[])
{
	arr[0] = BinToBcd(arr[0]);
	arr[1] = BinToBcd(arr[1]);
	arr[2] = BinToBcd(arr[2]);
	return;
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
  MX_I2C2_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  send_initialisation_signals();
  clear_all_screens();
    //
  bool should_colon_be_shown = false;
  uint8_t starting_time[3] = {33,22,11}; //seconds,minutes,hours
  uint8_t read_time_as_array[3]; //seconds,minutes,hours
  unsigned int timedigit[4];
  encode_array_to_BCD(starting_time);
  //HAL_I2C_Mem_Write(&hi2c2, DS3231_address, 0x00, 1, starting_time, 3, 1000);
  while (1)
  {
	  HAL_I2C_Mem_Read(&hi2c2, DS3231_address, 0x00, 1, read_time_as_array, 3, 1000);
	  decode_BCD_array(read_time_as_array);
	  if(read_time_as_array[0]%2 == 1)
	  {
	     HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, 1);
	     should_colon_be_shown = true;
	  }
	  else
	  {
	     HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, 0);
	     should_colon_be_shown = false;
	  }
	  //
	  timedigit[0] = read_time_as_array[2]/10;
	  timedigit[1] = read_time_as_array[2]%10;
	  timedigit[2] = read_time_as_array[1]/10;
	  timedigit[3] = read_time_as_array[1]%10;
	  show_4_digits_like_clock(timedigit[0], timedigit[1], timedigit[2], timedigit[3], should_colon_be_shown);
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
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
