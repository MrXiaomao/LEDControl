#pragma once
#define WM_FOUNDCOMM WM_USER + 1 //自定义消息WM_FOUNDCOMM,收到该消息表示串口已经找到 
#define WM_READCOMM WM_USER + 2 //自定义消息WM_READCOMM,收到该消息缓冲区有数据,可以读取 
extern void FindComm(); //申明为外部函数 
extern void OpenComm(int nBaud, int nData, int nStop, int nCal); 
extern void CloseComm(); extern UINT ThreadFunc(LPVOID pParam); //申明全局线程处理函数 
extern bool ComIsOK; //申明为外部变量
extern bool SingleTriggerStatus; // 是否处于SingleTrigger工作状态，申明为外部变量
extern bool LoopTriggerStatus;   // 是否处于LoopTrigger工作状态，申明为外部变量
extern HANDLE hCom; 
extern CString strcomname;
extern CRITICAL_SECTION m_csCommunicationSync;
