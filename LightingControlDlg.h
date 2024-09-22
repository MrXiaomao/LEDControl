
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
	DWORD ReadComm();

	int nBaud; //波特率
	int nData; //数据位
	int nStop; //停止位
	int nCal;  //校验位

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
	DECLARE_MESSAGE_MAP()
public:
	CComboBox m_comlist;
	MyButton m_comcontrol;
	CString m_strStatus;
	LEDButton m_NetStatusLED;
};
