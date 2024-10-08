
// LEDControlDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "LEDControl.h"
#include "LEDControlDlg.h"
#include "afxdialogex.h"

#include "MyConst.h"
#include "comm.h"
#include "Order.h"
#include "Log.h"

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
	virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 支持

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

// CLEDControlDlg 对话框

CLEDControlDlg::CLEDControlDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_LEDCONTROL_DIALOG, pParent),
	  pReceiveThread(nullptr),
	  config_nBaud(115200), config_nData(8), config_nStop(1), config_nCal(0),
	  config_minV(2000), config_maxV(3500),
	  config_minLEDWidth(1),config_maxLEDWidth(255),
	  config_PowerStableTime(10),
	  config_triggerHLPoints(11),
	  config_p1_A(-10.87), config_p2_A(33150.0),
	  config_p1_B(-10.92), config_p2_B(33240.0),
    TCPfeedback(FALSE),
    LastSendMsg(NULL),
    RecvMsg(NULL),
    recievedFBLength(0),
    FeedbackLen(5),
	  m_LEDSwitchA(0x00), m_LEDSwitchB(0x00),
	  m_strLog(_T("")),
	  m_CalibrationTime(10),
	  m_LEDDelay(9999),
	  m_tempLEDWidth(10),
	  m_tempVoltA(2700),
	  m_tempVoltB(2700),
	  timer(0),
	  VoltFile(_T(""))
	, send_num(0)
	, cache_num(0)
	, recv_num(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON1);
	vec_VoltA.resize(0);
	vec_VoltB.resize(0);
	vec_TimeWidth.resize(0);
}

void CLEDControlDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SERIALPORT_ID, m_comlist);
	DDX_Control(pDX, IDC_CONNECT_SERIALPORT, m_comcontrol);
	DDX_Control(pDX, IDC_LED_STATUS, m_NetStatusLED);

	DDX_Text(pDX, IDC_EDIT_LOG, m_strLog);
	DDX_Text(pDX, IDC_CALIBRATION_TIME, m_CalibrationTime);
	DDX_Text(pDX, IDC_LED_DELAY, m_LEDDelay);
	DDX_Text(pDX, IDC_LED_WIDTH, m_tempLEDWidth);
	DDX_Text(pDX, IDC_VOLTA, m_tempVoltA);
	DDX_Text(pDX, IDC_VOLTB, m_tempVoltB);
	DDX_Control(pDX, IDC_EDIT_LOG, m_LogEdit);
	//DDX_Text(pDX, IDC_SEND_NUM, send_num);
	//DDX_Text(pDX, IDC_CACHE_NUM, cache_num);
	//DDX_Text(pDX, IDC_RECV_NUM, recv_num);
}

BEGIN_MESSAGE_MAP(CLEDControlDlg, CDialogEx)
ON_WM_SYSCOMMAND()
ON_WM_PAINT()
ON_WM_QUERYDRAGICON()
ON_BN_CLICKED(IDC_CONNECT_SERIALPORT, &CLEDControlDlg::OnComcontrol)
ON_BN_CLICKED(IDC_ONE_TRIGGER, &CLEDControlDlg::OnBnClickedOneTrigger)
ON_WM_CLOSE()
ON_BN_CLICKED(IDC_CHECKA1, &CLEDControlDlg::OnBnClickedCheckA1)
ON_BN_CLICKED(IDC_CHECKA2, &CLEDControlDlg::OnBnClickedCheckA2)
ON_BN_CLICKED(IDC_CHECKA3, &CLEDControlDlg::OnBnClickedCheckA3)
ON_BN_CLICKED(IDC_CHECKA4, &CLEDControlDlg::OnBnClickedCheckA4)
ON_BN_CLICKED(IDC_CHECKA5, &CLEDControlDlg::OnBnClickedCheckA5)
ON_BN_CLICKED(IDC_CHECKA6, &CLEDControlDlg::OnBnClickedCheckA6)
ON_BN_CLICKED(IDC_CHECKA7, &CLEDControlDlg::OnBnClickedCheckA7)
ON_BN_CLICKED(IDC_CHECKALL_A, &CLEDControlDlg::OnBnClickedCheckALL_A)
ON_BN_CLICKED(IDC_CHECKB1, &CLEDControlDlg::OnBnClickedCheckB1)
ON_BN_CLICKED(IDC_CHECKB2, &CLEDControlDlg::OnBnClickedCheckB2)
ON_BN_CLICKED(IDC_CHECKB3, &CLEDControlDlg::OnBnClickedCheckB3)
ON_BN_CLICKED(IDC_CHECKB4, &CLEDControlDlg::OnBnClickedCheckB4)
ON_BN_CLICKED(IDC_CHECKB5, &CLEDControlDlg::OnBnClickedCheckB5)
ON_BN_CLICKED(IDC_CHECKB6, &CLEDControlDlg::OnBnClickedCheckB6)
ON_BN_CLICKED(IDC_CHECKB7, &CLEDControlDlg::OnBnClickedCheckB7)
ON_BN_CLICKED(IDC_CHECKALL_B, &CLEDControlDlg::OnBnClickedCheckALL_B)
ON_WM_TIMER()
ON_BN_CLICKED(IDC_VOLT_LOOP_FILE, &CLEDControlDlg::ChoseVoltLoopFile)
ON_BN_CLICKED(IDC_LOOP_TRIGGER, &CLEDControlDlg::OnBnClickedLoopTrigger)
ON_EN_KILLFOCUS(IDC_VOLTA, &CLEDControlDlg::OnEnKillfocusVoltA)
ON_EN_KILLFOCUS(IDC_VOLTB, &CLEDControlDlg::OnEnKillfocusVoltB)
ON_EN_KILLFOCUS(IDC_LED_WIDTH, &CLEDControlDlg::OnEnKillfocusLEDWidth)
ON_EN_KILLFOCUS(IDC_LED_DELAY, &CLEDControlDlg::OnEnKillfocusLEDDelay)
ON_EN_KILLFOCUS(IDC_CALIBRATION_TIME, &CLEDControlDlg::OnEnKillfocusCalibrationTime)
ON_BN_CLICKED(IDC_RESET_KERNAL, &CLEDControlDlg::ResetSystem)
ON_BN_CLICKED(IDC_CLEAR_LOG, &CLEDControlDlg::OnBnClickedClearLog)
END_MESSAGE_MAP()

// CLEDControlDlg 消息处理程序

BOOL CLEDControlDlg::OnInitDialog()
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
	SetIcon(m_hIcon, TRUE);	 // 设置大图标
	SetIcon(m_hIcon, FALSE); // 设置小图标

	// ShowWindow(SW_MAXIMIZE); //初始化弹出时最大化

	// ShowWindow(SW_MINIMIZE); //初始化弹出时最小化

	// TODO: 在此添加额外的初始化代码
	// 设置软件标题名称
	CString AppTitle = _T("灯光控制界面"); // 默认名称
	Json::Value jsonSetting = ReadSetting(_T("Setting.json"));
	if (!jsonSetting.isNull())
	{
		if (jsonSetting.isMember("SoftwareTitle"))
		{
			const char *s = jsonSetting["SoftwareTitle"].asCString();
			int nLenW = ::MultiByteToWideChar(CP_UTF8, 0, s, -1, NULL, 0);
			wchar_t *wszBuffer = new wchar_t[nLenW];
			::MultiByteToWideChar(CP_UTF8, 0, s, -1, wszBuffer, nLenW);

			// 将 Unicode 编码转换为 GB2312 编码（也就是简体中文编码）
			int nLenA = ::WideCharToMultiByte(CP_ACP, 0, wszBuffer, -1, NULL, 0, NULL, NULL);
			char *szBuffer = new char[nLenA];
			::WideCharToMultiByte(CP_ACP, 0, wszBuffer, -1, szBuffer, nLenA, NULL, NULL);

			// 输出结果
			std::string strResult(szBuffer);
			const char *tmp = strResult.c_str();
			AppTitle = tmp;
		}
		else
		{
			string pStrTitle = _UnicodeToUtf8(AppTitle);
			// char* pStrTitle = CstringToWideCharArry(AppTitle);
			jsonSetting["SoftwareTitle"] = pStrTitle;
		}

		WriteSetting(_T("Setting.json"), jsonSetting);
	}
	SetWindowText(AppTitle);

	//---------------初始化界面输入参数，读取上一次的设置参数---------------
	InitSettingByHistoryInput();
	//---------------读取配置参数-----------------
	InitConfigSetting();
	//---------------初始化状态栏---------------
	InitBarSettings();

	//----------调用自动找串口函数 ---------------
	FindComm();
	SingleTriggerStatus = FALSE;
	LoopTriggerStatus = FALSE;

	//------------设置控件初始状态的可用性--------
	m_NetStatusLED.RefreshWindow(FALSE, _T("OFF"));	   // 设置指示灯
	GetDlgItem(IDC_ONE_TRIGGER)->EnableWindow(false);  // 设置单次触发按钮不可用
	GetDlgItem(IDC_LOOP_TRIGGER)->EnableWindow(false); // 设置循环触发按钮不可用
	GetDlgItem(IDC_RESET_KERNAL)->EnableWindow(false); // 设置重置内核按钮不可用

	return TRUE; // 除非将焦点设置到控件，否则返回 TRUE
}

void CLEDControlDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CLEDControlDlg::InitConfigSetting()
{
	CString info;
	Json::Value jsonSetting = ReadSetting(_T("Setting.json"));
	if (!jsonSetting.isNull())
	{
		//-------------读取电源开启后的稳压时长-------------
		if (jsonSetting.isMember("PowerStableTime"))
		{
			config_PowerStableTime = jsonSetting["PowerStableTime"].asInt();
		}
		else
		{
			jsonSetting["PowerStableTime"] = config_PowerStableTime;
		}

		//--------------读取串口参数-----------------
		if (jsonSetting.isMember("BaudRate"))
		{
			config_nBaud = jsonSetting["BaudRate"].asInt();
		}
		else
		{
			jsonSetting["BaudRate"] = config_nBaud;
		}

		if (jsonSetting.isMember("DataBits"))
		{
			config_nData = jsonSetting["DataBits"].asInt();
		}
		else
		{
			jsonSetting["DataBits"] = config_nData;
		}

		if (jsonSetting.isMember("StopBits"))
		{
			config_nStop = jsonSetting["StopBits"].asInt();
		}
		else
		{
			jsonSetting["StopBits"] = config_nStop;
		}

		if (jsonSetting.isMember("CheckBits"))
		{
			config_nCal = jsonSetting["CheckBits"].asInt();
		}
		else
		{
			jsonSetting["CheckBits"] = config_nCal;
		}

		//-------------------读取电压到DAC的刻度曲线----------------
		// VoltA_P1
		if (jsonSetting.isMember("CalibrationVoltA_P1"))
		{
			config_p1_A = jsonSetting["CalibrationVoltA_P1"].asDouble();
		}
		else
		{
			jsonSetting["CalibrationVoltA_P1"] = config_p1_A;
		}
		// VoltA_P1
		if (jsonSetting.isMember("CalibrationVoltA_P2"))
		{
			config_p2_A = jsonSetting["CalibrationVoltA_P2"].asDouble();
		}
		else
		{
			jsonSetting["CalibrationVoltA_P2"] = config_p2_A;
		}

		// VoltB_P1
		if (jsonSetting.isMember("CalibrationVoltB_P1"))
		{
			config_p1_B = jsonSetting["CalibrationVoltB_P1"].asDouble();
		}
		else
		{
			jsonSetting["CalibrationVoltB_P1"] = config_p1_B;
		}
		// VoltB_P2
		if (jsonSetting.isMember("CalibrationVoltB_P2"))
		{
			config_p2_B = jsonSetting["CalibrationVoltB_P2"].asDouble();
		}
		else
		{
			jsonSetting["CalibrationVoltB_P2"] = config_p2_B;
		}

		// 根据DAC范围，利用刻度曲线可计算出电压可以输入的范围，DAC：1~4095
		// 必须确保配置参数中的电压输入范围小于DAC转化后的电压范围。
		// 注意：A组和B组共用一套范围限制，因此得确保范围都满足A组与B组。
		int minDAC = 1;
		int maxDAC = 4095;
		int DAC2VoltA_min = 0;
		int DAC2VoltA_max = 0;
		int DAC2VoltB_min = 0;
		int DAC2VoltB_max = 0;
		int DAC2Volt_min = 0;
		int DAC2Volt_max = 0;
		DAC2VoltA_min = (int)ceil((maxDAC - config_p2_A) / config_p1_A);
		DAC2VoltA_max = (int)floor((minDAC - config_p2_A) / config_p1_A);
		DAC2VoltB_min = (int)ceil((maxDAC - config_p2_B) / config_p1_B);
		DAC2VoltB_max = (int)floor((minDAC - config_p2_B) / config_p1_B);
		DAC2Volt_min = DAC2VoltA_min < DAC2VoltB_min ? DAC2VoltB_min : DAC2VoltA_min;
		DAC2Volt_max = DAC2VoltA_max > DAC2VoltB_max ? DAC2VoltB_max : DAC2VoltA_max;

		//---------------读取配置文件中的LED电压上下限------------
		// 并对电压范围进行检查，其转换为DAC的范围必须在1~4095范围之内
		if (jsonSetting.isMember("MinVolt"))
		{
			config_minV = jsonSetting["MinVolt"].asInt();
			if (config_minV < DAC2Volt_min)
			{
				info.Format(_T("Warning: according to 'CalibrationVoltA_P1','CalibrationVoltA_P2','CalibrationVoltB_P1','CalibrationVoltB_P2',\
				 the 'MinVolt':%d is samll than DAC2Volt_min:%d. The 'MinVolt' is reset to %d"),
							config_minV, DAC2Volt_min, DAC2Volt_min);
				PrintLog(info);
				jsonSetting["MinVolt"] = DAC2Volt_min;
				config_minV = DAC2Volt_min;
			}
		}
		else
		{
			jsonSetting["MinVolt"] = config_minV;
			info.Format(_T("Warning：Cannot find key words 'MinVolt' in 'Setting.json' file, MinVolt is set to %d！"), config_minV);
			PrintLog(info);
		}
		if (jsonSetting.isMember("MaxVolt"))
		{
			config_maxV = jsonSetting["MaxVolt"].asInt();
			if (config_maxV > DAC2Volt_max)
			{
				info.Format(_T("Warning: according to 'CalibrationVoltA_P1','CalibrationVoltA_P2','CalibrationVoltB_P1','CalibrationVoltB_P2',\
				the 'MaxVolt':%d is more than DAC2Volt_max:%d. The 'MaxVolt' is reset to %d"),
							config_maxV, DAC2Volt_max, DAC2Volt_max);
				PrintLog(info);
				jsonSetting["MaxVolt"] = DAC2Volt_max;
				config_maxV = DAC2Volt_max;
			}
		}
		else
		{
			jsonSetting["MaxVolt"] = config_maxV;
			info.Format(_T("Warning：Cannot find key words 'MaxVolt' in 'Setting.json' file, MinVolt is set to %d！"), config_maxV);
			PrintLog(info);
		}

		if (jsonSetting.isMember("MinLEDWidth"))
		{
			config_minLEDWidth = jsonSetting["MinLEDWidth"].asInt();
			if (config_minLEDWidth < 1)
			{
				info.Format(_T("Warning: the 'MinLEDWidth':%d is less than 1. The 'MinLEDWidth' is reset to 1"),
							config_minLEDWidth);
				PrintLog(info);
				config_minLEDWidth = 1;
				jsonSetting["MinLEDWidth"] = config_minLEDWidth;
			}
		}
		else
		{
			jsonSetting["MinLEDWidth"] = config_minLEDWidth;
			info.Format(_T("Warning: cannot find key words 'MinLEDWidth' in 'Setting.json' file, The 'MinLEDWidth' is set to 1"), config_minLEDWidth);
			PrintLog(info);
		}

		if (jsonSetting.isMember("MaxLEDWidth"))
		{
			config_maxLEDWidth = jsonSetting["MaxLEDWidth"].asInt();
			if (config_maxLEDWidth > 255)
			{
				info.Format(_T("Warning: the 'MaxLEDWidth':%d is more than 255. The 'MaxLEDWidth' is reset to 255"),
							config_maxLEDWidth);
				PrintLog(info);
				config_maxLEDWidth = 1;
				jsonSetting["MaxLEDWidth"] = config_maxLEDWidth;
			}
		}
		else
		{
			jsonSetting["MaxLEDWidth"] = config_maxLEDWidth;
			info.Format(_T("Warning: cannot find key words 'MaxLEDWidth' in 'Setting.json' file, The 'MaxLEDWidth' is set to 255"), config_maxLEDWidth);
			PrintLog(info);
		}

		// 更新界面电压控件输入范围
		CString str_Volt;
		str_Volt.Format(_T("range:%d~%d"), config_minV, config_maxV);
		GetDlgItem(IDC_VOLTA_NOTICE)->SetWindowText(str_Volt);
		GetDlgItem(IDC_VOLTB_NOTICE)->SetWindowText(str_Volt);

		//---------------------读取硬件触发高电平点数------------------
		if (!jsonSetting.isNull())
		{
			if (jsonSetting.isMember("TriggerHLPoints"))
			{
				config_triggerHLPoints = jsonSetting["TriggerHLPoints"].asInt();
			}
			else
			{
				jsonSetting["TriggerHLPoints"] = config_triggerHLPoints;
			}
		}
		if (config_triggerHLPoints < 256 && config_triggerHLPoints > 0)
			Order::TriggerHLPointsSet[3] = config_triggerHLPoints;
		else
		{
			config_triggerHLPoints = 11;
			Order::TriggerHLPointsSet[3] = config_triggerHLPoints;
			jsonSetting["TriggerHLPoints"] = config_triggerHLPoints;
			PrintLog(_T("TriggerHLPoints is over range of 1~255,It's reset to 11. See it in Setting.json"));
		}

		// 更新配置文件
		WriteSetting(_T("Setting.json"), jsonSetting);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CLEDControlDlg::OnPaint()
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

// 当用户拖动最小化窗口时系统调用此函数取得光标
// 显示。
HCURSOR CLEDControlDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CLEDControlDlg::InitBarSettings()
{
	CRect rectDlg;
	GetClientRect(rectDlg); // 获得窗体的大小
	// 添加状态栏
	UINT nID[] = {1001, 1002};
	// 创建状态栏
	m_statusBar.Create(this);
	// 添加状态栏面板，参数为ID数组和面板数量
	m_statusBar.SetIndicators(nID, sizeof(nID) / sizeof(UINT));
	// 设置面板序号，ID，样式和宽度，SBPS_NORMAL为普通样式，固定宽度，SBPS_STRETCH为弹簧样式，会自动扩展它的空间
	m_statusBar.SetPaneInfo(0, 1001, SBPS_NORMAL, int(0.5 * rectDlg.Width()));
	m_statusBar.SetPaneInfo(1, 1002, SBPS_NORMAL, int(0.5 * rectDlg.Width()));
	// 设置状态栏位置
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);
	// 设置状态栏面板文本，参数为面板序号和对应文本
	m_statusBar.SetPaneText(0, _T("ready"));
	m_statusBar.SetPaneText(1, _T("日期"));
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);
	// 开启定时器，刷新状态栏参数
	SetTimer(3, 1000, NULL);
}

void CLEDControlDlg::InitSettingByHistoryInput()
{
	Json::Value jsonSetting = ReadSetting(_T("Setting.json"));
	if (!jsonSetting.isNull())
	{
		// m_tempVoltA
		if (jsonSetting.isMember("tempVoltA"))
		{
			m_tempVoltA = jsonSetting["tempVoltA"].asInt();
		}
		else
		{
			jsonSetting["tempVoltA"] = m_tempVoltA;
		}
		// m_tempVoltB
		if (jsonSetting.isMember("tempVoltB"))
		{
			m_tempVoltB = jsonSetting["tempVoltB"].asInt();
		}
		else
		{
			jsonSetting["tempVoltB"] = m_tempVoltB;
		}
		// m_CalibrationTime
		if (jsonSetting.isMember("CalibrationTime"))
		{
			m_CalibrationTime = jsonSetting["CalibrationTime"].asInt();
		}
		else
		{
			jsonSetting["CalibrationTime"] = m_CalibrationTime;
		}
		// m_LEDDelay
		if (jsonSetting.isMember("LEDDelay"))
		{
			m_LEDDelay = jsonSetting["LEDDelay"].asInt();
		}
		else
		{
			jsonSetting["LEDDelay"] = m_LEDDelay;
		}
		// m_tempLEDWidth
		if (jsonSetting.isMember("tempLEDWidth"))
		{
			m_tempLEDWidth = jsonSetting["tempLEDWidth"].asInt();
		}
		else
		{
			jsonSetting["tempLEDWidth"] = m_tempLEDWidth;
		}
		// LED开关状态
		if (jsonSetting.isMember("LEDSwitchA"))
		{
			int valueA = jsonSetting["LEDSwitchA"].asInt();
			m_LEDSwitchA = (BYTE)(valueA & 0xFF);
		}
		else
		{
			jsonSetting["LEDSwitchA"] = m_LEDSwitchA;
		}
		// LED开关状态
		if (jsonSetting.isMember("LEDSwitchB"))
		{
			int valueB = jsonSetting["LEDSwitchB"].asInt();
			m_LEDSwitchB = (BYTE)(valueB & 0xFF);
		}
		else
		{
			jsonSetting["LEDSwitchB"] = m_LEDSwitchB;
		}
	}
	WriteSetting(_T("Setting.json"), jsonSetting);
	// 更新将数值刷新到控件上
	UpdateData(FALSE);
	// 勾选框的状态刷新需要特别设定
	UpdateLEDCheck();
}

BOOL CLEDControlDlg::ReadLoopFile(const CString file)
{
	vec_VoltA.resize(0);
	vec_VoltB.resize(0);
	vec_TimeWidth.resize(0);
	// 提醒用户选择预设电压json文件
	if (file == _T(""))
	{
		MessageBox(_T("You have not selected the volt loop file,you must choose it before 'Loop Trigger'"),
				   _T("error"), MB_ICONINFORMATION);
		return FALSE;
	}

	int NumVoltA = 0;
	int NumVoltB = 0;
	int NumLEDWidth = 0;
	// 提取数据并做类型、数值是否在范围内的检查
	Json::Value jsonVoltSetting = ReadSetting(file);
	if (!jsonVoltSetting.isNull())
	{
		if (jsonVoltSetting.isMember("voltA"))
		{
			NumVoltA = jsonVoltSetting["voltA"].size();
			if (NumVoltA > 0)
			{
				for (int i = 0; i < NumVoltA; i++)
				{
					if (jsonVoltSetting["voltA"][i].isInt())
					{
						int volt = jsonVoltSetting["voltA"][i].asInt();
						if (volt < config_minV || volt > config_maxV)
						{
							CString info;
							info.Format(_T("ERROR: %d of 'voltA' is over range:%d~%d,In file: "), volt, config_minV, config_maxV);
							info += file;
							PrintLog(info);
							vec_VoltA.resize(0);
							return FALSE;
						}
						vec_VoltA.push_back(volt);
					}
					else
					{
						PrintLog(_T("ERROR: The input of 'voltA' is wrong; It must be set to an integer. In file: ") + file);
						return FALSE;
					}
				}
			}
			else
			{
				PrintLog(_T("ERROR: The input size of 'voltA' is zero. Please check the file in ") + file);
				return FALSE;
			}
		}
		else
		{
			PrintLog(_T("ERROR: Cannot find 'voltA'. See it in ") + file);
			return FALSE;
		}

		if (jsonVoltSetting.isMember("voltB"))
		{
			NumVoltB = jsonVoltSetting["voltB"].size();

			if (NumVoltB > 0)
			{
				for (int i = 0; i < NumVoltB; i++)
				{
					if (jsonVoltSetting["voltB"][i].isInt())
					{
						int volt = jsonVoltSetting["voltB"][i].asInt();
						if (volt < config_minV || volt > config_maxV)
						{
							CString info;
							info.Format(_T("ERROR: %d of 'voltB' is over range:%d~%d,In file: "), volt, config_minV, config_maxV);
							info += file;
							PrintLog(info);
							vec_VoltB.resize(0);
							return FALSE;
						}
						vec_VoltB.push_back(volt);
					}
					else
					{
						PrintLog(_T("ERROR: The input of 'voltB' is wrong; It must be set to an integer. In file: ") + file);
						return FALSE;
					}
				}
			}
			else
			{
				PrintLog(_T("ERROR: The input size of 'voltB' is zero. Please check the file in ") + file);
				return FALSE;
			}
		}
		else
		{
			PrintLog(_T("ERROR: Cannot find 'voltB'. See it in ") + file);
			return FALSE;
		}

		if (jsonVoltSetting.isMember("LEDWidth"))
		{
			NumLEDWidth = jsonVoltSetting["LEDWidth"].size();

			if (NumLEDWidth > 0)
			{
				for (int i = 0; i < NumLEDWidth; i++)
				{
					if (jsonVoltSetting["LEDWidth"][i].isInt())
					{
						int width = jsonVoltSetting["LEDWidth"][i].asInt();
						if (width < config_minLEDWidth || width > config_maxLEDWidth)
						{
							CString info;
							info.Format(_T("ERROR: %d of 'LEDWidth' is over range:%d~%d,In file: "), width, config_minLEDWidth, config_minLEDWidth);
							info += file;
							PrintLog(info);
							vec_TimeWidth.resize(0);
							return FALSE;
						}
						vec_TimeWidth.push_back(width);
					}
					else
					{
						PrintLog(_T("ERROR: The input of 'LEDWidth' is wrong; It must be set to an integer. In file: ") + file);
						return FALSE;
					}
				}
			}
			else
			{
				PrintLog(_T("ERROR: The input size of 'LEDWidth' is zero. Please check the file in ") + file);
				return FALSE;
			}
		}
		else
		{
			PrintLog(_T("ERROR: Cannot find 'LEDWidth'. See it in ") + file);
			return FALSE;
		}
	}

	if (NumVoltA > 0 && (NumVoltA == NumVoltB) && (NumVoltA == NumLEDWidth))
	{
		m_tempVoltA = vec_VoltA[0]; // 界面当前电压值更新
		m_tempVoltB = vec_VoltB[0]; // 界面当前电压值更新
		m_tempLEDWidth = vec_TimeWidth[0]; //界面当前时间宽度
	}
	else
	{
		CString info;
		info.Format(_T("ERROR: the size of  'voltA' is %d,the size of 'VoltB' is %d,the size \
			of 'LEDWidth' is %d.They are not equal.See it in"), NumVoltA, NumVoltB, NumLEDWidth);
		PrintLog(info + file);
		return FALSE;
	}

	// 刷新界面控件的内容
	UpdateData(FALSE);
	m_LogEdit.LineScroll(m_LogEdit.GetLineCount()); // 每次刷新后都显示最底部
	return TRUE;
}

LRESULT CLEDControlDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: 在此添加专用代码和/或调用基类
	switch (message)
	{
	case WM_FOUNDCOMM:
	{// 已找到串口,串口号以字符串形式由wParam传递
		m_comlist.AddString((LPCTSTR)wParam); // 用AddString添加一个字符串即COM号到m_comlist列表框中
		break;
	}
	case WM_READCOMM:
	{ // 读串口消息
		// ReadComm();
		this->SendDlgItemMessage(IDC_EDIT_LOG, WM_VSCROLL, SB_BOTTOM, 0); // 滚动条始终在底部
		break;
	}
	}

	return CDialogEx::WindowProc(message, wParam, lParam);
}

DWORD CLEDControlDlg::ReadComm()
{
	OVERLAPPED m_osRead;
	memset(&m_osRead, 0, sizeof(OVERLAPPED));
	m_osRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	unsigned char lpInBuffer[1024];
	DWORD dwBytesRead = 1024;
	BOOL bReadStatus;
	bReadStatus = ReadFile(hCom, lpInBuffer, dwBytesRead, &dwBytesRead, &m_osRead);
	if (!bReadStatus) // 如果ReadFile函数返回FALSE
	{
		if (GetLastError() == ERROR_IO_PENDING) // GetLastError()函数返回ERROR_IO_PENDING,表明串口正在进行读操作
		{
			WaitForSingleObject(m_osRead.hEvent, 2000); // 使用WaitForSingleObject函数等待，直到读操作完成或延时已达到2000ms
			// 当串口读操作进行完毕后，m_osRead的hEvent事件会变为有信号
			PurgeComm(hCom, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
			return dwBytesRead;
		}
		return 0;
	}
	lpInBuffer[dwBytesRead] = NULL;
	int nLength = static_cast<int>(dwBytesRead); //本次接收数据长度
	// strTemp = lpInBuffer;
	recv_num += nLength;
	int receLen = 0; //当前已经接收的总反馈指令长度
	receLen = recievedFBLength + nLength;
	BYTE* tempChar = (BYTE*)malloc(receLen);
	//先取旧数据
	if (RecvMsg != NULL) {
		for (int i = 0; i < recievedFBLength; i++) {
			tempChar[i] = *(RecvMsg + i);
		}
	}

	// 拼接新数据
	for (int i = 0; i < nLength; i++) {
		tempChar[recievedFBLength + i] = lpInBuffer[i];
	}
	RecvMsg = tempChar;
	recievedFBLength +=nLength;

	//::SendMessage(::AfxGetMainWnd()->m_hWnd,WM_READCOMM,1,0); //发送消息,已读到 ，采用排队消息处理方式
	return 1;
}

void CLEDControlDlg::ShowStatus()
{
	// 状态栏显示状态
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

	strBaud.Format(_T("%d"), config_nBaud);
	strStop.Format(_T("%d"), config_nData);
	strData.Format(_T("%d"), config_nStop);
	strCal.Format(_T("%d"), config_nCal);

	m_strLog = _T("串口: ") + comnum + _T("  ") + _T("状态: ") + _T("波特率: ") + strBaud +
			   _T(", ") + _T("数据位: ") + strData + _T(", ") + _T("停止位: ") + strStop + _T(", ") + _T("校验位: ") + strCal;
	UpdateData(FALSE);
	m_LogEdit.LineScroll(m_LogEdit.GetLineCount()); // 每次刷新后都显示最底部
}

void CLEDControlDlg::OnComcontrol()
{
	// 获取配置参数，并连接串口
	Json::Value jsonSetting = ReadSetting(_T("Setting.json"));

	int commnum_buf;
	commnum_buf = m_comlist.GetCurSel();
	if (commnum_buf < 0)
	{
		MessageBox(_T("获取串口错误"), _T("错误"), MB_ICONERROR);
		ComIsOK = FALSE;
		return;
	}
	m_comlist.GetLBText(commnum_buf, strcomname);
	strcomname = _T("\\\\.\\") + strcomname;
	if (!ComIsOK)
	{
		// 注意，这里需要转换停止位
		int nTemp = 0;
		switch (config_nStop)
		{
		case 1:
			config_nStop = ONESTOPBIT;
			break;
		// case 1.5: config_nStop = ONE5STOPBITS; break;
		case 2:
			config_nStop = TWOSTOPBITS;
			break;
		}

		OpenComm(config_nBaud, config_nData, nTemp, config_nCal); // 调用打开串口函数OpenComm()
		if (ComIsOK)
		{
			pReceiveThread = AfxBeginThread(ThreadFunc, this, THREAD_PRIORITY_LOWEST);
			// 先对FPGA进行复位
			ResetFPGA();
		}
		// 启动接收线程
		//  ShowStatus();
		if (!ComIsOK)
			m_comcontrol.SetWindowText(_T("Connect"));
		else
		{
			m_comcontrol.SetText(_T("Close"));					// 按钮显示状态改变
			m_comcontrol.SetForeColor(RGB(0, 155, 0));			// 串口打开后文本颜色变绿
			m_NetStatusLED.RefreshWindow(TRUE, _T("ON"));		// 设置指示灯
			m_comlist.EnableWindow(false);						// 设置串口号下拉框不可用
			GetDlgItem(IDC_ONE_TRIGGER)->EnableWindow(true);	// 设置单次触发按钮可用
			GetDlgItem(IDC_LOOP_TRIGGER)->EnableWindow(true);	// 设置循环触发按钮可用
			GetDlgItem(IDC_RESET_KERNAL)->EnableWindow(true); // 设置重置按钮可用

			// 日志打印
			CString info = _T("Establish connection!");
			PrintLog(info);
		}
		return;
	}
	else
	{
		// 先对FPGA进行复位
		if (ComIsOK)
		{
			ResetFPGA();
		}

		CloseComm(); // 调用关闭串口函数CloseComm()
		CString info = _T("Disconnect!");
		PrintLog(info);

		// TerminateThread(pReceiveThread,0);
		//  ShowStatus();
		m_comcontrol.SetText(_T("Connect"));
		m_comcontrol.SetForeColor(RGB(255, 0, 0));
		m_NetStatusLED.RefreshWindow(FALSE, _T("OFF")); // 设置指示灯
		m_comlist.EnableWindow(true);					// 设置串口号下拉框可用

		GetDlgItem(IDC_ONE_TRIGGER)->EnableWindow(false);	// 设置单次触发按钮不可用
		GetDlgItem(IDC_LOOP_TRIGGER)->EnableWindow(false);	// 设置循环触发按钮不可用
		GetDlgItem(IDC_RESET_KERNAL)->EnableWindow(false); // 设置重置按钮不可用

		return;
	}
}

void CLEDControlDlg::OnBnClickedOneTrigger()
{
	// 先锁死按键，防止用户误触
	GetDlgItem(IDC_ONE_TRIGGER)->EnableWindow(false);

	// 先刷新各个控制的值
	UpdateData(TRUE);
	// TODO: 在此添加控件通知处理程序代码
	if (!SingleTriggerStatus)
	{
		if (ComIsOK == FALSE)
		{
			MessageBox(_T("请先打开串口"), _T("提示"), MB_ICONINFORMATION);
			return; // return 0;
		}
		else
		{
			GetDlgItem(IDC_ONE_TRIGGER)->SetWindowText(_T("SingleTrigger Stop"));
			// 触发运行中部分控件不可用
			EnableControl(false);
			GetDlgItem(IDC_LOOP_TRIGGER)->EnableWindow(false);

			// 日志打印
			CString info = _T("\r\nSingleTrigger is begin!");
			PrintLog(info);

			//-------------------发送指令------------
			// ①先对FPGA进行复位
			ResetFPGA();
			// ②FPGA初始化
			FPGAInit();
			// ③配置LED发光宽度
			sendLEDwidth();
			// ③配置DAV数据，也就是设置电压
			sendLEDVolt();
			// ④写入DAC数据
			BackSend(Order::WriteData_DAC, 5);
			// ⑤开启电源，并延时指定时长
			info = _T("The power of LED is turn on!");
			PrintLog(info);
			BackSend(Order::CommonVolt_On, 5, config_PowerStableTime); // 开启外设电源需要发送指令后延时，电源上升需要一定时间稳定
			
			// ⑥开启A触发，并开始定时// 日志打印
			info = _T("Group A Trigger is open!");
			PrintLog(info);
			BackSend(Order::TriggerOn_A, 5);
			SetTimer(1, m_CalibrationTime * 1000, NULL); // 设置定时器

			SingleTriggerStatus = TRUE;
		}
	}
	else
	{
		KillTimer(1);
		// 先对FPGA进行复位
		ResetFPGA();
		GetDlgItem(IDC_ONE_TRIGGER)->SetWindowText(_T("SingleTrigger Start"));
		// 恢复使用
		EnableControl(true);
		GetDlgItem(IDC_LOOP_TRIGGER)->EnableWindow(true);

		// 日志打印
		CString info = _T("\r\nSingleTrigger was forcibly stopped by the user!");
		PrintLog(info);
		SingleTriggerStatus = FALSE;
		timer = 0;
	}
	// 恢复使用
	GetDlgItem(IDC_ONE_TRIGGER)->EnableWindow(true);
}

void CLEDControlDlg::OnBnClickedLoopTrigger()
{
	// 先禁用，防止用户多次点击
	GetDlgItem(IDC_LOOP_TRIGGER)->EnableWindow(false);

	// 先刷新各个控制的值
	UpdateData(TRUE);

	// 开始
	if (!LoopTriggerStatus)
	{
		if (ComIsOK == FALSE)
		{
			MessageBox(_T("请先打开串口"), _T("提示"), MB_ICONINFORMATION);
			return;
		}

		// 禁用控件
		EnableControl(false);
		GetDlgItem(IDC_ONE_TRIGGER)->EnableWindow(false);

		// 先读取参数，判断电压预设文件是否正常,不正常则恢复按键使用状态
		if (!ReadLoopFile(VoltFile))
		{
			EnableControl(true);
			GetDlgItem(IDC_ONE_TRIGGER)->EnableWindow(true);
			GetDlgItem(IDC_LOOP_TRIGGER)->EnableWindow(true);
			return;
		}

		// 刷新本控件文本内容
		GetDlgItem(IDC_LOOP_TRIGGER)->SetWindowText(_T("LoopTrigger Stop"));

		// 日志打印
		CString info = _T("\r\nLoopTrigger is Begin!");
		PrintLog(info);

		//--------------------下发指令------------
		// 先对FPGA进行复位
		ResetFPGA();
		// ②FPGA初始化
		FPGAInit();
		// ③配置LED发光宽度
		sendLEDwidth();
		// ③配置DAV数据，也就是设置第一组电压
		sendLEDVolt();
		// ④写入DAC数据
		BackSend(Order::WriteData_DAC, 5);
		// ⑤开启电源，并延时指定时长
		info = _T("The power of LED is turn on!");
		PrintLog(info);
		BackSend(Order::CommonVolt_On, 5, config_PowerStableTime); // 开启外设电源需要发送指令后延时，电源上升需要一定时间稳定
		
		// ⑥开启A触发，并开始定时
		// 日志打印
		info = _T("Group A Trigger is open!");
		PrintLog(info);

		LoopTimeCount = CTime::GetCurrentTime();

		BackSend(Order::TriggerOn_A, 5);
		SetTimer(2, m_CalibrationTime * 1000, NULL); // 设置定时器

		LoopTriggerStatus = TRUE;
	}
	else
	{ 
		// 关闭
		KillTimer(2);

		//打印时长
		CTime tmpTime = CTime::GetCurrentTime();
		CTimeSpan span;
		span = tmpTime - LoopTimeCount;
		long loopTime = span.GetSeconds() + span.GetMinutes()*60 + span.GetHours()*3600 + span.GetDays()*3600*24;
		CString info;
		info.Format(_T("\r\nLoop total time:%lds!"),loopTime);
		PrintLog(info);

		// 先对FPGA进行复位
		ResetFPGA();
		GetDlgItem(IDC_LOOP_TRIGGER)->SetWindowText(_T("LoopTrigger Start"));

		// 恢复控件
		EnableControl(true);
		GetDlgItem(IDC_ONE_TRIGGER)->EnableWindow(true);

		// 日志打印
		info = _T("\r\nLoopTrigger was forcibly stopped by the user!");
		PrintLog(info);

		LoopTriggerStatus = FALSE;
		timer = 0;
	}
	// 恢复本按键使用
	GetDlgItem(IDC_LOOP_TRIGGER)->EnableWindow(true);
}

void CLEDControlDlg::EnableControl(BOOL flag)
{
	GetDlgItem(IDC_CONNECT_SERIALPORT)->EnableWindow(flag);
	GetDlgItem(IDC_VOLT_LOOP_FILE)->EnableWindow(flag);
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
	GetDlgItem(IDC_LED_WIDTH)->EnableWindow(flag);
	GetDlgItem(IDC_LED_DELAY)->EnableWindow(flag);
	GetDlgItem(IDC_CALIBRATION_TIME)->EnableWindow(flag);
}

void CLEDControlDlg::FPGAInit()
{
	sendLEDDelay();
	BackSend(Order::RegisterClockRate, 5);
	BackSend(Order::TriggerHLPointsSet, 5);
	sendShiftRegisterData();
	BackSend(Order::ShiftRegisterOn, 5);
	BackSend(Order::ReferenceVolt_DAC, 5);
}

BOOL CLEDControlDlg::BackSend(BYTE* msg, int msgLength, int sleepTime, int maxWaitingTime, BOOL isShow)
{
	m_statusBar.SetPaneText(0, _T("sending message"));
	CString info;
	DWORD dwBytesWritten = 5;
	dwBytesWritten = (DWORD)msgLength;
	DWORD init =5;
	BOOL bWriteStat;
	DWORD dwErrorFlags;
	COMSTAT ComStat;

	OVERLAPPED m_osWrite;
	memset(&m_osWrite, 0, sizeof(OVERLAPPED));
	m_osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	ClearCommError(hCom, &dwErrorFlags, &ComStat);

	// 初始化反馈相关参数
	TCPfeedback = FALSE;
	LastSendMsg = NULL;
	RecvMsg = NULL;
	recievedFBLength = 0;

	// 若超时未检测到反馈指令，则再次发送指令到FPGA。循环等待三次。
	for (int i = 0; i < 3; i++)
	{
		int times = 0; //等待时长
		CTime tm1;
		tm1 = CTime::GetCurrentTime();
		BOOL flag = FALSE;

		// // 初始化反馈相关参数
		// TCPfeedback = FALSE;
		// LastSendMsg = NULL;
		// RecvMsg = NULL;
		// recievedFBLength = 0;

		// 发送指令
		bWriteStat = WriteFile(hCom, msg, init, &dwBytesWritten, &m_osWrite);
		send_num += init;
		if (!bWriteStat)
		{
			if (GetLastError() == ERROR_IO_PENDING)
			{
				WaitForSingleObject(m_osWrite.hEvent, maxWaitingTime*1000);
			}

			LastSendMsg = msg;
			FeedbackLen = msgLength;
			info.Format(_T("IO_PENDING SEND HEX(%d)(length=%d):"), i+1,dwBytesWritten);
			info = info + Char2HexCString(LastSendMsg, msgLength);
			PrintLog(info, isShow); 
		}
		else
		{
			// 清空缓存区
			PurgeComm(hCom, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);

			LastSendMsg = msg;
			FeedbackLen = msgLength;
			info.Format(_T("SEND HEX(%d)(length=%d):"), i+1,dwBytesWritten);
			info = info + Char2HexCString(LastSendMsg, msgLength);
			PrintLog(info, isShow); 
		}
		// 阻塞式判断等待反馈指令，并进行判断是否与发送指令相同
		do
		{ 
			// 判断接收指令与发送指令是否相同
			if (recievedFBLength == msgLength)
			{
				info = _T("RECV HEX:");
				info += Char2HexCString(RecvMsg, recievedFBLength);
				PrintLog(info, isShow);
				if (compareBYTE(RecvMsg, LastSendMsg, msgLength)){
					TCPfeedback = TRUE;
				}
				if (!TCPfeedback) {
					RecvMsg = NULL;
					recievedFBLength = 0;
				}
			}

			if (TCPfeedback)
			{
				info = _T("指令反馈校验成功:");
				info += Char2HexCString(RecvMsg, recievedFBLength);
				PrintLog(info, isShow);

				//接收到反馈指令，重新初始化反馈相关数据
				TCPfeedback = FALSE;
				
				LastSendMsg = NULL;
				RecvMsg = NULL;
				recievedFBLength = 0;

				//检验成功，再延时
				Sleep(sleepTime);
				m_statusBar.SetPaneText(0, _T("ready"));
				return TRUE;
			}

			// 计算等待时长
			CTime tm2;
			tm2 = CTime::GetCurrentTime();
			CTimeSpan span;
			span = tm2 - tm1;
			times = span.GetSeconds();
		} while (times < maxWaitingTime); 

		info.Format(_T("等待指令反馈时间%ds,超出最大设置时长%ds,"), times, maxWaitingTime);
		PrintLog(info, FALSE);
	}

	info = _T("尝试3次下发指令，均无法接受到反馈指令，SEND HEX: ");
	info = info + Char2HexCString(LastSendMsg, msgLength);
	PrintLog(info, FALSE);

	// 恢复指令反馈相关参数
	TCPfeedback = FALSE;
	LastSendMsg = NULL;
	RecvMsg = NULL;
	recievedFBLength = 0;

	Sleep(sleepTime);
	m_statusBar.SetPaneText(0, _T("ready"));
	return FALSE;
}

void CLEDControlDlg::sendLEDwidth()
{
	Order::LEDWidth[3] = m_tempLEDWidth;
	BackSend(Order::LEDWidth, 5);
  // 日志打印
	CString info;
	info.Format(_T("LEDwidth:%dns"), m_tempLEDWidth*10);
	PrintLog(info);
}

void CLEDControlDlg::sendLEDDelay()
{
	Order::LEDDelay1[2] = (m_LEDDelay >> 24) & 0xFF;
	Order::LEDDelay1[3] = (m_LEDDelay >> 16) & 0xFF;
	Order::LEDDelay2[2] = (m_LEDDelay >> 8)  & 0xFF;
	Order::LEDDelay2[3] = m_LEDDelay & 0xFF;
	BackSend(Order::LEDDelay1, 5);
	BackSend(Order::LEDDelay2, 5);
}

void CLEDControlDlg::sendShiftRegisterData()
{
	Order::LEDSwitch[2] = m_LEDSwitchB;
	Order::LEDSwitch[3] = m_LEDSwitchA;
	BackSend(Order::LEDSwitch, 5);
}

void CLEDControlDlg::sendLEDVolt()
{
	// 需要对电压到DAC数值的刻度曲线
	// DAC = P1*Volt + P2
	int DAC_A = (int)(m_tempVoltA * config_p1_A + config_p2_A);
	int DAC_B = (int)(m_tempVoltB * config_p1_B + config_p2_B);

	Order::VoltA_LED[2] = (DAC_A >> 8)  & 0xFF;
	Order::VoltA_LED[3] = DAC_A & 0xFF;
	BackSend(Order::VoltA_LED, 5);

	Order::VoltB_LED[2] = (DAC_B >> 8)  & 0xFF;
	Order::VoltB_LED[3] = DAC_B & 0xFF;
	BackSend(Order::VoltB_LED, 5);

	// 日志打印
	CString info;
	info.Format(_T("Group A Volt:%dmV; Group B Volt:%dmV"), m_tempVoltA, m_tempVoltB);
	PrintLog(info);
}

// 刷新所有LED勾选框数值
// state:二进制码，
// status:勾选状态，1为勾选，0为不勾选
// LEDID，0为A组灯；1为B组灯
void CLEDControlDlg::UpdateCheckValue(const BYTE stateBit, BOOL status, int LEDID)
{
	// A灯组
	if (LEDID == 0)
	{
		if (status)
		{
			m_LEDSwitchA = m_LEDSwitchA | stateBit; // 勾选做或运算
		}
		else
		{
			m_LEDSwitchA = m_LEDSwitchA & stateBit; // 取消勾选做与运算
		}
	}

	// B灯组
	if (LEDID == 1)
	{
		if (status)
		{
			m_LEDSwitchB = m_LEDSwitchB | stateBit;
		}
		else
		{
			m_LEDSwitchB = m_LEDSwitchB & stateBit;
		}
	}
	UpdateLEDCheck();
}

void CLEDControlDlg::UpdateLEDCheck()
{
	// A组LED
	((CButton *)GetDlgItem(IDC_CHECKA1))->SetCheck(m_LEDSwitchA & 0b00000001);
	((CButton *)GetDlgItem(IDC_CHECKA2))->SetCheck(m_LEDSwitchA & 0b00000010);
	((CButton *)GetDlgItem(IDC_CHECKA3))->SetCheck(m_LEDSwitchA & 0b00000100);
	((CButton *)GetDlgItem(IDC_CHECKA4))->SetCheck(m_LEDSwitchA & 0b00001000);
	((CButton *)GetDlgItem(IDC_CHECKA5))->SetCheck(m_LEDSwitchA & 0b00100000);
	((CButton *)GetDlgItem(IDC_CHECKA6))->SetCheck(m_LEDSwitchA & 0b01000000);
	((CButton *)GetDlgItem(IDC_CHECKA7))->SetCheck(m_LEDSwitchA & 0b10000000);
	((CButton *)GetDlgItem(IDC_CHECKALL_A))->SetCheck(m_LEDSwitchA == 0b11101111);

	// B组LED
	((CButton *)GetDlgItem(IDC_CHECKB1))->SetCheck(m_LEDSwitchB & 0b00000001);
	((CButton *)GetDlgItem(IDC_CHECKB2))->SetCheck(m_LEDSwitchB & 0b00000010);
	((CButton *)GetDlgItem(IDC_CHECKB3))->SetCheck(m_LEDSwitchB & 0b00000100);
	((CButton *)GetDlgItem(IDC_CHECKB4))->SetCheck(m_LEDSwitchB & 0b00001000);
	((CButton *)GetDlgItem(IDC_CHECKB5))->SetCheck(m_LEDSwitchB & 0b00100000);
	((CButton *)GetDlgItem(IDC_CHECKB6))->SetCheck(m_LEDSwitchB & 0b01000000);
	((CButton *)GetDlgItem(IDC_CHECKB7))->SetCheck(m_LEDSwitchB & 0b10000000);
	((CButton *)GetDlgItem(IDC_CHECKALL_B))->SetCheck(m_LEDSwitchB == 0b11101111);
}

void CLEDControlDlg::ChoseVoltLoopFile()
{
	// TODO: 在此添加控件通知处理程序代码
	ChooseFile(VoltFile);
	GetDlgItem(IDC_EDIT1)->SetWindowText(VoltFile);
	ReadLoopFile(VoltFile);
}

void CLEDControlDlg::OnBnClickedCheckA1()
{
	BYTE LEDBit = 0b00000001;
	BOOL status = 1;
	int state = ((CButton*)GetDlgItem(IDC_CHECKA1))->GetCheck();
	if (state != 1)
	{
		LEDBit = 0b11101111 & (~LEDBit); // 非勾选状态
		status = 0;
	}
	UpdateCheckValue(LEDBit, status, 0);
}

void CLEDControlDlg::OnBnClickedCheckA2()
{
	BYTE LEDBit = 0b00000010;
	BOOL status = 1;
	int state = ((CButton*)GetDlgItem(IDC_CHECKA2))->GetCheck();
	if (state != 1)
	{
		LEDBit = 0b11101111 & (~LEDBit); // 非勾选状态
		status = 0;
	}
	UpdateCheckValue(LEDBit, status, 0);
}

void CLEDControlDlg::OnBnClickedCheckA3()
{
	BYTE LEDBit = 0b00000100;
	BOOL status = 1;
	int state = ((CButton*)GetDlgItem(IDC_CHECKA3))->GetCheck();
	if (state != 1)
	{
		LEDBit = 0b11101111 & (~LEDBit); // 非勾选状态
		status = 0;
	}
	UpdateCheckValue(LEDBit, status, 0);
}

void CLEDControlDlg::OnBnClickedCheckA4()
{
	BYTE LEDBit = 0b00001000;
	BOOL status = 1;
	int state = ((CButton*)GetDlgItem(IDC_CHECKA4))->GetCheck();
	if (state != 1)
	{
		LEDBit = 0b11101111 & (~LEDBit); // 非勾选状态
		status = 0;
	}
	UpdateCheckValue(LEDBit, status, 0);
}

void CLEDControlDlg::OnBnClickedCheckA5()
{
	BYTE LEDBit = 0b00100000;
	BOOL status = 1;
	int state = ((CButton*)GetDlgItem(IDC_CHECKA5))->GetCheck();
	if (state != 1)
	{
		LEDBit = 0b11101111 & (~LEDBit); // 非勾选状态
		status = 0;
	}
	UpdateCheckValue(LEDBit, status, 0);
}

void CLEDControlDlg::OnBnClickedCheckA6()
{
	BYTE LEDBit = 0b01000000;
	BOOL status = 1;
	int state = ((CButton*)GetDlgItem(IDC_CHECKA6))->GetCheck();
	if (state != 1)
	{
		LEDBit = 0b11101111 & (~LEDBit); // 非勾选状态
		status = 0;
	}
	UpdateCheckValue(LEDBit, status, 0);
}

void CLEDControlDlg::OnBnClickedCheckA7()
{
	BYTE LEDBit = 0b10000000;
	BOOL status = 1;
	int state = ((CButton*)GetDlgItem(IDC_CHECKA7))->GetCheck();
	if (state != 1)
	{
		LEDBit = 0b11101111 & (~LEDBit); // 非勾选状态
		status = 0;
	}
	UpdateCheckValue(LEDBit, status, 0);
}

void CLEDControlDlg::OnBnClickedCheckALL_A()
{
	BYTE LEDBit = 0b11101111;
	BOOL status = 1;
	int state = ((CButton*)GetDlgItem(IDC_CHECKALL_A))->GetCheck();
	if (state != 1)
	{
		LEDBit = 0b11101111 & (~LEDBit); // 非勾选状态
		status = 0;
	}
	UpdateCheckValue(LEDBit, status, 0);
}

void CLEDControlDlg::OnBnClickedCheckB1()
{
	BYTE LEDBit = 0b00000001;
	BOOL status = 1;
	int state = ((CButton*)GetDlgItem(IDC_CHECKB1))->GetCheck();
	if (state != 1)
	{
		LEDBit = 0b11101111 & (~LEDBit); // 非勾选状态
		status = 0;
	}
	UpdateCheckValue(LEDBit, status, 1);
}

void CLEDControlDlg::OnBnClickedCheckB2()
{
	BYTE LEDBit = 0b00000010;
	BOOL status = 1;
	int state = ((CButton*)GetDlgItem(IDC_CHECKB2))->GetCheck();
	if (state != 1)
	{
		LEDBit = 0b11101111 & (~LEDBit); // 非勾选状态
		status = 0;
	}
	UpdateCheckValue(LEDBit, status, 1);
}

void CLEDControlDlg::OnBnClickedCheckB3()
{
	BYTE LEDBit = 0b00000100;
	BOOL status = 1;
	int state = ((CButton*)GetDlgItem(IDC_CHECKB3))->GetCheck();
	if (state != 1)
	{
		LEDBit = 0b11101111 & (~LEDBit); // 非勾选状态
		status = 0;
	}
	UpdateCheckValue(LEDBit, status, 1);
}

void CLEDControlDlg::OnBnClickedCheckB4()
{
	BYTE LEDBit = 0b00001000;
	BOOL status = 1;
	int state = ((CButton*)GetDlgItem(IDC_CHECKB4))->GetCheck();
	if (state != 1)
	{
		LEDBit = 0b11101111 & (~LEDBit); // 非勾选状态
		status = 0;
	}
	UpdateCheckValue(LEDBit, status, 1);
}

void CLEDControlDlg::OnBnClickedCheckB5()
{
	BYTE LEDBit = 0b00100000;
	BOOL status = 1;
	int state = ((CButton*)GetDlgItem(IDC_CHECKB5))->GetCheck();
	if (state != 1)
	{
		LEDBit = 0b11101111 & (~LEDBit); // 非勾选状态
		status = 0;
	}
	UpdateCheckValue(LEDBit, status, 1);
}

void CLEDControlDlg::OnBnClickedCheckB6()
{
	BYTE LEDBit = 0b01000000;
	BOOL status = 1;
	int state = ((CButton*)GetDlgItem(IDC_CHECKB6))->GetCheck();
	if (state != 1)
	{
		LEDBit = 0b11101111 & (~LEDBit); // 非勾选状态
		status = 0;
	}
	UpdateCheckValue(LEDBit, status, 1);
}

void CLEDControlDlg::OnBnClickedCheckB7()
{
	BYTE LEDBit = 0b10000000;
	BOOL status = 1;
	int state = ((CButton*)GetDlgItem(IDC_CHECKB7))->GetCheck();
	if (state != 1)
	{
		LEDBit = 0b11101111 & (~LEDBit); // 非勾选状态
		status = 0;
	}
	UpdateCheckValue(LEDBit, status, 1);
}

void CLEDControlDlg::OnBnClickedCheckALL_B()
{
	BYTE LEDBit = 0b11101111;
	BOOL status = 1;
	int state = ((CButton*)GetDlgItem(IDC_CHECKALL_B))->GetCheck();
	if (state != 1)
	{
		LEDBit = 0b11101111 & (~LEDBit); // 非勾选状态
		status = 0;
	}
	UpdateCheckValue(LEDBit, status, 1);
}

void CLEDControlDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	switch (nIDEvent)
	{
	case 1:
	{
		timer++;
		if (timer % 3 == 1)
		{
			// 日志打印
			CString info = _T("Group A Trigger is close!");
			PrintLog(info);
			KillTimer(1);
			BackSend(Order::TriggerOff, 5);	 // 停止触发
			
			info = _T("Group B Trigger is open!");
			PrintLog(info);
			BackSend(Order::TriggerOn_B, 5); // B组触发
			SetTimer(1, m_CalibrationTime * 1000, NULL); // 设置定时器			
		}
		if (timer % 3 == 2)
		{
			// 日志打印
			CString info = _T("Group B Trigger is close!");
			PrintLog(info);
			KillTimer(1);
			BackSend(Order::TriggerOff, 5);	  // 停止触发

			info = _T("Group AB Trigger is open!");
			PrintLog(info);
			BackSend(Order::TriggerOn_AB, 5); // AB组触发
			SetTimer(1, m_CalibrationTime * 1000, NULL); // 设置定时器
		}
		if (timer % 3 == 0)
		{
			CString info = _T("Group AB Trigger is close!");
			PrintLog(info);
			KillTimer(1);
			BackSend(Order::TriggerOff, 5); // 停止触发

			BackSend(Order::CommonVolt_Off, 5); // 关闭外设电源
			info = _T("The power of LED is turn off!");
			PrintLog(info);

			BackSend(Order::DAC_Off, 5); // 关闭DAC配置

			BackSend(Order::Reset, 5); // 复位

			// 日志打印
			info = _T("SingleTrigger is Finished!\r\n");
			PrintLog(info);

			// 重置部分参数
			SingleTriggerStatus = FALSE;
			timer = 0;
			// 恢复部分控件可用
			EnableControl(TRUE);
			GetDlgItem(IDC_LOOP_TRIGGER)->EnableWindow(true);
			GetDlgItem(IDC_ONE_TRIGGER)->SetWindowText(_T("SingleTrigger Start"));
		}
	}
	break;
	case 2:
	{
		timer++;
		// 提取当前属于第几组电压值
		int VoltID = timer / 3;
		// 判断当前属于内循环哪一个状态
		if (timer % 3 == 1)
		{
			// 日志打印
			CString info = _T("Group A Trigger is close!");
			PrintLog(info);
			KillTimer(2);
			BackSend(Order::TriggerOff, 5);	 // 停止触发
			
			info = _T("Group B Trigger is open!");
			PrintLog(info);
			BackSend(Order::TriggerOn_B, 5); // B组触发
			SetTimer(2, m_CalibrationTime * 1000, NULL); // 设置定时器
		}
		if (timer % 3 == 2)
		{
			// 日志打印
			CString info = _T("Group B Trigger is close!");
			PrintLog(info);
			KillTimer(2);
			BackSend(Order::TriggerOff, 5);	  // 停止触发

			info = _T("Group AB Trigger is open!");
			PrintLog(info);
			BackSend(Order::TriggerOn_AB, 5); // AB组触发	
			SetTimer(2, m_CalibrationTime * 1000, NULL); // 设置定时器		
		}
		if (timer % 3 == 0)
		{
			// 结束所有循环
			if (VoltID == vec_VoltA.size())
			{
				//-------------------结束最后一次触发------------
				//计算运行时长
				CTime tmpTime = CTime::GetCurrentTime();
				CTimeSpan span;
				span = tmpTime - LoopTimeCount;
				long loopTime = span.GetSeconds() + span.GetMinutes()*60 + span.GetHours()*3600 + span.GetDays()*3600*24;

				// 日志打印
				CString info = _T("Group AB Trigger is close!");
				PrintLog(info);
				KillTimer(2);
				
				//打印时长
				info.Format(_T("\r\nLoop total time:%lds!"),loopTime);
				PrintLog(info);
				BackSend(Order::TriggerOff, 5); // 停止触发

				BackSend(Order::CommonVolt_Off, 5); // 关闭外设电源
				info = _T("The power of LED is turn off!");
				PrintLog(info);

				BackSend(Order::DAC_Off, 5); // 关闭DAC配置
				BackSend(Order::Reset, 5);	 // 复位

				// 重置部分参数
				timer = 0;
				LoopTriggerStatus = FALSE;
				vec_VoltA.resize(0);
				vec_VoltB.resize(0);
				EnableControl(true);
				GetDlgItem(IDC_ONE_TRIGGER)->EnableWindow(true);
				GetDlgItem(IDC_LOOP_TRIGGER)->SetWindowText(_T("LoopTrigger Start"));
				info = _T("LoopTrigger is Finished!\r\n");
				PrintLog(info);
			}
			else
			{
				// （1）结束上一次的内循环
				// 日志打印
				CString info = _T("Group AB Trigger is close!");
				PrintLog(info);
				KillTimer(2);
				BackSend(Order::TriggerOff, 5); // 停止触发

				BackSend(Order::CommonVolt_Off, 5); // 关闭外设电源
				info = _T("The power of LED is turn off!");
				PrintLog(info);

				BackSend(Order::DAC_Off, 5); // 关闭DAC配置

				// （2）-①更新电压，开始下一次内循环触发
				m_tempVoltA = vec_VoltA[VoltID]; // 界面当前电压值更新
				m_tempVoltB = vec_VoltB[VoltID]; // 界面当前电压值更新
				m_tempLEDWidth = vec_TimeWidth[VoltID]; //界面当前时间宽度更新
				sendLEDwidth(); 
				sendLEDVolt();
				BackSend(Order::WriteData_DAC, 5);
				info = _T("The power of LED is turn on!");
				PrintLog(info);
				BackSend(Order::CommonVolt_On, 5, config_PowerStableTime); // 这里需要消耗一定时长，不能在定时器内。

				// （3）重新开始定时器
				// 日志打印
				info = _T("Group A Trigger is open!");
				PrintLog(info);
				BackSend(Order::TriggerOn_A, 5);
				SetTimer(2, m_CalibrationTime * 1000, NULL);

				// （4）-②刷新界面控件的内容
				UpdateData(FALSE);
        		m_LogEdit.LineScroll(m_LogEdit.GetLineCount()); // 每次刷新后都显示最底部

				// （5）更新json中的"tempVoltA"，"tempVoltB"
				Json::Value jsonSetting = ReadSetting(_T("Setting.json"));
				if (!jsonSetting.isNull())
				{
					jsonSetting["tempVoltA"] = m_tempVoltA;
					jsonSetting["tempVoltB"] = m_tempVoltB;
					jsonSetting["LEDWidth"] = m_tempLEDWidth;
					WriteSetting(_T("Setting.json"), jsonSetting);
				}
			}
		}
	}
	break;
	case 3:
		CTime t = CTime::GetCurrentTime();
		CString strInfo = t.Format(_T("%Y-%m-%d %H:%M:%S"));
		m_statusBar.SetPaneText(1, strInfo);
	}
	CDialogEx::OnTimer(nIDEvent);
}

void CLEDControlDlg::OnClose()
{
	KillTimer(3);

	if (ComIsOK)
	{
		ResetFPGA(); // 先对FPGA进行复位
		CloseComm(); // 调用关闭串口函数CloseComm()
	}

	// 保留界面的参数
	Json::Value jsonSetting = ReadSetting(_T("Setting.json"));
	if (!jsonSetting.isNull())
	{
		jsonSetting["LEDSwitchA"] = m_LEDSwitchA;
		jsonSetting["LEDSwitchB"] = m_LEDSwitchB;
		jsonSetting["tempVoltA"] = m_tempVoltA;
		jsonSetting["tempVoltB"] = m_tempVoltB;
		jsonSetting["CalibrationTime"] = m_CalibrationTime;
		jsonSetting["LEDDelay"] = m_LEDDelay;
		jsonSetting["tempLEDWidth"] = m_tempLEDWidth;
		WriteSetting(_T("Setting.json"), jsonSetting);
	}

	// TODO: 在此添加消息处理程序代码和/或调用默认值
	TerminateThread(pReceiveThread, 0); // 程序退出时,关闭串口监听线程
	WaitForSingleObject(pReceiveThread, INFINITE);
	CDialogEx::OnClose();
}

void CLEDControlDlg::PrintLog(CString info, BOOL isShow)
{
	// 添加日志到文件
	CLog::SetPrefix(_T("LED"));
	CLog::WriteMsg(info);

	if (!isShow) return;

	// 添加日志到界面
	CTime t = CTime::GetCurrentTime();
	CString strTime = t.Format(_T("[%Y-%m-%d %H:%M:%S]# "));
	m_strLog = m_strLog + strTime + info + _T("\r\n");

	UpdateData(FALSE);
	m_LogEdit.LineScroll(m_LogEdit.GetLineCount()); // 每次刷新后都显示最底部
}

void CLEDControlDlg::OnEnKillfocusVoltA()
{
	UpdateData(TRUE);

	if ((m_tempVoltA < config_minV) || (m_tempVoltA > config_maxV))
	{
		// 注意这里要先刷新值，再弹出框提醒用户，因为编辑框响应“光标移除”与“enter"键两种信号，后刷新值会导致重复触发本函数
		if (m_tempVoltA > config_maxV)
		{
			m_tempVoltA = config_maxV;
		}
		else
		{
			m_tempVoltA = config_minV;
		}
		UpdateData(FALSE);
		m_LogEdit.LineScroll(m_LogEdit.GetLineCount()); // 每次刷新后都显示最底部

		CString message;
		message.Format(_T("The range of Volt GroupA is %d~%dmV\n"), config_minV, config_maxV);
		MessageBox(message);
	}
}

void CLEDControlDlg::OnEnKillfocusVoltB()
{
	UpdateData(TRUE);

	if ((m_tempVoltB < config_minV) || (m_tempVoltB > config_maxV))
	{
		// 注意这里要先刷新值，再弹出框提醒用户，因为编辑框响应“光标移除”与“enter"键两种信号，后刷新值会导致重复触发本函数
		if (m_tempVoltB > config_maxV)
		{
			m_tempVoltB = config_maxV;
		}
		else
		{
			m_tempVoltB = config_minV;
		}
		UpdateData(FALSE);
		m_LogEdit.LineScroll(m_LogEdit.GetLineCount()); // 每次刷新后都显示最底部

		CString message;
		message.Format(_T("The range of Volt GroupB is %d~%dmV\n"), config_minV, config_maxV);
		MessageBox(message);
	}
}

void CLEDControlDlg::OnEnKillfocusLEDWidth()
{
	UpdateData(TRUE);
	int minValue = 1;
	int maxValue = 255;
	if ((m_tempLEDWidth < minValue) || (m_tempLEDWidth > maxValue))
	{
		// 注意这里要先刷新值，再弹出框提醒用户，因为编辑框响应“光标移除”与“enter"键两种信号，后刷新值会导致重复触发本函数
		if (m_tempLEDWidth > maxValue)
		{
			m_tempLEDWidth = maxValue;
		}
		else
		{
			m_tempLEDWidth = minValue;
		}
		UpdateData(FALSE);
		m_LogEdit.LineScroll(m_LogEdit.GetLineCount()); // 每次刷新后都显示最底部

		CString message;
		message.Format(_T("The range of LEDWidth is %d~%dns\n"), minValue * 10, maxValue * 10);
		MessageBox(message);
	}
}

void CLEDControlDlg::OnEnKillfocusLEDDelay()
{
	UpdateData(TRUE);
	int minValue = 10;
	int maxValue = 1000000;
	if ((m_LEDDelay < minValue) || (m_LEDDelay > maxValue))
	{
		// 注意这里要先刷新值，再弹出框提醒用户，因为编辑框响应“光标移除”与“enter"键两种信号，后刷新值会导致重复触发本函数
		if (m_LEDDelay > maxValue)
		{
			m_LEDDelay = maxValue;
		}
		else
		{
			m_LEDDelay = minValue;
		}
		UpdateData(FALSE);
		m_LogEdit.LineScroll(m_LogEdit.GetLineCount()); // 每次刷新后都显示最底部

		CString message;
		message.Format(_T("The range of LEDDelay is %d~%dns\n"), minValue*10, maxValue*10);
		MessageBox(message);
	}
}

void CLEDControlDlg::OnEnKillfocusCalibrationTime()
{
	UpdateData(TRUE);
	int minValue = 1;
	int maxValue = 255;
	if ((m_CalibrationTime < minValue) || (m_CalibrationTime > maxValue))
	{
		// 注意这里要先刷新值，再弹出框提醒用户，因为编辑框响应“光标移除”与“enter"键两种信号，后刷新值会导致重复触发本函数
		if (m_CalibrationTime > maxValue)
		{
			m_CalibrationTime = maxValue;
		}
		else
		{
			m_CalibrationTime = minValue;
		}
		UpdateData(FALSE);
		m_LogEdit.LineScroll(m_LogEdit.GetLineCount()); // 每次刷新后都显示最底部

		CString message;
		message.Format(_T("The range of CalibrationTime is %d~%ds\n"), minValue, maxValue);
		MessageBox(message);
	}
}

void CLEDControlDlg::ResetFPGA()
{
	// TODO: 在此添加控件通知处理程序代码
	BackSend(Order::TriggerOff, 5);		// 停止触发
	BackSend(Order::CommonVolt_Off, 5); // 关闭外设电源
	BackSend(Order::DAC_Off, 5);		// 关闭DAC配置
	BackSend(Order::Reset, 5);			// 复位
}

void CLEDControlDlg::ResetSystem()
{
	if (SingleTriggerStatus)
		KillTimer(1);
	if (LoopTriggerStatus)
		KillTimer(2);
	ResetFPGA();

	// 重新读取配置参数
	InitConfigSetting();

	// 界面恢复初始
	GetDlgItem(IDC_ONE_TRIGGER)->SetWindowText(_T("SingleTrigger Start"));
	GetDlgItem(IDC_LOOP_TRIGGER)->SetWindowText(_T("LoopTrigger Start"));

	// 恢复控件使用
	EnableControl(true);
	GetDlgItem(IDC_ONE_TRIGGER)->EnableWindow(true);
	GetDlgItem(IDC_LOOP_TRIGGER)->EnableWindow(true);

	// 恢复运行状态标志位
	SingleTriggerStatus = FALSE;
	LoopTriggerStatus = FALSE;
	timer = 0;

	CString info = _T("The Trigger is close!");
	PrintLog(info);
	info = _T("The power of LED is turn off!");
	PrintLog(info);
}

void CLEDControlDlg::OnBnClickedClearLog()
{
	// TODO: 在此添加控件通知处理程序代码
	m_strLog = _T("");
	UpdateData(FALSE);
}
