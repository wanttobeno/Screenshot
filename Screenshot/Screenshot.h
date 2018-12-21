// Screenshot.h : PROJECT_NAME 应用程序的主头文件
//

#pragma once

#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含“stdafx.h”以生成 PCH 文件"
#endif

#include "resource.h"		// 主符号

// CScreenshotApp:
// 有关此类的实现，请参阅 Screenshot.cpp
//

class CScreenshotApp : public CWinApp
{
public:
	CScreenshotApp();

// 重写
	public:
	virtual BOOL InitInstance();


	HWND m_hwndDlg;
	virtual BOOL ProcessMessageFilter(int code, LPMSG lpMsg);

// 实现

	DECLARE_MESSAGE_MAP()
private:
	ULONG_PTR m_gdiplusToken;
public:
	virtual int ExitInstance();
};

extern CScreenshotApp theApp;