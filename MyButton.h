#if !defined(AFX_MYBUTTON_H__B834D0A9_C834_4584_BC86_9AD8264EB109__INCLUDED_)
#define AFX_MYBUTTON_H__B834D0A9_C834_4584_BC86_9AD8264EB109__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MyButton.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// MyButton window

class MyButton : public CButton
{

private:
	int		m_Style;	//按钮形状(0-正常，1-当前，2-按下，3-锁定) 
	bool	b_InRect;	//鼠标进入标志
	CString		m_strText;	//按钮文字
	COLORREF	m_ForeColor;//文本颜色
	COLORREF	m_MouseInColor;//鼠标进入时文本颜色
	COLORREF	m_BackColor;//背景颜色
	COLORREF	m_LockForeColor; //锁定按钮的文字颜色
	CRect	m_ButRect;	//按钮尺寸
	CFont* p_Font; //字体
	void DrawButton(CDC* pDC);	//画正常按钮

// Construction
public:
	MyButton();
	void SetText(CString str);	//设置文字
	void SetForeColor(COLORREF color);	//设置文本颜色
	void SetBkColor(COLORREF color);		//设置背景颜色
	void SetTextFont(int FontHight, LPCTSTR FontName);	//设置字体
// Attributes
public:

	// Operations
public:

	// Overrides
		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(MyButton)
public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
protected:
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~MyButton();

	// Generated message map functions
protected:
	//{{AFX_MSG(MyButton)
	afx_msg void OnNcMouseMove(UINT nHitTest, CPoint point);
	afx_msg void OnNcMButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnNcMButtonUp(UINT nHitTest, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MYBUTTON_H__B834D0A9_C834_4584_BC86_9AD8264EB109__INCLUDED_)
