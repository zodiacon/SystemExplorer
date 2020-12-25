#pragma once

#include "resource.h"
#include "VirtualListView.h"
#include <Token.h>

class CTokenPropertiesDlg : 
	public CDialogImpl<CTokenPropertiesDlg>,
	public CVirtualListView<CTokenPropertiesDlg> {
public:
	enum { IDD = IDD_TOKEN };

	CTokenPropertiesDlg(HANDLE hToken) : m_Token(hToken) {}

	void OnFinalMessage(HWND) override;
	CString GetColumnText(HWND h, int row, int col) const;
	void DoSort(const SortInfo* si);
	int GetRowImage(HWND, int row) const;

	BEGIN_MSG_MAP(CTokenPropertiesDlg)
		NOTIFY_HANDLER(IDC_PRIVILEGES, LVN_ITEMCHANGED, OnPrivItemChanged)
		MESSAGE_HANDLER(WM_CTLCOLORDLG, OnDialogColor)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnDialogColor)
		MESSAGE_HANDLER(WM_CTLCOLOREDIT, OnDialogColor)
		MESSAGE_HANDLER(WM_CTLCOLORSCROLLBAR, OnDialogColor)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDC_ENABLE, OnEnablePrivilege)
		COMMAND_ID_HANDLER(IDC_DISABLE, OnDisablePrivilege)
		CHAIN_MSG_MAP(CVirtualListView<CTokenPropertiesDlg>)
	END_MSG_MAP()

private:
	void InitToken();

	LRESULT OnDialogColor(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnEnablePrivilege(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnDisablePrivilege(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnPrivItemChanged(int /*idCtrl*/, LPNMHDR hdr, BOOL& /*bHandled*/);

private:
	WinSys::Token m_Token;
	CListViewCtrl m_GroupList, m_PrivList;
	std::vector<WinSys::TokenGroup> m_Groups;
	std::vector<WinSys::TokenPrivilege> m_Privileges;
};

