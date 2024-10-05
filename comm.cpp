#include "pch.h"
#include "stdafx.h" 
#include "LEDControl.h" 
#include "LEDControlDlg.h"
#include "comm.h"
#include "Log.h"

char ConvertHexChar(char ch); 
HANDLE hCom; //���ھ�� 
CString strcomname; //������,��"COM1" 
bool ComIsOK; //���ڴ�״̬��ʶ,Ϊ���ʾ�Ѵ�,����δ�� 
bool SingleTriggerStatus; // �Ƿ���SingleTrigger����״̬��Ϊ���ʾ�����У�
bool LoopTriggerStatus;   // �Ƿ���LoopTrigger����״̬��Ϊ���ʾ�����У�

//============�Զ�Ѱ�Ҵ��ں���================================= //
//��������:ͨ��ɨ��ע������ҳ���ǰ����������
//�������:�� 
//��������:�� 
//˵    ��:�������ɹ�,��ÿ�ѵ�һ�����ڱ㷢����Ϣ֪ͨ���Ի���,�������ں���WPARAM���� 
void FindComm() 
{ 
	//ö�ٵ�ǰϵͳ�еĴ��� 
	LONG result = 0; 
	HKEY key = NULL; 

	result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, //��Ҫ�򿪵�����������              
		_T("HARDWARE\\DEVICEMAP\\SERIALCOMM"),
		//��Ҫ�򿪵��Ӽ�������,�豸����  
		0, //��������������Ϊ0  
		KEY_READ, //��ȫ���ʱ�ǣ�Ҳ����Ȩ��  
		&key); //�õ��Ľ�Ҫ�򿪼��ľ������������Ҫ����� 
			   //������� RegCloseKey �ر��� 
	if( result ) 
	{ 
		AfxMessageBox(_T("�޷���ȡ���ڣ���ȷ���Ƿ�װ�����Ӵ���!"));
		return; 
	} 
	TCHAR portname[250]; //������ 
	TCHAR data[250]; 
	DWORD portnamelen = 0; //���������� 
	DWORD datalen = 0; 
	int index = 0; 
	while(1) //����COM������ 
	{ 
		portnamelen = 255; 
		datalen = 255; 
		result = RegEnumValue(key, 
			//Long��һ���Ѵ���ľ��������ָ��һ����׼����               
			index++, 
			//Long������ȡֵ��������ע���һ��ֵ���������Ϊ��   
			portname, 
			//String������װ��λ��ָ��������ֵ����һ��������   
			&portnamelen, 
			//Long������װ��lpValueName���������ȵ�һ��������
			//һ�����أ�������Ϊʵ�����뻺�������ַ�����   
			NULL, 
			//Long��δ�ã���Ϊ��   
			NULL, 
			//Long������װ��ֵ�����ʹ���ı���   
			(LPBYTE)data, //Byte������װ��ֵ���ݵ�һ�������� 
			&datalen); //Long������װ��lpData���������ȵ�һ��������
		   //һ�����أ�������Ϊʵ�����뻺�������ַ����� 
		if( result )  
			break;

		   //������Ϣ,WM_USER+1Ϊ�Զ�����Ϣ,���ҵ����ڵ�,�������ں�"COMx"ͨ��WPARA M�������͸����Ի��򴰿� 
		   //::AfxGetMainWnd()->m_hWnd,������Ի����� 
		   //(WPARAM)(LPCTSTR)data,����ת�� 
		::SendMessage(::AfxGetMainWnd()->m_hWnd,WM_FOUNDCOMM,(WPARAM)(LPCTSTR)data,0); 
	} 
	RegCloseKey(key); //���� RegCloseKey �رմ򿪼��ľ�� 
} 
//==========���ڴ򿪺���=========================== 
//��    ��:�򿪴���,���Ѵ򿪵Ĵ��ھ����ֵ��hCom,�������ڴ�״̬ComIsOK,��ɴ���״̬ ���� 
//�������:������,����λ,ֹͣλ,У��λ 
//��������:�� 

void OpenComm(int nBaud, int nData, int nStop, int nCal) {
	hCom = CreateFile(strcomname, //���ں� 
		GENERIC_READ | GENERIC_WRITE, //�������д 
		0, //��ռ��ʽ 
		NULL,
		OPEN_EXISTING, //�򿪶����Ǵ��� 
		0,//ͬ��ͨѶ��ʽ��Ҳ��������ʽͨ��
		//FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,//�ص���ʽ,�����첽ͨ��
		NULL );
		if (hCom == INVALID_HANDLE_VALUE) 
		{ 
			AfxMessageBox(_T("��COMʧ�ܣ����ڲ����ڻ��ѱ�ռ��!"));
			ComIsOK = false; return; 
		} 
		ComIsOK = true; 
		SetCommMask(hCom, EV_TXEMPTY | EV_RXCHAR); //�����¼�����,��ʱû���� 
		SetupComm(hCom,1024,1024); //�������뻺����������������Ĵ�С����1024
		COMMTIMEOUTS TimeOuts; //�趨����ʱ 
		TimeOuts.ReadIntervalTimeout = MAXDWORD; 
		TimeOuts.ReadTotalTimeoutConstant = 0; 
		TimeOuts.ReadTotalTimeoutMultiplier = 0; //�趨д��ʱ 
		TimeOuts.WriteTotalTimeoutConstant = 500; 
		TimeOuts.WriteTotalTimeoutMultiplier = 100; 
		if(SetCommTimeouts(hCom,&TimeOuts) == false) 
		{ 
			CloseHandle(hCom); 
			ComIsOK = false; return; 
		} //������������ 
		DCB dcb; 
		GetCommState(hCom,&dcb); 
		dcb.BaudRate=nBaud; //dcb.BaudRate=9600; //������Ϊ9600 
		dcb.ByteSize=nData; //dcb.ByteSize=8; //ÿ���ֽ�Ϊ8λ 
		dcb.StopBits=nStop; //dcb.StopBits=ONESTOPBIT;   //1λֹͣλ 
		dcb.Parity=nCal; //dcb.Parity=NOPARITY; //����ż����λ 
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

//==========���ڹرտ��ƺ���===================== 
void CloseComm() 
{ 
	CloseHandle(hCom); 
	hCom = NULL; 
	ComIsOK = false; 
} 


//==========���ڼ����̺߳���====================== 
UINT ThreadFunc(LPVOID pParam) 
{ 
	CLEDControlDlg* pdlg = (CLEDControlDlg*)pParam; //����ָ��ָ�����Ի��� 
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
			Sleep(3);//continue;//ʹ��continueʱ���򿪴��ں�CPUռ���ʷǳ��� 
		} 
		else 
		{
			// ::SendMessage(::AfxGetMainWnd()->m_hWnd,WM_READCOMM,1,0); //������Ϣ,�Ѷ��� �������Ŷ���Ϣ����ʽ
			pdlg->ReadComm(); //���̴߳�����ռ�ý����߳���Դ
		}
	} 
	return 0; 
}

char ConvertHexChar(char ch)
{
	//��һ���ַ�ת��Ϊ��Ӧ��ʮ������ 
	if ((ch >= '0') && (ch <= '9'))
		return ch - 48;//0x30; 
	else if ((ch >= 'A') && (ch <= 'F'))
		return ch - 'A' + 10;
	else if ((ch >= 'a') && (ch <= 'f'))
		return ch - 'a' + 10;
	else return (-1);
}
