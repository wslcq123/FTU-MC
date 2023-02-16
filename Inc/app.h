//////////////////////////////////////////////////////////////////////
//文件名称：app.h
//文件功能：app头文件
//作者：丁强
//时间：2021年11月23日
//备注：
//历史:
//////////////////////////////////////////////////////////////////////
#ifndef _APP_H
#define _APP_H

#ifdef __cplusplus
extern "C" {
#endif 


//以下是为0.1ms周期
#define TIMES_HZ        400     //合闸固定脉冲40ms
#define TIMES_FZ        80      //分闸固定脉冲8ms

#define TIMES_HZ_PMT    500     //永磁合闸固定脉冲50ms
#define TIMES_FZ_PMT    200     //永磁分闸固定脉冲20ms

#define TIMES_YKSHAKE   10      //防抖时间1ms

#define TIMES_TICK      100     //tick为10ms
#define TIMES_AD        10      //AD取值周期1ms
//以下为10ms周期
#define TIME10_RUN      50      //运行灯周期500ms
#define TIME10_FAULT    1000    //故障灯点亮10S
#define TIMES_SHAKE     20      //防抖时间200ms
//AD相关定义
#define ADNUM       10      //采样转换个数
#define ADCOEF      145     //AD采样系数
#define CNLIMIT     3500    //储能限值，最大65535


typedef struct MC_APP{  //主要全局参数
    uint32_t    Times_hz;
    uint32_t    Times_fz;
    uint32_t    Times_shake;
    uint32_t    Times_YKshake;
    
}stMCAPP_t;

typedef struct CN_AD{   //AD原始值
    uint16_t    Val_UCN;
    
    uint16_t    ADCur;//当前值
    uint16_t    ADValue[ADNUM];//历史值数组

    uint16_t    Coefa; //什么的系数？
    uint16_t    Coefb; //什么的系数？
    
    uint16_t    ADRead;
    
}stMCAD_t;

void AppInit(void);
void AppTask(void);
void ProcTick(void);
void ProcYX(void);
void ProcPMT(uint8_t Flag);
void ProcLedRun(void);
void ProcLedFault(void);
void ProcCN(void);
void ProcAD(void);
void ProcSem(void);
void ProcTimer(uint32_t Times);
void HZStart(void);
void HZEnd(void);
void FZStart(void);
void FZEnd(void);


#ifdef __cplusplus
}
#endif

#endif