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
#include "dma.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "as5600.h"
#include "stdio.h"
#include "string.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* 基础参数：烧录不同节点时只需修改 NODE_ID */

uint8_t g_chain_frame[FRAME_SIZE]; // 核心数据包
volatile uint8_t g_rx_ready = 0;   // 接收完成标志

uint32_t last_rx_tick = 0; // 用于记录上一次成功接收的时间点
extern DMA_HandleTypeDef hdma_usart1_rx;
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
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_I2C2_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
if (NODE_ID < 8) {
    /* 1. 核心修正：重定向 DMAMUX */
    DMAMUX1_Channel0->CCR = 15; 

    /* 2. 强力解锁：解决 HAL_LOCKED 问题 */
    huart1.RxState = HAL_UART_STATE_READY;
    huart1.Lock = HAL_UNLOCKED;
    hdma_usart1_rx.State = HAL_DMA_STATE_READY;
    hdma_usart1_rx.Lock = HAL_UNLOCKED;

    /* 3. 清除硬件标志 */
    USART1->ICR = USART_ICR_IDLECF | USART_ICR_ORECF | USART_ICR_NECF | USART_ICR_FECF;
    
    /* 4. 彻底重置并启动 DMA */
    HAL_UART_DMAStop(&huart1); 
    if (HAL_UART_Receive_DMA(&huart1, g_chain_frame, FRAME_SIZE) != HAL_OK) {
        // 如果依然返回非 HAL_OK，请检查 MX_DMA_Init 的顺序
        Error_Handler(); 
    }
    __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);

    last_rx_tick = HAL_GetTick();
}
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		// 1. 实时获取本地 AS5600 角度（取整 0-360）
//				uint16_t my_angle = (uint16_t)AS5600_Get_Degree(); 
				uint16_t my_angle = 180; 

				uint32_t current_time = HAL_GetTick();

				if (NODE_ID == 8) 
				{
						/* 8号模块：节拍发生器（源头） */
						memset(g_chain_frame, 0, FRAME_SIZE);
						g_chain_frame[0]  = HEAD_BYTE;  // 0xAA
						g_chain_frame[17] = TAIL_BYTE;  // 0x55
						
						// 填充8号位数据
						g_chain_frame[1] = (uint8_t)(my_angle >> 8);
						g_chain_frame[2] = (uint8_t)(my_angle & 0xFF);

						// 8号通过 USART2 发送给 7号
						HAL_UART_Transmit(&huart2, g_chain_frame, FRAME_SIZE, 10);
						
						HAL_Delay(20); // 50Hz 刷新率
				}
				else 
				{
						/* 7-1号模块：链式接力与补位 */
						
						// --- 情况 A：成功接收到上级数据包 ---
						if (g_rx_ready == 1) 
						{
								// 按照 NodeID 计算填充偏移
								int offset = 1 + (8 - NODE_ID) * 2;
								
								g_chain_frame[offset]     = (uint8_t)(my_angle >> 8);
								g_chain_frame[offset + 1] = (uint8_t)(my_angle & 0xFF);

								// 通过 USART2 转发给下一级
								HAL_UART_Transmit(&huart2, g_chain_frame, FRAME_SIZE, 10);
								
								// 状态复位
								g_rx_ready = 0;
								last_rx_tick = current_time; // 更新喂狗时间
								
								// 必须重新挂载 USART1 的 DMA 接收
								HAL_UART_Receive_DMA(&huart1, g_chain_frame, FRAME_SIZE);
						}
						
						// --- 情况 B：超时自发（上级链路断开） ---
						else if (current_time - last_rx_tick > 50) 
						{
								// 构造包含自身数据的包
								memset(g_chain_frame, 0, FRAME_SIZE);
								g_chain_frame[0]  = HEAD_BYTE;
								g_chain_frame[17] = TAIL_BYTE;
								
								int offset = 1 + (8 - NODE_ID) * 2;
								g_chain_frame[offset]     = (uint8_t)(my_angle >> 8);
								g_chain_frame[offset + 1] = (uint8_t)(my_angle & 0xFF);

								HAL_UART_Transmit(&huart2, g_chain_frame, FRAME_SIZE, 10);
								
								last_rx_tick = current_time; // 喂狗，防止死循环轰炸
								
								// 发生超时后，强制重置 USART1 的 DMA 状态，防止 ORE 等错误挂起
								HAL_UART_DMAStop(&huart1);
								HAL_UART_Receive_DMA(&huart1, g_chain_frame, FRAME_SIZE);
						}
				}
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
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 8;
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
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

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
