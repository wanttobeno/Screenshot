/******************************************************************************
*  FileName        :   MyEdit.CPP
*  Author          :   Unknown
*  Mender          :   sudami
*  Time            :   2007/09/09
*  
*  Comment         :  
*--------------------------------------------------------
*         截图程序中的一个控件封装类.
*  将控件和该类关联后,可以显示出当前截图的一些提示信息 
*--------------------------------------------------------
******************************************************************************/

#include "stdafx.h"
#include "MyEdit.h"
#include "resource.h"

///////////////////////////////////////////////////////////////////////////////
// 构造函数、析构函数
//
CMyEdit::CMyEdit()
{
	m_bMove=TRUE;
}

CMyEdit::~CMyEdit()
{

}

BEGIN_MESSAGE_MAP(CMyEdit, CEdit)
    /* 2个小消息 */
	ON_WM_CREATE() 
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
	/* 3个大消息 */
	ON_WM_MOUSEMOVE()  
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
//
int CMyEdit::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CEdit::OnCreate(lpCreateStruct) == -1)
		return -1;
	return 0;
}


void CMyEdit::OnPaint()
{
	CPaintDC dc(this);

	CDC dcCompatible;
	dcCompatible.CreateCompatibleDC(&dc);

	dcCompatible.SetBkMode(TRANSPARENT);
	CBitmap bitmap;
	bitmap.LoadBitmap(IDB_BITMAP_BK);
	dcCompatible.SelectObject(&bitmap);

	RECT rt = {5,5,0,0};
	RECT rtClient = {0};
	GetClientRect(&rtClient);
	rt.right = rtClient.right;
	rt.bottom = rtClient.bottom;

	CString str;
	GetWindowText(str);
	CFont font;
	CFont * pOldFont;
	font.CreatePointFont(90, _T("宋体"));
	pOldFont = dcCompatible.SelectObject(&font);
	dcCompatible.DrawText(str,&rt,DT_LEFT);
	dcCompatible.SelectObject(pOldFont);

	CRect rect;
	GetClientRect(&rect);
	dc.BitBlt(0,0,rect.Width(),rect.Height(),&dcCompatible,0,0,SRCCOPY);
}

/////////////////////////////////////////////////////////////////////////////
//              <响应 WM_MOUSEMOVE 消息>
// 和QQ的截图差不多的效果,只要鼠标挪到该控件区域,该区域就变换位置
//
void CMyEdit::OnMouseMove(UINT nFlags, CPoint point) 
{	
	CEdit::OnMouseMove(nFlags, point);
	CRect rect;
	GetWindowRect(&rect);
	int xScreen = GetSystemMetrics(SM_CXSCREEN);
	if(m_bMove)
	{
		//移动到左上角
		MoveWindow(10,10,rect.Width(),rect.Height());
		m_bMove=FALSE;
	}
	else
	{
		//移动到右上角
		MoveWindow(xScreen-180,10,rect.Width(),rect.Height());
		m_bMove=TRUE;
	}
}

/////////////////////////////////////////////////////////////////////////////
//
void CMyEdit::OnSetFocus(CWnd* pOldWnd) 
{
	CEdit::OnSetFocus(pOldWnd);

	// 隐藏光标提示符
	this->HideCaret();
}

BOOL CMyEdit::OnEraseBkgnd(CDC* pDC) 
{
	return TRUE;
}
