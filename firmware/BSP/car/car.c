/**
 * @file    car.c
 * @brief   小车控制模块实现文件
 * @details 基于电机驱动模块实现小车整体运动控制，包括前进、后退、左转、右转和停止
 *          提供速度设置接口用于PID闭环控制
 */

#include "car.h"
#include "motor.h"

/** 小车速度状态全局变量 */
Car_Speed_TypeDef Car_Speed = {0.0f, 0.0f};

/**
 * @brief   小车初始化函数
 * @details 初始化电机驱动模块，设置小车初始状态为停止
 * @param   None
 * @retval  None
 */
void Car_Init(void)
{
    /* 初始化电机驱动 */
    Motor_Init();
    /* 停止电机 */
    Car_Stop();
}

/**
 * @brief   小车前进控制
 * @details 设置左右轮正向速度，实现小车直线前进
 *          左电机: 正向(+speed)
 *          右电机: 反向(-speed) - 因安装方向相反
 * @param   speed: 速度值(0~1000)
 * @retval  None
 */
void Car_Forward(uint16_t speed)
{
    Motor_Left(speed);
    Motor_Right(-speed);
}

/**
 * @brief   小车后退控制
 * @details 设置左右轮反向速度，实现小车直线后退
 *          左电机: 反向(-speed)
 *          右电机: 正向(+speed) - 因安装方向相反
 * @param   speed: 速度值(0~1000)
 * @retval  None
 */
void Car_Backward(uint16_t speed)
{
    Motor_Left(-speed);
    Motor_Right(speed);
}

/**
 * @brief   小车左转控制
 * @details 设置左右轮同向速度，实现小车原地左转
 *          左电机: 正向(+speed)
 *          右电机: 正向(+speed)
 * @param   speed: 速度值(0~1000)
 * @retval  None
 */
void Car_Left(uint16_t speed)
{
    Motor_Left(speed);
    Motor_Right(speed);
}

/**
 * @brief   小车右转控制
 * @details 设置左右轮同向速度，实现小车原地右转
 *          左电机: 反向(-speed)
 *          右电机: 反向(-speed)
 * @param   speed: 速度值(0~1000)
 * @retval  None
 */
void Car_Right(uint16_t speed)
{
    Motor_Left(-speed);
    Motor_Right(-speed);
}

/**
 * @brief   小车停止控制
 * @details 清零目标速度，停止所有电机
 * @param   None
 * @retval  None
 */
void Car_Stop(void)
{
    /* 清零目标速度 */
    Car_Speed.target_speed_left = 0.0f;
    Car_Speed.target_speed_right = 0.0f;
    /* 停止电机 */
    Motor_Stop();
}

/**
 * @brief   设置小车目标速度
 * @details 分别设置左右轮的目标速度(米/秒)，用于PID速度闭环控制
 *          实际电机控制由主循环中的PID控制器完成
 * @param   left: 左轮目标速度(m/s)
 * @param   right: 右轮目标速度(m/s)
 * @retval  None
 */
void Car_SetSpeed(float left, float right)
{
    Car_Speed.target_speed_left = left;
    Car_Speed.target_speed_right = right;
}
