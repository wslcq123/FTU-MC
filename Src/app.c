//////////////////////////////////////////////////////////////////////
//�ļ����ƣ�app.c
//�ļ����ܣ�ʵ�ִſ�ģ��Ĺ���
//���ߣ���ǿ
//ʱ�䣺2021��12��23��
//��ע��
//��ʷ:
//////////////////////////////////////////////////////////////////////
#include "main.h"
#include "app.h"
#include "string.h"

stMCAPP_t   stMC;//�ֺ�բʱ�䡢����ʱ�����
stMCAD_t    stAD;//���ܲ������
uint32_t    ADValue=0;                  //AD���ܲ���ֵ
uint32_t    Timer6=0;                   //0.1ms�ж��ۼ���
uint32_t    Times_Shake = 0, Times_Old = 0, Times_Tick=0;  
uint32_t    Times_HZ=0, Times_FZ=0;     //�Ϸ�բ�ۼ���
uint32_t    Times_Run=0, Times_Fault=0;   //���еƣ����ϵ�
uint8_t     Firq_OC=0, Firq_HZ=0, Firq_FZ=0;    //�ź��жϱ�־����Ȧ��������բ����բ
uint8_t     State_Local=0;  //Զ��/���أ��ߵ�ƽΪ����
uint8_t     State_CN=0;     //����״̬���ߵ�ƽΪ�������
uint8_t     State_HZ=0;     //�жϺ�բ���͵�ƽ��Ч
uint8_t     State_FZ=0;     //�жϷ�բ���͵�ƽ��Ч
uint32_t    StateYX[4];


extern TIM_HandleTypeDef htim6;


///////////////////////////////////////////////////////////////
//��������:AppInit()
//��������:Ӧ������ʼ��
//����˵��:
//����˵����
///////////////////////////////////////////////////////////////
void AppInit(void)
{
    memset (&stMC, 0, sizeof(stMC));
    memset (&stAD, 0, sizeof(stAD));
    //��������ͨ����ģ���������
    ProcPMT(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14));//�ߵ�ƽ���ſط�ʽ���͵�ƽ�����ŷ�ʽ
    stMC.Times_shake = TIMES_SHAKE;
    stMC.Times_YKshake = TIMES_YKSHAKE;
    
    stAD.Coefa = 100;
    stAD.Coefb = 100 * 40960 / 33 / ADCOEF;     //12λAD��Ӧ4096����ֵ��3.3��1V��Ӧʵ��ADCOEF
    
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET);    //��ʼ�����ϵ���
        
    HAL_TIM_Base_Start_IT(&htim6);      //������ʱ��

}

///////////////////////////////////////////////////////////////
//��������:AppTask()
//��������:Ӧ��������
//����˵��:
//����˵����
///////////////////////////////////////////////////////////////
void AppTask(void)
{
    uint32_t times = 0;
  
    
    //���������ź�
    ProcSem();

    //����ʱ��
    if (Times_Old != Timer6)    //��ʱ����
    {
        times = Timer6 - Times_Old;
        Times_Old = Timer6;

        ProcTimer(times);
        
        
        if ( (Times_Old - Times_Tick) >= TIMES_TICK)    //10ms����
        {
            Times_Tick = Times_Old;
            ProcTick();
        }
    }
    
}

///////////////////////////////////////////////////////////////
//��������:ProcTick
//��������:����10ms����
//����˵��:
//����˵����
///////////////////////////////////////////////////////////////
void ProcTick(void)
{
    //��������
    ProcYX();
    
    //������
    ProcCN();
    
    //�������е�
    ProcLedRun();
    
    //������ϵ�
    ProcLedFault();
}

///////////////////////////////////////////////////////////////
//��������:ProcYX
//��������:����ң��,10ms����
//����˵��:
//����˵����
///////////////////////////////////////////////////////////////
void ProcYX(void)
{
    uint8_t     tmp;
    
    //����Զ��/�����ź�
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

    //������ң��
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
//��������:ProcPMT
//��������:����ģ��
//����˵��:flag:1�ߵ�ƽ���ſط�ʽ��0�͵�ƽ�����ŷ�ʽ
//����˵����
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
//��������:ProcLedRun
//��������:�������е�
//����˵��:
//����˵����
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
//��������:ProcLedFault
//��������:������ϵ�,��ʱʱ�䵽Ϩ��
//����˵��:
//����˵����
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
//��������:ProcCN
//��������:������
//����˵��:
//����˵����
///////////////////////////////////////////////////////////////
void ProcCN(void)
{
    if (State_CN)
    {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);    //���ң��
    }
    else
    {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);  //���ң��
    }
    
}

///////////////////////////////////////////////////////////////
//��������:ProcSem
//��������:�����յ��ĺ�բ����բ�������źţ�����Զ�������ж�
//����˵��:
//����˵����
///////////////////////////////////////////////////////////////
void ProcSem(void)
{
    if ( Firq_HZ && State_HZ)   //�͵غ�բ�ź�
    {
        HZStart();
    }
    
    if ( Firq_FZ && State_FZ)   //�͵ط�բ�ź�
    {
        FZStart();
    }
    
}

///////////////////////////////////////////////////////////////
//��������:ProcTimer
//��������:����ʱ����0.1ms;�����բ����բ�����ݹ����źŵ������ϵ�
//����˵��:
//����˵����
///////////////////////////////////////////////////////////////
void ProcTimer(uint32_t Times)
{
    uint8_t     tmp;
    
    //�����բ
    tmp = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_3);
    if (tmp == 0)
    {
        StateYX[1]++;
    }
    else
    {
        StateYX[1] = 0;
    }
    
    if (StateYX[1] == stMC.Times_YKshake)     //����ʱ�䵽
    {
        State_HZ = 1;
    }
        
    //�����բ
    tmp = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_4);
    if (tmp == 0)
    {
        StateYX[2]++;
        
    }
    else
    {
        StateYX[2] = 0;
    }
    
    if (StateYX[2] == stMC.Times_YKshake)     //����ʱ�䵽
    {
        State_FZ = 1;
    }



    if (Firq_OC)    //�й����ź�
    {
        Firq_OC += Times;//��1.1ms��ʼ
        if (Firq_OC >= 2)   //��ʱ0.1ms
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
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET);    //�������ϵ�
        }
    }
 

    if (Times_HZ)   //����բʱ��
    {
        Times_HZ += Times;
        if (Times_HZ > stMC.Times_hz)  //��բʱ�䵽
        {
            HZEnd();
        }
    }
    
    if (Times_FZ)   //����բʱ��
    {
        Times_FZ += Times;
        if (Times_FZ > stMC.Times_fz)  //��բʱ�䵽
        {
            FZEnd();
        }
    }
    

}
    

///////////////////////////////////////////////////////////////
//��������:HZStart()
//��������:��բ����������ע��Ҫ���㴢�ܵ�״̬��
//����˵��:
//����˵����
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
//��������:HZEnd
//��������:��բ��������
//����˵��:
//����˵����
///////////////////////////////////////////////////////////////
void HZEnd(void)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
    Times_HZ = 0;
}

///////////////////////////////////////////////////////////////
//��������:FZStart
//��������:��բ��������
//����˵��:
//����˵����
///////////////////////////////////////////////////////////////
void FZStart(void)
{
    //��բ����ʱȡ����բ����
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
//��������:FZEnd
//��������:��բ��������
//����˵��:
//����˵����
///////////////////////////////////////////////////////////////
void FZEnd(void)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);
    Times_FZ = 0;
   
}

