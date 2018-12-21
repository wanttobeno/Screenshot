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
#include "stdafx.h"
#include "popupmenu.h"
#define CPopUpMenu_ICON_SIZE_INCREMENT 2 // icon size increment used for shadow
#define CPopUpMenu_ICON_SHADOW_POSITION_DELTA 2
#define CPopUpMenu_TRANSPARENCY_INCREASE 130

//-----------------------------------------------------------------------------
// Name: CPopUpMenu
// Object: Constructor.
// Parameters :
//     in  : 
//     out :
//     return : 
//-----------------------------------------------------------------------------
CPopUpMenu::CPopUpMenu()
{
    this->CommonConstructor();
    this->ParentPopUpMenu=NULL;
}
CPopUpMenu::CPopUpMenu(CPopUpMenu* ParentPopUpMenu)
{
    this->CommonConstructor();
    this->ParentPopUpMenu=ParentPopUpMenu;
}
//#include <Uxtheme.h>
//#pragma comment (lib,"uxtheme.lib")
void CPopUpMenu::CommonConstructor()
{
    this->bThemingEnabledForVistaOrNewer = FALSE;
    if ( ((LOBYTE(LOWORD(::GetVersion())))>=6) ) // if vista or newer os
    {
        // dynamic loading to avoid static linking
        HMODULE hmod = ::GetModuleHandle(_T("uxtheme.dll"));
        if (!hmod)
             hmod = ::LoadLibrary(_T("uxtheme.dll"));
        if( hmod )
        {
            FARPROC pfIsThemeActive = ::GetProcAddress(hmod,"IsThemeActive");
            this->bThemingEnabledForVistaOrNewer = pfIsThemeActive();
        }
        
    }
    this->bAllowIconsEffects=TRUE;
    this->MenuSelectCallback=NULL;
    this->MenuSelectUserParam=NULL;
    this->MouseRightButtonUpCallback=NULL;
    this->MouseRightButtonUpUserParam=NULL;

    this->hPopUpMenu=::CreatePopupMenu();
    this->CurrentMenuId=0;
    MENUINFO mnfo;
    mnfo.cbSize = sizeof(mnfo);
    mnfo.fMask = MIM_STYLE | MIM_MENUDATA;
    mnfo.dwStyle = MNS_CHECKORBMP; //| MNS_AUTODISMISS can make menu to disappear when used on toolbar
    mnfo.dwMenuData = (ULONG_PTR)this;
    ::SetMenuInfo(this->hPopUpMenu, &mnfo);
}

//-----------------------------------------------------------------------------
// Name: ~CPopUpMenu
// Object: Destructor. SUBMENU OBJECT ARE NOT DELETED
// Parameters :
//     in  : 
//     out :
//     return : 
//-----------------------------------------------------------------------------
CPopUpMenu::~CPopUpMenu()
{
    std::vector<HBITMAP>::iterator Iterator;
    for (Iterator = this->ListLoadedBitmapToFree.begin(); Iterator!=this->ListLoadedBitmapToFree.end(); Iterator++)
    {
        ::DeleteObject(*Iterator);
    }
    ::DestroyMenu(this->hPopUpMenu);
}

//-----------------------------------------------------------------------------
// Name: GetControlHandle
// Object: return the handle to the control
// Parameters :
//     in  : 
//     out :
//     return : HWND handle to the control
//-----------------------------------------------------------------------------
HMENU CPopUpMenu::GetControlHandle()
{
    return this->hPopUpMenu;
}

//-----------------------------------------------------------------------------
// Name: GetParentPopUpMenu
// Object: return pointer to the parent menu or NULL if no parent menu
// Parameters :
//     in  : 
//     out :
//     return : pointer to the parent menu or NULL if no parent menu
//-----------------------------------------------------------------------------
CPopUpMenu* CPopUpMenu::GetParentPopUpMenu()
{
    return this->ParentPopUpMenu;
}

//-----------------------------------------------------------------------------
// Name: GetNextMenuId
// Object: get next unique menu Id for all Menu and SubMenu
// Parameters :
//     in  : 
//     out : 
//     return : next unique menu Id for all Menu and SubMenu
//-----------------------------------------------------------------------------
UINT CPopUpMenu::GetNextMenuId()
{
    if (this->ParentPopUpMenu)
        return this->ParentPopUpMenu->GetNextMenuId();
    else 
    {
        // this assume we'll never return a 0 menuId 
        // (as 0 is returned by TrackPopupMenuEx if no item is selected)
        this->CurrentMenuId++;
        return this->CurrentMenuId;
    }
}

//-----------------------------------------------------------------------------
// Name: GetNextMenuId
// Object: get max used unique menu Id for all Menu and SubMenu
// Parameters :
//     in  : 
//     out : 
//     return : max used unique menu Id for all Menu and SubMenu
//-----------------------------------------------------------------------------
UINT CPopUpMenu::GetMaxMenuId()
{
    if (this->ParentPopUpMenu)
        return this->ParentPopUpMenu->GetMaxMenuId();
    else 
    {
        return this->CurrentMenuId;
    }
}

//-----------------------------------------------------------------------------
// Name: GetSubMenu
// Object: get sub menu associated menu id
// Parameters :
//     in  : UINT MenuID : menu id
//     out :
//     return : CPopUpMenu object pointer on success, NULL if no sub menu associated (or on error)
//-----------------------------------------------------------------------------
CPopUpMenu* CPopUpMenu::GetSubMenu(UINT MenuID)
{
    MENUITEMINFO mii={0};
    mii.cbSize=sizeof(MENUITEMINFO);
    mii.fMask=MIIM_SUBMENU | MIIM_DATA;
    if (!::GetMenuItemInfo(this->hPopUpMenu,MenuID,FALSE,&mii))
        return NULL;
    if (!mii.hSubMenu)
        return NULL;

    MENUINFO mi={0};
    mi.cbSize = sizeof(MENUINFO);
    mi.fMask = MIM_MENUDATA;
    if (!::GetMenuInfo(mii.hSubMenu,&mi))
        return NULL;
    return (CPopUpMenu*)mi.dwMenuData;
}

//-----------------------------------------------------------------------------
// Name: SetIcon
// Object: set menu item icon (loads default icon size)
// Parameters :
//     in  : UINT MenuID : menu id
//           int IdIcon : resource icon id
//           HINSTANCE hInstance : module handle containing icon id
//     out : 
//     return : TRUE on success
//-----------------------------------------------------------------------------
BOOL CPopUpMenu::SetIcon(UINT MenuID,int IdIcon,HINSTANCE hInstance)
{
    return this->SetIcon(MenuID,IdIcon,hInstance,0,0);
}

//-----------------------------------------------------------------------------
// Name: SetIcon
// Object: set menu item icon
// Parameters :
//     in  : UINT MenuID : menu id
//           int IdIcon : resource icon id
//           int Width : icon width
//           int Height : icon height
//           HINSTANCE hInstance : module handle containing icon id
//     out : 
//     return : TRUE on success
//-----------------------------------------------------------------------------
BOOL CPopUpMenu::SetIcon(UINT MenuID,int IdIcon,HINSTANCE hInstance,int Width,int Height)
{
    return this->SetIcon(MenuID,(HICON)::LoadImage(hInstance, MAKEINTRESOURCE(IdIcon),IMAGE_ICON,Width,Height,LR_SHARED));
}

//-----------------------------------------------------------------------------
// Name: SetIcon
// Object: set menu item icon
// Parameters :
//     in  : UINT MenuID : menu id
//           HICON hIcon : icon handle
//     out : 
//     return : TRUE on success
//-----------------------------------------------------------------------------
BOOL CPopUpMenu::SetIcon(UINT MenuID,HICON hIcon)
{
    MENUITEMINFO mii={0};
    mii.cbSize=sizeof(MENUITEMINFO);
    if (!hIcon)
        return FALSE;

    this->SetMenuItemBitmapInfo(&mii,hIcon);

    return ::SetMenuItemInfo(this->hPopUpMenu,MenuID,FALSE,&mii);
}

//-----------------------------------------------------------------------------
// Name: Add
// Object: 
// Parameters :
//     in  : TCHAR* Name : name of menu
//     out : 
//     return : Id of item menu (used to remove it),0 if failed
//-----------------------------------------------------------------------------
UINT CPopUpMenu::Add(TCHAR* Name)
{
    return this->Add(Name,NULL,this->GetItemCount());
}

//-----------------------------------------------------------------------------
// Name: Add
// Object: 
// Parameters :
//     in  : TCHAR* Name : name of menu
//           UINT Index : 0 based menu index
//     out : 
//     return : Id of item menu (used to remove it),0 if failed
//-----------------------------------------------------------------------------
UINT CPopUpMenu::Add(TCHAR* Name,UINT Index)
{
    return this->Add(Name,NULL,Index);
}

//-----------------------------------------------------------------------------
// Name: Add
// Object: 
// Parameters :
//     in  : TCHAR* Name : name of menu
//           HICON hIcon : icon handle
//     out : 
//     return : Id of item menu (used to remove it),0 if failed
//-----------------------------------------------------------------------------
UINT CPopUpMenu::Add(TCHAR* Name,HICON hIcon)
{
    return this->Add(Name,hIcon,this->GetItemCount());
}

//-----------------------------------------------------------------------------
// Name: Add
// Object: 
// Parameters :
//     in  : TCHAR* Name : name of menu
//           int IdIcon : icon id
//           HINSTANCE hInstance : hinstance containing icon id
//     out : 
//     return : Id of item menu (used to remove it),0 if failed
//-----------------------------------------------------------------------------
UINT CPopUpMenu::Add(TCHAR* Name,int IdIcon,HINSTANCE hInstance)
{
    // load icon
    HICON hIcon=(HICON)::LoadImage(hInstance, MAKEINTRESOURCE(IdIcon),IMAGE_ICON,0,0,LR_SHARED);
    return this->Add(Name,hIcon,this->GetItemCount());
}

//-----------------------------------------------------------------------------
// Name: Add
// Object: 
// Parameters :
//     in  : TCHAR* Name : name of menu
//           int IdIcon : icon id
//           HINSTANCE hInstance : hinstance containing icon id
//           UINT Index : 0 based menu index
//     out : 
//     return : Id of item menu (used to remove it),0 if failed
//-----------------------------------------------------------------------------
UINT CPopUpMenu::Add(TCHAR* Name,int IdIcon,HINSTANCE hInstance,UINT Index)
{
    return this->Add(Name,IdIcon,hInstance,0,0,Index);
}
//-----------------------------------------------------------------------------
// Name: Add
// Object: 
// Parameters :
//     in  : TCHAR* Name : name of menu
//           int IdIcon : icon id
//           HINSTANCE hInstance : hinstance containing icon id
//           int Width : icon width
//           int Height : icon height
//           UINT Index : 0 based menu index
//     out : 
//     return : Id of item menu (used to remove it),0 if failed
//-----------------------------------------------------------------------------
UINT CPopUpMenu::Add(TCHAR* Name,int IdIcon,HINSTANCE hInstance,int Width,int Height,UINT Index)
{
    // load icon
    HICON hIcon=(HICON)::LoadImage(hInstance, MAKEINTRESOURCE(IdIcon),IMAGE_ICON,Width,Height,LR_SHARED);
    return this->Add(Name,hIcon,Index);
}

//-----------------------------------------------------------------------------
// Name: Add
// Object: 
// Parameters :
//     in  : TCHAR* Name : name of menu
//           HICON hIcon : icon handle
//           UINT Index : 0 based menu index
//     out : 
//     return : Id of item menu (used to remove it),0 if failed
//-----------------------------------------------------------------------------
UINT CPopUpMenu::Add(TCHAR* Name,HICON hIcon,UINT Index)
{
    UINT MenuId=this->GetNextMenuId();

    MENUITEMINFO mii={0};
    mii.cbSize=sizeof(MENUITEMINFO);
    mii.fMask = MIIM_FTYPE|MIIM_STRING|MIIM_ID;
    mii.fType = MFT_STRING;
    mii.dwTypeData=Name;
    mii.wID=MenuId;

    this->SetMenuItemBitmapInfo(&mii,hIcon);

    // insert menu
    if (!::InsertMenuItem(this->hPopUpMenu,
                        Index,
                        TRUE,
                        &mii
                        )
        )
        return 0;

    return MenuId;
}

//-----------------------------------------------------------------------------
// Name: ReplaceMenuId
// Object: change menu id. 
//         WARNING NO CHECK IS DONE IF NEW ID IS ALREADY EXISTING : if it's the case
//          you will run into trouble for the multiple id access
//        
// Parameters :
//     in  : UINT OldMenuID : old menu id
//           UINT NewMenuID : new menu id
//     out : 
//     return : TRUE on success
//-----------------------------------------------------------------------------
BOOL CPopUpMenu::ReplaceMenuId(UINT OldMenuID,UINT NewMenuID)
{
    MENUITEMINFO mii={0};
    mii.cbSize=sizeof(MENUITEMINFO);
    mii.fMask=MIIM_ID;
    mii.wID=NewMenuID;
    return ::SetMenuItemInfo(this->hPopUpMenu,OldMenuID,FALSE,&mii);
}

//-----------------------------------------------------------------------------
// Name: SetCheckedState
// Object: 
// Parameters :
//     in  : UINT MenuID : menu id
//           BOOL bChecked : new checked state
//     out : 
//     return : 
//-----------------------------------------------------------------------------
void CPopUpMenu::SetCheckedState(UINT MenuID,BOOL bChecked)
{
    UINT uCheck;
    if(bChecked)
        uCheck=MF_CHECKED;
    else
        uCheck=MF_UNCHECKED;
    ::CheckMenuItem(this->hPopUpMenu,MenuID,MF_BYCOMMAND|uCheck);
}

//-----------------------------------------------------------------------------
// Name: IsChecked
// Object: 
// Parameters :
//     in  : UINT MenuID : menu id
//     out : 
//     return : TRUE if menu item is checked
//-----------------------------------------------------------------------------
BOOL CPopUpMenu::IsChecked(UINT MenuID)
{
    if (MF_CHECKED & ::GetMenuState(this->hPopUpMenu,MenuID,MF_BYCOMMAND))
        return TRUE;
    else
        return FALSE;
}

//-----------------------------------------------------------------------------
// Name: SetEnabledState
// Object: 
// Parameters :
//     in  : UINT MenuID : menu id
//           BOOL bEnabled : new enabled state
//     out : 
//     return : 
//-----------------------------------------------------------------------------
void CPopUpMenu::SetEnabledState(UINT MenuID,BOOL bEnabled)
{
    UINT uEnable;
    if(bEnabled)
        uEnable=MF_ENABLED;
    else
        uEnable=MF_GRAYED;
    ::EnableMenuItem(this->hPopUpMenu,MenuID,MF_BYCOMMAND|uEnable);
}

//-----------------------------------------------------------------------------
// Name: IsEnabled
// Object: 
// Parameters :
//     in  : UINT MenuID : menu id
//     out : 
//     return : 
//-----------------------------------------------------------------------------
BOOL CPopUpMenu::IsEnabled(UINT MenuID)
{
    UINT uEnable=::GetMenuState(this->hPopUpMenu,MenuID,MF_BYCOMMAND);
    if (uEnable==(UINT)-1)
        return FALSE;

    return !(uEnable & (MF_DISABLED|MF_GRAYED));
}


//-----------------------------------------------------------------------------
// Name: Remove
// Object: 
// Parameters :
//     in  : UINT MenuID
//     out : 
//     return : 
//-----------------------------------------------------------------------------
void CPopUpMenu::Remove(UINT MenuID)
{
    ::RemoveMenu(this->hPopUpMenu,MenuID,MF_BYCOMMAND);
}

//-----------------------------------------------------------------------------
// Name: AddSeparator
// Object: 
// Parameters :
//     in  : 
//     out : 
//     return : Id of item menu (used to remove it),0 if failed
//-----------------------------------------------------------------------------
UINT CPopUpMenu::AddSeparator()
{
    UINT MenuId=this->GetNextMenuId();
    if(!::AppendMenu( this->hPopUpMenu,MF_SEPARATOR,MenuId,_T("")))
        return 0;
    return MenuId;
}

//-----------------------------------------------------------------------------
// Name: AddSeparator
// Object: 
// Parameters :
//     in  : UINT Index : menu index
//     out : 
//     return : Id of item menu (used to remove it),0 if failed
//-----------------------------------------------------------------------------
UINT CPopUpMenu::AddSeparator(UINT Index)
{
    UINT MenuId=this->GetNextMenuId();
    if (!::InsertMenu( this->hPopUpMenu,Index, MF_SEPARATOR|MF_BYPOSITION,MenuId,_T("")))
        return 0;
    return MenuId;
}

//-----------------------------------------------------------------------------
// Name: AddSubMenu
// Object: 
// Parameters :
//     in  : TCHAR* SubMenuName : Name of submenu
//           CPopUpMenu* SubMenu : pointer to an initialized CPopUpMenu
//           UINT Index : SubMenu 0 based index in the current menu
//     out : 
//     return : Id of item menu (used to remove it),0 if failed
//-----------------------------------------------------------------------------
UINT CPopUpMenu::AddSubMenu(TCHAR* SubMenuName,CPopUpMenu* SubMenu)
{
    return this->AddSubMenu(SubMenuName,SubMenu,(UINT)-1);
}

//-----------------------------------------------------------------------------
// Name: AddSubMenu
// Object: 
// Parameters :
//     in  : TCHAR* SubMenuName : Name of submenu
//           CPopUpMenu* SubMenu : pointer to an initialized CPopUpMenu
//           UINT Index : SubMenu 0 based index in the current menu
//     out : 
//     return : Id of item menu (used to remove it),0 if failed
//-----------------------------------------------------------------------------
UINT CPopUpMenu::AddSubMenu(TCHAR* SubMenuName,CPopUpMenu* SubMenu,UINT Index)
{
    return this->AddSubMenu(SubMenuName,SubMenu,0,Index);
}

//-----------------------------------------------------------------------------
// Name: AddSubMenu
// Object: add submenu with icon
// Parameters :
//     in  : TCHAR* SubMenuName : Name of submenu
//           CPopUpMenu* SubMenu : pointer to an initialized CPopUpMenu
//           int IdIcon : icon id
//           HINSTANCE hInstance : instance containing icon id
//           UINT Index : SubMenu 0 based index in the current menu
//     out : 
//     return : Id of item menu (used to remove it),0 if failed
//-----------------------------------------------------------------------------
UINT CPopUpMenu::AddSubMenu(TCHAR* SubMenuName,CPopUpMenu* SubMenu,int IdIcon,HINSTANCE hInstance)
{
    return this->AddSubMenu(SubMenuName,SubMenu,IdIcon,hInstance,(UINT)-1);
}

//-----------------------------------------------------------------------------
// Name: AddSubMenu
// Object: add submenu with icon
// Parameters :
//     in  : TCHAR* SubMenuName : Name of submenu
//           CPopUpMenu* SubMenu : pointer to an initialized CPopUpMenu
//           int IdIcon : icon id
//           HINSTANCE hInstance : instance containing icon id
//           UINT Index : SubMenu 0 based index in the current menu
//     out : 
//     return : Id of item menu (used to remove it),0 if failed
//-----------------------------------------------------------------------------
UINT CPopUpMenu::AddSubMenu(TCHAR* SubMenuName,CPopUpMenu* SubMenu,int IdIcon,HINSTANCE hInstance,UINT Index)
{
    HICON hIcon=(HICON)::LoadImage(hInstance, MAKEINTRESOURCE(IdIcon),IMAGE_ICON,0,0,LR_SHARED);
    return this->AddSubMenu(SubMenuName,SubMenu,hIcon,Index);
}

//-----------------------------------------------------------------------------
// Name: AddSubMenu
// Object: add submenu with icon
// Parameters :
//     in  : TCHAR* SubMenuName : Name of submenu
//           CPopUpMenu* SubMenu : pointer to an initialized CPopUpMenu
//           HICON hIcon : submenu icon handle
//     out : 
//     return : Id of item menu (used to remove it),0 if failed
//-----------------------------------------------------------------------------
UINT CPopUpMenu::AddSubMenu(TCHAR* SubMenuName,CPopUpMenu* SubMenu,HICON hIcon)
{
    return this->AddSubMenu(SubMenuName,SubMenu,hIcon,(UINT)-1);
}

//-----------------------------------------------------------------------------
// Name: AddSubMenu
// Object: add submenu with icon
// Parameters :
//     in  : TCHAR* SubMenuName : Name of submenu
//           CPopUpMenu* SubMenu : pointer to an initialized CPopUpMenu
//           HICON hIcon : submenu icon handle
//           UINT Index : SubMenu 0 based index in the current menu
//     out : 
//     return : Id of item menu (used to remove it),0 if failed
//-----------------------------------------------------------------------------
UINT CPopUpMenu::AddSubMenu(TCHAR* SubMenuName,CPopUpMenu* SubMenu,HICON hIcon,UINT Index)
{
    MENUITEMINFO MenuItem={0};
    UINT MenuId=this->GetNextMenuId();

    MenuItem.cbSize=sizeof(MENUITEMINFO);
    MenuItem.fMask=MIIM_SUBMENU|MIIM_STRING|MIIM_ID;
    MenuItem.hSubMenu=SubMenu->GetControlHandle();
    MenuItem.fType=MFT_STRING;
    MenuItem.cch=(UINT)_tcslen(SubMenuName);
    MenuItem.dwTypeData=SubMenuName;
    MenuItem.wID=MenuId;

    this->SetMenuItemBitmapInfo(&MenuItem,hIcon);

    if(!::InsertMenuItem(this->hPopUpMenu,Index,TRUE,&MenuItem))
        return 0;
    return MenuId;
}

void CPopUpMenu::SetMenuItemBitmapInfo(MENUITEMINFO* pMenuItem,HICON hIcon)
{
    if (hIcon)
    {
        // on vista or newer os, if theming is enabled selected icon is not color-inverted,
        // so there's no need to draw icon by ourselves, let system do it
        if (this->bThemingEnabledForVistaOrNewer)
        {
            SIZE IconSize;
            ICONINFO IconInfo;
            if (::GetIconInfo(hIcon,&IconInfo))
            {
                IconSize.cx = IconInfo.xHotspot*2;
                IconSize.cy = IconInfo.yHotspot*2;

                // free icon infos
                ::DeleteObject(IconInfo.hbmColor);
                ::DeleteObject(IconInfo.hbmMask);
            }
            else
            {
                IconSize.cx = ::GetSystemMetrics(SM_CXICON);
                IconSize.cy = ::GetSystemMetrics(SM_CYICON);
            }


            BITMAPV5HEADER bi;
            void *lpBits;

            ZeroMemory(&bi,sizeof(BITMAPV5HEADER));
            bi.bV5Size           = sizeof(BITMAPV5HEADER);
            bi.bV5Width           = IconSize.cx;
            bi.bV5Height          = IconSize.cy;
            bi.bV5Planes = 1;
            bi.bV5BitCount = 32;
            bi.bV5Compression = BI_BITFIELDS;
            // The following mask specification specifies a supported 32 BPP
            // alpha format for Windows XP.
            bi.bV5RedMask   =  0x00FF0000;
            bi.bV5GreenMask =  0x0000FF00;
            bi.bV5BlueMask  =  0x000000FF;
            bi.bV5AlphaMask =  0xFF000000; 

            HDC hdcScreen = ::GetDC(NULL);
            HDC hMemDC = ::CreateCompatibleDC(hdcScreen);
            HBITMAP hBitmap = ::CreateDIBSection(hdcScreen, (BITMAPINFO *)&bi, DIB_RGB_COLORS, (void **)&lpBits, NULL, (DWORD)0);
            // HBITMAP hBitmap = ::CreateCompatibleBitmap(hMemDC, IconSize.cx, IconSize.cy);
            HBITMAP hOldBitmap = (HBITMAP)::SelectObject(hMemDC, hBitmap);
            HBRUSH hBackgroundBrush = ::CreateSolidBrush(::GetSysColor(COLOR_MENU));
            //::DrawIcon(hMemDC, 0, 0, hIcon);
            ::DrawIconEx(hMemDC,0,0,hIcon,IconSize.cx,IconSize.cy,0,
                /*0*/  hBackgroundBrush, /* some icons needs a background color*/
                DI_NORMAL /*| DI_NOMIRROR*/ );

            ::SelectObject(hMemDC, hOldBitmap);
            ::DeleteObject(hBackgroundBrush);
            ::DeleteDC(hMemDC);
            ::ReleaseDC(NULL,hdcScreen);

            pMenuItem->fMask|=MIIM_BITMAP;
            pMenuItem->hbmpItem = hBitmap;
            ListLoadedBitmapToFree.push_back(hBitmap);
// to do : MenuItem.hbmpItem with must be deleted
        }
        else
        {
            // before vista icon of selected item in menu was inverted by default.
            // to avoid this we have to manually draw icon by oursleves inside callback

            // set item data for WM_MEASUREITEM and WM_DRAWITEM messages
            pMenuItem->fMask|=MIIM_DATA;
            pMenuItem->dwItemData=(ULONG_PTR)hIcon;

            // set owner drawn for icon drawing
            pMenuItem->fMask|=MIIM_BITMAP;
            pMenuItem->hbmpItem = HBMMENU_CALLBACK;

            //// for full owner drawn type use
            //pMenuItem->fType|= MFT_OWNERDRAW; 
        }
    }
}

//-----------------------------------------------------------------------------
// Name: GetItemCount
// Object: 
// Parameters :
//     in  : 
//     out : 
//     return : number of item in the menu
//-----------------------------------------------------------------------------
int CPopUpMenu::GetItemCount()
{
    return ::GetMenuItemCount(this->hPopUpMenu);
}

//-----------------------------------------------------------------------------
// Name: GetText
// Object: 
// Parameters :
//     in  : UINT MenuID : menu ID
//           int pszTextMaxSize : pszText max size in TCHAR. If 0, GetItemCount returns the 
//                                number of required TCHAR
//     out : TCHAR* pszText : Pointer to the buffer that receives the null-terminated string
//     return : If the function succeeds, the return value specifies the number of characters copied to the buffer,
//              not including the terminating null character
//              If the function fails, the return value is zero
//-----------------------------------------------------------------------------
int CPopUpMenu::GetText(UINT MenuID,OUT TCHAR* pszText,IN int pszTextMaxSize)
{
    return ::GetMenuString(this->hPopUpMenu,MenuID,pszText,pszTextMaxSize,MF_BYCOMMAND);
}

//-----------------------------------------------------------------------------
// Name: SetText
// Object: 
// Parameters :
//     in  : UINT MenuID : menu ID
//           TCHAR* pszText : Pointer to the input buffer
//     out : 
//     return : TRUE on success, FALSE on error
//-----------------------------------------------------------------------------
BOOL CPopUpMenu::SetText(UINT MenuID,TCHAR* pszText)
{
    MENUITEMINFO MenuItemInfo={0};
    MenuItemInfo.cbSize=sizeof(MENUITEMINFO);
    MenuItemInfo.fMask=MIIM_STRING;
    MenuItemInfo.fType=MFT_STRING;
    MenuItemInfo.cch=(UINT)_tcslen(pszText);
    MenuItemInfo.dwTypeData=pszText;

    return ::SetMenuItemInfo(this->hPopUpMenu,MenuID,FALSE,&MenuItemInfo);
}

//-----------------------------------------------------------------------------
// Name: GetID
// Object: retrieve menu ID from menu index
// Parameters :
//     in  : UINT Menu index : index
//     out : 
//     return : -1 if not found, else menu id
//-----------------------------------------------------------------------------
int CPopUpMenu::GetID(UINT MenuIndex)
{
    MENUITEMINFO MenuItemInfo={0};
    MenuItemInfo.cbSize=sizeof(MENUITEMINFO);
    MenuItemInfo.fMask=MIIM_ID;
    if (!::GetMenuItemInfo(this->hPopUpMenu,MenuIndex,TRUE,&MenuItemInfo))
        return -1;
    return (int)MenuItemInfo.wID;
    // return GetMenuItemID(this->hPopUpMenu,MenuIndex);
}

//-----------------------------------------------------------------------------
// Name: GetIndex
// Object: retrieve menu index from menu ID
// Parameters :
//     in  : UINT MenuId : menu id
//     out : 
//     return : -1 if not found, else menu index
//-----------------------------------------------------------------------------
int CPopUpMenu::GetIndex(UINT MenuId)
{
    int Cnt;
    for (Cnt=0;Cnt<this->GetItemCount();Cnt++)
    {
        if ((UINT)this->GetID(Cnt)==MenuId)
        {
            return Cnt;
        }
    }
    return -1;
}

//-----------------------------------------------------------------------------
// Name: Show
// Object: 
// Parameters :
//     in  : int x : Horizontal location of the shortcut menu, in screen coordinates
//           int y : Vertical location of the shortcut menu, in screen coordinates
//           HWND hOwner : owner control handle
//     out : 
//     return : 0 if no item selected, item id else
//-----------------------------------------------------------------------------
UINT CPopUpMenu::Show(int x,int y, HWND hOwner)
{
    return this->Show(x,y,hOwner,FALSE);
}
//-----------------------------------------------------------------------------
// Name: Show
// Object: 
// Parameters :
//     in  : int x : Horizontal location of the shortcut menu, in screen coordinates
//           int y : Vertical location of the shortcut menu, in screen coordinates
//           HWND hOwner : owner control handle
//           BOOL PositionRelativeToOwner : TRUE if position is relative to owner
//                                          FALSE for an absolute position
//     out : 
//     return : 0 if no item selected, item id else
//-----------------------------------------------------------------------------
UINT CPopUpMenu::Show(int x,int y, HWND hOwner,BOOL PositionRelativeToOwner)
{
    return this->Show(x,y,hOwner,PositionRelativeToOwner,FALSE);
}

//-----------------------------------------------------------------------------
// Name: Show
// Object: 
// Parameters :
//     in  : int x : Horizontal location of the shortcut menu, in screen coordinates
//           int y : Vertical location of the shortcut menu, in screen coordinates
//           HWND hOwner : owner control handle
//           BOOL PositionRelativeToOwner : TRUE if position is relative to owner
//                                          FALSE for an absolute position
//           BOOL ShowUpper : TRUE to show menu upper y point
//     out : 
//     return : 0 if no item selected, item id else
//-----------------------------------------------------------------------------
UINT CPopUpMenu::Show(int x,int y, HWND hOwner,BOOL PositionRelativeToOwner,BOOL ShowUpper)
{
    if (PositionRelativeToOwner)
    {
        RECT Rect;
        ::GetWindowRect(hOwner,&Rect);
        x+=Rect.left;
        y+=Rect.top;
    }
    // menu flag must be without TPM_RIGHTBUTTON flag to get WM_MENURBUTTONUP messages
    UINT fuFlags=TPM_LEFTALIGN|TPM_RETURNCMD;
    if (ShowUpper)
    {
        fuFlags|=TPM_BOTTOMALIGN;
    }
    else
    {
        fuFlags|=TPM_TOPALIGN;
    }

    UINT TrackPopupMenuRet;
    HWND hWnd=hOwner;



    // install a subclassing wndproc
    UINT_PTR uIdSubclass=0;
    DWORD_PTR dwRefData=(DWORD_PTR)this;
    ::SetWindowSubclass(hWnd,CPopUpMenu::SubClassWndProc,uIdSubclass,dwRefData);

    ::SetForegroundWindow(hWnd); // in case of use with notify icons
    TrackPopupMenuRet=(UINT)::TrackPopupMenuEx(this->hPopUpMenu,fuFlags,x,y,hOwner,NULL);

    if (TrackPopupMenuRet==0)
    {
        if (::GetLastError()==ERROR_POPUP_ALREADY_ACTIVE)
            TrackPopupMenuRet=(UINT)::TrackPopupMenuEx(this->hPopUpMenu,fuFlags|TPM_RECURSE,x,y,hOwner,NULL);
    }

    // remove subclassing wndproc
    ::RemoveWindowSubclass(hWnd,CPopUpMenu::SubClassWndProc,uIdSubclass);


    // MSDN : 
    //However, when the current window is the foreground window,
    //the second time this menu is displayed, it displays and then immediately disappears.
    //To correct this, you must force a task switch to the application that called 
    //TrackPopupMenu at some time in the near future. 
    //This is done by posting a benign message to the window or thread
    ::PostMessage(hWnd, WM_NULL, 0, 0);

    return TrackPopupMenuRet;
}

//-----------------------------------------------------------------------------
// Name: OnMeasureItem
// Object: return needed pRect size to display picture (full item  menu for MFT_OWNERDRAW)
// Parameters :
//     in  : HWND hwnd : hwnd
//     inout : LPMEASUREITEMSTRUCT lpmis : pointer measured item struct
//     out : 
//     return : TRUE if message has been proceed
//-----------------------------------------------------------------------------
BOOL CPopUpMenu::OnMeasureItem(HWND hwnd, LPMEASUREITEMSTRUCT lpmis) 
{ 
    UNREFERENCED_PARAMETER(hwnd);

    // assume type is a menu
    if (lpmis->CtlType!=ODT_MENU)
        return FALSE;

    // assume to have item associated data
    if (!lpmis->itemData)
        return FALSE;

    // extract icon infos from item data
    ICONINFO IconInfo;
    if (!::GetIconInfo((HICON)lpmis->itemData, &IconInfo))
        return FALSE;

    // get bitmap from icon
    BITMAP Bitmap;
    ::GetObject(IconInfo.hbmColor, sizeof(Bitmap), &Bitmap);

    // free icon infos
    ::DeleteObject(IconInfo.hbmColor);
    ::DeleteObject(IconInfo.hbmMask);

    // use bitmap size infos
    lpmis->itemWidth = Bitmap.bmWidth;
    lpmis->itemHeight = Bitmap.bmHeight;

    if (this->bAllowIconsEffects)
    {
        lpmis->itemWidth+= CPopUpMenu_ICON_SIZE_INCREMENT;
        lpmis->itemHeight+= CPopUpMenu_ICON_SIZE_INCREMENT;
    }

    return TRUE;
} 



HICON CreateGrayscaleIcon(HICON hIcon)
{
    HICON		hGrayIcon = NULL;
    HDC			hdc = NULL, hMemDC1 = NULL, hMemDC2 = NULL;
    HBITMAP		hOldBmp1 = NULL, hOldBmp2 = NULL;
    ICONINFO	csII, csGrayII;
    BOOL		bRetValue = FALSE;

    bRetValue = ::GetIconInfo(hIcon, &csII);
    if (bRetValue == FALSE)	
        return NULL;

    hdc = ::GetDC(NULL);
    if (!hdc)
        return NULL;

    hMemDC1 = ::CreateCompatibleDC(hdc);
    if (!hMemDC1)
    {
        ::ReleaseDC(NULL, hdc);
        return NULL;
    }

    hMemDC2 = ::CreateCompatibleDC(hdc);
    if (!hMemDC2)
    {
        ::DeleteDC(hMemDC1);
        ::ReleaseDC(NULL, hdc);
        return NULL;
    }


    BITMAPV5HEADER bi;
    DWORD Width;
    DWORD Height;
    void *lpBits;

    Width  = csII.xHotspot*2;  // width of Icon
    Height = csII.yHotspot*2;  // height of Icon

    ZeroMemory(&bi,sizeof(BITMAPV5HEADER));
    bi.bV5Size           = sizeof(BITMAPV5HEADER);
    bi.bV5Width           = Width;
    bi.bV5Height          = Height;
    bi.bV5Planes = 1;
    bi.bV5BitCount = 32;
    bi.bV5Compression = BI_BITFIELDS;
    // The following mask specification specifies a supported 32 BPP
    // alpha format for Windows XP.
    bi.bV5RedMask   =  0x00FF0000;
    bi.bV5GreenMask =  0x0000FF00;
    bi.bV5BlueMask  =  0x000000FF;
    bi.bV5AlphaMask =  0xFF000000; 

    // Create the DIB section with an alpha channel.
    csGrayII.hbmColor = ::CreateDIBSection(hdc, (BITMAPINFO *)&bi, DIB_RGB_COLORS, (void **)&lpBits, NULL, (DWORD)0);

    if (csGrayII.hbmColor)
    {
        hOldBmp1 = (HBITMAP)::SelectObject(hMemDC1, csII.hbmColor);
        hOldBmp2 = (HBITMAP)::SelectObject(hMemDC2, csGrayII.hbmColor);

        ::BitBlt(hMemDC2, 0, 0, Width, Height, hMemDC1, 0, 0, SRCCOPY);

        DWORD		dwLoopY = 0, dwLoopX = 0;
        COLORREF	crPixel = 0;
        BYTE		byNewPixel = 0;
        DWORD *lpdwPixel;
        lpdwPixel = (DWORD *)lpBits;

        for (dwLoopY = 0; dwLoopY < Height; dwLoopY++)
        {
            for (dwLoopX = 0; dwLoopX < Width; dwLoopX++)
            {
                crPixel = (*lpdwPixel) & 0x00FFFFFF;

                if (crPixel)
                    byNewPixel = (BYTE)((GetRValue(crPixel) * 0.299) + (GetGValue(crPixel) * 0.587) + (GetBValue(crPixel) * 0.114));
                else
                    byNewPixel = 0;

                // Clear the color bits
                *lpdwPixel &= 0xFF000000;
                // Set the new alpha bits
                *lpdwPixel |= RGB(byNewPixel, byNewPixel, byNewPixel) ;

                lpdwPixel++;
            } // for
        } // for

        ::SelectObject(hMemDC1, hOldBmp1);
        ::SelectObject(hMemDC2, hOldBmp2);

        csGrayII.hbmMask = csII.hbmMask;

        csGrayII.fIcon = TRUE;
        hGrayIcon = ::CreateIconIndirect(&csGrayII);
        ::DeleteObject(csGrayII.hbmColor);
    } // if

    ::DeleteObject(csII.hbmColor);
    ::DeleteObject(csII.hbmMask);
    ::DeleteDC(hMemDC1);
    ::DeleteDC(hMemDC2);
    ::ReleaseDC(NULL, hdc);

    return hGrayIcon;
} // End of CreateGrayscaleIcon


HICON CreateAlphaIcon(HICON hIcon,int AlphaChange,HBRUSH BackgroundBrush)
{
    HDC hMemDC;
    DWORD Width, Height;
    BITMAPV5HEADER bi;
    HBITMAP hBitmap, hOldBitmap;
    void *lpBits;
    DWORD x,y;
    HICON hNewIcon = NULL;

    ICONINFO OriginalIconInfos;
    if(!::GetIconInfo(hIcon,&OriginalIconInfos))
        return NULL;

    ICONINFO ii;
    ii.fIcon = TRUE;  // Change fIcon to TRUE to create an alpha icon
    ii.xHotspot = 0;
    ii.yHotspot = 0;

    if (!AlphaChange)
    {
        ii.hbmMask = OriginalIconInfos.hbmMask;
        ii.hbmColor = OriginalIconInfos.hbmColor;

        // Create the alpha cursor with the alpha DIB section.
        hNewIcon = ::CreateIconIndirect(&ii);

        ::DeleteObject(OriginalIconInfos.hbmColor);
        ::DeleteObject(OriginalIconInfos.hbmMask);

        return hNewIcon;
    }

    Width  = OriginalIconInfos.xHotspot*2;  // width of Icon
    Height = OriginalIconInfos.yHotspot*2;  // height of Icon

    ZeroMemory(&bi,sizeof(BITMAPV5HEADER));
    bi.bV5Size           = sizeof(BITMAPV5HEADER);
    bi.bV5Width           = Width;
    bi.bV5Height          = Height;
    bi.bV5Planes = 1;
    bi.bV5BitCount = 32;
    bi.bV5Compression = BI_BITFIELDS;
    // The following mask specification specifies a supported 32 BPP
    // alpha format for Windows XP.
    bi.bV5RedMask   =  0x00FF0000;
    bi.bV5GreenMask =  0x0000FF00;
    bi.bV5BlueMask  =  0x000000FF;
    bi.bV5AlphaMask =  0xFF000000; 

    HDC hdc;
    hdc = ::GetDC(NULL);

    // Create the DIB section with an alpha channel.
    hBitmap = ::CreateDIBSection(hdc, (BITMAPINFO *)&bi, DIB_RGB_COLORS, (void **)&lpBits, NULL, (DWORD)0);
    hMemDC = ::CreateCompatibleDC(hdc);

    // select bitmap
    hOldBitmap = (HBITMAP)::SelectObject(hMemDC, hBitmap);

    // Draw something on the DIB section.
    ::DrawIconEx(hMemDC,0,0,hIcon,Width,Height,0,
        /*0*/  BackgroundBrush, /* some icons needs a background color*/
        DI_NORMAL);

    // Create an empty mask bitmap.
    HBITMAP hMonoBitmap = ::CreateBitmap(Width,Height,1,1,NULL);

    DWORD *lpdwPixel;
    lpdwPixel = (DWORD *)lpBits;

    BYTE bAlpha;
    for (x=0;x<Width;x++)
    {
        for (y=0;y<Height;y++)
        {
            bAlpha = ( (*lpdwPixel & 0xFF000000) >> 24);

            if (AlphaChange<0)
            {
                if (!bAlpha)
                    bAlpha = 0xFF;
                // increase transparency
                bAlpha = (BYTE)__max(bAlpha+AlphaChange,1); // 0 : no transparency
            }
            else
            {
                // reduce transparency only for not fully transparent bytes
                if (bAlpha)
                {
                    bAlpha = (BYTE)__min(bAlpha+AlphaChange,0xff);
                }
            }

            // Clear the alpha bits
            *lpdwPixel &= 0x00FFFFFF;
            // Set the new alpha bits
            *lpdwPixel |= ( bAlpha <<24) ;

            lpdwPixel++;
        }
    }

    ii.hbmMask = hMonoBitmap;
    ii.hbmColor = hBitmap;

    // Create the alpha cursor with the alpha DIB section.
    hNewIcon = ::CreateIconIndirect(&ii);

    ::SelectObject(hMemDC, hOldBitmap);
    ::DeleteDC(hMemDC);

    ::DeleteObject(hBitmap);
    ::DeleteObject(hMonoBitmap); 

    ::DeleteObject(OriginalIconInfos.hbmColor);
    ::DeleteObject(OriginalIconInfos.hbmMask);

    ::ReleaseDC(NULL,hdc);

    return hNewIcon;
} 

//-----------------------------------------------------------------------------
// Name: CreateAlphaOrGreyscaleIcon
// Object: This function creates an alpha modified or a greyscale icon starting from a given icon.
//         The resulting icon will have the same size of the original one.
//          Notice the return icon handle must be free by calling ::DeleteObject
// Parameters :
//     in  : HWND hwnd : hwnd
//           int Alpha [-255;255] negative value to increase alpha transparency
//           BOOL bGreyscale TRUE to convert to greyscale
//     out : 
//     return : NULL on error hIcon on success
//-----------------------------------------------------------------------------
HICON CreateAlphaOrGreyscaleIcon(HICON hIcon,BOOL bGreyIcon,int AlphaChange,HBRUSH BackgroundBrush)
{
    HICON hGreyIcon = hIcon;
    HICON hNewIcon;

    if (bGreyIcon)
        hGreyIcon = CreateGrayscaleIcon(hIcon);

    hNewIcon = CreateAlphaIcon(hGreyIcon,AlphaChange,BackgroundBrush);

    if (bGreyIcon)
        ::DestroyIcon(hGreyIcon);

    return hNewIcon;
}


//-----------------------------------------------------------------------------
// Name: OnDrawItem
// Object: do picture rendering (full item rendering if MFT_OWNERDRAW)
// Parameters :
//     in  : HWND hwnd : hwnd
//           LPDRAWITEMSTRUCT lpdis : pointer display item info struct
//     out : 
//     return : TRUE if message has been proceed
//-----------------------------------------------------------------------------
BOOL CPopUpMenu::OnDrawItem(HWND hwnd, LPDRAWITEMSTRUCT lpdis) 
{ 
    UNREFERENCED_PARAMETER(hwnd);

    // assume type is a menu
    if (lpdis->CtlType!=ODT_MENU)
        return FALSE;

    // assume handle match our menu or sub menu
    if (this->hPopUpMenu!=(HMENU)lpdis->hwndItem)
    {
        if (!this->IsSubMenu(this->hPopUpMenu,(HMENU)lpdis->hwndItem))
            return FALSE;
    }

    // assume to have item associated data
    if (!lpdis->itemData)
        return FALSE;

    ICONINFO IconInfo;
    // extract icon infos from item data
    if (!::GetIconInfo((HICON)lpdis->itemData, &IconInfo))
        return FALSE;

    UINT Width = IconInfo.xHotspot*2;
    UINT Height = IconInfo.yHotspot*2;

    // free icon infos
    ::DeleteObject(IconInfo.hbmColor);
    ::DeleteObject(IconInfo.hbmMask);

    //// render bitmap
    //::DrawIconEx(lpdis->hDC, lpdis->rcItem.left, lpdis->rcItem.top,(HICON) lpdis->itemData, Bitmap.bmWidth, Bitmap.bmHeight, 0, NULL, DI_NORMAL);

    BOOL bSelected = ( (lpdis->itemState & ODS_SELECTED) == ODS_SELECTED);
    BOOL bDisabled = ( (lpdis->itemState & ODS_DISABLED) == ODS_DISABLED);
    BOOL bChecked = ( (lpdis->itemState & ODS_CHECKED) == ODS_CHECKED);
    //HBITMAP hBmp = LoadBitmap (NULL, MAKEINTRESOURCE(OBM_CHECK));
    // bChecked is always FALSE, else we have to play with MFT_OWNERDRAW

    HBRUSH CurrentBrush = NULL;
    UINT Delta=0;

    if (this->bAllowIconsEffects)
        Delta =( (bSelected && !bDisabled) ? (CPopUpMenu_ICON_SIZE_INCREMENT-CPopUpMenu_ICON_SHADOW_POSITION_DELTA)/2 : CPopUpMenu_ICON_SIZE_INCREMENT/2 );

    // if item is disabled, draw only grey scale icon
    if ( bDisabled )
    {
        HICON hGreyIcon=CreateGrayscaleIcon((HICON) lpdis->itemData);
        ::DrawIconEx(lpdis->hDC, lpdis->rcItem.left+Delta, lpdis->rcItem.top+Delta,hGreyIcon, Width, Height, 0, NULL, DI_NORMAL);
        ::DestroyIcon(hGreyIcon);
        return TRUE;
    }

    if (this->bAllowIconsEffects)
    {
        // if item is checked, draw rectangle around icon
        if (bChecked)
        {
            // get system colors
            COLORREF CheckedColor;
            COLORREF MenuHilightColor = ::GetSysColor(COLOR_MENUHILIGHT);
            COLORREF MenuColor = ::GetSysColor(COLOR_MENU);

            // get luminosity difference
            int LuminosityDelta = // grey highlighted menu color - grey menu color
            (int)((GetRValue(MenuHilightColor) * 0.299) + (GetGValue(MenuHilightColor) * 0.587) + (GetBValue(MenuHilightColor) * 0.114))
            - (int)((GetRValue(MenuColor) * 0.299) + (GetGValue(MenuColor) * 0.587) + (GetBValue(MenuColor) * 0.114));
            LuminosityDelta/=2;

            // forge checked color
            CheckedColor = RGB( 
                                __max( __min( GetRValue(MenuHilightColor) - LuminosityDelta , 255 ) , 0 ),
                                __max( __min( GetGValue(MenuHilightColor) - LuminosityDelta , 255 ) , 0 ),
                                __max( __min( GetBValue(MenuHilightColor) - LuminosityDelta , 255 ) , 0 )
                                );

            // create pen and select it
            HPEN hPen = ::CreatePen(PS_SOLID, 1, (bSelected ? ::GetSysColor(COLOR_MENU) : ::GetSysColor(COLOR_MENUHILIGHT)));
            HPEN OldPen= (HPEN)::SelectObject(lpdis->hDC,hPen);
            // create brush and select it
            CurrentBrush = ::CreateSolidBrush(CheckedColor);
            HBRUSH OldBrush = (HBRUSH)::SelectObject(lpdis->hDC,CurrentBrush);

            // draw rectangle around icon with selected pen and brush
            ::Rectangle(lpdis->hDC, lpdis->rcItem.left,lpdis->rcItem.top,lpdis->rcItem.right,lpdis->rcItem.bottom);

            // select original brush and pen
            ::SelectObject(lpdis->hDC,OldBrush);
            ::SelectObject(lpdis->hDC,OldPen);
        }
        else
        {
            if ( bSelected )
                CurrentBrush = ::CreateSolidBrush(::GetSysColor(COLOR_MENUHILIGHT));
            else
                CurrentBrush = ::CreateSolidBrush(::GetSysColor(COLOR_MENU));
        }

        if ( bSelected )
        {
            UINT DeltaShadow=(CPopUpMenu_ICON_SIZE_INCREMENT+CPopUpMenu_ICON_SHADOW_POSITION_DELTA)/2;
            HICON hAlphaIcon=CreateAlphaOrGreyscaleIcon((HICON) lpdis->itemData,TRUE,-CPopUpMenu_TRANSPARENCY_INCREASE,CurrentBrush);
            ::DrawIconEx(lpdis->hDC, lpdis->rcItem.left+DeltaShadow, lpdis->rcItem.top+DeltaShadow,hAlphaIcon, Width, Height, 0, NULL, DI_NORMAL);
            ::DestroyIcon(hAlphaIcon);
        }
    }

    // render bitmap
    // ::DrawState(lpdis->hDC, (HBRUSH)NULL, NULL,(LPARAM)(HICON)lpdis->itemData, 0, lpdis->rcItem.left+Delta, lpdis->rcItem.top+Delta, Bitmap.bmWidth, Bitmap.bmHeight, DSS_NORMAL | DST_ICON);
    ::DrawIconEx(lpdis->hDC, lpdis->rcItem.left+Delta, lpdis->rcItem.top+Delta,(HICON) lpdis->itemData, Width, Height, 0, NULL, DI_NORMAL);

    if (CurrentBrush)
        ::DeleteObject(CurrentBrush);

    return TRUE;
} 

//-----------------------------------------------------------------------------
// Name: IsSubMenu
// Object: check if hSubMenu is a submenu of hMenu
// Parameters :
//     in  : HMENU hMenu : a menu handle
//           HMENU hSubMenu : sub menu handle to check
//     out : 
//     return : TRUE hMenu is a submenu of current menu
//-----------------------------------------------------------------------------
BOOL CPopUpMenu::IsSubMenu(HMENU hMenu,HMENU hSubMenu)
{
    int NbItems=::GetMenuItemCount(hMenu);

    MENUITEMINFO mii;
    mii.cbSize=sizeof(MENUITEMINFO);
    mii.fMask=MIIM_SUBMENU;
    for (int Cnt=0;Cnt<NbItems;Cnt++)
    {
        if (!::GetMenuItemInfo(hMenu,Cnt,TRUE,&mii))
            continue;

        if (!mii.hSubMenu)
            continue;

        if (mii.hSubMenu==hSubMenu)
            return TRUE;

        if (this->IsSubMenu(mii.hSubMenu,hSubMenu))
            return TRUE;

    }

    return FALSE;
}

//-----------------------------------------------------------------------------
// Name: SetMouseRightButtonUpCallback
// Object: set callback for WM_MENURBUTTONUP message
// Parameters :
//     in  : 
//     out : 
//     return : TRUE on success
//-----------------------------------------------------------------------------
BOOL CPopUpMenu::SetMouseRightButtonUpCallback(pfMessageCallback Callback,PVOID UserParam)
{
    // check callback validity
    if (::IsBadCodePtr((FARPROC)Callback))
        return FALSE;

    this->MouseRightButtonUpCallback=Callback;
    this->MouseRightButtonUpUserParam=UserParam;

    return TRUE;
}

//-----------------------------------------------------------------------------
// Name: SetMenuSelectCallback
// Object: set callback for WM_MENUSELECT message
// Parameters :
//     in  : 
//     out : 
//     return : TRUE on success
//-----------------------------------------------------------------------------
BOOL CPopUpMenu::SetMenuSelectCallback(pfMessageCallback Callback,PVOID UserParam)
{
    // check callback validity
    if (::IsBadCodePtr((FARPROC)Callback))
        return FALSE;

    this->MenuSelectCallback=Callback;
    this->MenuSelectUserParam=UserParam;

    return TRUE;
}

//-----------------------------------------------------------------------------
// Name: OnMouseRightButtonUp
// Object: called on WM_MENURBUTTONUP message, call callback if any
// Parameters :
//     in  : 
//     out : 
//     return :
//-----------------------------------------------------------------------------
void CPopUpMenu::OnMouseRightButtonUp(WPARAM wParam, LPARAM lParam)
{
    if (this->MouseRightButtonUpCallback)
        this->MouseRightButtonUpCallback(wParam,lParam,this->MouseRightButtonUpUserParam);
}

//-----------------------------------------------------------------------------
// Name: OnMenuSelect
// Object: called on WM_MENUSELECT message, call callback if any
// Parameters :
//     in  : 
//     out : 
//     return :
//-----------------------------------------------------------------------------
void CPopUpMenu::OnMenuSelect(WPARAM wParam, LPARAM lParam)
{
    if (this->MenuSelectCallback)
        this->MenuSelectCallback(wParam,lParam,this->MenuSelectUserParam);
}

//-----------------------------------------------------------------------------
// Name: SubClassWndProc
// Object: subclassing window proc
// Parameters :
//     in  : WndProc param
//     out : 
//     return : 
//-----------------------------------------------------------------------------
LRESULT CALLBACK CPopUpMenu::SubClassWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,UINT_PTR uIdSubclass,DWORD_PTR dwRefData)
{
    UNREFERENCED_PARAMETER(uIdSubclass);
    CPopUpMenu* pPopUpMenu=(CPopUpMenu*)dwRefData;
    if (!pPopUpMenu)
        return ::DefSubclassProc(hWnd,uMsg,wParam,lParam);

    switch (uMsg)
    {
    case WM_MEASUREITEM: 
        return pPopUpMenu->OnMeasureItem(hWnd, (LPMEASUREITEMSTRUCT) lParam); 

    case WM_DRAWITEM: 
        return pPopUpMenu->OnDrawItem(hWnd, (LPDRAWITEMSTRUCT) lParam); 

    case WM_MENURBUTTONUP:
        // FIXME : WM_MENURBUTTONUP is not send for sub menus when they are expanded
        pPopUpMenu->OnMouseRightButtonUp(wParam,lParam);
        break;
    case WM_MENUSELECT:
        pPopUpMenu->OnMenuSelect(wParam,lParam);
        break;
    }

    return ::DefSubclassProc(hWnd,uMsg,wParam,lParam);
}