/**
 * @file    pump.c
 * @brief   水泵PWM驱动模块实现文件
 * @details 基于STM32 HAL库实现水泵PWM控制，使用TIM8_CH3(PC8)输出PWM信号
 *          PWM配置参数: ARR=50, PSC已配置, PWM范围0~50
 *          所有变量均为static，避免全局污染
 */

#include "pump.h"
#include "tim.h"

/**
 * @defgroup 水泵PWM配置参数
 * @{
 */
#define PUMP_TIM_HANDLE            (&htim8)      /* TIM8句柄 */
#define PUMP_TIM_CHANNEL           TIM_CHANNEL_3 /* 使用通道3 */
#define PUMP_PWM_MAX               50            /* PWM最大值(对应ARR) */
#define PUMP_PWM_MIN               0             /* PWM最小值 */
/** @} */

/**
 * @brief   当前PWM值
 * @note    static变量，模块内部使用，不对外暴露
 */
static uint16_t pump_pwm = 0;

/**
 * @brief   水泵PWM驱动模块初始化函数
 * @details 启动TIM8_CH3通道PWM输出，水泵初始状态为关闭(PWM=0)
 *          使用HAL_TIM_PWM_Start()启动PWM，不阻塞
 * @param   None
 * @retval  None
 */
void Pump_Init(void)
{
    /* 设置初始PWM值为0，确保水泵关闭 */
    __HAL_TIM_SET_COMPARE(PUMP_TIM_HANDLE, PUMP_TIM_CHANNEL, PUMP_PWM_MIN);
    
    /* 启动TIM8_CH3通道PWM输出 */
    HAL_TIM_PWM_Start(PUMP_TIM_HANDLE, PUMP_TIM_CHANNEL);
    
    /* 更新当前PWM值 */
    pump_pwm = PUMP_PWM_MIN;
}

/**
 * @brief   启动水泵
 * @details 如果当前PWM值为0，则默认设置为100%输出(PWM=50)
 *          如果当前已有非零PWM值，则保持不变继续运行
 * @param   None
 * @retval  None
 */
void Pump_Start(void)
{
    if(pump_pwm == PUMP_PWM_MIN)
    {
        /* 当前PWM为0，默认开启100%输出 */
        Pump_SetPWM(PUMP_PWM_MAX);
    }
}

/**
 * @brief   停止水泵
 * @details 将PWM输出设为0，水泵停止工作
 * @param   None
 * @retval  None
 */
void Pump_Stop(void)
{
    /* 设置PWM值为0 */
    Pump_SetPWM(PUMP_PWM_MIN);
}

/**
 * @brief   设置水泵PWM值
 * @details 直接设置PWM比较值，范围0~50，超出范围自动限幅
 *          使用__HAL_TIM_SET_COMPARE()更新PWM，不阻塞
 * @param   pwm: PWM值，范围0~50
 * @retval  None
 */
void Pump_SetPWM(uint16_t pwm)
{
    /* PWM值限幅处理 */
    if(pwm > PUMP_PWM_MAX)
    {
        pwm = PUMP_PWM_MAX;
    }
    
    /* 更新当前PWM值 */
    pump_pwm = pwm;
    
    /* 设置TIM8_CH3通道比较值，更新PWM输出 */
    __HAL_TIM_SET_COMPARE(PUMP_TIM_HANDLE, PUMP_TIM_CHANNEL, pump_pwm);
}

/**
 * @brief   设置水泵百分比
 * @details 将百分比转换为PWM值，0%对应PWM=0，100%对应PWM=50
 *          计算公式: PWM = percent * 50 / 100
 * @param   percent: 百分比，范围0~100
 * @retval  None
 */
void Pump_SetPercent(uint8_t percent)
{
    uint16_t pwm_value;
    
    /* 百分比限幅处理 */
    if(percent > 100)
    {
        percent = 100;
    }
    
    /* 将百分比转换为PWM值 */
    pwm_value = (uint16_t)((uint32_t)percent * PUMP_PWM_MAX / 100);
    
    /* 设置PWM值 */
    Pump_SetPWM(pwm_value);
}

/**
 * @brief   获取当前PWM值
 * @details 返回水泵当前的PWM比较值
 * @param   None
 * @retval  当前PWM值，范围0~50
 */
uint16_t Pump_GetPWM(void)
{
    return pump_pwm;
}

/**
 * @brief   判断水泵是否正在运行
 * @details 根据当前PWM值判断水泵状态
 * @param   None
 * @retval  1: 水泵正在运行(PWM>0), 0: 水泵已停止(PWM=0)
 */
uint8_t Pump_IsRunning(void)
{
    return (pump_pwm > PUMP_PWM_MIN) ? 1 : 0;
}
