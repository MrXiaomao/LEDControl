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
	int		m_Style;	//��ť��״(0-������1-��ǰ��2-���£�3-����) 
	bool	b_InRect;	//�������־
	CString		m_strText;	//��ť����
	COLORREF	m_ForeColor;//�ı���ɫ
	COLORREF	m_MouseInColor;//������ʱ�ı���ɫ
	COLORREF	m_BackColor;//������ɫ
	COLORREF	m_LockForeColor; //������ť��������ɫ
	CRect	m_ButRect;	//��ť�ߴ�
	CFont* p_Font; //����
	void DrawButton(CDC* pDC);	//��������ť

// Construction
public:
	MyButton();
	void SetText(CString str);	//��������
	void SetForeColor(COLORREF color);	//�����ı���ɫ
	void SetBkColor(COLORREF color);		//���ñ�����ɫ
	void SetTextFont(int FontHight, LPCTSTR FontName);	//��������
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
