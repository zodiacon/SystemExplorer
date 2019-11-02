#include "stdafx.h"
#include "ProcessSelectDlg.h"
#include <string>

int CProcessSelectDlg::GetSelectedProcess() const {
	return m_SelectedPid;
}

LRESULT CProcessSelectDlg::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&) {
	CenterWindow(GetParent());
	m_List.Attach(GetDlgItem(IDC_PROCLIST));

	m_List.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);

	m_List.InsertColumn(0, L"Name", LVCFMT_LEFT, 150);
	m_List.InsertColumn(1, L"PID", LVCFMT_RIGHT, 70);
	m_List.InsertColumn(2, L"Session", LVCFMT_CENTER, 70);

	InitProcessList();

	return TRUE;
}

LRESULT CProcessSelectDlg::OnCloseCmd(WORD, WORD wID, HWND, BOOL&) {
	EndDialog(wID);
	return 0;
}

LRESULT CProcessSelectDlg::OnColumnClick(int, LPNMHDR hdr, BOOL&) {
	auto lv = reinterpret_cast<NMLISTVIEW*>(hdr);
	auto oldSortColumn = m_SortColumn;
	m_SortColumn = lv->iSubItem;
	if (m_SortColumn != oldSortColumn)
		m_SortAscending = true;
	else
		m_SortAscending = !m_SortAscending;
	m_List.SortItemsEx(CompareItems, reinterpret_cast<LPARAM>(this));

	return 0;
}

LRESULT CProcessSelectDlg::OnItemChanged(int, LPNMHDR, BOOL&) {
	auto selected = m_List.GetSelectedIndex();
	GetDlgItem(IDOK).EnableWindow(selected >= 0);
	if (selected >= 0) {
		CString text;
		m_List.GetItemText(selected, 1, text);
		m_SelectedPid = _wtoi(text);
	}

	return 0;
}

void CProcessSelectDlg::InitProcessList() {
	m_ObjMgr.EnumProcesses();
	m_List.DeleteAllItems();
	m_List.LockWindowUpdate();

	for (auto& pi : m_ObjMgr.GetProcesses()) {
		int n = m_List.InsertItem(0, pi.Name);
		m_List.SetItemText(n, 1, std::to_wstring(pi.Id).c_str());
		DWORD session = 0;
		::ProcessIdToSessionId(pi.Id, &session);
		m_List.SetItemText(n, 2, std::to_wstring(session).c_str());
	}

	m_List.LockWindowUpdate(FALSE);
}

int CProcessSelectDlg::CompareItems(LPARAM i1, LPARAM i2, LPARAM pThis) {
	return reinterpret_cast<CProcessSelectDlg*>(pThis)->CompareItems(i1, i2);
}

int CProcessSelectDlg::CompareItems(LPARAM i1, LPARAM i2) {
	CString name1, name2;
	auto n1 = static_cast<int>(i1);
	auto n2 = static_cast<int>(i2);
	int index = 0, result;

	switch (m_SortColumn) {
		case 0:
			m_List.GetItemText(n1, 0, name1);
			m_List.GetItemText(n2, 0, name2);
			result = name1.CompareNoCase(name2);
			break;

		case 1:
			index = 1;
		case 2:
			if (index == 0)
				index = 2;
			m_List.GetItemText(n1, index, name1);
			m_List.GetItemText(n2, index, name2);
			result = _wtoi(name1) - _wtoi(name2);
			break;
	}
	return m_SortAscending ? result : -result;
}
