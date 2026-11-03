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
#include "usart.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define BREAKFAST_START       15000
#define BREAKFAST_TIMEOUT    25000
#define BREAKFAST_MISSED     30000

#define DINNER_START         40000
#define DINNER_TIMEOUT       50000
#define DINNER_MISSED        55000

#define SYSTEM_END           60000

#define DEBOUNCE_TIME           50

#define LED_GREEN_PORT GPIOC
#define LED_GREEN_PIN  GPIO_PIN_8

#define LED_YELLOW_PORT GPIOC
#define LED_YELLOW_PIN  GPIO_PIN_9

#define LED_RED_PORT GPIOC
#define LED_RED_PIN  GPIO_PIN_10

#define BUTTON_PORT GPIOC
#define BUTTON_PIN  GPIO_PIN_13
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
volatile uint32_t system_ms = 0;

volatile uint32_t button_time = 0;
volatile uint8_t button_event = 0;

volatile uint32_t last_button_time = 0;
volatile uint8_t debounce_initialized = 0;

typedef enum
{
    RESULT_PENDING = 0,
    RESULT_ON_TIME,
    RESULT_LATE,
    RESULT_MISSED
} MedicationResult;

typedef enum
{
    STATE_WAIT = 0,
    STATE_BREAKFAST_REMINDER,
    STATE_BREAKFAST_TIMEOUT,
    STATE_BREAKFAST_MISSED,
    STATE_DINNER_REMINDER,
    STATE_DINNER_TIMEOUT,
    STATE_DINNER_MISSED,
    STATE_FINISHED
} SystemState;

MedicationResult breakfast_result = RESULT_PENDING;
MedicationResult dinner_result = RESULT_PENDING;

SystemState current_state = STATE_WAIT;

uint8_t final_output_done = 0;

char uart_buffer[200];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void LED_All_Off(void);
void LED_Green_On(void);
void LED_Yellow_On(void);
void LED_Red_On(void);

void Process_Button_Event(void);
void Process_State_Machine(void);

void UART_Send(char *message);
void UART_Final_Report(void);

const char* Get_Result_Name(MedicationResult result);
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
  MX_LPUART1_UART_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  LED_All_Off();

 system_ms = 0;

 breakfast_result = RESULT_PENDING;
 dinner_result = RESULT_PENDING;

 current_state = STATE_WAIT;

 final_output_done = 0;


 HAL_TIM_Base_Start_IT(&htim2);

 UART_Send("\r\n");
 UART_Send("================================\r\n");
 UART_Send(" Smart Medicine Reminder System\r\n");
 UART_Send(" System Started!\r\n");
 UART_Send("================================\r\n");

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  /* USER CODE END WHILE */

	   /* USER CODE BEGIN 3 */
	   Process_Button_Event();

	   Process_State_Machine();
	   /* USER CODE END 3 */
  }
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
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV4;
  RCC_OscInitStruct.PLL.PLLN = 85;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void LED_All_Off(void)
{
    HAL_GPIO_WritePin(GPIOC,
                      GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10,
                      GPIO_PIN_RESET);
}

void LED_Green_On(void)
{
    LED_All_Off();

    HAL_GPIO_WritePin(LED_GREEN_PORT,
                      LED_GREEN_PIN,
                      GPIO_PIN_SET);
}

void LED_Yellow_On(void)
{
    LED_All_Off();

    HAL_GPIO_WritePin(LED_YELLOW_PORT,
                      LED_YELLOW_PIN,
                      GPIO_PIN_SET);
}

void LED_Red_On(void)
{
    LED_All_Off();

    HAL_GPIO_WritePin(LED_RED_PORT,
                      LED_RED_PIN,
                      GPIO_PIN_SET);
}

void UART_Send(char *message)
{
    HAL_UART_Transmit(&hlpuart1,
                      (uint8_t*)message,
                      strlen(message),
                      100);
}

const char* Get_Result_Name(MedicationResult result)
{
    switch(result)
    {
        case RESULT_ON_TIME:
            return "ON TIME";

        case RESULT_LATE:
            return "LATE";

        case RESULT_MISSED:
            return "MISSED - NOT TAKEN";

        default:
            return "PENDING";
    }
}
void Process_Button_Event(void)
{
    uint8_t event;
    uint32_t press_time;

    __disable_irq();

    event = button_event;
    press_time = button_time;

    button_event = 0;

    __enable_irq();

    if(event == 0)
    {
        return;
    }

    /* 早餐判斷 */

    if(breakfast_result == RESULT_PENDING)
    {
        if(press_time >= BREAKFAST_START &&
           press_time < BREAKFAST_TIMEOUT)
        {
            breakfast_result = RESULT_ON_TIME;

            LED_Green_On();

            UART_Send("[BREAKFAST] ON TIME\r\n");

            return;
        }

        else if(press_time >= BREAKFAST_TIMEOUT &&
                press_time < BREAKFAST_MISSED)
        {
            breakfast_result = RESULT_LATE;

            LED_Green_On();

            UART_Send("[BREAKFAST] LATE\r\n");

            return;
        }
    }

    /* 晚餐判斷 */

    if(dinner_result == RESULT_PENDING)
    {
        if(press_time >= DINNER_START &&
           press_time < DINNER_TIMEOUT)
        {
            dinner_result = RESULT_ON_TIME;

            LED_Green_On();

            UART_Send("[DINNER] ON TIME\r\n");

            return;
        }

        else if(press_time >= DINNER_TIMEOUT &&
                press_time < DINNER_MISSED)
        {
            dinner_result = RESULT_LATE;

            LED_Green_On();

            UART_Send("[DINNER] LATE\r\n");

            return;
        }
    }
}

void Process_State_Machine(void)
{
    uint32_t now = system_ms;

    /* ========================= */
    /* WAIT */
    /* ========================= */

    if(now < BREAKFAST_START)
    {
        current_state = STATE_WAIT;

        LED_All_Off();
    }

    /* ========================= */
    /* BREAKFAST REMINDER */
    /* ========================= */

    else if(now < BREAKFAST_TIMEOUT)
    {
        if(breakfast_result == RESULT_PENDING)
        {
            current_state = STATE_BREAKFAST_REMINDER;

            LED_Yellow_On();
        }
    }

    /* ========================= */
    /* BREAKFAST TIMEOUT */
    /* ========================= */

    else if(now < BREAKFAST_MISSED)
    {
        if(breakfast_result == RESULT_PENDING)
        {
            current_state = STATE_BREAKFAST_TIMEOUT;

            LED_Red_On();
        }
    }

    /* ========================= */
    /* BREAKFAST MISSED */
    /* ========================= */

    else if(now < DINNER_START)
    {
        if(breakfast_result == RESULT_PENDING)
        {
            breakfast_result = RESULT_MISSED;

            UART_Send("[BREAKFAST] MISSED - NOT TAKEN\r\n");
        }

        current_state = STATE_BREAKFAST_MISSED;

        LED_All_Off();
    }

    /* ========================= */
    /* DINNER REMINDER */
    /* ========================= */

    else if(now < DINNER_TIMEOUT)
    {
        if(dinner_result == RESULT_PENDING)
        {
            current_state = STATE_DINNER_REMINDER;

            LED_Yellow_On();
        }
    }

    /* ========================= */
    /* DINNER TIMEOUT */
    /* ========================= */

    else if(now < DINNER_MISSED)
    {
        if(dinner_result == RESULT_PENDING)
        {
            current_state = STATE_DINNER_TIMEOUT;

            LED_Red_On();
        }
    }

    /* ========================= */
    /* DINNER MISSED */
    /* ========================= */

    else if(now < SYSTEM_END)
    {
        if(dinner_result == RESULT_PENDING)
        {
            dinner_result = RESULT_MISSED;

            UART_Send("[DINNER] MISSED - NOT TAKEN\r\n");
        }

        current_state = STATE_DINNER_MISSED;

        LED_All_Off();
    }

    /* ========================= */
    /* FINISHED */
    /* ========================= */

    else
    {
        current_state = STATE_FINISHED;

        LED_All_Off();

        if(final_output_done == 0)
        {
            final_output_done = 1;

            UART_Final_Report();
        }
    }
}

void UART_Final_Report(void)
{
    snprintf(uart_buffer,
             sizeof(uart_buffer),
             "\r\n");

    UART_Send(uart_buffer);

    UART_Send("================================\r\n");
    UART_Send("       DAILY FINAL REPORT       \r\n");
    UART_Send("================================\r\n");

    snprintf(uart_buffer,
             sizeof(uart_buffer),
             "Breakfast : %s\r\n",
             Get_Result_Name(breakfast_result));

    UART_Send(uart_buffer);

    snprintf(uart_buffer,
             sizeof(uart_buffer),
             "Dinner    : %s\r\n",
             Get_Result_Name(dinner_result));

    UART_Send(uart_buffer);

    UART_Send("--------------------------------\r\n");

    if(breakfast_result == RESULT_ON_TIME &&
       dinner_result == RESULT_ON_TIME)
    {
        UART_Send("Daily Status: ALL ON TIME\r\n");
    }
    else if(breakfast_result == RESULT_MISSED &&
            dinner_result == RESULT_MISSED)
    {
        UART_Send("Daily Status: ALL MEDICATIONS MISSED\r\n");
    }
    else
    {
        UART_Send("Daily Status: COMPLETED WITH EXCEPTIONS\r\n");
    }

    UART_Send("================================\r\n");
    UART_Send("       SYSTEM FINISHED          \r\n");
    UART_Send("================================\r\n");
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim->Instance == TIM2)
    {
        system_ms++;
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if(GPIO_Pin == GPIO_PIN_13)
    {
        uint32_t now = system_ms;

        /* Debounce: 50ms */

        if(debounce_initialized == 0 ||
           (now - last_button_time) >= DEBOUNCE_TIME)
        {
            last_button_time = now;

            debounce_initialized = 1;

            button_time = now;

            button_event = 1;
        }
    }
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
#ifdef USE_FULL_ASSERT
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
