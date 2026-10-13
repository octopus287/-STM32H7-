/**
 * @file    dht11.c
 * @brief   DHT11温湿度传感器驱动实现文件
 * @details 基于单总线协议实现DHT11传感器的数据读取
 *          使用DWT计数器实现微秒级精确延时
 *          DHT11数据引脚：GPIOD_PIN_4
 *          数据格式：湿度整数(1字节)+湿度小数(1字节)+温度整数(1字节)+温度小数(1字节)+校验和(1字节)
 */

#include "dht11.h"
#include "gpio.h"
#include "core_cm7.h"

/** DHT11数据引脚定义 */
#define DHT11_GPIO_PORT           GPIOD
#define DHT11_GPIO_PIN            GPIO_PIN_4

/**
 * @brief   DHT11微秒延时函数
 * @details 使用DWT计数器实现微秒级精确延时，精度可达1us
 * @param   us: 延时时间(微秒)
 * @retval  None
 */
static void DHT11_DelayUs(uint32_t us)
{
    uint32_t ticks_start = DWT->CYCCNT;
    uint32_t ticks_needed = us * (SystemCoreClock / 1000000);
    
    while((DWT->CYCCNT - ticks_start) < ticks_needed);
}

/**
 * @brief   DHT11毫秒延时函数
 * @details 基于微秒延时函数实现毫秒级延时
 * @param   ms: 延时时间(毫秒)
 * @retval  None
 */
static void DHT11_DelayMs(uint32_t ms)
{
    for(uint32_t i = 0; i < ms; i++)
    {
        DHT11_DelayUs(1000);
    }
}

/**
 * @brief   将DHT11引脚设置为输出模式
 * @details 配置引脚为开漏输出模式，上拉电阻使能
 * @param   None
 * @retval  None
 */
static void DHT11_SetOutput(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    GPIO_InitStruct.Pin = DHT11_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DHT11_GPIO_PORT, &GPIO_InitStruct);
}

/**
 * @brief   将DHT11引脚设置为输入模式
 * @details 配置引脚为输入模式，上拉电阻使能，用于接收传感器数据
 * @param   None
 * @retval  None
 */
static void DHT11_SetInput(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    GPIO_InitStruct.Pin = DHT11_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(DHT11_GPIO_PORT, &GPIO_InitStruct);
}

/**
 * @brief   读取DHT11的一位数据
 * @details 根据高电平持续时间判断数据位(0或1)
 *          0: 低电平50us + 高电平26-28us
 *          1: 低电平50us + 高电平70us
 * @param   None
 * @retval  读取的位值(0或1)，超时返回0
 */
static uint8_t DHT11_ReadBit(void)
{
    uint8_t retry = 0;
    
    /* 等待低电平结束 */
    while(HAL_GPIO_ReadPin(DHT11_GPIO_PORT, DHT11_GPIO_PIN) == GPIO_PIN_SET)
    {
        if(retry++ > 100) return 0;
        DHT11_DelayUs(1);
    }
    
    retry = 0;
    
    /* 等待高电平开始 */
    while(HAL_GPIO_ReadPin(DHT11_GPIO_PORT, DHT11_GPIO_PIN) == GPIO_PIN_RESET)
    {
        if(retry++ > 100) return 0;
        DHT11_DelayUs(1);
    }
    
    /* 延时40us后判断高电平状态 */
    DHT11_DelayUs(40);
    
    if(HAL_GPIO_ReadPin(DHT11_GPIO_PORT, DHT11_GPIO_PIN) == GPIO_PIN_SET) return 1;
    else return 0;
}

/**
 * @brief   读取DHT11的一个字节数据
 * @details 依次读取8位数据，组合成一个字节
 * @param   None
 * @retval  读取的字节数据
 */
static uint8_t DHT11_ReadByte(void)
{
    uint8_t byte = 0;
    
    for(uint8_t i = 0; i < 8; i++)
    {
        byte <<= 1;
        byte |= DHT11_ReadBit();
    }
    
    return byte;
}

/**
 * @brief   DHT11传感器初始化函数
 * @details 初始化DHT11数据引脚，配置DWT计数器用于微秒延时
 *          引脚初始状态为高电平，等待传感器稳定(1秒)
 * @param   None
 * @retval  None
 */
void DHT11_Init(void)
{
    /* 使能GPIOD时钟 */
    __HAL_RCC_GPIOD_CLK_ENABLE();
    
    /* 初始化DWT计数器用于微秒延时 */
    if((DWT->CTRL & DWT_CTRL_CYCCNTENA_Msk) == 0)
    {
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
        DWT->CYCCNT = 0;
        DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    }
    
    /* 设置引脚为输出模式 */
    DHT11_SetOutput();
    /* 初始状态为高电平 */
    HAL_GPIO_WritePin(DHT11_GPIO_PORT, DHT11_GPIO_PIN, GPIO_PIN_SET);
    
    /* 等待传感器稳定(至少1秒) */
    DHT11_DelayMs(1000);
}

/**
 * @brief   读取DHT11温湿度数据
 * @details 执行完整的DHT11通信时序：
 *          1. 主机发送开始信号(低电平18ms + 高电平30us)
 *          2. 等待传感器响应(低电平80us + 高电平80us)
 *          3. 读取40位数据(5字节)
 *          4. 校验数据完整性
 * @param   data: 指向DHT11_Data_t结构体的指针，用于存储读取的数据
 * @retval  0: 读取成功, -1: 参数错误或传感器未响应, -2: 响应低电平超时, 
 *          -3: 响应高电平超时, -4: 校验错误
 */
int8_t DHT11_ReadData(DHT11_Data_t *data)
{
    if(data == NULL) return -1;
    
    uint8_t buf[5] = {0};
    
    /* 主机发送开始信号 */
    DHT11_SetOutput();
    HAL_GPIO_WritePin(DHT11_GPIO_PORT, DHT11_GPIO_PIN, GPIO_PIN_RESET);
    DHT11_DelayMs(18);
    
    /* 释放总线 */
    HAL_GPIO_WritePin(DHT11_GPIO_PORT, DHT11_GPIO_PIN, GPIO_PIN_SET);
    DHT11_DelayUs(30);
    
    /* 切换为输入模式，等待传感器响应 */
    DHT11_SetInput();
    
    /* 等待传感器拉低总线(响应信号) */
    if(HAL_GPIO_ReadPin(DHT11_GPIO_PORT, DHT11_GPIO_PIN) == GPIO_PIN_SET)
    {
        return -1;
    }
    
    /* 等待低电平结束(80us) */
    uint8_t retry = 0;
    while(HAL_GPIO_ReadPin(DHT11_GPIO_PORT, DHT11_GPIO_PIN) == GPIO_PIN_RESET)
    {
        if(retry++ > 200) return -2;
        DHT11_DelayUs(1);
    }
    
    /* 等待高电平结束(80us) */
    retry = 0;
    while(HAL_GPIO_ReadPin(DHT11_GPIO_PORT, DHT11_GPIO_PIN) == GPIO_PIN_SET)
    {
        if(retry++ > 200) return -3;
        DHT11_DelayUs(1);
    }
    
    /* 读取5字节数据 */
    for(uint8_t i = 0; i < 5; i++)
    {
        buf[i] = DHT11_ReadByte();
    }
    
    /* 恢复引脚为输出模式，释放总线 */
    DHT11_SetOutput();
    HAL_GPIO_WritePin(DHT11_GPIO_PORT, DHT11_GPIO_PIN, GPIO_PIN_SET);
    
    /* 校验数据完整性 */
    if(buf[4] == buf[0] + buf[1] + buf[2] + buf[3])
    {
        data->humidity_integer = buf[0];
        data->humidity_decimal = buf[1];
        data->temperature_integer = buf[2];
        data->temperature_decimal = buf[3];
        data->checksum = buf[4];
        return 0;
    }
    
    return -4;
}
