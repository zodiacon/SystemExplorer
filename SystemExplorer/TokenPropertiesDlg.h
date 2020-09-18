#pragma once

#include "resource.h"

class CTokenPropertiesDlg : public CDialogImpl<CTokenPropertiesDlg> {
public:
	enum { IDD = IDD_TOKEN };

	CTokenPropertiesDlg(HANDLE hToken) : m_hToken(hToken) {}

	void OnFinalMessage(HWND) override;

	BEGIN_MSG_MAP(CTokenPropertiesDlg)
		MESSAGE_HANDLER(WM_CTLCOLORDLG, OnDialogColor)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnDialogColor)
		MESSAGE_HANDLER(WM_CTLCOLOREDIT, OnDialogColor)
		MESSAGE_HANDLER(WM_CTLCOLORSCROLLBAR, OnDialogColor)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
	END_MSG_MAP()

private:
	void InitToken();

	LRESULT OnDialogColor(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

private:
	HANDLE m_hToken;
};

