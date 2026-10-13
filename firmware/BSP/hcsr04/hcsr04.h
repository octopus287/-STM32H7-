#ifndef __HCSR04_H
#define __HCSR04_H

#include "main.h"

/**
 * @defgroup HCSR04参数配置
 * @{
 */
#define HCSR04_MAX_DISTANCE        400.0f   /* 最大测量距离(cm) */
#define HCSR04_MIN_DISTANCE        2.0f     /* 最小测量距离(cm) */
#define HCSR04_TIMEOUT_MS          100      /* 测量超时时间(ms) */
#define HCSR04_SAMPLE_COUNT        10       /* 采样次数 */
#define HCSR04_OUTLIER_THRESHOLD   30.0f    /* 异常值过滤阈值(cm) */
/** @} */

/**
 * @brief   HC-SR04超声波模块初始化函数
 * @details 启动TIM6定时器，用于测量Echo信号高电平持续时间
 *          使用外部中断(PC1)检测Echo信号的上升沿和下降沿
 *          Trig触发引脚：PC0
 *          Echo接收引脚：PC1(外部中断)
 * @param   None
 * @retval  None
 */
void HCSR04_Init(void);

/**
 * @brief   获取单次测量距离
 * @details 触发超声波模块并等待回波信号，根据TIM6计数值计算距离
 *          距离计算公式：Distance(cm) = time_us * 0.0343 / 2
 *          超时返回-1，超出有效范围(2~400cm)也返回-1
 * @param   None
 * @retval  距离值(cm)，返回-1表示测量失败
 */
float HCSR04_GetDistance(void);

/**
 * @brief   获取多次测量平均距离(带滤波)
 * @details 进行10次测量，使用异常值过滤和中值平均算法提高测量精度
 *          异常值判定：与前一个有效值相差超过30cm则视为异常值
 *          有效采样数不足一半时返回-1
 *          采样间隔：20ms
 * @param   None
 * @retval  距离值(cm)，返回-1表示测量失败
 */
float HCSR04_GetDistance_Avg(void);

/**
 * @brief   Echo信号中断回调函数
 * @details 由外部中断处理函数调用，用于检测Echo信号的上升沿和下降沿
 *          上升沿时启动TIM6计数，下降沿时停止计数并计算距离
 * @param   state: 0-下降沿(回波结束), 1-上升沿(回波开始)
 * @retval  None
 */
void HCSR04_Echo_Callback(uint8_t state);

#endif
