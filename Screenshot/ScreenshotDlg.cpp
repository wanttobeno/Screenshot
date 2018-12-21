// ScreenshotDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Screenshot.h"
#include "ScreenshotDlg.h"

#include "CatchScreenDlg.h"


#ifdef _DEBUG
//#define new DEBUG_NEW
#endif


// CScreenshotDlg 对话框


CScreenshotDlg::CScreenshotDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CScreenshotDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CScreenshotDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CScreenshotDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BTN_START, &CScreenshotDlg::OnBnClickedBtnStart)
	ON_WM_CTLCOLOR()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()


// CScreenshotDlg 消息处理程序

BOOL CScreenshotDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	// 使窗体在最顶层
	::SetWindowPos(GetSafeHwnd(), HWND_TOPMOST, 150, 150, 0, 0, 
		SWP_NOMOVE|SWP_NOSIZE|SWP_NOREDRAW);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CScreenshotDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CScreenshotDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//----------------------------------------------------------------------
// 线程函数,用来截图
//
UINT SccreenShot_Thread (LPVOID lpParam)
{
	HWND hWndMain = (HWND) lpParam;
	CCatchScreenDlg dlg;
	dlg.DoModal();

	::ShowWindow(hWndMain,SW_SHOW);
	return 0;
}

void CScreenshotDlg::OnBnClickedBtnStart()
{
	::ShowWindow (m_hWnd, SW_HIDE);
	//使得被激活窗口出现在前景           
	::AfxBeginThread (SccreenShot_Thread, (LPVOID)GetSafeHwnd());
	//::ShowWindow (GetSafeHwnd(), SW_SHOW);
}

