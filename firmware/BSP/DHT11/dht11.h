#ifndef __DHT11_H
#define __DHT11_H

#include "main.h"

/**
 * @brief   DHT11温湿度传感器数据结构
 */
typedef struct {
    uint8_t humidity_integer;    /* 湿度整数部分(0-99%) */
    uint8_t humidity_decimal;    /* 湿度小数部分(DHT11固定为0) */
    uint8_t temperature_integer; /* 温度整数部分(-20~60℃) */
    uint8_t temperature_decimal; /* 温度小数部分(DHT11固定为0) */
    uint8_t checksum;            /* 校验和 */
} DHT11_Data_t;

/**
 * @brief   DHT11初始化函数
 * @note    初始化DHT11数据引脚，配置为推挽输出模式
 *          DHT11数据引脚：GPIOD_PIN_4
 * @param   None
 * @retval  None
 */
void DHT11_Init(void);

/**
 * @brief   读取DHT11温湿度数据
 * @note    执行完整的DHT11通信时序，读取40bit数据并进行校验
 *          若校验失败或超时，返回错误状态
 * @param   data: 指向DHT11_Data_t结构体的指针，用于存储读取的数据
 * @retval  0: 读取成功, -1: 读取失败(超时或校验错误)
 */
int8_t DHT11_ReadData(DHT11_Data_t *data);

#endif
