
// LightingControlDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "LightingControl.h"
#include "LightingControlDlg.h"
#include "afxdialogex.h"

#include "MyConst.h"
#include "comm.h"
#include "Order.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BYTE* DataReceive; // 串口接收的数据，缓存下来，接收完后再存储到文件中。

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CLightingControlDlg 对话框



CLightingControlDlg::CLightingControlDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_LIGHTINGCONTROL_DIALOG, pParent),
	pReceiveThread(nullptr),
	nBaud(115200),nData(8),nStop(1),nCal(0),
	LightSwitchA(0x00), LightSwitchB(0x00), 
	m_strStatus(_T("")),
	m_CalibrationTime(10),
	m_LightDelay(9999),
	m_LightWidth(10),
	m_VoltA(2700),
	m_VoltB(2700),
	timer(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON1);
}

void CLightingControlDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SERIALPORT_ID, m_comlist);
	DDX_Control(pDX, IDC_CONNECT_SERIALPORT, m_comcontrol);
	DDX_Control(pDX, IDC_LED_STATUS, m_NetStatusLED);

	DDX_Text(pDX, IDC_EDIT_STATUS, m_strStatus);
	DDX_Text(pDX, IDC_CALIBRATION_TIME, m_CalibrationTime);
	DDX_Text(pDX, IDC_LIGHTING_DELAY, m_LightDelay);
	DDX_Text(pDX, IDC_LIGHTING_WIDTH, m_LightWidth);
	DDX_Text(pDX, IDC_VOLTA, m_VoltA);
	DDX_Text(pDX, IDC_VOLTB, m_VoltB);
	DDV_MinMaxInt(pDX, m_CalibrationTime, 1, 255);
	DDV_MinMaxInt(pDX, m_LightDelay, 1, 65535);
	DDV_MinMaxInt(pDX, m_LightWidth, 1, 255);
	DDV_MinMaxInt(pDX, m_VoltA, 2700, 3100);
	DDV_MinMaxInt(pDX, m_VoltB, 2700, 3100);
}

BEGIN_MESSAGE_MAP(CLightingControlDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_CONNECT_SERIALPORT, &CLightingControlDlg::OnComcontrol)
	ON_BN_CLICKED(IDC_ONE_TRIGGER, &CLightingControlDlg::OnBnClickedOneTrigger)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_CHECKA1, &CLightingControlDlg::OnBnClickedCheckA1)
	ON_BN_CLICKED(IDC_CHECKA2, &CLightingControlDlg::OnBnClickedCheckA2)
	ON_BN_CLICKED(IDC_CHECKA3, &CLightingControlDlg::OnBnClickedCheckA3)
	ON_BN_CLICKED(IDC_CHECKA4, &CLightingControlDlg::OnBnClickedCheckA4)
	ON_BN_CLICKED(IDC_CHECKA5, &CLightingControlDlg::OnBnClickedCheckA5)
	ON_BN_CLICKED(IDC_CHECKA6, &CLightingControlDlg::OnBnClickedCheckA6)
	ON_BN_CLICKED(IDC_CHECKA7, &CLightingControlDlg::OnBnClickedCheckA7)
	ON_BN_CLICKED(IDC_CHECKALL_A, &CLightingControlDlg::OnBnClickedCheckALL_A)
	ON_BN_CLICKED(IDC_CHECKB1, &CLightingControlDlg::OnBnClickedCheckB1)
	ON_BN_CLICKED(IDC_CHECKB2, &CLightingControlDlg::OnBnClickedCheckB2)
	ON_BN_CLICKED(IDC_CHECKB3, &CLightingControlDlg::OnBnClickedCheckB3)
	ON_BN_CLICKED(IDC_CHECKB4, &CLightingControlDlg::OnBnClickedCheckB4)
	ON_BN_CLICKED(IDC_CHECKB5, &CLightingControlDlg::OnBnClickedCheckB5)
	ON_BN_CLICKED(IDC_CHECKB6, &CLightingControlDlg::OnBnClickedCheckB6)
	ON_BN_CLICKED(IDC_CHECKB7, &CLightingControlDlg::OnBnClickedCheckB7)
	ON_BN_CLICKED(IDC_CHECKALL_B, &CLightingControlDlg::OnBnClickedCheckALL_B)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CLightingControlDlg 消息处理程序

BOOL CLightingControlDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标
	
	//ShowWindow(SW_MAXIMIZE); //初始化弹出时最大化

	//ShowWindow(SW_MINIMIZE); //初始化弹出时最小化

	// TODO: 在此添加额外的初始化代码
	//设置软件标题名称
	CString AppTitle = _T("灯光控制界面");//默认名称
	Json::Value jsonSetting = ReadSetting(_T("Setting.json"));
	if (!jsonSetting.isNull()) {
		if (jsonSetting.isMember("SoftwareTitle"))
		{
			const char* s = jsonSetting["SoftwareTitle"].asCString();
			int nLenW = ::MultiByteToWideChar(CP_UTF8, 0, s, -1, NULL, 0);
			wchar_t* wszBuffer = new wchar_t[nLenW];
			::MultiByteToWideChar(CP_UTF8, 0, s, -1, wszBuffer, nLenW);

			// 将 Unicode 编码转换为 GB2312 编码（也就是简体中文编码）
			int nLenA = ::WideCharToMultiByte(CP_ACP, 0, wszBuffer, -1, NULL, 0, NULL, NULL);
			char* szBuffer = new char[nLenA];
			::WideCharToMultiByte(CP_ACP, 0, wszBuffer, -1, szBuffer, nLenA, NULL, NULL);

			// 输出结果
			std::string strResult(szBuffer);
			const char* tmp = strResult.c_str();
			AppTitle = tmp;
		}
		else {
			string pStrTitle = _UnicodeToUtf8(AppTitle);
			// char* pStrTitle = CstringToWideCharArry(AppTitle);
			jsonSetting["SoftwareTitle"] = pStrTitle;
		}
	}
	WriteSetting(_T("Setting.json"), jsonSetting);
	SetWindowText(AppTitle);

	FindComm(); //调用自动找串口函数 
	m_NetStatusLED.RefreshWindow(FALSE, _T("OFF"));//设置指示灯

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CLightingControlDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CLightingControlDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CLightingControlDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

LRESULT CLightingControlDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: 在此添加专用代码和/或调用基类
	switch (message) {
	case WM_FOUNDCOMM:
	{//已找到串口,串口号以字符串形式由wParam传递
		m_comlist.AddString((LPCTSTR)wParam); //用AddString添加一个字符串即COM号到m_comlist列表框中
		break;
	}
	case WM_READCOMM:
	{   //读串口消息 
		ReadComm();
		this->SendDlgItemMessage(IDC_EDIT_STATUS, WM_VSCROLL, SB_BOTTOM, 0); //滚动条始终在底部
		break;
	}
	}

	return CDialogEx::WindowProc(message, wParam, lParam);
}

DWORD CLightingControlDlg::ReadComm()
{
	CString strTemp;
	OVERLAPPED m_osRead;
	memset(&m_osRead, 0, sizeof(OVERLAPPED));
	m_osRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	char lpInBuffer[1024];
	DWORD dwBytesRead = 1024;
	BOOL bReadStatus;
	bReadStatus = ReadFile(hCom, lpInBuffer, dwBytesRead, &dwBytesRead, &m_osRead);
	if (!bReadStatus) //如果ReadFile函数返回FALSE 
	{
		if (GetLastError() == ERROR_IO_PENDING) //GetLastError()函数返回ERROR_IO_PENDING,表明串口正在进行读操作 
		{
			WaitForSingleObject(m_osRead.hEvent, 2000); //使用WaitForSingleObject函数等待，直到读操作完成或延时已达到2000ms 
			//当串口读操作进行完毕后，m_osRead的hEvent事件会变为有信号 
			PurgeComm(hCom, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
			return dwBytesRead;
		}
		return 0;
	}
	lpInBuffer[dwBytesRead] = NULL;
	strTemp = lpInBuffer;
	//DataReceive += (BYTE)lpInBuffer;
	
	ShowStatus();
	return 1;
}

void CLightingControlDlg::ShowStatus()
{
	//状态栏显示状态 
	CString strTXcnt;
	CString strRXcnt;
	CString sTemp;
	CString comnum;
	CString strBaud, strStop, strData, strCal;
	UpdateData(true);
	if (ComIsOK)
	{
		m_comlist.GetLBText(m_comlist.GetCurSel(), sTemp);
		comnum = sTemp + _T("已打开");
	}
	else
		comnum = _T("未打开串口");
	//strTXcnt.Format(_T("发送:%d"), m_intTxCnt);
	//strRXcnt.Format(_T("接收:%d"), m_intRxCnt);
	strBaud.Format(_T("%d"), nBaud);
	strStop.Format(_T("%d"), nData);
	strData.Format(_T("%d"), nStop);
	strCal.Format(_T("%d"), nCal);

	m_strStatus = _T("串口: ") + comnum + _T("  ") + _T("状态: ") + _T("波特率: ") + strBaud +
		_T(", ") + _T("数据位: ") + strData + _T(", ") + _T("停止位: ") + strStop + _T(", ") + _T("校验位: ") + strCal;
	UpdateData(FALSE);
}

void CLightingControlDlg::OnComcontrol()
{
	//获取配置参数，并连接串口
	Json::Value jsonSetting = ReadSetting(_T("Setting.json"));

	//nBaud = 115200;//波特率
	//nData = 8;//数据位
	//nStop = 1;//停止位
	//nCal = 0;//校验位
	if (!jsonSetting.isNull()) {
		if (jsonSetting.isMember("BaudRate"))
		{
			nBaud = jsonSetting["BaudRate"].asInt();
		}
		else {
			jsonSetting["BaudRate"] = nBaud;
		}

		if (jsonSetting.isMember("DataBits"))
		{
			nData = jsonSetting["DataBits"].asInt();
		}
		else {
			jsonSetting["DataBits"] = nData;
		}

		if (jsonSetting.isMember("StopBits"))
		{
			nStop = jsonSetting["StopBits"].asInt();
		}
		else {
			jsonSetting["StopBits"] = nStop;
		}

		if (jsonSetting.isMember("CheckBits"))
		{
			nCal = jsonSetting["CheckBits"].asInt();
		}
		else {
			jsonSetting["CheckBits"] = nCal;
		}
	}
	WriteSetting(_T("Setting.json"), jsonSetting);

	int commnum_buf;
	commnum_buf = m_comlist.GetCurSel();
	if (commnum_buf < 0)
	{
		MessageBox(_T("获取串口错误"), _T("错误"), MB_ICONERROR);
		ComIsOK = FALSE;
		return;
	}
	m_comlist.GetLBText(commnum_buf, strcomname);
	if (!ComIsOK)
	{
		//注意，这里需要转换停止位
		int nTemp = 0;
		switch (nStop)
		{
		case 1: nStop = ONESTOPBIT; break;
		//case 1.5: nStop = ONE5STOPBITS; break;
		case 2: nStop = TWOSTOPBITS; break;
		}

		OpenComm(nBaud, nData, nTemp, nCal); //调用打开串口函数OpenComm() 
		if (ComIsOK)
			pReceiveThread = AfxBeginThread(ThreadFunc, this, THREAD_PRIORITY_LOWEST);
		//启动接收线程
		ShowStatus();
		if (!ComIsOK)
			m_comcontrol.SetWindowText(_T("Connect"));
		else
		{
			m_comcontrol.SetText(_T("Close")); //按钮显示状态改变 
			m_comcontrol.SetForeColor(RGB(0, 155, 0)); //串口打开后文本颜色变绿 
			m_NetStatusLED.RefreshWindow(TRUE, _T("ON"));//设置指示灯
			m_comlist.EnableWindow(false); //设置串口号下拉框不可用
			GetDlgItem(IDC_ONE_TRIGGER)->EnableWindow(true); //设置单次触发按钮可用 
			GetDlgItem(IDC_LOOP_TRIGGER)->EnableWindow(true); //设置循环触发按钮可用 
		}
		return;
	}
	else {
		CloseComm(); //调用关闭串口函数CloseComm() 
		//TerminateThread(pReceiveThread,0); 
		ShowStatus();
		m_comcontrol.SetText(_T("Connect"));
		m_comcontrol.SetForeColor(RGB(255, 0, 0));
		m_NetStatusLED.RefreshWindow(FALSE, _T("OFF"));//设置指示灯
		m_comlist.EnableWindow(true); //设置串口号下拉框可用 
		
		GetDlgItem(IDC_ONE_TRIGGER)->EnableWindow(true); //设置单次触发按钮不可用 
		GetDlgItem(IDC_LOOP_TRIGGER)->EnableWindow(true); //设置循环触发按钮不可用 
		return;
	}
}


void CLightingControlDlg::OnBnClickedOneTrigger()
{
	// TODO: 在此添加控件通知处理程序代码
	if (ComIsOK == FALSE)
	{
		MessageBox(_T("请先打开串口"), _T("提示"), MB_ICONINFORMATION);
		return;//return 0;
	}
	EnableControl(false);
	FPGAInit();
	BackSend(Order::TriggerOn_A, 5);
	SetTimer(1, m_CalibrationTime*1000, NULL); //设置定时器
	return;
}

void CLightingControlDlg::EnableControl(BOOL flag)
{
	GetDlgItem(IDC_SERIALPORT_ID)->EnableWindow(flag);
	GetDlgItem(IDC_CHECKA1)->EnableWindow(flag);
	GetDlgItem(IDC_CHECKA2)->EnableWindow(flag);
	GetDlgItem(IDC_CHECKA3)->EnableWindow(flag);
	GetDlgItem(IDC_CHECKA4)->EnableWindow(flag);
	GetDlgItem(IDC_CHECKA5)->EnableWindow(flag);
	GetDlgItem(IDC_CHECKA6)->EnableWindow(flag);
	GetDlgItem(IDC_CHECKA7)->EnableWindow(flag);
	GetDlgItem(IDC_CHECKALL_A)->EnableWindow(flag);
	GetDlgItem(IDC_CHECKB1)->EnableWindow(flag);
	GetDlgItem(IDC_CHECKB2)->EnableWindow(flag);
	GetDlgItem(IDC_CHECKB3)->EnableWindow(flag);
	GetDlgItem(IDC_CHECKB4)->EnableWindow(flag);
	GetDlgItem(IDC_CHECKB5)->EnableWindow(flag);
	GetDlgItem(IDC_CHECKB6)->EnableWindow(flag);
	GetDlgItem(IDC_CHECKB7)->EnableWindow(flag);
	GetDlgItem(IDC_CHECKALL_B)->EnableWindow(flag); 
	
	GetDlgItem(IDC_VOLTA)->EnableWindow(flag);
	GetDlgItem(IDC_VOLTB)->EnableWindow(flag);
	GetDlgItem(IDC_LIGHTING_WIDTH)->EnableWindow(flag);
	GetDlgItem(IDC_LIGHTING_DELAY)->EnableWindow(flag);
	GetDlgItem(IDC_CALIBRATION_TIME)->EnableWindow(flag);
}

void CLightingControlDlg::FPGAInit()
{
	BackSend(Order::Reset, 5);
	sendLEDwidth();
	sendLEDDelay();
	BackSend(Order::RegisterClockRate, 5);
	sendTriggerHLPoints();
	BackSend(Order::ShiftRegisterOn, 5);
	BackSend(Order::ReferenceVolt_DAC, 5);
	sendLightingVolt();
	BackSend(Order::WriteData_DAC, 5);
	BackSend(Order::CommonVolt_On, 5);
	ShowStatus();
}

BOOL CLightingControlDlg::BackSend(BYTE* msg, int msgLength, int sleepTime, int maxWaitingTime)
{
	DWORD dwBytesWritten = 5;
	dwBytesWritten = (DWORD)msgLength;

	BOOL bWriteStat;
	DWORD dwErrorFlags;
	COMSTAT ComStat;

	OVERLAPPED m_osWrite;
	memset(&m_osWrite, 0, sizeof(OVERLAPPED));
	m_osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	ClearCommError(hCom, &dwErrorFlags, &ComStat);
	bWriteStat = WriteFile(hCom,  msg, dwBytesWritten, &dwBytesWritten, &m_osWrite);
	if (!bWriteStat)
	{
		if (GetLastError() == ERROR_IO_PENDING)
		{
			WaitForSingleObject(m_osWrite.hEvent, maxWaitingTime);
		}
		//HandSendNum = 0;//return 0; 
		return 0;
	}
	
	//清空缓存区
	PurgeComm(hCom, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
	//HandSendNum= dwBytesWritten;//return dwBytesWritten;
	Sleep(sleepTime);
	return 1;
}

void CLightingControlDlg::sendLEDwidth()
{
	UpdateData(TRUE);
	Order::LightingWidth[3] = m_LightWidth;
	BackSend(Order::LightingWidth, 5);
}

void CLightingControlDlg::sendLEDDelay()
{
	UpdateData(TRUE);
	Order::LightingDelay[2] = m_LightDelay / (16*16);
	Order::LightingDelay[3] = m_LightDelay % (16*16);
	BackSend(Order::LightingDelay, 5);
}

void CLightingControlDlg::sendTriggerHLPoints()
{
	UpdateData(TRUE);
	int points = 11;
	Json::Value jsonSetting = ReadSetting(_T("Setting.json"));
	if (!jsonSetting.isNull()) {
		if (jsonSetting.isMember("TriggerHLPoints"))
		{
			points = jsonSetting["TriggerHLPoints"].asInt();
		}
		else
		{
			jsonSetting["TriggerHLPoints"] = points;
		}
	}
	if(points<256 && points>0)
		Order::TriggerPointsSet[3] = points;
	else
	{
		points = 255;
		Order::TriggerPointsSet[3] = points;
		jsonSetting["TriggerHLPoints"] = points;
		MessageBox(_T("TriggerHLPoints is over range of 1~255,It's reset to 255. See it in Setting.json"), 
			_T("提示"), MB_ICONINFORMATION);
	}
	WriteSetting(_T("Setting.json"), jsonSetting);
	BackSend(Order::TriggerPointsSet, 5);
}

void CLightingControlDlg::sendShiftRegisterData()
{
	UpdateData(TRUE);
	Order::LightingSwitch[2] = LightSwitchA;
	Order::LightingSwitch[3] = LightSwitchB;
	BackSend(Order::LightingSwitch, 5);
}

void CLightingControlDlg::sendLightingVolt()
{
	// 需要对电压到DAC数值的转换算法？？？？
	UpdateData(TRUE);
	int DAC_A = (int)((3100 - m_VoltA) / 0.085);
	int DAC_B = (int)((3100 - m_VoltB) / 0.085);
	Order::VoltA_Lighting[2] = DAC_A / (16 * 16);
	Order::VoltA_Lighting[3] = DAC_A % (16 * 16);
	BackSend(Order::LightingSwitch, 5);
	Order::VoltB_Lighting[2] = DAC_B / (16 * 16);
	Order::VoltB_Lighting[3] = DAC_B % (16 * 16);
}

void CLightingControlDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	TerminateThread(pReceiveThread, 0); //程序退出时,关闭串口监听线程
	WaitForSingleObject(pReceiveThread, INFINITE);
	CDialogEx::OnClose();
}

//刷新所有勾选框
//state:二进制码，
//status:勾选状态，1为勾选，0为不勾选
//LightID，0为A组灯；1为B组灯
void CLightingControlDlg::UpdateAllCheck(const BYTE stateBit, BOOL status, int LightID){
	//A灯组
	if(LightID==0){
		//更新灯组值
		if (status) {
			LightSwitchA = LightSwitchA | stateBit; //勾选做或运算
		}
		else {
			LightSwitchA = LightSwitchA & stateBit; //取消勾选做与运算
		}

		//更新勾选状态
		((CButton*)GetDlgItem(IDC_CHECKA1))->SetCheck(LightSwitchA & 0b00000001);
		((CButton*)GetDlgItem(IDC_CHECKA2))->SetCheck(LightSwitchA & 0b00000010);
		((CButton*)GetDlgItem(IDC_CHECKA3))->SetCheck(LightSwitchA & 0b00000100);
		((CButton*)GetDlgItem(IDC_CHECKA4))->SetCheck(LightSwitchA & 0b00001000);
		((CButton*)GetDlgItem(IDC_CHECKA5))->SetCheck(LightSwitchA & 0b00100000);
		((CButton*)GetDlgItem(IDC_CHECKA6))->SetCheck(LightSwitchA & 0b01000000);
		((CButton*)GetDlgItem(IDC_CHECKA7))->SetCheck(LightSwitchA & 0b10000000);
		((CButton*)GetDlgItem(IDC_CHECKALL_A))->SetCheck(LightSwitchA == 0b11101111);
	}
	
	//B灯组
	if (LightID == 1) {
		//更新灯组值
		if (status) {
			LightSwitchB = LightSwitchB | stateBit;
		}
		else {
			LightSwitchB = LightSwitchB & stateBit;
		}
		
		//更新勾选状态
		((CButton*)GetDlgItem(IDC_CHECKB1))->SetCheck(LightSwitchB & 0b00000001);
		((CButton*)GetDlgItem(IDC_CHECKB2))->SetCheck(LightSwitchB & 0b00000010);
		((CButton*)GetDlgItem(IDC_CHECKB3))->SetCheck(LightSwitchB & 0b00000100);
		((CButton*)GetDlgItem(IDC_CHECKB4))->SetCheck(LightSwitchB & 0b00001000);
		((CButton*)GetDlgItem(IDC_CHECKB5))->SetCheck(LightSwitchB & 0b00100000);
		((CButton*)GetDlgItem(IDC_CHECKB6))->SetCheck(LightSwitchB & 0b01000000);
		((CButton*)GetDlgItem(IDC_CHECKB7))->SetCheck(LightSwitchB & 0b10000000);
		((CButton*)GetDlgItem(IDC_CHECKALL_B))->SetCheck(LightSwitchB == 0b11101111);
	}
}


void CLightingControlDlg::OnBnClickedCheckA1()
{
	BYTE LightBit = 0b00000001;
	BOOL status = 1;
	int state = ((CButton*)GetDlgItem(IDC_CHECKA1))->GetCheck();
	if (state != 1) {
		LightBit = 0b11101111 & (~LightBit); //非勾选状态
		status = 0;
	}
	UpdateAllCheck(LightBit, status, 0);
}

void CLightingControlDlg::OnBnClickedCheckA2()
{
	BYTE LightBit = 0b00000010;
	BOOL status = 1;
	int state = ((CButton*)GetDlgItem(IDC_CHECKA2))->GetCheck();
	if (state != 1) {
		LightBit = 0b11101111 & (~LightBit); //非勾选状态
		status = 0;
	}
	UpdateAllCheck(LightBit, status, 0);
}

void CLightingControlDlg::OnBnClickedCheckA3()
{
	BYTE LightBit = 0b00000100;
	BOOL status = 1;
	int state = ((CButton*)GetDlgItem(IDC_CHECKA3))->GetCheck();
	if (state != 1) {
		LightBit = 0b11101111 & (~LightBit); //非勾选状态
		status = 0;
	}
	UpdateAllCheck(LightBit, status, 0);
}

void CLightingControlDlg::OnBnClickedCheckA4()
{
	BYTE LightBit = 0b00001000;
	BOOL status = 1;
	int state = ((CButton*)GetDlgItem(IDC_CHECKA4))->GetCheck();
	if (state != 1) {
		LightBit = 0b11101111 & (~LightBit); //非勾选状态
		status = 0;
	}
	UpdateAllCheck(LightBit, status, 0);
}

void CLightingControlDlg::OnBnClickedCheckA5()
{
	BYTE LightBit = 0b00100000;
	BOOL status = 1;
	int state = ((CButton*)GetDlgItem(IDC_CHECKA5))->GetCheck();
	if (state != 1) {
		LightBit = 0b11101111 & (~LightBit); //非勾选状态
		status = 0;
	}
	UpdateAllCheck(LightBit, status, 0);
}

void CLightingControlDlg::OnBnClickedCheckA6()
{
	BYTE LightBit = 0b01000000;
	BOOL status = 1;
	int state = ((CButton*)GetDlgItem(IDC_CHECKA6))->GetCheck();
	if (state != 1) {
		LightBit = 0b11101111 & (~LightBit); //非勾选状态
		status = 0;
	}
	UpdateAllCheck(LightBit, status, 0);
}

void CLightingControlDlg::OnBnClickedCheckA7()
{
	BYTE LightBit = 0b10000000;
	BOOL status = 1;
	int state = ((CButton*)GetDlgItem(IDC_CHECKA7))->GetCheck();
	if (state != 1) {
		LightBit = 0b11101111 & (~LightBit); //非勾选状态
		status = 0;
	}
	UpdateAllCheck(LightBit, status, 0);
}

void CLightingControlDlg::OnBnClickedCheckALL_A()
{
	BYTE LightBit = 0b11101111;
	BOOL status = 1;
	int state = ((CButton*)GetDlgItem(IDC_CHECKALL_A))->GetCheck();
	if (state != 1) {
		LightBit = 0b11101111 & (~LightBit); //非勾选状态
		status = 0;
	}
	UpdateAllCheck(LightBit, status, 0);
}

void CLightingControlDlg::OnBnClickedCheckB1()
{
	BYTE LightBit = 0b00000001;
	BOOL status = 1;
	int state = ((CButton*)GetDlgItem(IDC_CHECKB1))->GetCheck();
	if (state != 1) {
		LightBit = 0b11101111 & (~LightBit); //非勾选状态
		status = 0;
	}
	UpdateAllCheck(LightBit, status, 1);
}

void CLightingControlDlg::OnBnClickedCheckB2()
{
	BYTE LightBit = 0b00000010;
	BOOL status = 1;
	int state = ((CButton*)GetDlgItem(IDC_CHECKB2))->GetCheck();
	if (state != 1) {
		LightBit = 0b11101111 & (~LightBit); //非勾选状态
		status = 0;
	}
	UpdateAllCheck(LightBit, status, 1);
}

void CLightingControlDlg::OnBnClickedCheckB3()
{
	BYTE LightBit = 0b00000100;
	BOOL status = 1;
	int state = ((CButton*)GetDlgItem(IDC_CHECKB3))->GetCheck();
	if (state != 1) {
		LightBit = 0b11101111 & (~LightBit); //非勾选状态
		status = 0;
	}
	UpdateAllCheck(LightBit, status, 1);
}

void CLightingControlDlg::OnBnClickedCheckB4()
{
	BYTE LightBit = 0b00001000;
	BOOL status = 1;
	int state = ((CButton*)GetDlgItem(IDC_CHECKB4))->GetCheck();
	if (state != 1) {
		LightBit = 0b11101111 & (~LightBit); //非勾选状态
		status = 0;
	}
	UpdateAllCheck(LightBit, status, 1);
}

void CLightingControlDlg::OnBnClickedCheckB5()
{
	BYTE LightBit = 0b00100000;
	BOOL status = 1;
	int state = ((CButton*)GetDlgItem(IDC_CHECKB5))->GetCheck();
	if (state != 1) {
		LightBit = 0b11101111 & (~LightBit); //非勾选状态
		status = 0;
	}
	UpdateAllCheck(LightBit, status, 1);
}

void CLightingControlDlg::OnBnClickedCheckB6()
{
	BYTE LightBit = 0b01000000;
	BOOL status = 1;
	int state = ((CButton*)GetDlgItem(IDC_CHECKB6))->GetCheck();
	if (state != 1) {
		LightBit = 0b11101111 & (~LightBit); //非勾选状态
		status = 0;
	}
	UpdateAllCheck(LightBit, status, 1);
}

void CLightingControlDlg::OnBnClickedCheckB7()
{
	BYTE LightBit = 0b10000000;
	BOOL status = 1;
	int state = ((CButton*)GetDlgItem(IDC_CHECKB7))->GetCheck();
	if (state != 1) {
		LightBit = 0b11101111 & (~LightBit); //非勾选状态
		status = 0;
	}
	UpdateAllCheck(LightBit, status, 1);
}

void CLightingControlDlg::OnBnClickedCheckALL_B()
{
	BYTE LightBit = 0b11101111;
	BOOL status = 1;
	int state = ((CButton*)GetDlgItem(IDC_CHECKALL_B))->GetCheck();
	if (state != 1) {
		LightBit = 0b11101111 & (~LightBit); //非勾选状态
		status = 0;
	}
	UpdateAllCheck(LightBit, status, 1);
}

void CLightingControlDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	switch (nIDEvent)
	{
	case 1:
	{
		timer++;
		if (timer %3 == 1)
		{
			BackSend(Order::TriggerOff, 5); //停止触发
			BackSend(Order::TriggerOn_B, 5); //B组触发
		}
		if (timer % 3 == 2)
		{
			BackSend(Order::TriggerOff, 5); //停止触发
			BackSend(Order::TriggerOn_AB, 5); //B组触发
		}
		if (timer % 3 == 0)
		{
			BackSend(Order::TriggerOff, 5); //停止触发
			KillTimer(1);
			BackSend(Order::CommonVolt_Off, 5); //关闭外设电源
			BackSend(Order::DAC_Off, 5); //关闭DAC配置

			//重置部分参数
			timer = 0;
			EnableControl(TRUE);
		}
	}
	break;
	case 2:
		break;
	}
	CDialogEx::OnTimer(nIDEvent);
}
