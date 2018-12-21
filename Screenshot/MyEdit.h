/*****************************************************************************
*  FileName        :   MyEdit.h
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

#if !defined(AFX_MYEDIT_H__A34EEA6D_E8FC_4D15_B03C_BAA42FDF6FCB__INCLUDED_)
#define AFX_MYEDIT_H__A34EEA6D_E8FC_4D15_B03C_BAA42FDF6FCB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif

//-----------------------------------------------------------------------------

#include <afxwin.h>

//-----------------------------------------------------------------------------

class CMyEdit : public CEdit
{
public:
	CMyEdit();
	virtual ~CMyEdit();
	
	BOOL m_bMove;      // 类似"单刀双掷开关"

protected:
	
	afx_msg int     OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void    OnSetFocus(CWnd* pOldWnd);

	afx_msg void	OnPaint();

	afx_msg void    OnMouseMove(UINT nFlags, CPoint point);

	afx_msg BOOL    OnEraseBkgnd(CDC* pDC);

	DECLARE_MESSAGE_MAP()
};

//////////////////////////////////////////////////////////////////////////////////

#endif 