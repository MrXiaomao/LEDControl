#pragma once
#define WM_FOUNDCOMM WM_USER + 1 //�Զ�����ϢWM_FOUNDCOMM,�յ�����Ϣ��ʾ�����Ѿ��ҵ� 
#define WM_READCOMM WM_USER + 2 //�Զ�����ϢWM_READCOMM,�յ�����Ϣ������������,���Զ�ȡ 
extern void FindComm(); //����Ϊ�ⲿ���� 
extern void OpenComm(int nBaud, int nData, int nStop, int nCal); 
extern void CloseComm(); extern UINT ThreadFunc(LPVOID pParam); //����ȫ���̴߳����� 
extern bool ComIsOK; //����Ϊ�ⲿ����
extern bool SingleTriggerStatus; // �Ƿ���SingleTrigger����״̬������Ϊ�ⲿ����
extern bool LoopTriggerStatus;   // �Ƿ���LoopTrigger����״̬������Ϊ�ⲿ����
extern HANDLE hCom; 
extern CString strcomname;
extern CRITICAL_SECTION m_csCommunicationSync;
