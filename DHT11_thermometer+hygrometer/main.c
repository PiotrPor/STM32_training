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
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
#include <stdio.h>
#include "i2c-lcd.h"
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

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void wait_microseconds(uint16_t how_long);
bool send_start_signal_to_DHT11();
bool read_bit_from_DHT11();
uint8_t read_one_byte_from_DHT11();
void make_decimal_number(uint8_t int_part, uint8_t dec_part, char* text);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void wait_microseconds(uint16_t how_long)
{
	TIM6->CNT = 0;
	while(TIM6->CNT < how_long)
	{
		//empty
	}
	return;
}

bool send_start_signal_to_DHT11()
{
	bool final_response=false;
	//
	HAL_GPIO_WritePin(ONEWIRE_GPIO_Port, ONEWIRE_Pin, 0);
	HAL_Delay(19);
	HAL_GPIO_WritePin(ONEWIRE_GPIO_Port, ONEWIRE_Pin, 1);
	wait_microseconds(70);
	if(!HAL_GPIO_ReadPin(ONEWIRE_GPIO_Port, ONEWIRE_Pin))
	{
		final_response = true;
	}
	while(!HAL_GPIO_ReadPin(ONEWIRE_GPIO_Port, ONEWIRE_Pin))
	{
		//wait until the low state ends
	}
	return final_response;
}

bool read_bit_from_DHT11()
{
	bool incoming = false;
	while(HAL_GPIO_ReadPin(ONEWIRE_GPIO_Port, ONEWIRE_Pin))
	{
		//wait until the initial high state goes by
	}
	while(!HAL_GPIO_ReadPin(ONEWIRE_GPIO_Port, ONEWIRE_Pin))
	{
		//wait through 50 micro_s of low state
	}
	wait_microseconds(35);
	if(HAL_GPIO_ReadPin(ONEWIRE_GPIO_Port, ONEWIRE_Pin))
	{
		incoming = true;
	}
	return incoming;
}

uint8_t read_one_byte_from_DHT11()
{
	uint8_t the_byte=0;
	bool bit=false;
	int i;
	for(i=1; i<=8; i++)
	{
		the_byte <<= 1;
		bit = read_bit_from_DHT11();
		if(bit)
		{
			the_byte |= 0b00000001;
		}
	}
	return the_byte;
}

void make_decimal_number(uint8_t int_part, uint8_t dec_part, char* text)
{
	uint32_t big_number;
	char integer_as_chars[2];
	char decimal_as_chars[3];
	itoa(int_part, integer_as_chars, 10);
	big_number = 100*dec_part;
	big_number /= 256;
	itoa(big_number, decimal_as_chars, 10);
	text[0] = integer_as_chars[0];
	text[1] = integer_as_chars[1];
	text[2] = '.';
	text[3] = decimal_as_chars[0];
	text[4] = decimal_as_chars[1];
	text[5] = decimal_as_chars[2];
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
  MX_TIM6_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start(&htim6);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  HAL_Delay(50);
  lcd_init();
  lcd_clear();
  HAL_Delay(50);
  lcd_put_cur(0,0);
  lcd_send_string("starting");
  //
  bool DHT11_response;
  uint8_t humidity_integer, humidity_decimal;
  uint8_t temperature_integer, temperature_decimal;
  uint8_t checksum, calculated_checksum;
  //
  char temperature_number_text[7];
  char humidity_number_text[7];
  char celsius_symbol[2] = {(char)223, 'C'};
  HAL_Delay(1500);
  while (1)
  {
      DHT11_response = send_start_signal_to_DHT11();
      if(DHT11_response)
      {
    	  humidity_integer = read_one_byte_from_DHT11();
    	  humidity_decimal = read_one_byte_from_DHT11();
    	  temperature_integer = read_one_byte_from_DHT11();
    	  temperature_decimal = read_one_byte_from_DHT11();
    	  checksum = read_one_byte_from_DHT11();
    	  //
    	  make_decimal_number(humidity_integer, humidity_decimal, humidity_number_text);
    	  make_decimal_number(temperature_integer, temperature_decimal, temperature_number_text);
    	  //
    	  calculated_checksum = 0;
    	  calculated_checksum += humidity_integer + humidity_decimal;
    	  calculated_checksum += temperature_integer + temperature_decimal;
    	  //
    	  if(checksum == calculated_checksum)
    	  {
    		  lcd_clear();
    		  HAL_Delay(50);
    		  lcd_put_cur(0,0);
    		  lcd_send_string(temperature_number_text);
    		  lcd_send_string(celsius_symbol);
    		  lcd_put_cur(1,0);
    		  lcd_send_string(humidity_number_text);
    		  lcd_send_string(" %");
    	  }
    	  else
    	  {
    		  lcd_clear();
    		  HAL_Delay(50);
    		  lcd_put_cur(0,0);
    		  lcd_send_string("bad checksum");
    	  }

      }
      else
      {
    	  lcd_clear();
    	  HAL_Delay(50);
    	  lcd_put_cur(0,0);
    	  lcd_send_string("no sensor :(");
      }
      HAL_Delay(1500); //wait 0.5 s before next display
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
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
