#include "pch.h"
#include "stdafx.h"
#include "LEDControl.h"
#include "MyButton.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// MyButton

MyButton::MyButton()
{
	m_Style = 1;	//m_Style = 0;	//��ť��״���
	b_InRect = false;	//�������־
	m_strText = _T("");	//��ť����(ʹ��Ĭ������)
	m_ForeColor = RGB(200, 0, 0);	//������ɫ(��ɫ)
	m_MouseInColor = RGB(0, 0, 255);	//������ʱ������ɫ(��ɫ)
	m_BackColor = RGB(200, 200, 230);	//m_BackColor = RGB(243,243,243);		//����ɫ(�Ұ�ɫ)
	m_LockForeColor = GetSysColor(COLOR_GRAYTEXT);	//������ť��������ɫ
	p_Font = NULL;	//����ָ��

}

MyButton::~MyButton()
{
}


BEGIN_MESSAGE_MAP(MyButton, CButton)
	//{{AFX_MSG_MAP(MyButton)
	ON_WM_NCMOUSEMOVE()
	ON_WM_NCMBUTTONDOWN()
	ON_WM_NCMBUTTONUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// MyButton message handlers



void MyButton::PreSubclassWindow()
{
	// TODO: Add your specialized code here and/or call the base class
	ModifyStyle(0, BS_OWNERDRAW);         //���ð�ť����Ϊ�Ի�ʽ
	//PreSubclassWindow()�ڰ�ť����ǰ�Զ�ִ�У��������ǿ�����������һЩ��ʼ������
	//����ֻ����һ���������Ϊ��ť��������Ϊ"�Ի�"ʽ���������û�����Ӱ�ť�󣬾Ͳ�������"Owner draw"�����ˡ�
	CButton::PreSubclassWindow();
}

void MyButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	// TODO: Add your code to draw the specified item
	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	m_ButRect = lpDrawItemStruct->rcItem;     //��ȡ��ť�ߴ�
	if (m_strText.IsEmpty())
		GetWindowText(m_strText);           //��ȡ��ť�ı�

	int nSavedDC = pDC->SaveDC();
	VERIFY(pDC);
	DrawButton(pDC);                 //���ư�ť
	pDC->RestoreDC(nSavedDC);

}

void MyButton::OnNcMouseMove(UINT nHitTest, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if (!b_InRect || GetCapture() != this)     //�����밴ť
	{
		b_InRect = true;     //���ý����־
		SetCapture();        //�������
		m_Style = 2;	//m_Style = 1;         //���ð�ť״̬
		Invalidate();        //�ػ水ť
	}
	else
	{
		if (!m_ButRect.PtInRect(point))     //����뿪��ť
		{
			b_InRect = false;    //��������־
			ReleaseCapture();    //�ͷŲ�������
			m_Style = 1;	//m_Style = 0;         //���ð�ť״̬
			Invalidate();        //�ػ水ť
		}
	}

	CButton::OnNcMouseMove(nHitTest, point);
}

void MyButton::OnNcMButtonDown(UINT nHitTest, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	m_Style = 2;
	Invalidate();         //�ػ水ť
	CButton::OnLButtonDown(nHitTest, point);
}

void MyButton::OnNcMButtonUp(UINT nHitTest, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	m_Style = 1;
	Invalidate();         //�ػ水ť

	CButton::OnNcMButtonUp(nHitTest, point);
}

void MyButton::DrawButton(CDC* pDC)
{
	//����״̬
	if (m_Style == 3) m_Style = 0;
	if (GetStyle() & WS_DISABLED)
		m_Style = 3;     //��ֹ״̬
	//����״̬�����߿���ɫ��������ɫ
	COLORREF bColor, fColor;     //bColorΪ�߿���ɫ��fColorΪ������ɫ
	switch (m_Style)
	{
	case 0: bColor = RGB(192, 192, 192); fColor = m_ForeColor; break;   //������ť
	case 1: bColor = RGB(255, 255, 255); fColor = m_ForeColor; break;   //������ʱ��ť
	case 2: bColor = RGB(192, 192, 192); fColor = m_MouseInColor; break;   //���µİ�ť
	case 3: bColor = m_BackColor; fColor = m_LockForeColor; break;    //�����İ�ť
	}
	//���ư�ť����
	CBrush Brush;
	Brush.CreateSolidBrush(m_BackColor);     //����ˢ
	pDC->SelectObject(&Brush);
	CPen Pen;
	Pen.CreatePen(PS_SOLID, 3, bColor);
	pDC->SelectObject(&Pen);
	pDC->RoundRect(&m_ButRect, CPoint(10, 10));    //��Բ�Ǿ���
	//���ư�ť����ʱ�ı߿�
	if (m_Style != 2)
	{
		CRect Rect;
		Rect.SetRect(m_ButRect.left + 1, m_ButRect.top + 1, m_ButRect.right, m_ButRect.bottom);
		pDC->DrawEdge(&Rect, BDR_RAISEDINNER, BF_RECT);     //���߿�
	}
	//���ư�ť����
	pDC->SetTextColor(fColor);         //������
	pDC->SetBkMode(TRANSPARENT);
	pDC->DrawText(m_strText, &m_ButRect, DT_SINGLELINE | DT_CENTER
		| DT_VCENTER | DT_END_ELLIPSIS);
	//����ӵ�н��㰴ť�����߿�
	if (GetFocus() == this)
	{
		CRect Rect;
		Rect.SetRect(m_ButRect.left + 3, m_ButRect.top + 2, m_ButRect.right - 3, m_ButRect.bottom - 2);
		pDC->DrawFocusRect(&Rect);     //��ӵ�н�������߿�
	}
}



//���ð�ť�ı�
void MyButton::SetText(CString str)
{
	m_strText = _T("");
	SetWindowText(str);
}

//�����ı���ɫ
void MyButton::SetForeColor(COLORREF color)
{
	m_ForeColor = color;
	Invalidate();
}

//���ñ�����ɫ
void MyButton::SetBkColor(COLORREF color)
{
	m_BackColor = color;
	Invalidate();
}

//��������(����߶ȡ�������)
void MyButton::SetTextFont(int FontHight, LPCTSTR FontName)
{
	if (p_Font)     delete p_Font;     //ɾ��������
	p_Font = new CFont;
	p_Font->CreatePointFont(FontHight, FontName);     //����������
	SetFont(p_Font);                 //��������
}

///������������ new ���ɣ�������ʽ���գ������������ CMyButton�� �����������н��У�

/*CMyButton::~CMyButton()
{
	 if ( p_Font )     delete p_Font;         //ɾ������
}
*/
//����һ����������ɫ������İ�ť��������ˡ�ʹ��ʱ�����ڶԻ����з��úð�ť������ ClassWizard Ϊ��ť��ӿ��Ʊ�����
//���ҽ���������������Ϊ CMyButton��֮�󣬿����øñ������ýӿں������ð�ť��ɫ�����塣
