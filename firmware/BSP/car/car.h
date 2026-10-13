#ifndef __CAR_H
#define __CAR_H

#include "main.h"

/**
 * @brief   小车速度状态结构体定义
 * @details 存储左右轮的目标速度值
 */
typedef struct {
    float target_speed_left;   /* 左轮目标速度(m/s) */
    float target_speed_right;  /* 右轮目标速度(m/s) */
} Car_Speed_TypeDef;

/** 小车速度状态全局变量 */
extern Car_Speed_TypeDef Car_Speed;

/**
 * @brief   小车初始化函数
 * @note    初始化电机驱动，设置初始状态为停止
 * @param   None
 * @retval  None
 */
void Car_Init(void);

/**
 * @brief   小车前进控制
 * @note    设置左右轮正向速度，小车直线前进
 * @param   speed: 速度值(0~1000)
 * @retval  None
 */
void Car_Forward(uint16_t speed);

/**
 * @brief   小车后退控制
 * @note    设置左右轮反向速度，小车直线后退
 * @param   speed: 速度值(0~1000)
 * @retval  None
 */
void Car_Backward(uint16_t speed);

/**
 * @brief   小车左转控制
 * @note    设置左右轮同向速度，小车原地左转
 * @param   speed: 速度值(0~1000)
 * @retval  None
 */
void Car_Left(uint16_t speed);

/**
 * @brief   小车右转控制
 * @note    设置左右轮同向速度，小车原地右转
 * @param   speed: 速度值(0~1000)
 * @retval  None
 */
void Car_Right(uint16_t speed);

/**
 * @brief   小车停止控制
 * @note    清零目标速度，停止电机
 * @param   None
 * @retval  None
 */
void Car_Stop(void);

/**
 * @brief   设置小车目标速度
 * @note    分别设置左右轮的目标速度(米/秒)，用于PID速度闭环控制
 * @param   left: 左轮目标速度(m/s)
 * @param   right: 右轮目标速度(m/s)
 * @retval  None
 */
void Car_SetSpeed(float left, float right);

#endif

