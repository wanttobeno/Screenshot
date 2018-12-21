/*
Copyright (C) 2004 Jacquelin POTIER <jacquelin.potier@free.fr>
Dynamic aspect ratio code Copyright (C) 2004 Jacquelin POTIER <jacquelin.potier@free.fr>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

//-----------------------------------------------------------------------------
// Object: class helper for popupmenu control
//-----------------------------------------------------------------------------

#pragma once
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501 // for xp os
#endif
#include <windows.h>
#pragma warning (push)
#pragma warning(disable : 4005)// for '_stprintf' : macro redefinition in tchar.h
#include <TCHAR.h>
#pragma warning (pop)
#include <vector>

#include <commctrl.h>
#pragma comment (lib,"comctl32.lib")

class CPopUpMenu
{
public:
    typedef void (*pfMessageCallback)(WPARAM wParam, LPARAM lParam,PVOID UserParam);
    CPopUpMenu();
    CPopUpMenu(CPopUpMenu* ParentPopUpMenu);
    ~CPopUpMenu();

    HMENU GetControlHandle();
    CPopUpMenu* GetParentPopUpMenu();

    UINT Add(TCHAR* Name);
    UINT Add(TCHAR* Name,UINT Index);
    UINT Add(TCHAR* Name,HICON hIcon);
    UINT Add(TCHAR* Name,HICON hIcon,UINT Index);
    UINT Add(TCHAR* Name,int IdIcon,HINSTANCE hInstance);
    UINT Add(TCHAR* Name,int IdIcon,HINSTANCE hInstance,UINT Index);
    UINT Add(TCHAR* Name,int IdIcon,HINSTANCE hInstance,int Width,int Height,UINT Index);

    UINT AddSeparator();
    UINT AddSeparator(UINT Index);

    UINT AddSubMenu(TCHAR* SubMenuName,CPopUpMenu* SubMenu,UINT Index);
    UINT AddSubMenu(TCHAR* SubMenuName,CPopUpMenu* SubMenu);
    UINT AddSubMenu(TCHAR* SubMenuName,CPopUpMenu* SubMenu,int IdIcon,HINSTANCE hInstance,UINT Index);
    UINT AddSubMenu(TCHAR* SubMenuName,CPopUpMenu* SubMenu,int IdIcon,HINSTANCE hInstance);
    UINT AddSubMenu(TCHAR* SubMenuName,CPopUpMenu* SubMenu,HICON hIcon,UINT Index);
    UINT AddSubMenu(TCHAR* SubMenuName,CPopUpMenu* SubMenu,HICON hIcon);

    void SetCheckedState(UINT MenuID,BOOL bChecked);
    BOOL IsChecked(UINT MenuID);
    void SetEnabledState(UINT MenuID,BOOL bEnabled);
    BOOL IsEnabled(UINT MenuID);
    BOOL SetText(UINT MenuID,TCHAR* pszText);
    BOOL SetIcon(UINT MenuID,int IdIcon,HINSTANCE hInstance);
    BOOL SetIcon(UINT MenuID,int IdIcon,HINSTANCE hInstance,int Width,int Height);
    BOOL SetIcon(UINT MenuID,HICON hIcon);
    int  GetText(UINT MenuID,TCHAR* pszText,int pszTextMaxSize);
    CPopUpMenu* GetSubMenu(UINT MenuID);

    void Remove(UINT MenuID);

    int GetItemCount();
    int GetID(UINT MenuIndex);
    int GetIndex(UINT MenuId);

    UINT Show(int x,int y, HWND hOwner);
    UINT Show(int x,int y, HWND hOwner,BOOL PositionRelativeToOwner);
    UINT Show(int x,int y, HWND hOwner,BOOL PositionRelativeToOwner,BOOL ShowUpper);

    UINT GetNextMenuId();
    UINT GetMaxMenuId();
    BOOL ReplaceMenuId(UINT OldMenuID,UINT NewMenuID);

    BOOL SetMouseRightButtonUpCallback(pfMessageCallback Callback,PVOID UserParam);
    BOOL SetMenuSelectCallback(pfMessageCallback Callback,PVOID UserParam);

    BOOL bAllowIconsEffects;
private:
    CPopUpMenu* ParentPopUpMenu;
    HMENU hPopUpMenu;
    int CurrentMenuId;
    BOOL bThemingEnabledForVistaOrNewer;
    std::vector<HBITMAP> ListLoadedBitmapToFree;

    pfMessageCallback MouseRightButtonUpCallback;
    PVOID MouseRightButtonUpUserParam;
    pfMessageCallback MenuSelectCallback;
    PVOID MenuSelectUserParam;

    void CommonConstructor();
    void SetMenuItemBitmapInfo(MENUITEMINFO* pMenuItem,HICON hIcon);
    BOOL IsSubMenu(HMENU hMenu,HMENU hSubMenu);
    BOOL OnMeasureItem(HWND hwnd, LPMEASUREITEMSTRUCT lpmis);
    BOOL OnDrawItem(HWND hwnd, LPDRAWITEMSTRUCT lpdis);
    void OnMouseRightButtonUp(WPARAM wParam, LPARAM lParam);
    void OnMenuSelect(WPARAM wParam, LPARAM lParam);
    void FreeItemMemory(UINT MenuID);
    void FreeItemBitmap(UINT MenuID);
    static LRESULT CALLBACK SubClassWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,UINT_PTR uIdSubclass,DWORD_PTR dwRefData);
};
