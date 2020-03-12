#pragma once

#define OM_ACTIVATE_PAGE (WM_APP+1)

const DWORD ListViewDefaultStyle = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
LVS_REPORT | LVS_SHOWSELALWAYS | LVS_OWNERDATA | LVS_SINGLESEL | LVS_SHAREIMAGELISTS;

struct ObjectInfo;

struct IObjectsView abstract {
	virtual CString GetTitle() = 0;
	virtual bool ShowObject(ObjectInfo*) {
		return true;
	}

	virtual bool GetContextMenu(UINT& menuId, int& index) const {
		return false;
	}

	virtual void OnDoubleClick() {
	}
};

struct IMainFrame {
	virtual BOOL TrackPopupMenu(HMENU hMenu, HWND hWnd, POINT* pt = nullptr, UINT flags = 0) = 0;
	virtual HIMAGELIST GetImageList() = 0;
	virtual int GetIconIndexByType(PCWSTR type) const = 0;
	virtual void ShowAllHandles(PCWSTR typename) = 0;
	virtual void ShowAllObjects(PCWSTR typename) = 0;
	virtual CUpdateUIBase* GetUpdateUI() = 0;
	virtual int AddBand(HWND hControl, PCWSTR title = nullptr) = 0;
	virtual bool RemoveBand(int index) = 0;
	virtual bool AddToCommandBar(UINT id, UINT icon, HICON = nullptr) = 0;
	virtual bool AddMenu(HMENU hMenu) = 0;
	virtual bool AddToolBar(HWND hToolbar) = 0;
	virtual bool RemoveMenu(HMENU hMenu) = 0;
	virtual bool RemoveToolBar(HWND hToolbar) = 0;
};
