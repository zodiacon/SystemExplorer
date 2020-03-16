#include "stdafx.h"
#include "ProcessSelectDlg.h"
#include <string>
#include <TlHelp32.h>
#include <algorithm>
#include "SortHelper.h"

int CProcessSelectDlg::GetSelectedProcess(CString& name) const {
	name = m_Name;
	return m_SelectedPid;
}

void CProcessSelectDlg::DoSort(const SortInfo* si) {
	std::sort(m_Items.begin(), m_Items.end(), [si](const auto& p1, const auto& p2) {
		return CompareItems(p1, p2, si->SortColumn, si->SortAscending);
		});
}

LRESULT CProcessSelectDlg::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&) {
	CenterWindow(GetParent());
	m_List.Attach(GetDlgItem(IDC_PROCLIST));

	m_List.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);

	m_List.InsertColumn(0, L"Name", LVCFMT_LEFT, 170);
	m_List.InsertColumn(1, L"PID", LVCFMT_RIGHT, 130);
	m_List.InsertColumn(2, L"Session", LVCFMT_CENTER, 70);

	m_Images.Create(16, 16, ILC_COLOR32, 32, 16);
	m_Images.AddIcon(AtlLoadSysIcon(IDI_APPLICATION));
	m_List.SetImageList(m_Images, LVSIL_SMALL);

	InitProcessList();

	return TRUE;
}

LRESULT CProcessSelectDlg::OnCloseCmd(WORD, WORD wID, HWND, BOOL&) {
	EndDialog(wID);
	return 0;
}

LRESULT CProcessSelectDlg::OnItemChanged(int, LPNMHDR, BOOL&) {
	auto selected = m_List.GetSelectedIndex();
	GetDlgItem(IDOK).EnableWindow(selected >= 0);
	if (selected >= 0) {
		m_SelectedPid = m_Items[selected].Id;
		m_Name = m_Items[selected].Name;
	}

	return 0;
}

LRESULT CProcessSelectDlg::OnGetDispInfo(int, LPNMHDR hdr, BOOL&) {
	auto di = reinterpret_cast<NMLVDISPINFO*>(hdr);
	auto& item = di->item;
	auto row = item.iItem;
	auto col = item.iSubItem;
	auto& data = m_Items[row];

	if (item.mask & LVIF_TEXT) {
		switch (col) {
		case 0:		// name
			item.pszText = (PWSTR)(PCWSTR)data.Name;
			break;

		case 1:		// ID
			::StringCchPrintf(item.pszText, item.cchTextMax, L"%d (0x%X)", data.Id, data.Id);
			break;

		case 2:		// session
			::StringCchPrintf(item.pszText, item.cchTextMax, L"%d", data.Session);
			break;
		}
	}
	if (item.mask & LVIF_IMAGE)
		item.iImage = data.Image;

	return 0;
}

LRESULT CProcessSelectDlg::OnDblClickItem(int, LPNMHDR, BOOL&) {
	auto selected = m_List.GetSelectedIndex();
	if (selected >= 0)
		EndDialog(IDOK);
	return 0;
}

void CProcessSelectDlg::InitProcessList() {
	EnumProcesses();
	m_List.SetItemCount(static_cast<int>(m_Items.size()));
}

void CProcessSelectDlg::EnumProcesses() {
	auto hSnapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	ATLASSERT(hSnapshot != INVALID_HANDLE_VALUE);
	m_Items.clear();

	PROCESSENTRY32 pe;
	pe.dwSize = sizeof(pe);
	Process32First(hSnapshot, &pe);
	WCHAR imagePath[MAX_PATH];
	while(::Process32Next(hSnapshot, &pe)) {
		ProcessInfo pi;
		auto hProcess = ::OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pe.th32ProcessID);
		auto path = pe.szExeFile;
		if (hProcess) {
			DWORD size = _countof(imagePath);
			if (::QueryFullProcessImageName(hProcess, 0, imagePath, &size))
				path = imagePath;
		}
		HICON hIcon;
		if (::ExtractIconEx(path, 0, nullptr, &hIcon, 1) == 1) {
			pi.Image = m_Images.AddIcon(hIcon);
		}
		else
			pi.Image = 0;
		pi.Name = pe.szExeFile;
		pi.Id = pe.th32ProcessID;
		pi.Session = 0;
		::ProcessIdToSessionId(pi.Id, &pi.Session);
		m_Items.push_back(std::move(pi));
	}
	::CloseHandle(hSnapshot);
}

bool CProcessSelectDlg::CompareItems(const ProcessInfo& p1, const ProcessInfo& p2, int col, bool asc) {
	switch (col) {
	case 0:	return SortHelper::SortStrings(p1.Name, p2.Name, asc);
	case 1: return SortHelper::SortNumbers(p1.Id, p2.Id, asc);
	case 2: return SortHelper::SortNumbers(p1.Session, p2.Session, asc);
	}
	ATLASSERT(false);
	return false;
}
