#include "pch.h"
#include "ProcessSelectDlg.h"
#include <string>
#include "DialogHelper.h"
#include <TlHelp32.h>
#include <algorithm>
#include "SortHelper.h"

int CProcessSelectDlg::GetSelectedProcess(CString& name) const {
	name = m_Name;
	return m_SelectedPid;
}

CString CProcessSelectDlg::GetColumnText(HWND, int row, int col) const {
	auto& data = m_Items[row];
	CString text;

	switch (col) {
		case 0:	return data.Name;
		case 1:		// ID
			text.Format(L"%d (0x%X)", data.Id, data.Id);
			break;

		case 2:		// session
			text.Format(L"%d", data.Session);
			break;
	}
	return text;
}

int CProcessSelectDlg::GetRowImage(int row) const {
	return m_Items[row].Image;
}

void CProcessSelectDlg::DoSort(const SortInfo* si) {
	if (!si)
		return;

	std::sort(m_Items.begin(), m_Items.end(), [si](const auto& p1, const auto& p2) {
		return CompareItems(p1, p2, si->SortColumn, si->SortAscending);
		});
}

LRESULT CProcessSelectDlg::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&) {
	DialogHelper::AdjustOKCancelButtons(this);

	CenterWindow(GetParent());
	m_List.Attach(GetDlgItem(IDC_PROCLIST));

	m_List.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_LABELTIP);

	m_List.InsertColumn(0, L"Name", LVCFMT_LEFT, 170);
	m_List.InsertColumn(1, L"PID", LVCFMT_RIGHT, 130);
	m_List.InsertColumn(2, L"Session", LVCFMT_CENTER, 70);

	m_Images.Create(16, 16, ILC_COLOR32, 32, 16);
	m_Images.AddIcon(AtlLoadSysIcon(IDI_APPLICATION));
	m_List.SetImageList(m_Images, LVSIL_SMALL);

	((CButton)GetDlgItem(IDC_REFRESH)).SetIcon(AtlLoadIconImage(IDI_REFRESH, 0, 24, 24));

	SetIcon(AtlLoadIconImage(IDI_PROCESS, 0, 16, 16));

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

LRESULT CProcessSelectDlg::OnDblClickItem(int, LPNMHDR, BOOL&) {
	auto selected = m_List.GetSelectedIndex();
	if (selected >= 0)
		EndDialog(IDOK);
	return 0;
}

LRESULT CProcessSelectDlg::OnRefresh(WORD, WORD wID, HWND, BOOL&) {
	InitProcessList();

	return 0;
}

void CProcessSelectDlg::InitProcessList() {
	EnumProcesses();
	DoSort(GetSortInfo());
}

void CProcessSelectDlg::EnumProcesses() {
	m_ProcMgr.EnumProcesses();

	m_Items.clear();
	m_Items.reserve(m_ProcMgr.GetProcessCount());

	std::unordered_map<std::wstring, int> images;
	images.reserve(m_Items.size());

	WCHAR imagePath[MAX_PATH];
	for (auto& p : m_ProcMgr.GetProcesses()) {
		ProcessInfo pi;
		auto hProcess = ::OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, p->Id);
		auto path = p->GetImageName().c_str();
		if (hProcess) {
			DWORD size = _countof(imagePath);
			if (::QueryFullProcessImageName(hProcess, 0, imagePath, &size))
				path = imagePath;
		}
		auto it = images.find(path);
		if (it != images.end())
			pi.Image = it->second;
		else {
			HICON hIcon;
			if (::ExtractIconEx(path, 0, nullptr, &hIcon, 1) == 1) {
				pi.Image = m_Images.AddIcon(hIcon);
				images.insert({ path, pi.Image });
			}
			else
				pi.Image = 0;
		}
		pi.Name = p->GetImageName().c_str();
		pi.Id = p->Id;
		pi.Session = p->SessionId;
		m_Items.push_back(std::move(pi));
	}
	m_List.SetItemCountEx(static_cast<int>(m_Items.size()), LVSICF_NOSCROLL);
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
