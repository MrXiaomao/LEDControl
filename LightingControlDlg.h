﻿
// LightingControlDlg.h: 头文件
//

#pragma once

#include "MyButton.h"
#include "LEDButton.h"

// CLightingControlDlg 对话框
class CLightingControlDlg : public CDialogEx
{
// 构造
public:
	CLightingControlDlg(CWnd* pParent = nullptr);	// 标准构造函数
	CWinThread* pReceiveThread;
	void ShowStatus();
	// 更新勾选框状态
	void UpdateAllCheck(const BYTE stateBit, BOOL status, int LightID); 
	DWORD ReadComm();

	int nBaud; //波特率
	int nData; //数据位
	int nStop; //停止位
	int nCal;  //校验位
	BYTE LightSwitchA; //灯光开关控制A,1个字节，八个bit，00000000。从右往左数第五位是无效位。
	BYTE LightSwitchB; //灯光开关控制B，1个字节，八个bit，00000000。从右往左数第五位是无效位。

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_LIGHTINGCONTROL_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg void OnComcontrol(); //打开串口
	afx_msg HCURSOR OnQueryDragIcon(); 
	afx_msg void OnClose();
	afx_msg void OnBnClickedOneTrigger();
	DECLARE_MESSAGE_MAP()
public:
	CComboBox m_comlist;
	MyButton m_comcontrol;
	CString m_strStatus;
	LEDButton m_NetStatusLED;
	int m_CalibrationTime; //标定时长，单位s
	int m_LightDelay; //LED发光延迟时间,单位us
	int m_LightWidth; //LED发光宽度,单位为ns
	int m_VoltA; //A组LED电压，单位mV
	int m_VoltB; //B组LED电压，单位mV
	
	afx_msg void OnBnClickedCheckA1();
	afx_msg void OnBnClickedCheckA2();
	afx_msg void OnBnClickedCheckA3();
	afx_msg void OnBnClickedCheckA4();
	afx_msg void OnBnClickedCheckA5();
	afx_msg void OnBnClickedCheckA6();
	afx_msg void OnBnClickedCheckA7(); 
	afx_msg void OnBnClickedCheckALL_A();
	afx_msg void OnBnClickedCheckB1();
	afx_msg void OnBnClickedCheckB2();
	afx_msg void OnBnClickedCheckB3();
	afx_msg void OnBnClickedCheckB4();
	afx_msg void OnBnClickedCheckB5();
	afx_msg void OnBnClickedCheckB6();
	afx_msg void OnBnClickedCheckB7();
	afx_msg void OnBnClickedCheckALL_B();
};
