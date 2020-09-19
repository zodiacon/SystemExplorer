#pragma once

#include "resource.h"
#include "VirtualListView.h"

class CEnvironmentDlg : 
	public CDialogImpl<CEnvironmentDlg>,
	public CDynamicDialogLayout<CEnvironmentDlg>,
	public CVirtualListView<CEnvironmentDlg> {
public:
	enum { IDD = IDD_ENVVARS };

	CString GetColumnText(HWND, int row, int col) const;
	void DoSort(const SortInfo* si);

	CEnvironmentDlg(HANDLE hProcess) : m_hProcess(hProcess) {}

	BEGIN_MSG_MAP(CEnvironmentDlg)
		MESSAGE_HANDLER(WM_CTLCOLORDLG, OnDialogColor)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnDialogColor)
		MESSAGE_HANDLER(WM_CTLCOLOREDIT, OnDialogColor)
		MESSAGE_HANDLER(WM_CTLCOLORSCROLLBAR, OnDialogColor)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		COMMAND_ID_HANDLER(IDC_COPY, OnCopy)
		COMMAND_ID_HANDLER(IDC_COPYALL, OnCopyAll)
		CHAIN_MSG_MAP(CDynamicDialogLayout<CEnvironmentDlg>)
		CHAIN_MSG_MAP(CVirtualListView<CEnvironmentDlg>)
	END_MSG_MAP()

private:
	LRESULT OnDialogColor(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCopy(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCopyAll(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

private:
	HANDLE m_hProcess;
	CListViewCtrl m_List;
	std::vector<std::pair<std::wstring, std::wstring>> m_Vars;
};

