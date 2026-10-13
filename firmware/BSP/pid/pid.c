/**
 * @file    pid.c
 * @brief   PID控制器模块实现文件
 * @details 基于位置式PID算法实现，包含积分限幅和抗饱和积分(Anti-Windup)功能
 *          抗饱和积分原理: 当输出即将饱和且误差与积分同方向时，停止积分累积
 */

#include "pid.h"

/**
 * @brief   PID控制器初始化函数
 * @details 设置PID参数(Kp/Ki/Kd)、积分限幅和输出限幅，清零所有状态变量
 * @param   pid: PID控制器指针
 * @param   Kp: 比例系数，影响响应速度
 * @param   Ki: 积分系数，消除稳态误差
 * @param   Kd: 微分系数，抑制振荡
 * @param   integral_limit: 积分限幅值，防止积分饱和
 * @param   output_limit: 输出限幅值，限制输出范围
 * @retval  None
 */
void PID_Init(PID_TypeDef *pid, float Kp, float Ki, float Kd, 
              float integral_limit, float output_limit)
{
    /* 设置PID参数 */
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;
    
    /* 清零状态变量 */
    pid->target = 0.0f;
    pid->actual = 0.0f;
    pid->error = 0.0f;
    pid->last_error = 0.0f;
    
    /* 设置积分限幅 */
    pid->integral = 0.0f;
    pid->integral_max = integral_limit;
    pid->integral_min = -integral_limit;
    
    /* 设置输出限幅 */
    pid->output = 0.0f;
    pid->output_max = output_limit;
    pid->output_min = -output_limit;
    
    /* 清零饱和标志 */
    pid->saturated = 0;
}

/**
 * @brief   设置PID目标值
 * @details 更新PID控制器的目标值，用于闭环控制
 * @param   pid: PID控制器指针
 * @param   target: 目标值(如目标速度、目标位置等)
 * @retval  None
 */
void PID_SetTarget(PID_TypeDef *pid, float target)
{
    pid->target = target;
}

/**
 * @brief   PID计算函数
 * @details 根据当前实际值计算PID输出，包含以下功能:
 *          1. 计算比例项(P)
 *          2. 计算微分项(D)
 *          3. 带抗饱和积分的积分项(I)
 *          4. 积分限幅
 *          5. 输出限幅
 *          抗饱和积分逻辑: 当输出即将饱和且误差与积分同方向时，停止积分累积
 * @param   pid: PID控制器指针
 * @param   actual: 当前实际值(如实际速度、实际位置等)
 * @retval  PID输出值
 */
float PID_Calculate(PID_TypeDef *pid, float actual)
{
    /* 更新实际值 */
    pid->actual = actual;
    /* 计算当前误差 = 目标值 - 实际值 */
    pid->error = pid->target - pid->actual;
    
    /* 计算比例项: P = Kp * error */
    float p_out = pid->Kp * pid->error;
    /* 计算微分项: D = Kd * (error - last_error) */
    float d_out = pid->Kd * (pid->error - pid->last_error);
    
    /* 计算新的积分值: I_new = I + Ki * error */
    float integral_new = pid->integral + pid->Ki * pid->error;
    
    /* 预测输出值(用于抗饱和判断) */
    float output_test = p_out + integral_new + d_out;
    
    /* 抗饱和积分逻辑 */
    if(output_test > pid->output_max)
    {
        /* 输出即将正饱和 */
        if(pid->error > 0.0f && integral_new > pid->integral)
        {
            /* 误差为正且积分仍在增加，停止积分累积 */
            pid->saturated = 1;
        }
        else
        {
            /* 允许积分累积 */
            pid->integral = integral_new;
            pid->saturated = 0;
        }
    }
    else if(output_test < pid->output_min)
    {
        /* 输出即将负饱和 */
        if(pid->error < 0.0f && integral_new < pid->integral)
        {
            /* 误差为负且积分仍在减少，停止积分累积 */
            pid->saturated = 1;
        }
        else
        {
            /* 允许积分累积 */
            pid->integral = integral_new;
            pid->saturated = 0;
        }
    }
    else
    {
        /* 输出未饱和，允许积分累积 */
        pid->integral = integral_new;
        pid->saturated = 0;
    }
    
    /* 积分限幅保护 */
    if(pid->integral > pid->integral_max) pid->integral = pid->integral_max;
    if(pid->integral < pid->integral_min) pid->integral = pid->integral_min;
    
    /* 计算最终输出: Output = P + I + D */
    pid->output = p_out + pid->integral + d_out;
    
    /* 输出限幅保护 */
    if(pid->output > pid->output_max) pid->output = pid->output_max;
    if(pid->output < pid->output_min) pid->output = pid->output_min;
    
    /* 保存当前误差作为下次计算的上次误差 */
    pid->last_error = pid->error;
    
    return pid->output;
}

/**
 * @brief   复位PID控制器
 * @details 清零误差、积分和输出，重置饱和标志，用于控制器重新启动
 * @param   pid: PID控制器指针
 * @retval  None
 */
void PID_Reset(PID_TypeDef *pid)
{
    pid->error = 0.0f;
    pid->last_error = 0.0f;
    pid->integral = 0.0f;
    pid->output = 0.0f;
    pid->saturated = 0;
}
