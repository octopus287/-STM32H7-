#ifndef __MOTOR_H
#define __MOTOR_H

#include "main.h"

/**
 * @brief   电机驱动初始化函数
 * @note    启动TIM3的CH3和CH4通道PWM输出，用于驱动左右电机
 * @param   None
 * @retval  None
 */
void Motor_Init(void);

/**
 * @brief   左电机速度控制
 * @note    通过控制GPIO方向引脚和PWM占空比实现左电机正反转和调速
 * @param   speed: 速度值，范围-1000~1000，正值前进，负值后退
 * @retval  None
 */
void Motor_Left(int16_t speed);

/**
 * @brief   右电机速度控制
 * @note    通过控制GPIO方向引脚和PWM占空比实现右电机正反转和调速
 * @param   speed: 速度值，范围-1000~1000，正值前进，负值后退
 * @retval  None
 */
void Motor_Right(int16_t speed);

/**
 * @brief   电机停止函数
 * @note    将左右电机速度设为0
 * @param   None
 * @retval  None
 */
void Motor_Stop(void);

#endif


