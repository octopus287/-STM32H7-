/**
 * @file    buzz.c
 * @brief   蜂鸣器驱动实现文件
 * @details 基于STM32 HAL库实现有源蜂鸣器控制
 *          使用PD5引脚，低电平触发蜂鸣器发声
 *          高电平关闭蜂鸣器，低电平开启蜂鸣器
 */

#include "buzz.h"
#include "gpio.h"
#include "stm32h7xx_hal.h"

/** 蜂鸣器引脚定义 */
#define BUZZ_GPIO_PORT      GPIOD
#define BUZZ_GPIO_PIN       GPIO_PIN_5

/**
 * @brief   蜂鸣器初始化函数
 * @details 配置PD5为推挽输出模式，上拉电阻使能
 *          初始状态为高电平(关闭蜂鸣器)
 * @param   None
 * @retval  None
 */
void Buzz_Init(void)
{
    /* 使能GPIOD时钟 */
    __HAL_RCC_GPIOD_CLK_ENABLE();
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    GPIO_InitStruct.Pin = BUZZ_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    
    HAL_GPIO_Init(BUZZ_GPIO_PORT, &GPIO_InitStruct);
    
    /* 初始状态: 高电平，关闭蜂鸣器 */
    HAL_GPIO_WritePin(BUZZ_GPIO_PORT, BUZZ_GPIO_PIN, GPIO_PIN_SET);
}

/**
 * @brief   开启蜂鸣器
 * @details 输出低电平触发蜂鸣器发声
 * @param   None
 * @retval  None
 */
void Buzz_On(void)
{
    HAL_GPIO_WritePin(BUZZ_GPIO_PORT, BUZZ_GPIO_PIN, GPIO_PIN_RESET);
}

/**
 * @brief   关闭蜂鸣器
 * @details 输出高电平停止蜂鸣器发声
 * @param   None
 * @retval  None
 */
void Buzz_Off(void)
{
    HAL_GPIO_WritePin(BUZZ_GPIO_PORT, BUZZ_GPIO_PIN, GPIO_PIN_SET);
}

/**
 * @brief   蜂鸣器响一声
 * @details 开启蜂鸣器，持续指定时间后关闭
 *          使用HAL_Delay阻塞延时，调用期间无法执行其他任务
 * @param   duration_ms: 蜂鸣器持续时间(毫秒)
 * @retval  None
 */
void Buzz_Beep(uint16_t duration_ms)
{
    Buzz_On();
    HAL_Delay(duration_ms);
    Buzz_Off();
}

/**
 * @brief   蜂鸣器响两声
 * @details 开启蜂鸣器，持续指定时间后关闭，间隔指定时间后再次开启
 *          使用HAL_Delay阻塞延时，调用期间无法执行其他任务
 * @param   duration_ms: 每次蜂鸣器持续时间(毫秒)
 * @param   interval_ms: 两声之间的间隔时间(毫秒)
 * @retval  None
 */
void Buzz_BeepTwice(uint16_t duration_ms, uint16_t interval_ms)
{
    /* 第一声 */
    Buzz_On();
    HAL_Delay(duration_ms);
    Buzz_Off();
    
    /* 间隔时间 */
    HAL_Delay(interval_ms);
    
    /* 第二声 */
    Buzz_On();
    HAL_Delay(duration_ms);
    Buzz_Off();
}
