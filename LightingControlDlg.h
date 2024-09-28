
// LightingControlDlg.h: 头文件
//

#pragma once

#include "MyButton.h"
#include "LEDButton.h"
#include <vector>
using namespace std;

// CLightingControlDlg 对话框
class CLightingControlDlg : public CDialogEx
{
// 构造
public:
	CLightingControlDlg(CWnd* pParent = nullptr);	// 标准构造函数
	
	CWinThread* pReceiveThread;
	//初始化状态栏
	void InitBarSettings();
	//刷新日志内容
	void ShowStatus();
	
	/*添加日志信息
	* info 待添加的信息
	* isShow 控制是否在界面显示该条日志
	*/
	void PrintLog(CString info, BOOL isShow = TRUE);

	//更新勾选框对应的数值
	void UpdateCheckValue(const BYTE stateBit, BOOL status, int LightID); 
	//更新LED勾选状态
	void UpdateLEDCheck();
	//读取界面的各个控件的上一次设置参数
	void InitSettingByHistoryInput();
	//读取配置文件中的基本配置参数
	void InitConfigSetting();
	//读取串口
	DWORD ReadComm();
	//读取预设电压json文件的数据
	BOOL ReadVoltFile(const CString file);

	/*阻塞式发送，串口发送数据到FPGA，直到检测到指令反馈成功或者等待超时才退出。
	* msg 发送信息
	* msgLength 数据长度
	* sleepTime 发送指令后程序Sleep时间，单位：ms
	* maxWaitingTime 最大等待时间，单位：s
	*/
	BOOL BackSend(BYTE* msg, int msgLength, int sleepTime = 1, int maxWaitingTime = 1, BOOL isShow=FALSE);

	//重置FPGA
	void ResetFPGA();

	//配置FPGA，等待触发指令
	void FPGAInit();

	//配置LED发光宽度
	void sendLEDwidth();

	//配置LED发光延迟时间
	void sendLEDDelay();

	//配置移位寄存器数据,设置LED灯是否开启
	void sendShiftRegisterData();

	//配置DAC数据,也就是LED电压
	void sendLightingVolt();

	//所有参数设置控件是否可用，在正常工作时都禁用，其他时候恢复。
	void EnableControl(BOOL flag);

	CStatusBar m_statusBar; // 状态栏
	int config_nBaud; //波特率
	int config_nData; //数据位
	int config_nStop; //停止位
	int config_nCal;  //校验位
	int config_minV; //LED灯的电压下限值，从配置参数中获取
	int config_maxV; //LED灯的电压上限值，从配置参数中获取
	int config_PowerStableTime; //外设电源开启时，需要延时一定时间，使其稳定，这个参数放在setting.json中可调,单位ms
	int config_triggerHLPoints; //硬件触发高电平点数
	// 需要对电压到DAC数值的刻度曲线
	// DAC = P1*Volt + P2
	double config_p1_A; 
	double config_p2_A;
	double config_p1_B;
	double config_p2_B;

	BYTE m_LightSwitchA; //灯光开关控制A,1个字节，八个bit，00000000。从右往左数第五位是无效位。
	BYTE m_LightSwitchB; //灯光开关控制B，1个字节，八个bit，00000000。从右往左数第五位是无效位。
	int timer; // 计时器，满测量时长后则发送停止测量
	CString VoltFile; //存放预设电压的json文件名及其路径
	vector<int> vec_VoltA; //A组预设电压值，单位mV
	vector<int> vec_VoltB; //B组预设电压值，单位mV

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
	//打开串口
	afx_msg void OnComcontrol(); 
	afx_msg HCURSOR OnQueryDragIcon(); 
	afx_msg void OnClose();
	//点击单次设置电压触发
	afx_msg void OnBnClickedOneTrigger();
	//点击循环设置电压触发
	afx_msg void OnBnClickedLoopTrigger();
	DECLARE_MESSAGE_MAP()
public:
	CComboBox m_comlist;
	MyButton m_comcontrol;
	CString m_strLog;
	LEDButton m_NetStatusLED;
	CEdit m_LogEdit;//日志文本控件
	int m_CalibrationTime; //标定时长，单位s
	int m_LightDelay; //LED发光延迟时间,单位us
	int m_LightWidth; //LED发光宽度,单位为x10ns
	int m_tempVoltA;  //A组LED当前电压，单位mV
	int m_tempVoltB;  //B组LED当前电压，单位mV
	
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
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	//选择电压预设文件
	afx_msg void ChoseVoltLoopFile();

	//控件失去焦点，进行数值判断
	afx_msg void OnEnKillfocusVoltA();
	afx_msg void OnEnKillfocusVoltB();
	afx_msg void OnEnKillfocusLightWidth(); 
	afx_msg void OnEnKillfocusLightDelay();
	afx_msg void OnEnKillfocusCalibrationTime();
	
	//结束测量状态并重置FPGA
	afx_msg void ResetSystem();
	afx_msg void OnBnClickedClearLog();
};
