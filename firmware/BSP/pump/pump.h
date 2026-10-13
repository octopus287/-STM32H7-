#ifndef __PUMP_H
#define __PUMP_H

#include "main.h"

/**
 * @brief   水泵PWM驱动模块初始化函数
 * @details 启动TIM8_CH3通道PWM输出，水泵初始状态为关闭(PWM=0)
 * @param   None
 * @retval  None
 */
void Pump_Init(void);

/**
 * @brief   启动水泵
 * @details 如果当前PWM值为0，则默认设置为100%输出(PWM=50)
 *          如果当前已有非零PWM值，则保持不变
 * @param   None
 * @retval  None
 */
void Pump_Start(void);

/**
 * @brief   停止水泵
 * @details 将PWM输出设为0，水泵停止工作
 * @param   None
 * @retval  None
 */
void Pump_Stop(void);

/**
 * @brief   设置水泵PWM值
 * @details 直接设置PWM比较值，范围0~50，超出范围自动限幅
 * @param   pwm: PWM值，范围0~50，超过50自动限幅为50，负数自动限幅为0
 * @retval  None
 */
void Pump_SetPWM(uint16_t pwm);

/**
 * @brief   设置水泵百分比
 * @details 将百分比转换为PWM值，0%对应PWM=0，100%对应PWM=50
 * @param   percent: 百分比，范围0~100
 * @retval  None
 */
void Pump_SetPercent(uint8_t percent);

/**
 * @brief   获取当前PWM值
 * @details 返回水泵当前的PWM比较值
 * @param   None
 * @retval  当前PWM值，范围0~50
 */
uint16_t Pump_GetPWM(void);

/**
 * @brief   判断水泵是否正在运行
 * @details 根据当前PWM值判断水泵状态
 * @param   None
 * @retval  1: 水泵正在运行(PWM>0), 0: 水泵已停止(PWM=0)
 */
uint8_t Pump_IsRunning(void);

#endif
