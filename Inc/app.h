//////////////////////////////////////////////////////////////////////
//�ļ����ƣ�app.h
//�ļ����ܣ�appͷ�ļ�
//���ߣ���ǿ
//ʱ�䣺2021��11��23��
//��ע��
//��ʷ:
//////////////////////////////////////////////////////////////////////
#ifndef _APP_H
#define _APP_H

#ifdef __cplusplus
extern "C" {
#endif 


//������Ϊ0.1ms����
#define TIMES_HZ        400     //��բ�̶�����40ms
#define TIMES_FZ        80      //��բ�̶�����8ms

#define TIMES_HZ_PMT    500     //���ź�բ�̶�����50ms
#define TIMES_FZ_PMT    200     //���ŷ�բ�̶�����20ms

#define TIMES_YKSHAKE   10      //����ʱ��1ms

#define TIMES_TICK      100     //tickΪ10ms
#define TIMES_AD        10      //ADȡֵ����1ms
//����Ϊ10ms����
#define TIME10_RUN      50      //���е�����500ms
#define TIME10_FAULT    1000    //���ϵƵ���10S
#define TIMES_SHAKE     20      //����ʱ��200ms
//AD��ض���
#define ADNUM       10      //����ת������
#define ADCOEF      145     //AD����ϵ��
#define CNLIMIT     3500    //������ֵ�����65535


typedef struct MC_APP{  //��Ҫȫ�ֲ���
    uint32_t    Times_hz;
    uint32_t    Times_fz;
    uint32_t    Times_shake;
    uint32_t    Times_YKshake;
    
}stMCAPP_t;

typedef struct CN_AD{   //ADԭʼֵ
    uint16_t    Val_UCN;
    
    uint16_t    ADCur;//��ǰֵ
    uint16_t    ADValue[ADNUM];//��ʷֵ����

    uint16_t    Coefa; //ʲô��ϵ����
    uint16_t    Coefb; //ʲô��ϵ����
    
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