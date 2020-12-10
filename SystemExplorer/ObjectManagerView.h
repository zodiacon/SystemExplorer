#pragma once

#include "VirtualListView.h"
#include "Interfaces.h"
#include "resource.h"
#include "ViewBase.h"

class CObjectManagerView : 
	public CViewBase<CObjectManagerView>,
	public CVirtualListView<CObjectManagerView> {
public:
	CObjectManagerView(IMainFrame* frame);

	CString GetDirectoryPath() const;
	void OnFinalMessage(HWND) override;
	void DoSort(const SortInfo* si);
	bool IsSortable(int column) const;
	CString GetColumnText(HWND, int row, int col);
	int GetRowImage(HWND, int row) const;
	bool IsUpdating() const {
		return false;
	}

	BEGIN_MSG_MAP(CObjectManagerView)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		NOTIFY_CODE_HANDLER(TVN_SELCHANGED, OnTreeSelectionChanged)
		COMMAND_ID_HANDLER(ID_VIEW_REFRESH, OnRefresh)
		COMMAND_ID_HANDLER(ID_EDIT_SECURITY, OnEditSecurity)
		CHAIN_MSG_MAP(CVirtualListView<CObjectManagerView>)
		CHAIN_MSG_MAP(CViewBase<CObjectManagerView>)
	END_MSG_MAP()

private:
	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnTreeSelectionChanged(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT OnRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnEditSecurity(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	void InitTree();
	void UpdateList(bool newNode);
	void EnumDirectory(CTreeItem root, const CString& path);

	struct ObjectData {
		CString Name, FullName, Type;
	};
	static bool CompareItems(const ObjectData& data1, const ObjectData& data2, int col, bool asc);

private:
	CTreeViewCtrlEx m_Tree;
	CListViewCtrl m_List;
	std::vector<ObjectData> m_Objects;
	CSplitterWindow m_Splitter;
};

