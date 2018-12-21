/******************************************************************************
*  FileName        :   CatchScreenDlg.cpp
*  Author          :   Unknown
*  Mender          :   sudami
*  Time            :   2007/09/09
*
*  Comment         :
*
******************************************************************************/

#include "stdafx.h"
#include "Screenshot.h"
#include "CatchScreenDlg.h"

#include <GdiPlus.h>
using namespace  Gdiplus;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CCatchScreenDlg dialog

CCatchScreenDlg::CCatchScreenDlg(CWnd* pParent /*=NULL*/)
: CDialog(CCatchScreenDlg::IDD, pParent)
{
	m_bLBtnDown = FALSE;
	//初始化像皮筋类,新增的resizeMiddle 类型
	m_rectTracker.m_nStyle = CMyTracker::resizeMiddle | CMyTracker::solidLine;
	m_rectTracker.m_rect.SetRect(-1, -2, -3, -4);
	//设置矩形颜色
	m_rectTracker.SetRectColor(RGB(10, 100, 130));

	m_hCursor = AfxGetApp()->LoadCursor(IDC_CURSOR1);

	m_bDraw = FALSE;
	m_bFirstDraw = FALSE;
	m_bQuit = FALSE;
	m_bNeedShowMsg = FALSE;
	m_startPt = 0;

	//获取屏幕分辩率
	m_xScreen = GetSystemMetrics(SM_CXSCREEN);
	m_yScreen = GetSystemMetrics(SM_CYSCREEN);

	//截取屏幕到位图中
	CRect rect(0, 0, m_xScreen, m_yScreen);
	m_hBitmap = CopyScreenToBitmap(&rect);
	m_pBitmap = CBitmap::FromHandle(m_hBitmap);

	//初始化刷新窗口区域 m_rgn
	m_rgn.CreateRectRgn(0, 0, 50, 50);
}

void CCatchScreenDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_tipEdit);
}

BEGIN_MESSAGE_MAP(CCatchScreenDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_ERASEBKGND()
	ON_WM_SETCURSOR()
	ON_WM_RBUTTONUP()
	ON_WM_CTLCOLOR()
	ON_WM_RBUTTONDOWN()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCatchScreenDlg message handlers

BOOL CCatchScreenDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	//把对化框设置成全屏顶层窗口
	SetWindowPos(&wndTopMost, 0, 0, m_xScreen, m_yScreen, SWP_SHOWWINDOW);

	//移动操作提示窗口
	CRect rect;
	m_tipEdit.GetWindowRect(&rect);
	m_tipEdit.MoveWindow(10, 10, rect.Width(), rect.Height());


	m_toolBar.CreateToolBar(m_hWnd);
	m_toolBar.RemoveChildStyle();
	::MoveWindow(m_toolBar.GetHWND(),300,300,230,30,FALSE);


	UpdateTipString();
	SetEidtWndText();

	((CScreenshotApp *)AfxGetApp())->m_hwndDlg = AfxGetMainWnd()->GetSafeHwnd();
	return TRUE;
}
 
void CCatchScreenDlg::OnPaint()
{
	// 如果窗口是最小化状态
	if (IsIconic())
	{
		CPaintDC dc(this);

		SendMessage(WM_ICONERASEBKGND, (WPARAM)dc.GetSafeHdc(), 0);

		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

	}
	else  // 如果窗口正常显示
	{
		CPaintDC dc(this);

		CDC dcCompatible;

		dcCompatible.CreateCompatibleDC(&dc);
		RECT rect;
		::GetClientRect(m_hWnd, &rect);
		HBITMAP hBitmap = ::CreateCompatibleBitmap(dc.m_hDC,rect.right-rect.left,rect.bottom-rect.top);
		::SelectObject(dcCompatible.m_hDC,hBitmap);

		HBRUSH s_hBitmapBrush = CreatePatternBrush(m_hBitmap); 
		::FillRect(dcCompatible.m_hDC,&rect,s_hBitmapBrush);

		//显示截取矩形大小信息
		if (m_bNeedShowMsg && m_bFirstDraw)
		{
			CRect rect = m_rectTracker.m_rect;
			DrawMessage(rect, &dcCompatible);
		}

		//画出像皮筋矩形
		if (m_bFirstDraw)
		{
			m_rectTracker.Draw(&dcCompatible);
		}
		Gdiplus::Graphics graphics(dcCompatible);

		HRGN hgn1 = CreateRectRgn(m_rectTracker.m_rect.left,m_rectTracker.m_rect.top,
			m_rectTracker.m_rect.right,m_rectTracker.m_rect.bottom);
		Region region1(hgn1);

		HRGN hgn2 = CreateRectRgn(rect.left,rect.top,
			rect.right,rect.bottom);
		Region region2(hgn2);

		region2.Exclude(&region1);

		SolidBrush  solidBrush(Color(100, 128, 128, 128));
		graphics.FillRegion(&solidBrush,&region2);

		DeleteObject(hgn1);
		DeleteObject(hgn2);

		dc.BitBlt(0,0,rect.right, rect.bottom,&dcCompatible,0,0,SRCCOPY);
		DeleteObject(hBitmap);
		DeleteObject(s_hBitmapBrush);

		//CDialog::OnPaint();
	}
}

void CCatchScreenDlg::OnCancel()
{
	if (m_bFirstDraw)
	{
		//取消已画矩形变量
		m_bFirstDraw = FALSE;
		m_bDraw = FALSE;
		m_rectTracker.m_rect.SetRect(-1, -1, -1, -1);
		InvalidateRgnWindow();
	}
	else
	{
		CDialog::OnCancel();
	}
}

void CCatchScreenDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	if(m_bLBtnDown)
		m_toolBar.HideToolBar();
	else
		m_toolBar.ShowToolBar();
	if (m_bDraw)
	{
		//动态调整矩形大小,并刷新画出
		m_rectTracker.m_rect.SetRect(m_startPt.x + 4, m_startPt.y + 4, point.x, point.y);
		InvalidateRgnWindow();
	}
	
	//弥补调整大小和位置时,接收不到MouseMove消息
	CRect rect;
	m_tipEdit.GetWindowRect(&rect);
	if (rect.PtInRect(point))
		m_tipEdit.SendMessage(WM_MOUSEMOVE);

	UpdateMousePointRGBString();
	SetEidtWndText();

	CDialog::OnMouseMove(nFlags, point);
}

void CCatchScreenDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_bLBtnDown = TRUE;
	int nHitTest;
	nHitTest = m_rectTracker.HitTest(point);
	//判断击中位置
	if (nHitTest < 0)
	{
		if (!m_bFirstDraw)
		{
			//第一次画矩形
			m_startPt = point;
			m_bDraw = TRUE;
			m_bFirstDraw = TRUE;
			//设置当鼠标按下时最小的矩形大小
			m_rectTracker.m_rect.SetRect(point.x, point.y, point.x + 4, point.y + 4);

			//保证当鼠标当下时立刻显示信息
			if (m_bFirstDraw)
				m_bNeedShowMsg = TRUE;
			UpdateTipString();
			SetEidtWndText();
			InvalidateRgnWindow();
		}
	}
	else
	{
		//保证当鼠标当下时立刻显示信息
		m_bNeedShowMsg = TRUE;
		if (m_bFirstDraw)
		{
			//调束大小时,Track会自动调整矩形大小,在些期间,消息归CRectTracker内部处理
			m_rectTracker.Track(this, point, TRUE);
			InvalidateRgnWindow();
		}
	}

	CDialog::OnLButtonDown(nFlags, point);
}

void CCatchScreenDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_bLBtnDown = FALSE;
	m_bNeedShowMsg = FALSE;
	m_bDraw = FALSE;
	UpdateTipString();
	SetEidtWndText();
	m_toolBar.SetShowPlace(m_rectTracker.m_rect.right,m_rectTracker.m_rect.bottom);

	InvalidateRgnWindow();
	CDialog::OnLButtonUp(nFlags, point);
}

void CCatchScreenDlg::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	int nHitTest;
	nHitTest = m_rectTracker.HitTest(point);

	//如果在是矩形内部双击
	if (nHitTest == 8)
	{
		//保存位图到粘贴板中,bSave 为TRUE,
		CopyScreenToBitmap(m_rectTracker.m_rect, TRUE);
		PostQuitMessage(0);
	}

	CDialog::OnLButtonDblClk(nFlags, point);
}

void CCatchScreenDlg::OnRButtonDown(UINT nFlags, CPoint point)
{
	m_toolBar.HideToolBar();
	//InvalidateRgnWindow();
	CDialog::OnRButtonDown(nFlags, point);
}

void CCatchScreenDlg::OnRButtonUp(UINT nFlags, CPoint point)
{
	m_bLBtnDown = FALSE;
	if (m_bFirstDraw)
	{
		//如果已经截取矩则清除截取矩形
		m_bFirstDraw = FALSE;
		//清除矩形大小
		m_rectTracker.m_rect.SetRect(-1, -1, -1, -1);
		UpdateTipString();
		SetEidtWndText();
		InvalidateRgnWindow();
	}
	else
	{
		//关闭程序
		PostQuitMessage(0);
	}

	CDialog::OnRButtonUp(nFlags, point);
}

HBRUSH CCatchScreenDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	//设置操作提示窗口文本颜色
	if (pWnd->GetDlgCtrlID() == IDC_EDIT1)
	{
		pDC->SetTextColor(RGB(247,76,128));
	}

	return hbr;
}

BOOL CCatchScreenDlg::OnEraseBkgnd(CDC* pDC)
{
	return FALSE;
	//用整个桌面填充全屏对话框背景
	BITMAP bmp;
	m_pBitmap->GetBitmap(&bmp);

	CDC dcCompatible;
	dcCompatible.CreateCompatibleDC(pDC);

	dcCompatible.SelectObject(m_pBitmap);

	CRect rect;
	GetClientRect(&rect);
	pDC->BitBlt(0, 0, rect.Width(), rect.Height(), &dcCompatible, 0, 0, SRCCOPY);
	return TRUE;
}

BOOL CCatchScreenDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	//设置改变截取矩形大小时光标
	if (pWnd == this &&m_rectTracker.SetCursor(this, nHitTest)
		&& !m_bDraw &&m_bFirstDraw) //此处判断保截取时当标始中为彩色光标
	{
		return TRUE;
	}

	//设置彩色光标
	SetCursor(m_hCursor);
	return TRUE;
}

// 拷贝屏幕, 这段源码源自CSDN
// lpRect 代表选定区域
HBITMAP CCatchScreenDlg::CopyScreenToBitmap(LPRECT lpRect, BOOL bSave)

{
	HDC       hScrDC, hMemDC;
	// 屏幕和内存设备描述表
	HBITMAP    hBitmap, hOldBitmap;
	// 位图句柄
	int       nX, nY, nX2, nY2;
	// 选定区域坐标
	int       nWidth, nHeight;

	// 确保选定区域不为空矩形
	if (IsRectEmpty(lpRect))
		return NULL;
	//为屏幕创建设备描述表
	hScrDC = CreateDC(_T("DISPLAY"), NULL, NULL, NULL);

	//为屏幕设备描述表创建兼容的内存设备描述表
	hMemDC = CreateCompatibleDC(hScrDC);
	// 获得选定区域坐标
	nX = lpRect->left;
	nY = lpRect->top;
	nX2 = lpRect->right;
	nY2 = lpRect->bottom;

	//确保选定区域是可见的
	if (nX < 0)
		nX = 0;
	if (nY < 0)
		nY = 0;
	if (nX2 > m_xScreen)
		nX2 = m_xScreen;
	if (nY2 > m_yScreen)
		nY2 = m_yScreen;
	nWidth = nX2 - nX;
	nHeight = nY2 - nY;
	// 创建一个与屏幕设备描述表兼容的位图
	hBitmap = CreateCompatibleBitmap
		(hScrDC, nWidth, nHeight);
	// 把新位图选到内存设备描述表中
	hOldBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);
	// 把屏幕设备描述表拷贝到内存设备描述表中
	if (bSave)
	{
		//创建兼容DC,当bSave为中时把开始保存的全屏位图,按截取矩形大小保存
		CDC dcCompatible;
		dcCompatible.CreateCompatibleDC(CDC::FromHandle(hMemDC));
		dcCompatible.SelectObject(m_pBitmap);

		BitBlt(hMemDC, 0, 0, nWidth, nHeight,
			dcCompatible, nX, nY, SRCCOPY);
	}
	else
	{
		BitBlt(hMemDC, 0, 0, nWidth, nHeight,
			hScrDC, nX, nY, SRCCOPY);
	}

	hBitmap = (HBITMAP)SelectObject(hMemDC, hOldBitmap);
	//得到屏幕位图的句柄
	//清除 
	DeleteDC(hScrDC);
	DeleteDC(hMemDC);

	if (bSave)
	{
		if (OpenClipboard())
		{
			//清空剪贴板
			EmptyClipboard();
			//把屏幕内容粘贴到剪贴板上,
			//hBitmap 为刚才的屏幕位图句柄
			SetClipboardData(CF_BITMAP, hBitmap);
			//关闭剪贴板
			CloseClipboard();
		}
	}
	// 返回位图句柄
	return hBitmap;
}

// 显示操作提示信息
void CCatchScreenDlg::UpdateTipString()
{
	CString strTemp;
	if (!m_bDraw && !m_bFirstDraw)
	{
		strTemp = _T("\r\n\r\n·按下鼠标左键不放选择截取\r\n\r\n·ESC键、鼠标右键退出");
	}
	else 
		if (m_bDraw && m_bFirstDraw)
	{
		strTemp = _T("\r\n\r\n·松开鼠标左键确定截取范围\r\n\r\n·ESC键退出");
	}
	else if (!m_bDraw && m_bFirstDraw)
	{
		CString sudami(_T("\r\n\r\n·矩形内双击鼠标左键保存\r\n\r\n·点击鼠标右键重新选择"));
		strTemp = _T("\r\n\r\n·鼠标在矩形边缘调整大小");

		strTemp += sudami;
	}
	m_strEditTip = strTemp;
}

// 显示截取矩形信息
void CCatchScreenDlg::DrawMessage(CRect &inRect, CDC * pDC)
{
	//截取矩形大小信息离鼠标间隔
	const int space = 3;

	//设置字体颜色大小
	CPoint pt;
	CPen pen(PS_SOLID, 1, RGB(47, 79, 79));
	CPen *pOldPen;
	pOldPen = pDC->SelectObject(&pen);

	//pDC->SetTextColor(RGB(147,147,147));
	CFont font;
	CFont * pOldFont;
	font.CreatePointFont(90, _T("宋体"));
	pOldFont = pDC->SelectObject(&font);

	//得到字体宽度和高度
	GetCursorPos(&pt);
	int OldBkMode;
	OldBkMode = pDC->SetBkMode(TRANSPARENT);

	TEXTMETRIC tm;
	int charHeight;
	CSize size;
	int	lineLength;
	pDC->GetTextMetrics(&tm);
	charHeight = tm.tmHeight + tm.tmExternalLeading;
	size = pDC->GetTextExtent(_T("顶点位置  "), _tcslen(_T("顶点位置  ")));
	lineLength = size.cx;

	//初始化矩形, 以保证写下六行文字
	CRect rect(pt.x + space, pt.y - charHeight * 6 - space, pt.x + lineLength + space, pt.y - space);

	//创建临时矩形
	CRect rectTemp;
	//当矩形到达桌面边缘时调整方向和大小
	if ((pt.x + rect.Width()) >= m_xScreen)
	{
		//桌面上方显示不下矩形
		rectTemp = rect;
		rectTemp.left = rect.left - rect.Width() - space * 2;
		rectTemp.right = rect.right - rect.Width() - space * 2;;
		rect = rectTemp;
	}

	if ((pt.y - rect.Height()) <= 0)
	{
		//桌面右方显示不下矩形
		rectTemp = rect;
		rectTemp.top = rect.top + rect.Height() + space * 2;;
		rectTemp.bottom = rect.bottom + rect.Height() + space * 2;;
		rect = rectTemp;
	}

	//创建空画刷画矩形
	CBrush * pOldBrush;
	pOldBrush = pDC->SelectObject(CBrush::FromHandle((HBRUSH)GetStockObject(NULL_BRUSH)));

	pDC->Rectangle(rect);
	rect.top += 2;
	//在矩形中显示文字
	CRect outRect(rect.left, rect.top, rect.left + lineLength, rect.top + charHeight);
	CString string(_T("顶点位置"));
	pDC->DrawText(string, outRect, DT_CENTER);

	outRect.SetRect(rect.left, rect.top + charHeight, rect.left + lineLength, charHeight + rect.top + charHeight);
	string.Format(_T("(%d,%d)"), inRect.left, inRect.top);
	pDC->DrawText(string, outRect, DT_CENTER);


	outRect.SetRect(rect.left, rect.top + charHeight * 2, rect.left + lineLength, charHeight + rect.top + charHeight * 2);
	string = _T("矩形大小");
	pDC->DrawText(string, outRect, DT_CENTER);

	outRect.SetRect(rect.left, rect.top + charHeight * 3, rect.left + lineLength, charHeight + rect.top + charHeight * 3);
	string.Format(_T("(%d,%d)"), inRect.Width(), inRect.Height());
	pDC->DrawText(string, outRect, DT_CENTER);

	outRect.SetRect(rect.left, rect.top + charHeight * 4, rect.left + lineLength, charHeight + rect.top + charHeight * 4);
	string = _T("光标坐标");
	pDC->DrawText(string, outRect, DT_CENTER);

	outRect.SetRect(rect.left, rect.top + charHeight * 5, rect.left + lineLength, charHeight + rect.top + charHeight * 5);
	string.Format(_T("(%d,%d)"), pt.x, pt.y);
	pDC->DrawText(string, outRect, DT_CENTER);

	pDC->SetBkMode(OldBkMode);
	pDC->SelectObject(pOldFont);
	pDC->SelectObject(pOldBrush);
	pDC->SelectObject(pOldPen);
}

// 刷新局部窗口
void CCatchScreenDlg::InvalidateRgnWindow()
{
	//获取当全屏对话框窗口大小
	CRect rect1;
	GetWindowRect(rect1);

	//获取编辑框窗口大小
	CRect rect2;
	m_tipEdit.GetWindowRect(rect2);

	CRgn rgn1, rgn2;
	rgn1.CreateRectRgnIndirect(rect1);
	rgn2.CreateRectRgnIndirect(rect2);

	//获取更新区域,就是除了编辑框窗口不更新
	//m_rgn.CombineRgn(&rgn1, &rgn2, RGN_DIFF);

	// 添加ToolBar不刷新
	CRect rect3;
	::GetWindowRect(m_toolBar.GetHWND(),&rect3);
	CRgn rgn3;
	rgn3.CreateRectRgnIndirect(rect3);

	CRgn rgnTemp;
	rgnTemp.CreateRectRgn(0, 0, 50, 50);
	rgnTemp.CombineRgn(&rgn1, &rgn2, RGN_DIFF);
	m_rgn.CombineRgn(&rgnTemp, &rgn3, RGN_DIFF);

	InvalidateRgn(&m_rgn);
}

void CCatchScreenDlg::UpdateMousePointRGBString()
{
	static CString strOld("");

	CPoint pt;
	GetCursorPos(&pt);

	COLORREF color;
	CClientDC dc(this);
	color = dc.GetPixel(pt);
	BYTE rValue, gValue, bValue;
	rValue = GetRValue(color);
	gValue = GetGValue(color);
	bValue = GetGValue(color);

	//按格式排放字符串
	CString string;
	string.Format(_T("\r\n\r\n·当前像素RGB(%d,%d,%d)"), rValue, gValue, bValue);

	//如果当前颜色没变则不刷新RGB值,以免窗口有更多闪烁
	if (strOld != string)
	{
		m_strRgb = string;
	}
	strOld = string;
}

void  CCatchScreenDlg::SetEidtWndText()
{
	m_tipEdit.SetWindowText(this->GetEditText());
}

CString CCatchScreenDlg::GetEditText()
{
	CString str;
	str.Append(m_strRgb);
	str.Append(m_strEditTip);
	return str;
}

BOOL CCatchScreenDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	bool bHandle = true;
	HWND hWnd = m_toolBar.GetHWND();
	if(lParam == (LPARAM)m_toolBar.GetHWND())
	{
		int wmId  = LOWORD(wParam);
		switch(wmId)
		{
		case MyToolBar_ID:
			AfxMessageBox(_T("矩形"));
			break;
		case MyToolBar_ID+1:
			AfxMessageBox(_T("圆形"));
			break;
		case MyToolBar_ID +2:
			AfxMessageBox(_T("画笔"));
			break;
		case MyToolBar_ID +3:
			AfxMessageBox(_T("马赛克"));
			break;
		case MyToolBar_ID +4:
			AfxMessageBox(_T("文字"));
			break;
		case MyToolBar_ID +5:
			AfxMessageBox(_T("撤销"));
			break;
		case MyToolBar_ID +6:
			CopyScreenToBitmap(m_rectTracker.m_rect, TRUE);
			PostQuitMessage(0);
			break;
		case MyToolBar_ID +7:
			PostQuitMessage(0);
			break;
		case MyToolBar_ID +8:
			CopyScreenToBitmap(m_rectTracker.m_rect, TRUE);
			PostQuitMessage(0);
			break;
		default:
			bHandle = false;
			break;
		}
		::SetFocus(hWnd);
	}
	if (bHandle == false)
	{
		return CDialog::OnCommand(wParam,lParam);
	}
}

////////////////////////////////// END OF FILE ///////////////////////////////////////