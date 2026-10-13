/**
 * @file    motor.c
 * @brief   电机驱动模块实现文件
 * @details 基于STM32 HAL库实现双电机独立PWM调速控制，支持正反转
 *          左电机使用TIM3_CH3(PB0) + PA4/PA5方向引脚
 *          右电机使用TIM3_CH4(PB1) + PA6/PA7方向引脚
 */

#include "motor.h"
#include "tim.h"

/* TIM3句柄，用于PWM输出 */
extern TIM_HandleTypeDef htim3;

/**
 * @brief   电机驱动初始化函数
 * @details 启动TIM3的CH3和CH4通道PWM输出，电机初始状态为停止
 * @param   None
 * @retval  None
 */
void Motor_Init(void)
{
    /* 启动左电机PWM通道(TIM3_CH3) */
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
    /* 启动右电机PWM通道(TIM3_CH4) */
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
}

/**
 * @brief   左电机速度控制
 * @details 通过控制PA4/PA5方向引脚和TIM3_CH3 PWM占空比实现左电机正反转和调速
 *          速度范围-1000~1000，超出范围自动限幅
 *          正值: PA4=0, PA5=1 (前进)
 *          负值: PA4=1, PA5=0 (后退)
 * @param   speed: 速度值，范围-1000~1000
 * @retval  None
 */
void Motor_Left(int16_t speed)
{
    /* 速度限幅保护，防止超出PWM范围 */
    if(speed > 1000) speed = 1000;
    if(speed < -1000) speed = -1000;

    if(speed >= 0)
    {
        /* 前进方向: IN1=0, IN2=1 */
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);

        /* 设置PWM占空比 */
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, speed);
    }
    else
    {
        /* 后退方向: IN1=1, IN2=0 */
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

        /* 设置PWM占空比(取绝对值) */
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, -speed);
    }
}

/**
 * @brief   右电机速度控制
 * @details 通过控制PA6/PA7方向引脚和TIM3_CH4 PWM占空比实现右电机正反转和调速
 *          速度范围-1000~1000，超出范围自动限幅
 *          正值: PA6=0, PA7=1 (前进)
 *          负值: PA6=1, PA7=0 (后退)
 * @param   speed: 速度值，范围-1000~1000
 * @retval  None
 */
void Motor_Right(int16_t speed)
{
    /* 速度限幅保护，防止超出PWM范围 */
    if(speed > 1000) speed = 1000;
    if(speed < -1000) speed = -1000;

    if(speed >= 0)
    {
        /* 前进方向: IN3=0, IN4=1 */
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);

        /* 设置PWM占空比 */
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, speed);
    }
    else
    {
        /* 后退方向: IN3=1, IN4=0 */
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);

        /* 设置PWM占空比(取绝对值) */
        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, -speed);
    }
}

/**
 * @brief   电机停止函数
 * @details 将左右电机PWM占空比设为0，电机停止转动
 * @param   None
 * @retval  None
 */
void Motor_Stop(void)
{
    Motor_Left(0);
    Motor_Right(0);
}
