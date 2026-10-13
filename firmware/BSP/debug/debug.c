/**
 * @file    debug.c
 * @brief   调试模块实现文件
 * @details 重定向C标准库printf函数到USART1，实现串口调试输出
 *          使用USART1(PB14-TX, PB15-RX)，波特率115200
 */

#include "debug.h"
#include "usart.h"
#include <stdio.h>

/**
 * @brief   重定向fputc函数到USART1
 * @details 实现标准库printf函数的输出重定向，将字符发送到USART1
 *          每次发送一个字节，阻塞等待发送完成
 * @param   ch: 要发送的字符
 * @param   f: 文件指针(此处未使用)
 * @retval  发送的字符
 */
int fputc(int ch, FILE *f)
{
    /* 通过USART1发送字符 */
    HAL_UART_Transmit(
        &huart1,
        (uint8_t *)&ch,
        1,
        HAL_MAX_DELAY);

    return ch;
}

/**
 * @brief   调试模块初始化函数
 * @details 预留初始化接口，当前printf重定向已在fputc中实现
 *          如需添加其他调试功能可在此处扩展
 * @param   None
 * @retval  None
 */
void Debug_Init(void)
{
    /* 当前无需额外初始化，printf重定向已在fputc中实现 */
}
