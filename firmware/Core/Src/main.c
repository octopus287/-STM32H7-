/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : 智能小车主控程序入口文件
  * @details        : 实现双电机PID速度闭环控制，支持蓝牙遥控、OLED显示、
  *                  DHT11温湿度采集、蜂鸣器提示、超声波测距和水泵控制
  *                  
  *                  系统架构分层:
  *                  - BSP层(板级支持包): motor, encoder, pid, car, bluetooth, 
  *                    hcsr04, debug, OLED, DHT11, Buzz, pump
  *                  - APP层(应用层): bluetooth_control
  *                  - Core层(硬件抽象层): HAL库驱动初始化
  *                  
  *                  主循环流程(50ms周期, 20Hz控制频率):
  *                  1. 循环状态输出(每20次循环输出一次)
  *                  2. 蓝牙遥控指令处理
  *                  3. 更新PID目标速度
  *                  4. 读取编码器速度(左右轮)
  *                  5. PID计算(左右轮独立)
  *                  6. 输出PWM控制电机
  *                  7. 蓝牙连接状态检测与OLED显示更新
  *                  8. DHT11温湿度读取(每1秒)
  *                  9. 超声波距离读取(每1秒)
  *                  10. 循环延时(50ms)
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
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "motor.h"           /* 电机驱动模块 */
#include "car.h"             /* 小车控制模块 */
#include "encoder.h"         /* 编码器模块 */
#include "pid.h"             /* PID控制模块 */
#include "debug.h"           /* 调试模块(printf重定向) */
#include "bluetooth.h"       /* 蓝牙模块 */
#include "bluetooth_control.h" /* 蓝牙遥控模块 */
#include "OLED.h"            /* OLED显示模块 */
#include "buzz.h"            /* 蜂鸣器模块 */
#include "dht11.h"           /* DHT11温湿度传感器模块 */
#include "hcsr04.h"          /* 超声波模块 */
#include "stdio.h"           /* 标准输入输出库 */
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
/** PID控制器实例 - 左电机 */
PID_TypeDef pid_left;
/** PID控制器实例 - 右电机 */
PID_TypeDef pid_right;

/** DHT11温湿度数据 */
DHT11_Data_t dht11_data;
/** DHT11读取计数器 */
uint32_t dht11_counter = 0;

/**
 * @defgroup PID参数配置
 * @{
 */
#define TARGET_SPEED    0.5f    /* 默认目标速度(m/s) */
#define KP              200.0f  /* 比例系数 - 影响响应速度 */
#define KI              50.0f   /* 积分系数 - 消除稳态误差 */
#define KD              10.0f   /* 微分系数 - 抑制振荡 */
/** @} */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Config(void);
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

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

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
  MX_TIM3_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  MX_TIM6_Init();
  MX_TIM4_Init();
  MX_USART3_UART_Init();
  MX_I2C1_Init();
  MX_TIM8_Init();
  /* USER CODE BEGIN 2 */
  /* 初始化小车控制模块 */
  Car_Init();
  /* 初始化编码器模块 */
  Encoder_Init();
  /* 初始化调试模块(printf重定向) */
  Debug_Init();
  /* 初始化蓝牙模块 */
  Bluetooth_Init();
  /* 发送蓝牙就绪信号 */
  Bluetooth_SendString("Bluetooth Ready!\r\n");
  /* 初始化蓝牙遥控模块 */
  Bluetooth_Control_Init();
  /* 初始化蜂鸣器模块 */
  Buzz_Init();
  /* 初始化OLED显示模块 */
  OLED_Init();
  /* 清屏 */
  OLED_Clear();
  /* 显示蓝牙连接状态 */
  OLED_ShowString(0, 0, "BT: Disconnected", OLED_8X16);
  /* 显示温湿度标签 */
  OLED_ShowString(0, 16, "Temp:", OLED_8X16);
  OLED_ShowString(0, 32, "Humi:", OLED_8X16);
  /* 更新显示 */
  OLED_Update();
  /* 初始化DHT11温湿度传感器 */
  DHT11_Init();
  HAL_Delay(1000);
  /* 初始化超声波模块 */
  HCSR04_Init();
  /* 显示超声波距离标签 */
  OLED_ShowString(0, 48, "Dist:", OLED_8X16);
  OLED_Update();
  
  /* 初始化左电机PID控制器 */
  PID_Init(&pid_left, KP, KI, KD, 500.0f, 1000.0f);
  /* 初始化右电机PID控制器 */
  PID_Init(&pid_right, KP, KI, KD, 500.0f, 1000.0f);
  /* 设置左电机目标速度 */
  PID_SetTarget(&pid_left, Car_Speed.target_speed_left);
  /* 设置右电机目标速度 */
  PID_SetTarget(&pid_right, Car_Speed.target_speed_right);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
    {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
        static uint32_t loop_count = 0;
        if(loop_count++ % 20 == 0)
        {
            printf("Loop running...\r\n");
        }
        
        /* 蓝牙遥控指令处理 */
        Bluetooth_Control_Update();
        
        /* 更新PID目标速度 */
        PID_SetTarget(&pid_left, Car_Speed.target_speed_left);
        PID_SetTarget(&pid_right, Car_Speed.target_speed_right);
        
        /* 读取左电机编码器速度 */
        float speed1 = Encoder_GetSpeed1();
        /* 读取右电机编码器速度 */
        float speed2 = Encoder_GetSpeed2();
        
        /* 左电机PID计算 */
        float pwm_left = PID_Calculate(&pid_left, speed1);
        /* 右电机PID计算 */
        float pwm_right = PID_Calculate(&pid_right, speed2);
        
        /* 输出PWM控制左电机 */
        Motor_Left(pwm_left);
        /* 输出PWM控制右电机 */
        Motor_Right(-pwm_right);
        
        /* 串口调试输出 */
        //printf("TargetL: %.2f\tTargetR: %.2f\tSpeed1: %.2f\tPWM1: %.1f\tSpeed2: %.2f\tPWM2: %.1f\r\n", 
        //       Car_Speed.target_speed_left, Car_Speed.target_speed_right,
        //       speed1, pwm_left, speed2, pwm_right);
        
        /* 检查蓝牙连接状态 */
        static uint8_t last_bt_state = 0;
        Bluetooth_CheckConnectionTimeout();
        uint8_t bt_state = Bluetooth_IsConnected();
        if(bt_state != last_bt_state)
        {
            last_bt_state = bt_state;
            OLED_ClearArea(0, 0, 128, 16);
            if(bt_state == 1)
            {
                OLED_ShowString(0, 0, "BT: Connected", OLED_8X16);
                Buzz_BeepTwice(100, 200);
            }
            else
            {
                OLED_ShowString(0, 0, "BT: Disconnected", OLED_8X16);
            }
            OLED_Update();
            printf("BT State: %d\r\n", bt_state);
        }
        
        /* 读取温湿度数据(每1秒读取一次) */
        dht11_counter++;
        if(dht11_counter >= 20)
        {
            dht11_counter = 0;
            if(DHT11_ReadData(&dht11_data) == 0)
            {
                OLED_ClearArea(48, 16, 60, 16);
                OLED_ShowNum(48, 16, dht11_data.temperature_integer, 2, OLED_8X16);
                OLED_ShowChar(64, 16, '.', OLED_8X16);
                OLED_ShowNum(72, 16, dht11_data.temperature_decimal, 1, OLED_8X16);
                OLED_ShowChar(80, 16, 'C', OLED_8X16);
                
                OLED_ClearArea(48, 32, 60, 16);
                OLED_ShowNum(48, 32, dht11_data.humidity_integer, 2, OLED_8X16);
                OLED_ShowChar(64, 32, '.', OLED_8X16);
                OLED_ShowNum(72, 32, dht11_data.humidity_decimal, 1, OLED_8X16);
                OLED_ShowChar(80, 32, '%', OLED_8X16);
            }
            
            /* 读取超声波距离 */
            float distance = HCSR04_GetDistance_Avg();
            OLED_ClearArea(48, 48, 60, 16);
            if(distance > 0)
            {
                uint16_t dist_int = (uint16_t)distance;
                uint16_t dist_dec = (uint16_t)((distance - dist_int) * 10);
                OLED_ShowNum(48, 48, dist_int, 3, OLED_8X16);
                OLED_ShowChar(72, 48, '.', OLED_8X16);
                OLED_ShowNum(80, 48, dist_dec, 1, OLED_8X16);
                OLED_ShowChar(88, 48, 'c', OLED_8X16);
                OLED_ShowChar(96, 48, 'm', OLED_8X16);
            }
            else
            {
                OLED_ShowString(48, 48, "Err", OLED_8X16);
            }
            
            OLED_Update();
        }
        
        /* 50ms延时(20Hz控制频率) */
        HAL_Delay(50);
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

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

 /* MPU Configuration */

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x0;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}

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
