#include "pch.h"
#include "EnvironmentDlg.h"
#include "DialogHelper.h"
#include "SortHelper.h"

CString CEnvironmentDlg::GetColumnText(HWND, int row, int col) const {
	auto& var = m_Vars[row];
	switch (col) {
		case 0: return var.first.c_str();
		case 1: return var.second.c_str();
	}
	return L"";
}

void CEnvironmentDlg::DoSort(const SortInfo* si) {
	if (!si)
		return;

	std::sort(m_Vars.begin(), m_Vars.end(), [&](auto const& v1, auto const& v2) {
		switch (si->SortColumn) {
			case 0: return SortHelper::SortStrings(v1.first, v2.first, si->SortAscending);
			case 1: return SortHelper::SortStrings(v1.second, v2.second, si->SortAscending);
		}
		return false;
		});
}

LRESULT CEnvironmentDlg::OnDialogColor(UINT, WPARAM, LPARAM, BOOL&) {
	return (LRESULT)::GetSysColorBrush(COLOR_WINDOW);
}

LRESULT CEnvironmentDlg::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&) {
	InitDynamicLayout();

	CString text;
	text.Format(L"Environment (PID: %u)", ::GetProcessId(m_hProcess));
	SetWindowText(text);
	DialogHelper::SetDialogIcon(this, IDI_VARS);

	m_List.Attach(GetDlgItem(IDC_LIST));
	m_List.InsertColumn(0, L"Name", LVCFMT_LEFT, 210);
	m_List.InsertColumn(1, L"Value", LVCFMT_LEFT, 300);
	m_List.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_INFOTIP);

	m_Vars = WinSys::Process::GetEnvironment(m_hProcess);
	m_List.SetItemCount((int)m_Vars.size());

	return 0;
}

LRESULT CEnvironmentDlg::OnCloseCmd(WORD, WORD wID, HWND, BOOL&) {
	EndDialog(wID);
	return 0;
}
