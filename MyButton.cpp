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
	m_Style = 1;	//m_Style = 0;	//按钮形状风格
	b_InRect = false;	//鼠标进入标志
	m_strText = _T("");	//按钮文字(使用默认文字)
	m_ForeColor = RGB(200, 0, 0);	//文字颜色(黑色)
	m_MouseInColor = RGB(0, 0, 255);	//鼠标进入时文字颜色(蓝色)
	m_BackColor = RGB(200, 200, 230);	//m_BackColor = RGB(243,243,243);		//背景色(灰白色)
	m_LockForeColor = GetSysColor(COLOR_GRAYTEXT);	//锁定按钮的文字颜色
	p_Font = NULL;	//字体指针

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
	ModifyStyle(0, BS_OWNERDRAW);         //设置按钮属性为自画式
	//PreSubclassWindow()在按钮创建前自动执行，所以我们可以在其中做一些初始工作。
	//这里只做了一项工作，就是为按钮设置属性为"自绘"式，这样，用户在添加按钮后，就不需设置"Owner draw"属性了。
	CButton::PreSubclassWindow();
}

void MyButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	// TODO: Add your code to draw the specified item
	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	m_ButRect = lpDrawItemStruct->rcItem;     //获取按钮尺寸
	if (m_strText.IsEmpty())
		GetWindowText(m_strText);           //获取按钮文本

	int nSavedDC = pDC->SaveDC();
	VERIFY(pDC);
	DrawButton(pDC);                 //绘制按钮
	pDC->RestoreDC(nSavedDC);

}

void MyButton::OnNcMouseMove(UINT nHitTest, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if (!b_InRect || GetCapture() != this)     //鼠标进入按钮
	{
		b_InRect = true;     //设置进入标志
		SetCapture();        //捕获鼠标
		m_Style = 2;	//m_Style = 1;         //设置按钮状态
		Invalidate();        //重绘按钮
	}
	else
	{
		if (!m_ButRect.PtInRect(point))     //鼠标离开按钮
		{
			b_InRect = false;    //清除进入标志
			ReleaseCapture();    //释放捕获的鼠标
			m_Style = 1;	//m_Style = 0;         //设置按钮状态
			Invalidate();        //重绘按钮
		}
	}

	CButton::OnNcMouseMove(nHitTest, point);
}

void MyButton::OnNcMButtonDown(UINT nHitTest, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	m_Style = 2;
	Invalidate();         //重绘按钮
	CButton::OnLButtonDown(nHitTest, point);
}

void MyButton::OnNcMButtonUp(UINT nHitTest, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	m_Style = 1;
	Invalidate();         //重绘按钮

	CButton::OnNcMButtonUp(nHitTest, point);
}

void MyButton::DrawButton(CDC* pDC)
{
	//调整状态
	if (m_Style == 3) m_Style = 0;
	if (GetStyle() & WS_DISABLED)
		m_Style = 3;     //禁止状态
	//根据状态调整边框颜色和文字颜色
	COLORREF bColor, fColor;     //bColor为边框颜色，fColor为文字颜色
	switch (m_Style)
	{
	case 0: bColor = RGB(192, 192, 192); fColor = m_ForeColor; break;   //正常按钮
	case 1: bColor = RGB(255, 255, 255); fColor = m_ForeColor; break;   //鼠标进入时按钮
	case 2: bColor = RGB(192, 192, 192); fColor = m_MouseInColor; break;   //按下的按钮
	case 3: bColor = m_BackColor; fColor = m_LockForeColor; break;    //锁定的按钮
	}
	//绘制按钮背景
	CBrush Brush;
	Brush.CreateSolidBrush(m_BackColor);     //背景刷
	pDC->SelectObject(&Brush);
	CPen Pen;
	Pen.CreatePen(PS_SOLID, 3, bColor);
	pDC->SelectObject(&Pen);
	pDC->RoundRect(&m_ButRect, CPoint(10, 10));    //画圆角矩形
	//绘制按钮按下时的边框
	if (m_Style != 2)
	{
		CRect Rect;
		Rect.SetRect(m_ButRect.left + 1, m_ButRect.top + 1, m_ButRect.right, m_ButRect.bottom);
		pDC->DrawEdge(&Rect, BDR_RAISEDINNER, BF_RECT);     //画边框
	}
	//绘制按钮文字
	pDC->SetTextColor(fColor);         //画文字
	pDC->SetBkMode(TRANSPARENT);
	pDC->DrawText(m_strText, &m_ButRect, DT_SINGLELINE | DT_CENTER
		| DT_VCENTER | DT_END_ELLIPSIS);
	//绘制拥有焦点按钮的虚线框
	if (GetFocus() == this)
	{
		CRect Rect;
		Rect.SetRect(m_ButRect.left + 3, m_ButRect.top + 2, m_ButRect.right - 3, m_ButRect.bottom - 2);
		pDC->DrawFocusRect(&Rect);     //画拥有焦点的虚线框
	}
}



//设置按钮文本
void MyButton::SetText(CString str)
{
	m_strText = _T("");
	SetWindowText(str);
}

//设置文本颜色
void MyButton::SetForeColor(COLORREF color)
{
	m_ForeColor = color;
	Invalidate();
}

//设置背景颜色
void MyButton::SetBkColor(COLORREF color)
{
	m_BackColor = color;
	Invalidate();
}

//设置字体(字体高度、字体名)
void MyButton::SetTextFont(int FontHight, LPCTSTR FontName)
{
	if (p_Font)     delete p_Font;     //删除旧字体
	p_Font = new CFont;
	p_Font->CreatePointFont(FontHight, FontName);     //创建新字体
	SetFont(p_Font);                 //设置字体
}

///由于新字体由 new 生成，必须显式回收，这项工作可以在 CMyButton类 的析构函数中进行：

/*CMyButton::~CMyButton()
{
	 if ( p_Font )     delete p_Font;         //删除字体
}
*/
//这样一个可设置颜色、字体的按钮类就做好了。使用时，先在对话框中放置好按钮，再用 ClassWizard 为按钮添加控制变量，
//并且将变量的类型设置为 CMyButton。之后，可以用该变量调用接口函数设置按钮颜色和字体。
