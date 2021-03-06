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
#include "dma.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "nextion.h"

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
/* Private variables ---------------------------------------------------------*/
uint8_t Current_Page = 0;
uint8_t Page_RCV_Flag = 0;

Nextion_Object_t BTN_Object;
Nextion_Object_t Page_1_Object;
Nextion_Object_t Page_0_Object;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void BTN_Object_Pressed()
    {
    Nextion_Set_Text(BTN_Object.Name, "Pressed");
    }
void BTN_Object_Released()
    {
    Nextion_Set_Text(BTN_Object.Name, "Released");
    }

void Page_0_Touched()
    {
    //Nextion_Hide_Object(BTN_Object.Name,true);
    uint32_t timeout = 0xFFFF;
    Page_RCV_Flag = 0;
    Nextion_Get_Current_Page();
    while (!Page_RCV_Flag && --timeout)
	{
	}

    if (timeout)
	{
	if (Current_Page == 0)
	    {
	    Nextion_Set_BCK_Colour(BTN_Object.Name, 63488);
	    }
	}
    }

void Page_0_Released()
    {
    //Nextion_Hide_Object(BTN_Object.Name,false);
    static uint8_t Page_0_Touch_Count = 0;
    uint32_t timeout = 0xFFFF;
    Page_RCV_Flag = 0;
    Nextion_Get_Current_Page();
    while (!Page_RCV_Flag && --timeout)
	{
	}

    if (timeout)
	{
	if (Current_Page == 0)
	    {
	    Nextion_Set_BCK_Colour(BTN_Object.Name, 48631);
	    }
	}

    Page_0_Touch_Count++;

    if (Page_0_Touch_Count == 10)
	{
	Page_0_Touch_Count = 0;
	Nextion_Send_Command("page 4");
	}
    }

void Page_1_Touched()
    {
    //Nextion_Hide_Object(BTN_Object.Name,true);

    uint32_t timeout = 0xFFFF;
    Page_RCV_Flag = 0;
    Nextion_Get_Current_Page();
    while (!Page_RCV_Flag && --timeout)
	{
	}

    if (timeout)
	{
	if (Current_Page == 1)
	    {
	    Nextion_Set_BCK_Colour(BTN_Object.Name, 63488);
	    }
	}

    Nextion_Backlight_Brightness(50, 0);
    }

void Page_1_Released()
    {
    //Nextion_Hide_Object(BTN_Object.Name,false);
    uint32_t timeout = 0xFFFF;
    Page_RCV_Flag = 0;
    Nextion_Get_Current_Page();
    while (!Page_RCV_Flag && --timeout)
	{
	}

    if (timeout)
	{
	if (Current_Page == 1)
	    {
	    Nextion_Set_BCK_Colour(BTN_Object.Name, 48631);
	    }
	}

    Nextion_Backlight_Brightness(80, 0);
    }

void Nextion_RX_Page_ID_Callback(uint8_t Page_ID)
    {
    Page_RCV_Flag = 0;
    Current_Page = Page_ID;
    }

void Nextion_RX_String_Callback(const char* str)
    {

    }

void Nextion_RX_Number_Callback(uint32_t Number)
    {

    }

void Nextion_CMD_Finished_Callback()
    {

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
  MX_DMA_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

    Nextion_Init();


    Page_0_Object.Push_Callback = &Page_0_Touched;
    Page_0_Object.Pop_Callback = &Page_0_Released;
    Page_0_Object.Component_ID = 0;
    Page_0_Object.Page_ID = 0;
    Page_0_Object.Name = "page 0";

    Nextion_Add_Object(&Page_0_Object);


    Page_1_Object.Push_Callback = &Page_1_Touched;
    Page_1_Object.Pop_Callback = &Page_1_Released;
    Page_1_Object.Component_ID = 0;
    Page_1_Object.Page_ID = 1;
    Page_1_Object.Name = "page 1";

    Nextion_Add_Object(&Page_1_Object);


    BTN_Object.Push_Callback = NULL;
    BTN_Object.Pop_Callback = &BTN_Object_Released;
    BTN_Object.Component_ID = 6;
    BTN_Object.Page_ID = 0;
    BTN_Object.Name = "b0";

    Nextion_Add_Object(&BTN_Object);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
    while (1)
	{

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	Nextion_Loop();

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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
