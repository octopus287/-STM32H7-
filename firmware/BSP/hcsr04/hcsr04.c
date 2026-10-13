/**
 * @file    hcsr04.c
 * @brief   HC-SR04超声波测距模块驱动实现文件
 * @details 基于STM32 HAL库实现超声波距离测量
 *          使用TIM6测量Echo信号高电平持续时间
 *          使用外部中断(PC1)检测Echo信号的上升沿和下降沿
 *          Trig触发引脚：PC0
 *          距离计算公式：Distance = time_us * 0.0343 / 2
 *          （声速约为343m/s，往返时间除以2）
 */

#include "hcsr04.h"
#include "tim.h"
#include "gpio.h"

/* 私有变量定义 */
/** Echo信号结束时间戳(TIM6计数值) */
static uint32_t End_Time = 0;
/** 捕获状态标志：0-空闲，1-正在捕获 */
static uint8_t Capture_State = 0;
/** 计算得到的距离值(厘米) */
static float Distance = 0;
/** 捕获完成标志 */
static uint8_t Capture_Finished = 0;

/* 宏定义配置参数 */
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
 * @brief   触发超声波模块发送信号
 * @details 输出10us高电平脉冲到Trig引脚，触发HC-SR04发送超声波
 *          Trig引脚：PC0
 * @param   None
 * @retval  None
 */
static void HCSR04_Trigger(void)
{
    /* 确保Trig引脚为低电平 */
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_RESET);
    /* 延时1us确保低电平稳定 */
    uint32_t tickstart = HAL_GetTick();
    while((HAL_GetTick() - tickstart) < 1);
    
    /* 输出10us高电平触发信号 */
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_SET);
    tickstart = HAL_GetTick();
    while((HAL_GetTick() - tickstart) < 1);
    
    /* 恢复低电平 */
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_RESET);
}

/**
 * @brief   HC-SR04超声波模块初始化函数
 * @details 启动TIM6定时器，用于测量Echo信号高电平持续时间
 *          TIM6配置：预分频器已在CubeMX中配置，计数频率80MHz
 *          Echo信号使用外部中断(PC1)检测上升沿和下降沿
 * @param   None
 * @retval  None
 */
void HCSR04_Init(void)
{
    /* 启动TIM6基础定时器 */
    HAL_TIM_Base_Start(&htim6);
}

/**
 * @brief   获取单次测量距离
 * @details 触发超声波模块并等待回波信号，根据TIM6计数值计算距离
 *          距离计算公式：Distance(cm) = time_us * 0.0343 / 2
 *          超时返回-1，超出有效范围也返回-1
 * @param   None
 * @retval  距离值(cm)，返回-1表示测量失败
 */
float HCSR04_GetDistance(void)
{
    /* 重置状态变量 */
    Capture_Finished = 0;
    Capture_State = 0;
    Distance = 0;
    __HAL_TIM_SET_COUNTER(&htim6, 0);

    /* 触发超声波发送 */
    HCSR04_Trigger();

    /* 等待捕获完成或超时 */
    uint32_t tickstart = HAL_GetTick();
    while(Capture_Finished == 0)
    {
        if(HAL_GetTick() - tickstart > HCSR04_TIMEOUT_MS)
        {
            /* 超时，返回错误 */
            return -1;
        }
    }

    /* 检查距离是否在有效范围内 */
    if(Distance < HCSR04_MIN_DISTANCE || Distance > HCSR04_MAX_DISTANCE)
    {
        return -1;
    }

    return Distance;
}

/**
 * @brief   对采样数组进行排序并取中间值平均
 * @details 使用冒泡排序对数组排序，去除前1/4和后1/4的数据，取中间部分求平均
 *          有效减少异常值对测量结果的影响
 * @param   array: 采样数据数组指针
 * @param   size: 数组元素个数
 * @retval  中间值平均值
 */
static float HCSR04_SortArray(float *array, uint8_t size)
{
    /* 冒泡排序 */
    for(uint8_t i = 0; i < size - 1; i++)
    {
        for(uint8_t j = i + 1; j < size; j++)
        {
            if(array[i] > array[j])
            {
                float temp = array[i];
                array[i] = array[j];
                array[j] = temp;
            }
        }
    }
    
    /* 计算中间部分平均值(去除前后各1/4) */
    float sum = 0;
    uint8_t start = size / 4;
    uint8_t end = size - size / 4;
    
    for(uint8_t i = start; i < end; i++)
    {
        sum += array[i];
    }
    
    return sum / (end - start);
}

/**
 * @brief   获取多次测量平均距离(带滤波)
 * @details 进行10次测量，使用异常值过滤和中值平均算法提高测量精度
 *          异常值判定：与前一个有效值相差超过30cm则视为异常值
 *          有效采样数不足一半时返回-1
 * @param   None
 * @retval  距离值(cm)，返回-1表示测量失败
 */
float HCSR04_GetDistance_Avg(void)
{
    float samples[HCSR04_SAMPLE_COUNT];
    uint8_t valid_count = 0;

    /* 进行多次采样 */
    for(uint8_t i = 0; i < HCSR04_SAMPLE_COUNT; i++)
    {
        float dist = HCSR04_GetDistance();
        if(dist > 0)
        {
            /* 异常值过滤：与前一个有效值相差不超过阈值 */
            if(valid_count == 0 || 
               (dist > samples[valid_count - 1] - HCSR04_OUTLIER_THRESHOLD && 
                dist < samples[valid_count - 1] + HCSR04_OUTLIER_THRESHOLD))
            {
                samples[valid_count++] = dist;
            }
        }
        /* 采样间隔20ms */
        HAL_Delay(20);
    }

    /* 检查有效采样数 */
    if(valid_count < HCSR04_SAMPLE_COUNT / 2)
    {
        /* 有效采样数不足，返回错误 */
        return -1;
    }

    /* 返回中值平均值 */
    return HCSR04_SortArray(samples, valid_count);
}

/**
 * @brief   Echo信号中断回调函数
 * @details 由外部中断处理函数调用，用于检测Echo信号的上升沿和下降沿
 *          上升沿时启动TIM6计数，下降沿时停止计数并计算距离
 * @param   state: 0-下降沿(回波结束), 1-上升沿(回波开始)
 * @retval  None
 */
void HCSR04_Echo_Callback(uint8_t state)
{
    if(state == 1)
    {
        /* 上升沿：回波开始，启动计数 */
        Capture_State = 1;
        __HAL_TIM_SET_COUNTER(&htim6, 0);
    }
    else
    {
        /* 下降沿：回波结束，计算距离 */
        if(Capture_State == 1)
        {
            Capture_State = 0;
            /* 获取TIM6计数值 */
            End_Time = __HAL_TIM_GET_COUNTER(&htim6);
            
            /* 计算时间和距离 */
            /* TIM6时钟频率 = 80MHz / (预分频器 + 1) */
            float clock_freq = 80000000.0f / (htim6.Init.Prescaler + 1);
            /* 时间(微秒) = 计数值 / 时钟频率 * 1000000 */
            float time_us = End_Time * 1000000.0f / clock_freq;
            /* 距离(厘米) = 时间 * 声速(0.0343cm/us) / 2 */
            Distance = time_us * 0.0343f / 2.0f;
            
            /* 设置捕获完成标志 */
            Capture_Finished = 1;
        }
    }
}
