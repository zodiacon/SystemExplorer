#include "pch.h"
#include "LogonSessionsView.h"
#include <algorithm>
#include "SortHelper.h"
#include <atlsecurity.h>

CString CLogonSessionsView::GetColumnText(HWND, int row, int col) const {
	const auto& item = m_Sessions[row];
	CString text;

	switch (col) {
		case 0: text.Format(L"%2d", item.Index); break;
		case 1: text.Format(L"0x%08X`%08X", item.LogonId.HighPart, item.LogonId.LowPart); break;
		case 2: return item.UserName.empty() ? L"" : (item.LogonDomain + L"\\" + item.UserName).c_str();
		case 3: return item.LogonTime.QuadPart == 0 ? L"" : CTime(*(FILETIME*)&item.LogonTime.QuadPart).Format(L"%D %X");
		case 4: text.Format(L"%d", item.Session); break;
		case 5: return LogonTypeToString(item.LogonType);
		case 6: return item.AuthenticationPackage.c_str();
		case 7:	return item.Sid[0] == 0 ? L"" : CSid((const SID*)item.Sid).Sid();
	}

	return text;
}

void CLogonSessionsView::DoSort(const SortInfo* si) {
	if (si == nullptr)
		return;

	std::sort(m_Sessions.begin(), m_Sessions.end(), [=](const auto& s1, const auto& s2) {
		switch (si->SortColumn) {
			case 0: return SortHelper::SortNumbers(s1.Index, s2.Index, si->SortAscending);
			case 1: return SortHelper::SortNumbers(*(long long*)&s1.LogonId, *(long long*)&s2.LogonId, si->SortAscending);
			case 2: return SortHelper::SortStrings(s1.LogonDomain + L"\\" + s1.UserName, s2.LogonDomain + L"\\" + s2.UserName, si->SortAscending);
			case 3: return SortHelper::SortNumbers(s1.LogonTime.QuadPart, s2.LogonTime.QuadPart, si->SortAscending);
			case 4: return SortHelper::SortNumbers(s1.Session, s2.Session, si->SortAscending);
			case 5: return SortHelper::SortStrings(LogonTypeToString(s1.LogonType), LogonTypeToString(s2.LogonType), si->SortAscending);
			case 6: return SortHelper::SortStrings(s1.AuthenticationPackage, s2.AuthenticationPackage, si->SortAscending);
		}
		return false;
		});
}

bool CLogonSessionsView::IsSortable(int col) const {
	return col != 7;
}

PCWSTR CLogonSessionsView::LogonTypeToString(WinSys::SecurityLogonType type) {
	using namespace WinSys;

	switch (type) {
		case SecurityLogonType::Batch: return L"Batch";
		case SecurityLogonType::CachedInteractive: return L"Cached Interactive";
		case SecurityLogonType::CachedRemoteInteractive: return L"Cached Remote Interactive";
		case SecurityLogonType::CachedUnlock: return L"Cached Unlock";
		case SecurityLogonType::Interactive: return L"Interactive";
		case SecurityLogonType::Network: return L"Network";
		case SecurityLogonType::NetworkCleartext: return L"Network Cleartext";
		case SecurityLogonType::NewCredentials: return L"New Credentials";
		case SecurityLogonType::Proxy: return L"Proxy";
		case SecurityLogonType::RemoteInteractive: return L"Remote Interactive";
		case SecurityLogonType::Service: return L"Service";
		case SecurityLogonType::UndefinedLogonType: return L"Undefined";
		case SecurityLogonType::Unlock: return L"Unlock";
	}
	ATLASSERT(false);
	return L"(Unknown)";
}

LRESULT CLogonSessionsView::OnCreate(UINT, WPARAM, LPARAM, BOOL&) {
	m_hWndClient = m_List.Create(*this, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | LVS_SINGLESEL | LVS_REPORT | LVS_OWNERDATA | LVS_SHOWSELALWAYS);
	m_List.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_LABELTIP);

	auto cm = GetColumnManager(m_List);
	cm->AddColumn(L"#", LVCFMT_RIGHT, 40, ColumnFlags::Mandatory | ColumnFlags::Const | ColumnFlags::Visible);
	cm->AddColumn(L"Logon Session ID", LVCFMT_RIGHT, 130, ColumnFlags::Mandatory | ColumnFlags::Const | ColumnFlags::Visible);
	cm->AddColumn(L"Username", LVCFMT_LEFT, 200, ColumnFlags::Mandatory | ColumnFlags::Const | ColumnFlags::Visible);
	cm->AddColumn(L"Logon Time", LVCFMT_LEFT, 130, ColumnFlags::Const | ColumnFlags::Visible);
	cm->AddColumn(L"Session", LVCFMT_RIGHT, 60, ColumnFlags::Const | ColumnFlags::Visible);
	cm->AddColumn(L"Logon Type", LVCFMT_LEFT, 150, ColumnFlags::Const | ColumnFlags::Visible);
	cm->AddColumn(L"Auth Package", LVCFMT_LEFT, 150, ColumnFlags::Const | ColumnFlags::Visible);
	cm->AddColumn(L"SID", LVCFMT_LEFT, 230, ColumnFlags::Const | ColumnFlags::Visible);

	cm->UpdateColumns();

	Refresh();

	return 0;
}

LRESULT CLogonSessionsView::OnRefresh(WORD, WORD, HWND, BOOL&) {
	Refresh();

	return 0;
}

void CLogonSessionsView::Refresh() {
	m_Sessions = WinSys::LsaSecurity::EnumLogonSessions();
	DoSort(GetSortInfo(m_List));
	m_List.SetItemCountEx(static_cast<int>(m_Sessions.size()), LVSICF_NOSCROLL | LVSICF_NOINVALIDATEALL);
	m_List.RedrawItems(m_List.GetTopIndex(), m_List.GetTopIndex() + m_List.GetCountPerPage());
}
