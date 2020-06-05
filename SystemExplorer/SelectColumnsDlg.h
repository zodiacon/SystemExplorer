#pragma once

#include "DialogHelper.h"
#include "VirtualListView.h"
#include "resource.h"
#include "ColumnManager.h"

class CSelectColumnsDlg : 
	public CDialogImpl<CSelectColumnsDlg>,
	public CVirtualListView <CSelectColumnsDlg> {
public:
	enum { IDD = IDD_COLUMNS };

	CSelectColumnsDlg(ColumnManager* cm);

	CString GetColumnText(HWND, int row, int col) const;
	ListViewRowCheck IsRowChecked(int row) const;
	void DoSort(const SortInfo* si);

	BEGIN_MSG_MAP(CSelectColumnsDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		NOTIFY_CODE_HANDLER(LVN_KEYDOWN, OnListKeyDown)
		NOTIFY_CODE_HANDLER(NM_CLICK, OnListClick)
		NOTIFY_CODE_HANDLER(TVN_ITEMCHANGED, OnTreeItemChanged)
		COMMAND_RANGE_HANDLER(IDC_TYPE, IDC_TYPE + 1, OnSelectList)
		CHAIN_MSG_MAP(CVirtualListView<CSelectColumnsDlg>)
	END_MSG_MAP()

private:
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnListKeyDown(int /*idCtrl*/, LPNMHDR hdr, BOOL& /*bHandled*/);
	LRESULT OnListClick(int /*idCtrl*/, LPNMHDR hdr, BOOL& /*bHandled*/);
	LRESULT OnSelectList(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnTreeItemChanged(int /*idCtrl*/, LPNMHDR hdr, BOOL& /*bHandled*/);

	const ColumnManager::ColumnInfo& GetColumn(int index) const;
	void ToggleSelected();
	void UpdateVisibility();
	void CheckItemCategory(const CString& category, bool check);
	void CheckTreeChildren(HTREEITEM hParent, bool check);

private:
	struct TreeItem {
		CString Text;
		HTREEITEM hItem;
	};

	struct Item {
		CString Name, Category;
		int Index;
		HTREEITEM hItem;
		bool Visible;
	};
	std::vector<Item> m_Items;
	std::vector<TreeItem> m_TreeItems;

	ColumnManager* m_ColMgr;
	CListViewCtrl m_List;
	CTreeViewCtrl m_Tree;
	inline static bool s_UseList{ true };
	bool m_Init{ true };
};

