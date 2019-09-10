#pragma once

struct ObjectInfoEx;

struct IObjectsView abstract {
	virtual CString GetTitle() = 0;
	virtual bool ShowObject(ObjectInfoEx*) {
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
};
