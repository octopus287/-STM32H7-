#ifndef __PID_H
#define __PID_H

#include "main.h"

/**
 * @brief   PID控制器结构体定义
 * @details 包含PID参数、状态变量和输出限制
 */
typedef struct {
    float Kp;                  /* 比例系数 */
    float Ki;                  /* 积分系数 */
    float Kd;                  /* 微分系数 */
    
    float target;              /* 目标值 */
    float actual;              /* 当前实际值 */
    float error;               /* 当前误差 */
    float last_error;          /* 上次误差 */
    
    float integral;            /* 积分累积值 */
    float integral_max;        /* 积分上限 */
    float integral_min;        /* 积分下限 */
    
    float output;              /* PID输出值 */
    float output_max;          /* 输出上限 */
    float output_min;          /* 输出下限 */
    
    uint8_t saturated;         /* 饱和标志位，1表示输出饱和 */
} PID_TypeDef;

/**
 * @brief   PID控制器初始化函数
 * @note    设置PID参数、积分限幅和输出限幅，清零状态变量
 * @param   pid: PID控制器指针
 * @param   Kp: 比例系数
 * @param   Ki: 积分系数
 * @param   Kd: 微分系数
 * @param   integral_limit: 积分限幅值
 * @param   output_limit: 输出限幅值
 * @retval  None
 */
void PID_Init(PID_TypeDef *pid, float Kp, float Ki, float Kd, 
              float integral_limit, float output_limit);

/**
 * @brief   设置PID目标值
 * @note    更新PID控制器的目标值
 * @param   pid: PID控制器指针
 * @param   target: 目标值
 * @retval  None
 */
void PID_SetTarget(PID_TypeDef *pid, float target);

/**
 * @brief   PID计算函数
 * @note    根据当前实际值计算PID输出，包含积分限幅和抗饱和积分
 * @param   pid: PID控制器指针
 * @param   actual: 当前实际值
 * @retval  PID输出值
 */
float PID_Calculate(PID_TypeDef *pid, float actual);

/**
 * @brief   复位PID控制器
 * @note    清零误差、积分和输出，重置饱和标志
 * @param   pid: PID控制器指针
 * @retval  None
 */
void PID_Reset(PID_TypeDef *pid);

#endif

