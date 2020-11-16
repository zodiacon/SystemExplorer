#pragma once

#include "Interfaces.h"
#include "VirtualListView.h"
#include "ViewBase.h"

class CDeviceManagerView :
	public CCustomDraw<CDeviceManagerView>,
	public CVirtualListView<CDeviceManagerView>,
	public CViewBase<CDeviceManagerView> {
public:
	DECLARE_WND_CLASS(nullptr)

	CDeviceManagerView(IMainFrame* frame);

	bool IsSortable(int col) const;
	void DoSort(const SortInfo* si);
	
	BEGIN_MSG_MAP(CDeviceManagerView)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		NOTIFY_CODE_HANDLER(TVN_SELCHANGED, OnTreeSelectionChanged)
		NOTIFY_CODE_HANDLER(LVN_GETDISPINFO, OnListGetDispInfo)
		COMMAND_ID_HANDLER(ID_VIEW_REFRESH, OnRefresh)
		COMMAND_ID_HANDLER(ID_EDIT_FIND, OnFind)
		COMMAND_ID_HANDLER(ID_EDIT_FIND_NEXT, OnFindNext)
		CHAIN_MSG_MAP(CVirtualListView<CDeviceManagerView>)
		CHAIN_MSG_MAP(CViewBase<CDeviceManagerView>)
	END_MSG_MAP()

private:
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnTreeSelectionChanged(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT OnListGetDispInfo(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT OnRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnFind(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnFindNext(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

private:
	enum class ItemType {
		None, String, MultiString, Dword, Boolean, Guid, PowerData
	};
	struct ItemData {
		PCWSTR Name;
		ItemType Type;
		WinSys::DeviceRegistryPropertyType PropertyType;
		CString Value;
	};
	struct ClassItemData {
		PCWSTR Name;
		ItemType Type;
		WinSys::DeviceClassRegistryPropertyType PropertyType;
		CString Value;
	};

	void Refresh();
	void UpdateList();
	LRESULT GetDeviceClassInfo(LVITEM& item) const;
	CString GetDeviceClassProperty(const GUID* guid, int index) const;
	LRESULT GetDeviceInfo(LVITEM& item) const;
	CString GetDeviceProperty(WinSys::DeviceInfo& di, int index) const;
	static bool CompareItems(const CString& i1, const CString& i2, bool asc);

private:
	std::unique_ptr<WinSys::DeviceManager> m_DevMgr;
	std::vector<WinSys::DeviceInfo> m_Devices;
	std::vector<ItemData> m_Items;
	std::vector<ClassItemData> m_ClassItems;
	std::unordered_map<HTREEITEM, GUID> m_DeviceClasses;
	WinSys::DeviceInfo* m_SelectedDevice;
	const GUID* m_SelectedClass;
	CTreeViewCtrlEx m_Tree;
	CListViewCtrl m_List;
	CSplitterWindow m_Splitter;
	CToolBarCtrl m_Toolbar;
	int m_ComputerIcon;
};

