//////////////////////////////////////////////////////////////////////
//文件名称：app.c
//文件功能：实现磁控模块的功能
//作者：丁强
//时间：2021年12月23日
//备注：
//历史:
//////////////////////////////////////////////////////////////////////
#include "main.h"
#include "app.h"
#include "string.h"

stMCAPP_t   stMC;//分合闸时间、防抖时间相关
stMCAD_t    stAD;//储能采样相关
uint32_t    ADValue=0;                  //AD储能采样值
uint32_t    Timer6=0;                   //0.1ms中断累加数
uint32_t    Times_Shake = 0, Times_Old = 0, Times_Tick=0;  
uint32_t    Times_HZ=0, Times_FZ=0;     //合分闸累加数
uint32_t    Times_Run=0, Times_Fault=0;   //运行灯，故障灯
uint8_t     Firq_OC=0, Firq_HZ=0, Firq_FZ=0;    //信号中断标志：线圈过流，合闸，分闸
uint8_t     State_Local=0;  //远方/本地，高电平为本地
uint8_t     State_CN=0;     //储能状态，高电平为储能完毕
uint8_t     State_HZ=0;     //中断合闸，低电平有效
uint8_t     State_FZ=0;     //中断分闸，低电平有效
uint32_t    StateYX[4];


extern TIM_HandleTypeDef htim6;


///////////////////////////////////////////////////////////////
//函数名称:AppInit()
//函数功能:应用主初始化
//参数说明:
//返回说明：
///////////////////////////////////////////////////////////////
void AppInit(void)
{
    memset (&stMC, 0, sizeof(stMC));
    memset (&stAD, 0, sizeof(stAD));
    //保留可以通过主模块参数输入
    ProcPMT(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14));//高电平，磁控方式；低电平，永磁方式
    stMC.Times_shake = TIMES_SHAKE;
    stMC.Times_YKshake = TIMES_YKSHAKE;
    
    stAD.Coefa = 100;
    stAD.Coefb = 100 * 40960 / 33 / ADCOEF;     //12位AD对应4096；满值是3.3；1V对应实际ADCOEF
    
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET);    //初始化故障灯灭
        
    HAL_TIM_Base_Start_IT(&htim6);      //启动定时器

}

///////////////////////////////////////////////////////////////
//函数名称:AppTask()
//函数功能:应用主任务
//参数说明:
//返回说明：
///////////////////////////////////////////////////////////////
void AppTask(void)
{
    uint32_t times = 0;
  
    
    //处理输入信号
    ProcSem();

    //处理定时器
    if (Times_Old != Timer6)    //定时器到
    {
        times = Timer6 - Times_Old;
        Times_Old = Timer6;

        ProcTimer(times);
        
        
        if ( (Times_Old - Times_Tick) >= TIMES_TICK)    //10ms节拍
        {
            Times_Tick = Times_Old;
            ProcTick();
        }
    }
    
}

///////////////////////////////////////////////////////////////
//函数名称:ProcTick
//函数功能:处理10ms节拍
//参数说明:
//返回说明：
///////////////////////////////////////////////////////////////
void ProcTick(void)
{
    //处理输入
    ProcYX();
    
    //处理储能
    ProcCN();
    
    //处理运行灯
    ProcLedRun();
    
    //处理故障灯
    ProcLedFault();
}

///////////////////////////////////////////////////////////////
//函数名称:ProcYX
//函数功能:处理遥信,10ms周期
//参数说明:
//返回说明：
///////////////////////////////////////////////////////////////
void ProcYX(void)
{
    uint8_t     tmp;
    
    //处理远方/本地信号
    tmp = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_6);
    if (tmp != State_Local)
    {
        StateYX[0]++;
        if (StateYX[0] >= stMC.Times_shake)
        {
            State_Local = tmp;
            StateYX[0] = 0;
        }
    }
    else
    {
        StateYX[0] = 0;
    }

    //处理储能遥信
    tmp = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
    if (tmp != State_CN)
    {
        StateYX[3]++;
        if (StateYX[3] >= stMC.Times_shake)
        {
            State_CN = tmp;
            StateYX[3] = 0;
        }
    }
    else
    {
        StateYX[3] = 0;
    }
    
}

///////////////////////////////////////////////////////////////
//函数名称:ProcPMT
//函数功能:处理模块
//参数说明:flag:1高电平，磁控方式；0低电平，永磁方式
//返回说明：
///////////////////////////////////////////////////////////////
void ProcPMT(uint8_t Flag)
{
    if (Flag)
    {
        stMC.Times_hz = TIMES_HZ;
        stMC.Times_fz = TIMES_FZ;
    }
    else
    {
        stMC.Times_hz = TIMES_HZ_PMT;
        stMC.Times_fz = TIMES_FZ_PMT;
    }
    
}


///////////////////////////////////////////////////////////////
//函数名称:ProcLedRun
//函数功能:处理运行灯
//参数说明:
//返回说明：
///////////////////////////////////////////////////////////////
void ProcLedRun(void)
{
    Times_Run++;
    
    if (Times_Run == TIME10_RUN)
    {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
    }
    else if (Times_Run >= TIME10_RUN*2)
    {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
        Times_Run = 0;
    }
    
}

///////////////////////////////////////////////////////////////
//函数名称:ProcLedFault
//函数功能:处理故障灯,延时时间到熄灭
//参数说明:
//返回说明：
///////////////////////////////////////////////////////////////
void ProcLedFault(void)
{
    if (Times_Fault)
    {
        Times_Fault--;
        if (Times_Fault == 0)
        {
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET);
        }
    }

}

///////////////////////////////////////////////////////////////
//函数名称:ProcCN
//函数功能:处理储能
//参数说明:
//返回说明：
///////////////////////////////////////////////////////////////
void ProcCN(void)
{
    if (State_CN)
    {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);    //输出遥信
    }
    else
    {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);  //输出遥信
    }
    
}

///////////////////////////////////////////////////////////////
//函数名称:ProcSem
//函数功能:处理收到的合闸、分闸、过流信号，基于远方本地判断
//参数说明:
//返回说明：
///////////////////////////////////////////////////////////////
void ProcSem(void)
{
    if ( Firq_HZ && State_HZ)   //就地合闸信号
    {
        HZStart();
    }
    
    if ( Firq_FZ && State_FZ)   //就地分闸信号
    {
        FZStart();
    }
    
}

///////////////////////////////////////////////////////////////
//函数名称:ProcTimer
//函数功能:处理定时器，0.1ms;处理分闸、合闸，根据过流信号点亮故障灯
//参数说明:
//返回说明：
///////////////////////////////////////////////////////////////
void ProcTimer(uint32_t Times)
{
    uint8_t     tmp;
    
    //处理合闸
    tmp = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_3);
    if (tmp == 0)
    {
        StateYX[1]++;
    }
    else
    {
        StateYX[1] = 0;
    }
    
    if (StateYX[1] == stMC.Times_YKshake)     //防抖时间到
    {
        State_HZ = 1;
    }
        
    //处理分闸
    tmp = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_4);
    if (tmp == 0)
    {
        StateYX[2]++;
        
    }
    else
    {
        StateYX[2] = 0;
    }
    
    if (StateYX[2] == stMC.Times_YKshake)     //防抖时间到
    {
        State_FZ = 1;
    }



    if (Firq_OC)    //有过流信号
    {
        Firq_OC += Times;//从1.1ms开始
        if (Firq_OC >= 2)   //延时0.1ms
        {
            if (Times_HZ)
            {
                HZEnd();
            }
            
            if (Times_FZ)
            {
               FZEnd();
            }
            
            Firq_OC = 0;
            
            Times_Fault = TIME10_FAULT;
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET);    //点亮故障灯
        }
    }
 

    if (Times_HZ)   //检测合闸时间
    {
        Times_HZ += Times;
        if (Times_HZ > stMC.Times_hz)  //合闸时间到
        {
            HZEnd();
        }
    }
    
    if (Times_FZ)   //检测分闸时间
    {
        Times_FZ += Times;
        if (Times_FZ > stMC.Times_fz)  //分闸时间到
        {
            FZEnd();
        }
    }
    

}
    

///////////////////////////////////////////////////////////////
//函数名称:HZStart()
//函数功能:合闸启动操作，注意要满足储能的状态下
//参数说明:
//返回说明：
///////////////////////////////////////////////////////////////    
void HZStart(void)
{
    if (State_CN)
    {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
        Times_HZ = 1;
    }
    
    Firq_HZ = 0;
    State_HZ = 0;
}

///////////////////////////////////////////////////////////////
//函数名称:HZEnd
//函数功能:合闸结束操作
//参数说明:
//返回说明：
///////////////////////////////////////////////////////////////
void HZEnd(void)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
    Times_HZ = 0;
}

///////////////////////////////////////////////////////////////
//函数名称:FZStart
//函数功能:分闸启动操作
//参数说明:
//返回说明：
///////////////////////////////////////////////////////////////
void FZStart(void)
{
    //分闸启动时取消合闸操作
    if (Times_HZ)   
    {
        HZEnd();
    }    
    
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);
    Times_FZ = 1;
    
    Firq_FZ = 0;  
    State_FZ = 0;
}

///////////////////////////////////////////////////////////////
//函数名称:FZEnd
//函数功能:分闸结束操作
//参数说明:
//返回说明：
///////////////////////////////////////////////////////////////
void FZEnd(void)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);
    Times_FZ = 0;
   
}

