// aboutdlg.cpp : implementation of the CAboutDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "aboutdlg.h"
#include "DialogHelper.h"

LRESULT CAboutDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/) {
	DialogHelper::AdjustOKCancelButtons(this);

	CenterWindow(GetParent());
	return TRUE;
}

LRESULT CAboutDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	EndDialog(wID);
	return 0;
}

LRESULT CAboutDlg::OnClickSyslink(int, LPNMHDR, BOOL&) {
	::ShellExecute(nullptr, L"open", L"https://github.com/zodiacon/objectexplorer", nullptr, nullptr, SW_SHOWDEFAULT);

	return 0;
}
