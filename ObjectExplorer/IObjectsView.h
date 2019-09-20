#pragma once

#define OM_ACTIVATE_PAGE (WM_APP+1)

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
	virtual BOOL TrackPopupMenu(HMENU hMenu, HWND hWnd) = 0;
	virtual HIMAGELIST GetImageList() = 0;
	virtual int GetIconIndexByType(PCWSTR type) const = 0;
};
