#include "pch.h"
#include "TokenPropertiesDlg.h"
#include "DialogHelper.h"
#include "FormatHelper.h"
#include "SortHelper.h"

void CTokenPropertiesDlg::OnFinalMessage(HWND) {
}

CString CTokenPropertiesDlg::GetColumnText(HWND h, int row, int col) const {
	if (h == m_GroupList) {
		auto& g = m_Groups[row];
		switch (col) {
			case 0: return g.Name.c_str();
			case 1: return FormatHelper::SidNameUseToString(g.Use);
			case 2: return FormatHelper::SidAttributesToString(g.Attributes);
		}
	}
	else if(h == m_PrivList) {
		auto& p = m_Privileges[row];
		switch (col) {
			case 0: return p.Name.c_str();
			case 1: return FormatHelper::PrivilegeAttributesToString(p.Attributes);
		}
	}
	return L"";
}

void CTokenPropertiesDlg::DoSort(const SortInfo* si) {
	if (!si)
		return;

	if (si->hWnd == m_GroupList) {
		auto comparer = [&](const auto& g1, const auto& g2) -> bool {
			switch (si->SortColumn) {
				case 0: return SortHelper::SortStrings(g1.Name, g2.Name, si->SortAscending);
				case 1: return SortHelper::SortStrings(FormatHelper::SidNameUseToString(g1.Use), FormatHelper::SidNameUseToString(g2.Use), si->SortAscending);
				case 2: return SortHelper::SortNumbers(g1.Attributes, g2.Attributes, si->SortAscending);
			}
			return false;
		};
		std::sort(m_Groups.begin(), m_Groups.end(), comparer);
	}
	else if (si->hWnd == m_PrivList) {
		auto comparer = [&](const auto& p1, const auto& p2) -> bool {
			switch (si->SortColumn) {
				case 0: return SortHelper::SortStrings(p1.Name, p2.Name, si->SortAscending);
				case 1: return SortHelper::SortNumbers(p1.Attributes, p2.Attributes, si->SortAscending);
			}
			return false;
		};
		std::sort(m_Privileges.begin(), m_Privileges.end(), comparer);
	}
}

int CTokenPropertiesDlg::GetRowImage(HWND h, int row) const {
	if(h == m_PrivList)
		return (m_Privileges[row].Attributes & SE_PRIVILEGE_ENABLED) ? 0 : 1;
	return -1;
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

	m_Groups = token.EnumGroups();
	text.Format(L"Groups: %u\n", (ULONG)m_Groups.size());
	SetDlgItemText(IDC_GROUP_COUNT, text);
	m_GroupList.SetItemCount((int)m_Groups.size());

	m_Privileges = token.EnumPrivileges();
	text.Format(L"Privileges: %u\n", (ULONG)m_Privileges.size());
	SetDlgItemText(IDC_PRIV_COUNT, text);
	m_PrivList.SetItemCount((int)m_Privileges.size());

	CListBox lb(GetDlgItem(IDC_CAPS));
	auto caps = token.EnumGroups(true);
	text.Format(L"Capabilities: %u", (ULONG)caps.size());
	SetDlgItemText(IDC_CAPS_COUNT, text);
	for (auto& cap : caps) {
		lb.AddString(cap.Name.c_str());
	}
}

LRESULT CTokenPropertiesDlg::OnDialogColor(UINT, WPARAM, LPARAM, BOOL&) {
	return (LRESULT)::GetSysColorBrush(COLOR_WINDOW);
}

LRESULT CTokenPropertiesDlg::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&) {
	DialogHelper::SetDialogIcon(this, IDI_TOKEN);

	m_GroupList.Attach(GetDlgItem(IDC_GROUPS));
	m_GroupList.SetExtendedListViewStyle(LVS_EX_DOUBLEBUFFER | LVS_EX_INFOTIP | LVS_EX_FULLROWSELECT);
	m_GroupList.InsertColumn(0, L"Name", LVCFMT_LEFT, 260);
	m_GroupList.InsertColumn(1, L"Type", LVCFMT_LEFT, 110);
	m_GroupList.InsertColumn(2, L"Attributes", LVCFMT_LEFT, 170);

	m_PrivList.Attach(GetDlgItem(IDC_PRIVILEGES));
	m_PrivList.SetExtendedListViewStyle(LVS_EX_DOUBLEBUFFER | LVS_EX_INFOTIP | LVS_EX_FULLROWSELECT);
	m_PrivList.InsertColumn(0, L"Name", LVCFMT_LEFT, 230);
	m_PrivList.InsertColumn(1, L"Attributes", LVCFMT_LEFT, 150);

	CImageList images;
	images.Create(16, 16, ILC_COLOR32, 2, 2);
	images.AddIcon(AtlLoadIconImage(IDI_OK, 0, 16, 16));
	images.AddIcon(AtlLoadIconImage(IDI_DELETE, 0, 16, 16));
	m_PrivList.SetImageList(images, LVSIL_SMALL);

	InitToken();

	return 0;
}

LRESULT CTokenPropertiesDlg::OnCloseCmd(WORD, WORD wID, HWND, BOOL&) {
	EndDialog(wID);
	return 0;
}

LRESULT CTokenPropertiesDlg::OnEnablePrivilege(WORD, WORD wID, HWND, BOOL&) {
	return LRESULT();
}

LRESULT CTokenPropertiesDlg::OnDisablePrivilege(WORD, WORD wID, HWND, BOOL&) {
	return LRESULT();
}

LRESULT CTokenPropertiesDlg::OnPrivItemChanged(int, LPNMHDR hdr, BOOL&) {
	auto count = m_PrivList.GetSelectedCount();
	GetDlgItem(IDC_ENABLE).EnableWindow(count > 0);
	GetDlgItem(IDC_DISABLE).EnableWindow(count > 0);

	return 0;
}
