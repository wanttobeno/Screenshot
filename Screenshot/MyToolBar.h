#pragma once


#define MyToolBar_ID  600

class CMyToolBar
{
public:
	CMyToolBar();
	~CMyToolBar(void);
	
	BOOL CreateToolBar(HWND hWndParent);
	void AddChildStyle();
	void RemoveChildStyle();
	void ShowToolBar();
	void HideToolBar();
	void SetAtCurMousePlase();
	void SetShowPlace(int nCurPointX,int nCurPointY);
	HWND GetHWND();
private:
	HIMAGELIST m_hImageList;
	HWND m_hWndParent;
	HWND m_hWnd_toolbar;
};
