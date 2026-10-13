/**
 * @file    bluetooth_control.c
 * @brief   蓝牙遥控控制模块实现文件
 * @details 解析蓝牙接收到的控制指令，实现小车运动控制、速度调节和水泵控制
 *          支持的指令:
 *          - F: 前进
 *          - B: 后退
 *          - L: 左转
 *          - R: 右转
 *          - S: 停止
 *          - +: 加速
 *          - -: 减速
 *          - 1~5: 5档速度切换
 *          - P: 开启水泵
 *          - O: 关闭水泵
 *          - W0~W100: 设置水泵PWM百分比
 */

#include "bluetooth_control.h"
#include "bluetooth.h"
#include "car.h"
#include "pump.h"
#include "stdio.h"

/** 当前目标速度(m/s) */
float Car_TargetSpeed = SPEED_DEFAULT;

/**
 * @defgroup 水泵命令解析状态机
 * @{
 */
typedef enum {
    PUMP_STATE_IDLE,       /* 空闲状态 */
    PUMP_STATE_W_RECV,     /* 收到'W'字符，等待数字 */
    PUMP_STATE_W_DIGIT1,   /* 收到第一个数字 */
    PUMP_STATE_W_DIGIT2    /* 收到第二个数字 */
} PumpParseState_t;

/** 水泵命令解析状态 */
static PumpParseState_t pump_parse_state = PUMP_STATE_IDLE;
/** 水泵百分比临时值 */
static uint8_t pump_percent_temp = 0;
/** 水泵命令超时时间戳 */
static uint32_t pump_parse_timeout = 0;
/** 水泵命令超时时间(ms) */
#define PUMP_PARSE_TIMEOUT_MS    500
/** @} */

/**
 * @brief   解析水泵命令中的字符
 * @details 状态机解析W命令，支持W0~W100格式，非数字字符触发命令执行
 * @param   ch: 接收到的字符
 * @retval  1: 字符已被水泵解析器处理，0: 字符未被处理(需要主switch处理)
 */
static uint8_t Bluetooth_Control_ParsePumpChar(uint8_t ch);

/**
 * @brief   解析蓝牙控制命令
 * @details 解析小车运动控制命令和水泵控制命令(P/O/W)
 * @param   cmd: 命令字符
 * @retval  None
 */
static void Bluetooth_Control_ParseCmd(uint8_t cmd);

/**
 * @brief   蓝牙控制初始化函数
 * @details 初始化目标速度为默认值(SPEED_DEFAULT = 0.5m/s)
 * @param   None
 * @retval  None
 */
void Bluetooth_Control_Init(void)
{
    Car_TargetSpeed = SPEED_DEFAULT;
    
    Pump_Init();
}

/**
 * @brief   蓝牙控制更新函数
 * @details 检查蓝牙接收缓冲区，解析控制指令并执行相应动作
 *          指令格式为单字符，通过串口调试输出执行结果
 *          控制指令列表:
 *          F - 前进, B - 后退, L - 左转, R - 右转, S - 停止
 *          + - 加速(+0.2m/s), - - 减速(-0.2m/s)
 *          1~5 - 5档速度切换
 * @param   None
 * @retval  None
 */
void Bluetooth_Control_Update(void)
{
    /* 检查是否有蓝牙数据接收 */
    if(Bluetooth_IsReceived())
    {
        /* 获取接收到的指令 */
        uint8_t cmd = Bluetooth_GetRxData();
        
        /* 检查水泵命令超时 */
        if(pump_parse_state != PUMP_STATE_IDLE)
        {
            if((HAL_GetTick() - pump_parse_timeout) > PUMP_PARSE_TIMEOUT_MS)
            {
                pump_parse_state = PUMP_STATE_IDLE;
                printf("Invalid Pump Command\r\n");
            }
        }
        
        /* 如果正在解析水泵命令，优先处理 */
        if(pump_parse_state != PUMP_STATE_IDLE)
        {
            /* 路由到水泵解析器 */
            if(Bluetooth_Control_ParsePumpChar(cmd) == 0)
            {
                /* 水泵解析器未处理该字符，继续主switch处理 */
                /* 但在此之前，如果有未完成的水泵命令，先执行它 */
                if(pump_parse_state != PUMP_STATE_IDLE)
                {
                    /* 执行累积的水泵命令 */
                    printf("Pump %d%%\r\n", pump_percent_temp);
                    Pump_SetPercent(pump_percent_temp);
                    pump_parse_state = PUMP_STATE_IDLE;
                }
                
                /* 继续处理非水泵命令 */
                Bluetooth_Control_ParseCmd(cmd);
            }
        }
        else
        {
            /* 不在水泵解析状态，直接处理命令 */
            Bluetooth_Control_ParseCmd(cmd);
        }
    }
}

/**
 * @brief   解析蓝牙控制命令
 * @details 解析小车运动控制命令，包括前进、后退、左转、右转、停止、速度调节
 * @param   cmd: 命令字符
 * @retval  None
 */
static void Bluetooth_Control_ParseCmd(uint8_t cmd)
{
    switch(cmd)
    {
        case 'F':
            /* 前进: 左右轮均设为目标速度 */
            printf("BT Cmd: Forward, Speed: %.2f\r\n", Car_TargetSpeed);
            Car_SetSpeed(Car_TargetSpeed, Car_TargetSpeed);
            break;
            
        case 'B':
            /* 后退: 左右轮均设为负目标速度 */
            printf("BT Cmd: Backward, Speed: %.2f\r\n", Car_TargetSpeed);
            Car_SetSpeed(-Car_TargetSpeed, -Car_TargetSpeed);
            break;
            
        case 'L':
            /* 左转: 左轮减速，右轮保持速度 */
            printf("BT Cmd: Left\r\n");
            Car_SetSpeed(-Car_TargetSpeed * 0.6f, Car_TargetSpeed * 0.6f);
            break;
            
        case 'R':
            /* 右转: 右轮减速，左轮保持速度 */
            printf("BT Cmd: Right\r\n");
            Car_SetSpeed(Car_TargetSpeed * 0.6f, -Car_TargetSpeed * 0.6f);
            break;
            
        case 'S':
            /* 停止: 清零目标速度 */
            printf("BT Cmd: Stop\r\n");
            Car_Stop();
            break;
            
        case '+':
            /* 加速: 目标速度增加步进值 */
            Car_TargetSpeed += SPEED_STEP;
            /* 速度上限保护 */
            if(Car_TargetSpeed > SPEED_MAX)
                Car_TargetSpeed = SPEED_MAX;
            printf("BT Cmd: Speed Up, Current: %.2f m/s\r\n", Car_TargetSpeed);
            break;
            
        case '-':
            /* 减速: 目标速度减少步进值 */
            Car_TargetSpeed -= SPEED_STEP;
            /* 速度下限保护 */
            if(Car_TargetSpeed < SPEED_MIN)
                Car_TargetSpeed = SPEED_MIN;
            printf("BT Cmd: Speed Down, Current: %.2f m/s\r\n", Car_TargetSpeed);
            break;
            
        case '1':
            /* 1档速度(爬行): 0.2m/s */
            Car_TargetSpeed = SPEED_GEAR1;
            printf("BT Cmd: Gear 1 (Crawl), Speed: %.2f m/s\r\n", Car_TargetSpeed);
            break;
            
        case '2':
            /* 2档速度(慢速): 0.4m/s */
            Car_TargetSpeed = SPEED_GEAR2;
            printf("BT Cmd: Gear 2 (Slow), Speed: %.2f m/s\r\n", Car_TargetSpeed);
            break;
            
        case '3':
            /* 3档速度(正常): 0.7m/s */
            Car_TargetSpeed = SPEED_GEAR3;
            printf("BT Cmd: Gear 3 (Normal), Speed: %.2f m/s\r\n", Car_TargetSpeed);
            break;
            
        case '4':
            /* 4档速度(快速): 1.0m/s */
            Car_TargetSpeed = SPEED_GEAR4;
            printf("BT Cmd: Gear 4 (Fast), Speed: %.2f m/s\r\n", Car_TargetSpeed);
            break;
            
        case '5':
            /* 5档速度(极速): 1.3m/s */
            Car_TargetSpeed = SPEED_GEAR5;
            printf("BT Cmd: Gear 5 (Turbo), Speed: %.2f m/s\r\n", Car_TargetSpeed);
            break;
            
        case 'P':
            /* 开启水泵 */
            printf("Pump ON\r\n");
            Pump_Start();
            break;
            
        case 'O':
            /* 关闭水泵 */
            printf("Pump OFF\r\n");
            Pump_Stop();
            break;
            
        case 'W':
            /* 水泵百分比设置命令起始字符 */
            pump_parse_state = PUMP_STATE_W_RECV;
            pump_percent_temp = 0;
            pump_parse_timeout = HAL_GetTick();
            break;
            
        default:
            /* 未知指令 */
            printf("Invalid Pump Command\r\n");
            break;
    }
}

/**
 * @brief   解析水泵命令中的字符
 * @details 状态机解析W命令，支持W0~W100格式
 *          状态转换:
 *          IDLE -> W_RECV(收到W) -> W_DIGIT1(收到第一个数字) -> W_DIGIT2(收到第二个数字) -> 执行命令
 *          非数字字符触发命令执行并退出解析状态
 *          例如: W30 -> 收到W -> 收到3 -> 收到0 -> 设置30%
 *                W5P -> 收到W -> 收到5 -> 收到P(非数字) -> 设置5% -> 处理P命令
 * @param   ch: 接收到的字符
 * @retval  1: 字符已被水泵解析器处理，0: 字符未被处理(需要主switch处理)
 */
static uint8_t Bluetooth_Control_ParsePumpChar(uint8_t ch)
{
    /* 判断是否为数字字符 */
    if(ch >= '0' && ch <= '9')
    {
        uint8_t digit = ch - '0';
        
        switch(pump_parse_state)
        {
            case PUMP_STATE_W_RECV:
                /* 收到'W'后的第一个数字 */
                pump_percent_temp = digit;
                pump_parse_state = PUMP_STATE_W_DIGIT1;
                pump_parse_timeout = HAL_GetTick();
                break;
                
            case PUMP_STATE_W_DIGIT1:
                /* 收到第二个数字 */
                pump_percent_temp = pump_percent_temp * 10 + digit;
                
                /* 检查是否达到100 */
                if(pump_percent_temp >= 100)
                {
                    /* W100或更大，限制为100% */
                    pump_percent_temp = 100;
                    printf("Pump %d%%\r\n", pump_percent_temp);
                    Pump_SetPercent(pump_percent_temp);
                    pump_parse_state = PUMP_STATE_IDLE;
                }
                else
                {
                    /* 继续等待下一个数字 */
                    pump_parse_state = PUMP_STATE_W_DIGIT2;
                    pump_parse_timeout = HAL_GetTick();
                }
                break;
                
            case PUMP_STATE_W_DIGIT2:
                /* 收到第三个数字 */
                pump_percent_temp = pump_percent_temp * 10 + digit;
                
                /* 限制最大为100 */
                if(pump_percent_temp > 100)
                {
                    pump_percent_temp = 100;
                }
                
                /* 三位数完成，执行命令 */
                printf("Pump %d%%\r\n", pump_percent_temp);
                Pump_SetPercent(pump_percent_temp);
                pump_parse_state = PUMP_STATE_IDLE;
                break;
                
            default:
                /* 无效状态，重置 */
                pump_parse_state = PUMP_STATE_IDLE;
                break;
        }
        
        /* 数字字符已处理 */
        return 1;
    }
    else
    {
        /* 非数字字符，触发命令执行 */
        switch(pump_parse_state)
        {
            case PUMP_STATE_W_RECV:
                /* 收到W后没有数字就收到非数字字符，无效命令 */
                printf("Invalid Pump Command\r\n");
                pump_parse_state = PUMP_STATE_IDLE;
                break;
                
            case PUMP_STATE_W_DIGIT1:
            case PUMP_STATE_W_DIGIT2:
                /* 执行累积的水泵命令 */
                printf("Pump %d%%\r\n", pump_percent_temp);
                Pump_SetPercent(pump_percent_temp);
                pump_parse_state = PUMP_STATE_IDLE;
                break;
                
            default:
                break;
        }
        
        /* 非数字字符未处理，需要主switch处理 */
        return 0;
    }
}
