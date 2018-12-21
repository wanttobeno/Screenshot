#include "StdAfx.h"
#include "MyToolBar.h"
#include <GdiPlus.h>
#include "resource.h"



BOOL ImageFromIDResource(UINT nID, LPCTSTR sTR, Gdiplus::Bitmap * & pImg)
{
	HINSTANCE hInst = AfxGetResourceHandle();
	HRSRC hRsrc = ::FindResource (hInst,MAKEINTRESOURCE(nID),sTR); // type
	if (!hRsrc)
		return FALSE;
	// load resource into memory
	DWORD len = SizeofResource(hInst, hRsrc);
	BYTE* lpRsrc = (BYTE*)LoadResource(hInst, hRsrc);
	if (!lpRsrc)
		return FALSE;
	// Allocate global memory on which to create stream
	HGLOBAL m_hMem = GlobalAlloc(GMEM_FIXED, len);
	BYTE* pmem = (BYTE*)GlobalLock(m_hMem);
	memcpy(pmem,lpRsrc,len);
	IStream* pstm;
	CreateStreamOnHGlobal(m_hMem,FALSE,&pstm);
	// load from stream
	pImg=Gdiplus::Bitmap::FromStream(pstm);
	// free/release stuff
	GlobalUnlock(m_hMem);
	pstm->Release();
	FreeResource(lpRsrc);
	return TRUE;
}

CMyToolBar::CMyToolBar()
{
	m_hImageList = NULL;
	m_hWnd_toolbar = 0;
	m_hWndParent = 0;
}

CMyToolBar::~CMyToolBar(void)
{
	ImageList_Destroy(m_hImageList);
}

BOOL CMyToolBar::CreateToolBar(HWND hWndParent)
{	
	if (m_hImageList)
		return FALSE;
	m_hWndParent = hWndParent;

	m_hImageList= ImageList_Create(18,18,ILC_COLOR32,1,1);
	Gdiplus::Bitmap * pImage = NULL;
	for(int i=0;i< 9;i++)
	{
		ImageFromIDResource(IDB_RECTANGLE + i,_T("PNG"),pImage);
		HBITMAP pHbitmap=0;;
		if(pImage)
		{
			pImage->GetHBITMAP(Gdiplus::Color(0xff,0xff,0xff,0xff),&pHbitmap);
			ImageList_Add(m_hImageList,pHbitmap,NULL);
			delete pImage;
			pImage = NULL;
		}
	}

	m_hWnd_toolbar = ::CreateWindowEx(0,TOOLBARCLASSNAME,0,WS_CHILD | WS_VISIBLE|WS_BORDER ,
		0,0,0,0,hWndParent,NULL,GetModuleHandle(NULL),NULL);

	if (m_hWnd_toolbar == NULL)
		return FALSE;
	::SendMessage(m_hWnd_toolbar,TB_SETIMAGELIST, 0, (LPARAM) m_hImageList);
	::SendMessage(m_hWnd_toolbar,TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);


	::SendMessage(m_hWnd_toolbar,
		(UINT) TB_SETBITMAPSIZE,(WPARAM) 0,//not used, must be zero 
		(LPARAM) MAKELONG (18, 18)// = (LPARAM) MAKELONG (dxBitmap, dyBitmap)
		);

	//TCHAR tooltips[16][30]={_T("AAAA"),_T("BBBB"),_T("CCCC"),_T("DDDD")};
	for(int i = 0; i < 9; i++)
	{
		TBBUTTON tbbutton;
		// 换行
		int wrapnow = 0;
		//if (i % 2 == 1)
		//	wrapnow = TBSTATE_WRAP;

		ZeroMemory(&tbbutton, sizeof(TBBUTTON));
		//tbbutton.iString   = (INT_PTR) tooltips[i];
		tbbutton.fsStyle   = TBSTYLE_CHECKGROUP; // 单选属性
		tbbutton.fsState   = TBSTATE_ENABLED | wrapnow;
		tbbutton.idCommand = MyToolBar_ID + i; // 定义控件的id
		tbbutton.iBitmap   = i;

		::SendMessage(m_hWnd_toolbar,TB_ADDBUTTONS, 1, (LPARAM) &tbbutton);
	}
	::SendMessage(m_hWnd_toolbar, TB_AUTOSIZE, 0, 0); 
	return TRUE;
}

HWND CMyToolBar::GetHWND()
{
	return m_hWnd_toolbar;
}

void CMyToolBar::AddChildStyle()
{
	DWORD dwStyle = GetWindowLong(m_hWnd_toolbar, GWL_STYLE);
	dwStyle &= WS_CHILD; 
	SetWindowLong(m_hWnd_toolbar,GWL_STYLE,dwStyle);
}

void CMyToolBar::RemoveChildStyle()
{

	DWORD dwStyle = GetWindowLong(m_hWnd_toolbar, GWL_STYLE);
	dwStyle &= ~WS_CHILD; 
	SetWindowLong(m_hWnd_toolbar,GWL_STYLE,dwStyle);
}

void CMyToolBar::ShowToolBar()
{
	::ShowWindow(m_hWnd_toolbar,SW_SHOW);
}

void CMyToolBar::HideToolBar()
{
	::ShowWindow(m_hWnd_toolbar,SW_HIDE);
}

void CMyToolBar::SetAtCurMousePlase()
{
	RECT rtWin = {0};
	::GetWindowRect(m_hWnd_toolbar,&rtWin);
	POINT pt = {0};
	::GetCursorPos(&pt);
	this->SetShowPlace(pt.x,pt.y);
}

void CMyToolBar::SetShowPlace(int nCurPointX,int nCurPointY)
{

	RECT rtWin = {0};
	::GetWindowRect(m_hWnd_toolbar,&rtWin);
	::SetWindowPos(m_hWnd_toolbar,HWND_TOP,nCurPointX - (rtWin.right-rtWin.left),nCurPointY + 2,0,0,SWP_NOSIZE|SWP_SHOWWINDOW);
}