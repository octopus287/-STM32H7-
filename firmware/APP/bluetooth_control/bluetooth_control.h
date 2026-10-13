#ifndef __BLUETOOTH_CONTROL_H
#define __BLUETOOTH_CONTROL_H

#include "main.h"

/**
 * @defgroup 蓝牙控制速度参数配置
 * @{
 */
#define SPEED_MIN        0.2f    /* 最小速度(m/s) */
#define SPEED_MAX        1.5f    /* 最大速度(m/s) */
#define SPEED_STEP       0.2f    /* 速度步进值(m/s) */
#define SPEED_DEFAULT    0.5f    /* 默认速度(m/s) */

#define SPEED_GEAR1      0.2f    /* 1档速度(爬行): 0.2m/s */
#define SPEED_GEAR2      0.4f    /* 2档速度(慢速): 0.4m/s */
#define SPEED_GEAR3      0.7f    /* 3档速度(正常): 0.7m/s */
#define SPEED_GEAR4      1.0f    /* 4档速度(快速): 1.0m/s */
#define SPEED_GEAR5      1.3f    /* 5档速度(极速): 1.3m/s */
/** @} */

/** 当前目标速度全局变量 */
extern float Car_TargetSpeed;

/**
 * @brief   蓝牙控制初始化函数
 * @note    初始化目标速度为默认值
 * @param   None
 * @retval  None
 */
void Bluetooth_Control_Init(void);

/**
 * @brief   蓝牙控制更新函数
 * @note    检查蓝牙接收数据，解析控制指令并执行相应动作
 * @param   None
 * @retval  None
 */
void Bluetooth_Control_Update(void);

#endif
