#if !defined(AFX_CATCHSCREENDLG_H__536FDBC8_7DB2_4BEF_8943_70DBE8AD845F__INCLUDED_)
#define AFX_CATCHSCREENDLG_H__536FDBC8_7DB2_4BEF_8943_70DBE8AD845F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif 

//--------------------------------------------------------------------------
#include "Resource.h"
#include "MyEdit.h"

#ifndef MYTRACKER_
#include "MyTracker.h"
#endif

#include "MyToolBar.h"
//--------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////
// 
class CCatchScreenDlg : public CDialog
{
public:
	
	CCatchScreenDlg(CWnd* pParent = NULL);	
	
	enum { IDD = IDD_DIALOGFORIMG };
	CMyEdit	m_tipEdit;
	CMyToolBar m_toolBar;

public:
	
	int m_xScreen;
	int m_yScreen;
	

	BOOL m_bLBtnDown;

	BOOL m_bNeedShowMsg;            // 显示截取矩形大小信息
	BOOL m_bDraw;                   // 是否为截取状态
	BOOL m_bFirstDraw;              // 是否为首次截取
	BOOL m_bQuit;                   // 是否为退出
	CPoint m_startPt;				// 截取矩形左上角位置
	
	CMyTracker m_rectTracker;       // 像皮筋类
	CBrush   m_brush;			
    HCURSOR  m_hCursor;             // 光标
	CBitmap* m_pBitmap;             // Edit关联控件的背景位图
	HBITMAP m_hBitmap;
	
	CRgn m_rgn;						// 背景擦除区域

public:
	HBITMAP CopyScreenToBitmap(LPRECT lpRect,BOOL bSave=FALSE);   /* 拷贝桌面到位图 */
	void UpdateTipString();                            //显示操作提示信息
	void DrawMessage(CRect &inRect,CDC * pDC);       //显示截取矩形信息
	void InvalidateRgnWindow();                        //重画窗口
	void UpdateMousePointRGBString();

	CString m_strRgb;
	CString m_strEditTip;

	void SetEidtWndText();
	CString GetEditText();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual void OnCancel();

	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);


	DECLARE_MESSAGE_MAP()

	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
};

///////////////////////////////////////////////////////////////////////////////////////////

#endif 
