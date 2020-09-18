#pragma once

#include "resource.h"
#include "VirtualListView.h"

class CTokenPropertiesDlg : 
	public CDialogImpl<CTokenPropertiesDlg>,
	public CVirtualListView<CTokenPropertiesDlg> {
public:
	enum { IDD = IDD_TOKEN };

	CTokenPropertiesDlg(HANDLE hToken) : m_hToken(hToken) {}

	void OnFinalMessage(HWND) override;
	CString GetColumnText(HWND h, int row, int col) const;
	void DoSort(const SortInfo* si);

	BEGIN_MSG_MAP(CTokenPropertiesDlg)
		MESSAGE_HANDLER(WM_CTLCOLORDLG, OnDialogColor)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnDialogColor)
		MESSAGE_HANDLER(WM_CTLCOLOREDIT, OnDialogColor)
		MESSAGE_HANDLER(WM_CTLCOLORSCROLLBAR, OnDialogColor)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		CHAIN_MSG_MAP(CVirtualListView<CTokenPropertiesDlg>)
	END_MSG_MAP()

private:
	void InitToken();

	LRESULT OnDialogColor(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

private:
	HANDLE m_hToken;
	CListViewCtrl m_GroupList, m_PrivList;
	std::vector<WinSys::TokenGroup> m_Groups;
	std::vector<WinSys::TokenPrivilege> m_Privileges;
};

