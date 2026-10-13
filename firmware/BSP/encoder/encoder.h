#ifndef __ENCODER_H
#define __ENCODER_H

#include "main.h"

/**
 * @defgroup 编码器参数配置
 * @{
 */
#define ENCODER_RESOLUTION    120    /* 编码器每圈脉冲数(含减速比) */
#define ENCODER_QUADRATURE    4      /* 四倍频系数 */
#define WHEEL_DIAMETER        0.065f /* 车轮直径(米) */
#define WHEEL_CIRCUMFERENCE   (3.14159265f * WHEEL_DIAMETER) /* 车轮周长(米) */
/** @} */

/**
 * @brief   编码器初始化函数
 * @note    启动TIM2和TIM4编码器模式，配置初始时间和计数值
 * @param   None
 * @retval  None
 */
void Encoder_Init(void);

/**
 * @brief   获取左轮编码器计数值
 * @note    读取TIM2当前计数值并复位计数器，返回负值表示方向
 * @param   None
 * @retval  计数值(int16_t)
 */
int16_t Encoder_GetCount1(void);

/**
 * @brief   获取右轮编码器计数值
 * @note    读取TIM4当前计数值并复位计数器
 * @param   None
 * @retval  计数值(int16_t)
 */
int16_t Encoder_GetCount2(void);

/**
 * @brief   复位左轮编码器
 * @note    清零TIM2计数器和内部计数值
 * @param   None
 * @retval  None
 */
void Encoder_Reset1(void);

/**
 * @brief   复位右轮编码器
 * @note    清零TIM4计数器和内部计数值
 * @param   None
 * @retval  None
 */
void Encoder_Reset2(void);

/**
 * @brief   获取左轮速度(米/秒)
 * @note    根据编码器脉冲数和时间间隔计算实际速度
 * @param   None
 * @retval  速度值(m/s)
 */
float Encoder_GetSpeed1(void);

/**
 * @brief   获取右轮速度(米/秒)
 * @note    根据编码器脉冲数和时间间隔计算实际速度
 * @param   None
 * @retval  速度值(m/s)
 */
float Encoder_GetSpeed2(void);

#endif

