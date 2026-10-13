/**
 * @file    bluetooth.c
 * @brief   蓝牙模块驱动实现文件
 * @details 基于USART3实现蓝牙通信，支持中断接收和环形缓冲区
 *          使用USART3(PB10-TX, PB11-RX)，波特率9600
 *          接收采用环形缓冲区(64字节)，支持中断方式接收数据
 */

#include "bluetooth.h"
#include "usart.h"
#include "stdio.h"
#include "string.h"

/** 蓝牙模块状态结构体 */
static Bluetooth_TypeDef Bluetooth = {0};
/** 临时接收变量，用于中断回调 */
static uint8_t RxTemp = 0;
/** 上次接收数据的时间戳 */
static uint32_t Bluetooth_LastRxTime = 0;

/**
 * @brief   蓝牙模块初始化函数
 * @details 初始化环形缓冲区指针，启动USART3中断接收(单字节)
 * @param   None
 * @retval  None
 */
void Bluetooth_Init(void)
{
    /* 初始化缓冲区指针 */
    Bluetooth.RxHead = 0;
    Bluetooth.RxTail = 0;
    /* 清零接收标志 */
    Bluetooth.RxFlag = 0;
    /* 清零最新数据 */
    Bluetooth.LatestData = 0;
    /* 初始化连接状态为未连接 */
    Bluetooth.Connected = 0;
    /* 初始化最后接收时间 */
    Bluetooth_LastRxTime = HAL_GetTick();
    
    /* 启动USART3中断接收，每次接收1字节 */
    HAL_UART_Receive_IT(&huart3, &RxTemp, 1);
}

/**
 * @brief   蓝牙发送字符串
 * @details 通过USART3逐字符发送以'\0'结尾的字符串
 * @param   str: 字符串指针
 * @retval  None
 */
void Bluetooth_SendString(char *str)
{
    while(*str)
    {
        /* 发送单个字符 */
        Bluetooth_SendChar((uint8_t)*str++);
    }
}

/**
 * @brief   蓝牙发送单个字符
 * @details 通过USART3发送单个字节数据，阻塞等待发送完成
 * @param   ch: 要发送的字符(字节)
 * @retval  None
 */
void Bluetooth_SendChar(uint8_t ch)
{
    HAL_UART_Transmit(&huart3, &ch, 1, HAL_MAX_DELAY);
}

/**
 * @brief   蓝牙发送数据
 * @details 通过USART3发送指定长度的数据，阻塞等待发送完成
 * @param   data: 数据缓冲区指针
 * @param   len: 数据长度(字节数)
 * @retval  None
 */
void Bluetooth_SendData(uint8_t *data, uint16_t len)
{
    HAL_UART_Transmit(&huart3, data, len, HAL_MAX_DELAY);
}

/**
 * @brief   获取蓝牙接收数据
 * @details 从环形缓冲区读取一个字节数据，更新读取指针
 *          当缓冲区为空时返回0
 * @param   None
 * @retval  接收到的数据(若无数据返回0)
 */
uint8_t Bluetooth_GetRxData(void)
{
    uint8_t data = 0;
    
    /* 检查缓冲区是否有数据 */
    if(Bluetooth.RxHead != Bluetooth.RxTail)
    {
        /* 从缓冲区读取数据 */
        data = Bluetooth.RxBuffer[Bluetooth.RxTail];
        /* 更新读取指针(环形) */
        Bluetooth.RxTail = (Bluetooth.RxTail + 1) % BT_RX_BUFFER_SIZE;
        
        /* 检查缓冲区是否为空 */
        if(Bluetooth.RxHead == Bluetooth.RxTail)
        {
            /* 缓冲区为空，清零接收标志 */
            Bluetooth.RxFlag = 0;
        }
    }
    
    return data;
}

/**
 * @brief   清除接收标志
 * @details 清零RxFlag标志位，用于手动清除接收状态
 * @param   None
 * @retval  None
 */
void Bluetooth_ClearRxFlag(void)
{
    Bluetooth.RxFlag = 0;
}

/**
 * @brief   检查是否有数据接收
 * @details 返回RxFlag标志位状态，判断环形缓冲区是否有未读取数据
 * @param   None
 * @retval  1表示有数据，0表示无数据
 */
uint8_t Bluetooth_IsReceived(void)
{
    return Bluetooth.RxFlag;
}

uint8_t Bluetooth_IsConnected(void)
{
    return Bluetooth.Connected;
}

void Bluetooth_CheckConnectionTimeout(void)
{
    uint32_t current_time = HAL_GetTick();
    
    if(Bluetooth.Connected == 1)
    {
        if((current_time - Bluetooth_LastRxTime) > 5000)
        {
            Bluetooth.Connected = 0;
        }
    }
}

void Bluetooth_SetConnected(uint8_t connected)
{
    Bluetooth.Connected = connected;
    if(connected == 1)
    {
        Bluetooth_LastRxTime = HAL_GetTick();
    }
}

/**
 * @brief   USART3接收完成中断回调函数
 * @details 当USART3接收到一个字节时触发，将数据存入环形缓冲区
 *          同时回显接收到的数据，并重新启动中断接收
 * @param   huart: UART句柄指针
 * @retval  None
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART3)
    {
        /* 设置连接状态为已连接 */
        Bluetooth.Connected = 1;
        /* 更新最后接收时间 */
        Bluetooth_LastRxTime = HAL_GetTick();
        
        /* 保存最新接收数据 */
        Bluetooth.LatestData = RxTemp;
        
        /* 计算下一个写入位置 */
        uint16_t next_head = (Bluetooth.RxHead + 1) % BT_RX_BUFFER_SIZE;
        
        /* 检查缓冲区是否已满 */
        if(next_head != Bluetooth.RxTail)
        {
            /* 存入数据 */
            Bluetooth.RxBuffer[Bluetooth.RxHead] = RxTemp;
            /* 更新写入指针 */
            Bluetooth.RxHead = next_head;
            /* 设置接收标志 */
            Bluetooth.RxFlag = 1;
        }
        
        /* 串口调试输出接收到的数据 */
        printf("Bluetooth Receive:%c\r\n", RxTemp);
        
        /* 回显数据给蓝牙模块 */
        Bluetooth_SendString("Receive:");
        Bluetooth_SendChar(RxTemp);
        Bluetooth_SendString("\r\n");
        
        /* 重新启动中断接收 */
        HAL_UART_Receive_IT(&huart3, &RxTemp, 1);
    }
}
