/**
 * @file    encoder.c
 * @brief   编码器模块实现文件
 * @details 基于STM32 HAL库实现双路正交编码器读取和速度计算
 *          左轮使用TIM2编码器模式(PA0-CH1, PA1-CH2)
 *          右轮使用TIM4编码器模式(PB6-CH1, PB7-CH2)
 *          速度计算公式: v = (delta_count * WHEEL_CIRCUMFERENCE) / 
 *                           (ENCODER_RESOLUTION * ENCODER_QUADRATURE * delta_time)
 */

#include "encoder.h"
#include "tim.h"

/* TIM2句柄，用于左轮编码器 */
extern TIM_HandleTypeDef htim2;
/* TIM4句柄，用于右轮编码器 */
extern TIM_HandleTypeDef htim4;

/* 上次采样时间戳(ms) */
static uint32_t last_time1 = 0;
static uint32_t last_time2 = 0;
/* 上次采样计数值 */
static int16_t last_count1 = 0;
static int16_t last_count2 = 0;

/**
 * @brief   编码器初始化函数
 * @details 启动TIM2和TIM4编码器模式，初始化时间戳和计数值
 * @param   None
 * @retval  None
 */
void Encoder_Init(void)
{
    /* 启动TIM2编码器模式(左轮) */
    HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL);
    /* 启动TIM4编码器模式(右轮) */
    HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_ALL);
    
    /* 初始化时间戳 */
    last_time1 = HAL_GetTick();
    last_time2 = HAL_GetTick();
    /* 初始化计数值 */
    last_count1 = 0;
    last_count2 = 0;
}

/**
 * @brief   获取左轮编码器计数值
 * @details 读取TIM2当前计数值，复位计数器，返回负值表示方向
 * @param   None
 * @retval  计数值(int16_t)，负值表示反转
 */
int16_t Encoder_GetCount1(void)
{
    /* 读取计数值并取负(调整方向) */
    int16_t count = -(int16_t)__HAL_TIM_GET_COUNTER(&htim2);
    /* 复位计数器 */
    __HAL_TIM_SET_COUNTER(&htim2, 0);
    return count;
}

/**
 * @brief   获取右轮编码器计数值
 * @details 读取TIM4当前计数值，复位计数器
 * @param   None
 * @retval  计数值(int16_t)，负值表示反转
 */
int16_t Encoder_GetCount2(void)
{
    /* 读取计数值 */
    int16_t count = (int16_t)__HAL_TIM_GET_COUNTER(&htim4);
    /* 复位计数器 */
    __HAL_TIM_SET_COUNTER(&htim4, 0);
    return count;
}

/**
 * @brief   复位左轮编码器
 * @details 清零TIM2计数器和内部计数值
 * @param   None
 * @retval  None
 */
void Encoder_Reset1(void)
{
    __HAL_TIM_SET_COUNTER(&htim2, 0);
    last_count1 = 0;
}

/**
 * @brief   复位右轮编码器
 * @details 清零TIM4计数器和内部计数值
 * @param   None
 * @retval  None
 */
void Encoder_Reset2(void)
{
    __HAL_TIM_SET_COUNTER(&htim4, 0);
    last_count2 = 0;
}

/**
 * @brief   获取左轮速度(米/秒)
 * @details 根据编码器脉冲数变化和时间间隔计算实际速度
 *          速度 = (脉冲数变化 * 车轮周长) / (编码器分辨率 * 四倍频 * 时间间隔)
 * @param   None
 * @retval  速度值(m/s)
 */
float Encoder_GetSpeed1(void)
{
    /* 获取当前时间戳 */
    uint32_t current_time = HAL_GetTick();
    /* 获取当前计数值 */
    int16_t current_count = -(int16_t)__HAL_TIM_GET_COUNTER(&htim2);
    
    /* 计算时间间隔(ms) */
    uint32_t delta_time = current_time - last_time1;
    /* 计算计数值变化 */
    int16_t delta_count = current_count - last_count1;
    
    /* 避免除零错误 */
    if(delta_time == 0)
    {
        return 0.0f;
    }
    
    /* 速度计算公式: v = (delta_count * 周长) / (分辨率 * 四倍频 * 时间(秒)) */
    float speed = (float)delta_count * WHEEL_CIRCUMFERENCE / 
                  ((float)ENCODER_RESOLUTION * (float)ENCODER_QUADRATURE * (float)delta_time / 1000.0f);
    
    /* 更新时间戳和计数值 */
    last_time1 = current_time;
    last_count1 = current_count;
    
    return speed;
}

/**
 * @brief   获取右轮速度(米/秒)
 * @details 根据编码器脉冲数变化和时间间隔计算实际速度
 *          速度 = (脉冲数变化 * 车轮周长) / (编码器分辨率 * 四倍频 * 时间间隔)
 * @param   None
 * @retval  速度值(m/s)
 */
float Encoder_GetSpeed2(void)
{
    /* 获取当前时间戳 */
    uint32_t current_time = HAL_GetTick();
    /* 获取当前计数值 */
    int16_t current_count = (int16_t)__HAL_TIM_GET_COUNTER(&htim4);
    
    /* 计算时间间隔(ms) */
    uint32_t delta_time = current_time - last_time2;
    /* 计算计数值变化 */
    int16_t delta_count = current_count - last_count2;
    
    /* 避免除零错误 */
    if(delta_time == 0)
    {
        return 0.0f;
    }
    
    /* 速度计算公式: v = (delta_count * 周长) / (分辨率 * 四倍频 * 时间(秒)) */
    float speed = (float)delta_count * WHEEL_CIRCUMFERENCE / 
                  ((float)ENCODER_RESOLUTION * (float)ENCODER_QUADRATURE * (float)delta_time / 1000.0f);
    
    /* 更新时间戳和计数值 */
    last_time2 = current_time;
    last_count2 = current_count;
    
    return speed;
}
