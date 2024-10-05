#include "pch.h"
#include "stdafx.h" 
#include "LEDControl.h" 
#include "LEDControlDlg.h"
#include "comm.h"
#include "Log.h"

char ConvertHexChar(char ch); 
HANDLE hCom; //串口句柄 
CString strcomname; //串口名,如"COM1" 
bool ComIsOK; //串口打开状态标识,为真表示已打开,否则未打开 
bool SingleTriggerStatus; // 是否处于SingleTrigger工作状态，为真表示工作中，
bool LoopTriggerStatus;   // 是否处于LoopTrigger工作状态，为真表示工作中，

//============自动寻找串口函数================================= //
//函数功能:通过扫描注册表来找出当前所有物理串口
//输入参数:无 
//返回类型:无 
//说    明:若搜索成功,则每搜到一个串口便发送消息通知主对话框,并将串口号以WPARAM传递 
void FindComm() 
{ 
	//枚举当前系统中的串口 
	LONG result = 0; 
	HKEY key = NULL; 

	result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, //需要打开的主键的名称              
		_T("HARDWARE\\DEVICEMAP\\SERIALCOMM"),
		//需要打开的子键的名称,设备串口  
		0, //保留，必须设置为0  
		KEY_READ, //安全访问标记，也就是权限  
		&key); //得到的将要打开键的句柄，当不再需要句柄， 
			   //必须调用 RegCloseKey 关闭它 
	if( result ) 
	{ 
		AfxMessageBox(_T("无法获取串口，请确认是否安装并连接串口!"));
		return; 
	} 
	TCHAR portname[250]; //串口名 
	TCHAR data[250]; 
	DWORD portnamelen = 0; //串口名长度 
	DWORD datalen = 0; 
	int index = 0; 
	while(1) //找完COM后跳出 
	{ 
		portnamelen = 255; 
		datalen = 255; 
		result = RegEnumValue(key, 
			//Long，一个已打开项的句柄，或者指定一个标准项名               
			index++, 
			//Long，欲获取值的索引。注意第一个值的索引编号为零   
			portname, 
			//String，用于装载位于指定索引处值名的一个缓冲区   
			&portnamelen, 
			//Long，用于装载lpValueName缓冲区长度的一个变量。
			//一旦返回，它会设为实际载入缓冲区的字符数量   
			NULL, 
			//Long，未用；设为零   
			NULL, 
			//Long，用于装载值的类型代码的变量   
			(LPBYTE)data, //Byte，用于装载值数据的一个缓冲区 
			&datalen); //Long，用于装载lpData缓冲区长度的一个变量。
		   //一旦返回，它会设为实际载入缓冲区的字符数量 
		if( result )  
			break;

		   //发送消息,WM_USER+1为自定义消息,即找到串口的,并将串口号"COMx"通过WPARA M参数传送给主对话框窗口 
		   //::AfxGetMainWnd()->m_hWnd,获得主对话框句柄 
		   //(WPARAM)(LPCTSTR)data,类型转换 
		::SendMessage(::AfxGetMainWnd()->m_hWnd,WM_FOUNDCOMM,(WPARAM)(LPCTSTR)data,0); 
	} 
	RegCloseKey(key); //调用 RegCloseKey 关闭打开键的句柄 
} 
//==========串口打开函数=========================== 
//功    能:打开串口,将已打开的串口句柄赋值给hCom,给出串口打开状态ComIsOK,完成串口状态 设置 
//输入参数:波特率,数据位,停止位,校验位 
//返回类型:无 

void OpenComm(int nBaud, int nData, int nStop, int nCal) {
	hCom = CreateFile(strcomname, //串口号 
		GENERIC_READ | GENERIC_WRITE, //允许读或写 
		0, //独占方式 
		NULL,
		OPEN_EXISTING, //打开而不是创建 
		0,//同步通讯方式，也就是阻塞式通信
		//FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,//重叠方式,用于异步通信
		NULL );
		if (hCom == INVALID_HANDLE_VALUE) 
		{ 
			AfxMessageBox(_T("打开COM失败，串口不存在或已被占用!"));
			ComIsOK = false; return; 
		} 
		ComIsOK = true; 
		SetCommMask(hCom, EV_TXEMPTY | EV_RXCHAR); //设置事件掩码,暂时没用上 
		SetupComm(hCom,1024,1024); //设置输入缓冲区和输出缓冲区的大小都是1024
		COMMTIMEOUTS TimeOuts; //设定读超时 
		TimeOuts.ReadIntervalTimeout = MAXDWORD; 
		TimeOuts.ReadTotalTimeoutConstant = 0; 
		TimeOuts.ReadTotalTimeoutMultiplier = 0; //设定写超时 
		TimeOuts.WriteTotalTimeoutConstant = 500; 
		TimeOuts.WriteTotalTimeoutMultiplier = 100; 
		if(SetCommTimeouts(hCom,&TimeOuts) == false) 
		{ 
			CloseHandle(hCom); 
			ComIsOK = false; return; 
		} //串口属性配置 
		DCB dcb; 
		GetCommState(hCom,&dcb); 
		dcb.BaudRate=nBaud; //dcb.BaudRate=9600; //波特率为9600 
		dcb.ByteSize=nData; //dcb.ByteSize=8; //每个字节为8位 
		dcb.StopBits=nStop; //dcb.StopBits=ONESTOPBIT;   //1位停止位 
		dcb.Parity=nCal; //dcb.Parity=NOPARITY; //无奇偶检验位 
		SetCommState(hCom, &dcb); 
		PurgeComm(hCom, PURGE_TXCLEAR | PURGE_RXCLEAR); 
		if (SetCommState(hCom, &dcb) == false) 
		{ 
			CloseHandle(hCom); 
			ComIsOK = false; 
			return; 
		} 
		return;
}

//==========串口关闭控制函数===================== 
void CloseComm() 
{ 
	CloseHandle(hCom); 
	hCom = NULL; 
	ComIsOK = false; 
} 


//==========串口监听线程函数====================== 
UINT ThreadFunc(LPVOID pParam) 
{ 
	CLEDControlDlg* pdlg = (CLEDControlDlg*)pParam; //定义指针指向主对话框 
	COMSTAT ComStat; 
	DWORD dwErrorFlags; 
	while(ComIsOK) 
	{ 
		DWORD dwBytesRead = 100; 
		ClearCommError(hCom,&dwErrorFlags,&ComStat); 
		dwBytesRead = min(dwBytesRead,(DWORD)ComStat.cbInQue); 
		pdlg->cache_num += ComStat.cbInQue;
		if(!dwBytesRead) 
		{ 
			Sleep(3);//continue;//使用continue时，打开串口后CPU占用率非常高 
		} 
		else 
		{
			// ::SendMessage(::AfxGetMainWnd()->m_hWnd,WM_READCOMM,1,0); //发送消息,已读到 ，采用排队消息处理方式
			pdlg->ReadComm(); //多线程处理，不占用界面线程资源
		}
	} 
	return 0; 
}

char ConvertHexChar(char ch)
{
	//将一个字符转换为相应的十六进制 
	if ((ch >= '0') && (ch <= '9'))
		return ch - 48;//0x30; 
	else if ((ch >= 'A') && (ch <= 'F'))
		return ch - 'A' + 10;
	else if ((ch >= 'a') && (ch <= 'f'))
		return ch - 'a' + 10;
	else return (-1);
}
