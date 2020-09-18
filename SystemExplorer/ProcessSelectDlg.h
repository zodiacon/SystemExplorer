#pragma once

#include "resource.h"
#include "VirtualListView.h"

class CProcessSelectDlg :
	public CDialogImpl<CProcessSelectDlg>,
	public CVirtualListView<CProcessSelectDlg>,
	public CCustomDraw<CProcessSelectDlg> {
public:
	friend struct CVirtualListView<CProcessSelectDlg>;

	struct ProcessInfo {
		CString Name;
		CString Path;
		DWORD Id;
		DWORD Session;
		int Image;
		CString UserName;
	};

	enum { IDD = IDD_PROCSELECT };

	int GetSelectedProcess(CString& name) const;
	CString GetColumnText(HWND, int row, int col) const;
	int GetRowImage(int row) const;

	BEGIN_MSG_MAP(CProcessSelectDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		COMMAND_ID_HANDLER(IDC_REFRESH, OnRefresh)
		NOTIFY_CODE_HANDLER(LVN_ITEMCHANGED, OnItemChanged)
		NOTIFY_CODE_HANDLER(NM_DBLCLK, OnDblClickItem)
		CHAIN_MSG_MAP(CVirtualListView<CProcessSelectDlg>)
		CHAIN_MSG_MAP(CCustomDraw<CProcessSelectDlg>)
	END_MSG_MAP()

protected:
	void DoSort(const SortInfo* si);

private:
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnItemChanged(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	//LRESULT OnGetDispInfo(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT OnDblClickItem(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/);
	LRESULT OnRefresh(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	void InitProcessList();
	void EnumProcesses();
	static bool CompareItems(const ProcessInfo& p1, const ProcessInfo& p2, int col, bool asc);

private:
	std::vector<ProcessInfo> m_Items;
	int m_SelectedPid = -1;
	CString m_Name;
	CListViewCtrl m_List;
	CImageList m_Images;
	WinSys::ProcessManager m_ProcMgr;
};

