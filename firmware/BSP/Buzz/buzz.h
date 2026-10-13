#ifndef __BUZZ_H
#define __BUZZ_H

#include "main.h"

/**
 * @brief   蜂鸣器初始化函数
 * @note    配置PD5为推挽输出，初始状态为高电平（关闭蜂鸣器）
 * @param   None
 * @retval  None
 */
void Buzz_Init(void);

/**
 * @brief   蜂鸣器响一声
 * @note    低电平触发，持续时间由参数指定
 * @param   duration_ms: 蜂鸣器持续时间(毫秒)
 * @retval  None
 */
void Buzz_Beep(uint16_t duration_ms);

/**
 * @brief   蜂鸣器响两声
 * @note    低电平触发，两声之间有间隔
 * @param   duration_ms: 每次蜂鸣器持续时间(毫秒)
 * @param   interval_ms: 两声之间的间隔时间(毫秒)
 * @retval  None
 */
void Buzz_BeepTwice(uint16_t duration_ms, uint16_t interval_ms);

/**
 * @brief   打开蜂鸣器
 * @note    输出低电平触发蜂鸣器
 * @param   None
 * @retval  None
 */
void Buzz_On(void);

/**
 * @brief   关闭蜂鸣器
 * @note    输出高电平关闭蜂鸣器
 * @param   None
 * @retval  None
 */
void Buzz_Off(void);

#endif
