#include "pch.h"
#include "TokenPropertiesDlg.h"
#include "DialogHelper.h"
#include "FormatHelper.h"
#include "TokenObjectType.h"

void CTokenPropertiesDlg::OnFinalMessage(HWND) {
}

void CTokenPropertiesDlg::InitToken() {
	WinSys::Token token(m_hToken);
	auto username = token.GetUserNameAndSid();
	SetDlgItemText(IDC_USERNAME, username.first.c_str());
	SetDlgItemText(IDC_USERSID, username.second.AsString().c_str());

	SetDlgItemText(IDC_VIRT, FormatHelper::VirtualizationStateToString(token.GetVirtualizationState()));
	SetDlgItemText(IDC_INTEGRITY, FormatHelper::IntegrityToString(token.GetIntegrityLevel()));
	SetDlgItemInt(IDC_SESSION, token.GetSessionId());

	auto stats = token.GetStats();

	CString text;
	text.Format(L"0x%llX", *(ULONG64*)(&stats.AuthenticationId));
	SetDlgItemText(IDC_LOGONSESSION, text);
	SetDlgItemText(IDC_ELEVATED, token.IsElevated() ? L"Yes" : L"No");

	auto groups = token.EnumGroups();
	text.Format(L"Groups: %u\n", (ULONG)groups.size());
	SetDlgItemText(IDC_GROUP_COUNT, text);
}

LRESULT CTokenPropertiesDlg::OnDialogColor(UINT, WPARAM, LPARAM, BOOL&) {
	return (LRESULT)::GetSysColorBrush(COLOR_WINDOW);
}

LRESULT CTokenPropertiesDlg::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&) {
	DialogHelper::SetDialogIcon(this, IDI_TOKEN);

	InitToken();

	return 0;
}

LRESULT CTokenPropertiesDlg::OnCloseCmd(WORD, WORD wID, HWND, BOOL&) {
	EndDialog(wID);
	return 0;
}
