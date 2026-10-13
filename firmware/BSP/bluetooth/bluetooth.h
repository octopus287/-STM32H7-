#ifndef __BLUETOOTH_H
#define __BLUETOOTH_H

#include "main.h"

/**
 * @defgroup 蓝牙模块参数配置
 * @{
 */
#define BT_RX_BUFFER_SIZE      64    /* 接收缓冲区大小 */
/** @} */

/**
 * @brief   蓝牙模块状态结构体定义
 * @details 包含环形缓冲区、读写指针、接收标志和最新数据
 */
typedef struct
{
    uint8_t  RxBuffer[BT_RX_BUFFER_SIZE];  /* 环形接收缓冲区 */
    uint16_t RxHead;                       /* 写入指针 */
    uint16_t RxTail;                       /* 读取指针 */
    uint8_t  RxFlag;                       /* 接收完成标志 */
    uint8_t  LatestData;                   /* 最新接收到的数据 */
    uint8_t  Connected;                    /* 连接状态: 1已连接, 0未连接 */
} Bluetooth_TypeDef;

/**
 * @brief   蓝牙模块初始化函数
 * @note    初始化接收缓冲区，启动USART3中断接收
 * @param   None
 * @retval  None
 */
void Bluetooth_Init(void);

/**
 * @brief   蓝牙发送字符串
 * @note    通过USART3发送以'\0'结尾的字符串
 * @param   str: 字符串指针
 * @retval  None
 */
void Bluetooth_SendString(char *str);

/**
 * @brief   蓝牙发送单个字符
 * @note    通过USART3发送单个字节
 * @param   ch: 要发送的字符
 * @retval  None
 */
void Bluetooth_SendChar(uint8_t ch);

/**
 * @brief   蓝牙发送数据
 * @note    通过USART3发送指定长度的数据
 * @param   data: 数据缓冲区指针
 * @param   len: 数据长度
 * @retval  None
 */
void Bluetooth_SendData(uint8_t *data, uint16_t len);

/**
 * @brief   获取蓝牙接收数据
 * @note    从环形缓冲区读取一个字节数据，更新读取指针
 * @param   None
 * @retval  接收到的数据(若无数据返回0)
 */
uint8_t Bluetooth_GetRxData(void);

/**
 * @brief   清除接收标志
 * @note    清零RxFlag标志位
 * @param   None
 * @retval  None
 */
void Bluetooth_ClearRxFlag(void);

/**
 * @brief   检查是否有数据接收
 * @note    返回RxFlag标志位状态
 * @param   None
 * @retval  1表示有数据，0表示无数据
 */
uint8_t Bluetooth_IsReceived(void);

/**
 * @brief   检查蓝牙连接状态
 * @note    返回Connected标志位状态
 * @param   None
 * @retval  1表示已连接，0表示未连接
 */
uint8_t Bluetooth_IsConnected(void);

/**
 * @brief   检查蓝牙连接超时
 * @note    如果超过5秒没有接收到数据，认为连接断开
 * @param   None
 * @retval  None
 */
void Bluetooth_CheckConnectionTimeout(void);

/**
 * @brief   强制设置蓝牙连接状态
 * @note    用于外部手动设置连接状态
 * @param   connected: 1表示已连接, 0表示未连接
 * @retval  None
 */
void Bluetooth_SetConnected(uint8_t connected);

#endif
