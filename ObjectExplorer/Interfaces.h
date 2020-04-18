#pragma once

#define OM_ACTIVATE_PAGE (WM_APP+1)

const DWORD ListViewDefaultStyle = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
	LVS_REPORT | LVS_SHOWSELALWAYS | LVS_OWNERDATA | LVS_SINGLESEL | LVS_SHAREIMAGELISTS;

struct IMainFrame {
	virtual BOOL TrackPopupMenu(HMENU hMenu, HWND hWnd, POINT* pt = nullptr, UINT flags = 0) = 0;
	virtual HIMAGELIST GetImageList() = 0;
	virtual int GetIconIndexByType(PCWSTR type) const = 0;
	virtual void ShowAllHandles(PCWSTR typename) = 0;
	virtual void ShowAllObjects(PCWSTR typename) = 0;
	virtual CUpdateUIBase* GetUpdateUI() = 0;
};
