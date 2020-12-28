#include "pch.h"
#include "resource.h"
#include "InstallServiceDlg.h"
#include "DialogHelper.h"

const WinSys::ServiceInstallParams& CInstallServiceDlg::GetInstallParams() const {
	return m_InstallParams;
}

LRESULT CInstallServiceDlg::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&) {
	DialogHelper::AdjustOKCancelButtons(this);
	DialogHelper::SetDialogIcon(this, IDI_SERVICE);

	::SHAutoComplete(GetDlgItem(IDC_PATH), SHACF_FILESYSTEM);
	::SHAutoComplete(GetDlgItem(IDC_TARGET_PATH), SHACF_FILESYS_DIRS);

	{
		ComboData data[] = {
			{ L"Kernel Driver", 1 },
			{ L"File System Driver/Filter", 2 },
			{ L"Service Own Process", 0x10 },
			{ L"Service Shared Process", 0x20 },
			{ L"Service User Own Process", 0x50 },
			{ L"Service User Shared Process", 0x60 },
		};
		FillComboBox(IDC_TYPE, data, _countof(data), 2);
	}

	{
		ComboData data[] = {
			{ L"Boot Start (Drivers only)", 0 },
			{ L"System Start (Drivers only)", 1 },
			{ L"Auto Start", 2 },
			{ L"Delayed Auto Start", 2 | 0x80 },
			{ L"Manual Start", 3 },
			{ L"Disabled", 4 },
		};
		FillComboBox(IDC_STARTUP, data, _countof(data), 4);
	}

	{
		ComboData data[] = {
			{ L"Ignore", 0 },
			{ L"Normal", 1 },
			{ L"Severe", 2 },
			{ L"Critical", 3 }
		};
		FillComboBox(IDC_ERROR_CONTROL, data, _countof(data), 1);
	}

	{
		ComboData data[] = {
			{ L"Local System", 1 },
			{ L"Network Service", 2 },
			{ L"Local Service", 3 },
		};
		FillComboBox(IDC_ACCOUNT, data, _countof(data));
	}

	return 0;
}

LRESULT CInstallServiceDlg::OnCloseCmd(WORD, WORD wID, HWND, BOOL&) {
	if (wID == IDOK) {
		m_InstallParams.ServiceName = GetDlgItemText(IDC_NAME);
		m_InstallParams.DisplayName = GetDlgItemText(IDC_DISPLAYNAME);
		CComboBox cb(GetDlgItem(IDC_STARTUP));
		auto type = cb.GetItemData(cb.GetCurSel());
		m_InstallParams.DelayedAutoStart = (type & 0x80) == 0x80;
		type &= 0xf;
		m_InstallParams.StartupType = static_cast<WinSys::ServiceStartType>(type);
		cb.Detach();
		cb.Attach(GetDlgItem(IDC_TYPE));
		m_InstallParams.ServiceType = static_cast<WinSys::ServiceType>(cb.GetItemData(cb.GetCurSel()));
		cb.Detach();
		cb.Attach(GetDlgItem(IDC_ERROR_CONTROL));
		m_InstallParams.ErrorControl = static_cast<WinSys::ServiceErrorControl>(cb.GetItemData(cb.GetCurSel()));
		m_InstallParams.AccountName = GetDlgItemText(IDC_ACCOUNT);
		m_InstallParams.Password = GetDlgItemText(IDC_PASSWORD);
		m_InstallParams.ImagePath = GetDlgItemText(IDC_PATH);
		if (m_InstallParams.ImagePath.empty()) {
			AtlMessageBox(*this, L"Image path cannot be empty", IDS_TITLE, MB_ICONERROR);
			return 0;
		}
		m_InstallParams.TargetPath = GetDlgItemText(IDC_TARGET_PATH);
		m_InstallParams.Dependencies = GetDlgItemText(IDC_DEP);
		m_InstallParams.LoadOrderGroup = GetDlgItemText(IDC_GROUP);
	}

	EndDialog(wID);

	return 0;
}

LRESULT CInstallServiceDlg::OnNameChanged(WORD, WORD wID, HWND, BOOL&) {
	GetDlgItem(IDOK).EnableWindow(GetDlgItem(IDC_NAME).GetWindowTextLength() > 0);

	return 0;
}

LRESULT CInstallServiceDlg::OnBrowseFile(WORD, WORD wID, HWND, BOOL&) {
	CSimpleFileDialog dlg(TRUE, nullptr, nullptr, OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST,
		L"Executables (*.exe)\0*.exe\0Drivers (*.sys)\0*.sys\0All Files\0*.*\0", *this);
	if (dlg.DoModal() == IDOK)
		SetDlgItemText(IDC_PATH, dlg.m_szFileName);
	return 0;
}

std::wstring CInstallServiceDlg::GetDlgItemText(UINT id) {
	CWindow win(GetDlgItem(id));
	auto len = win.GetWindowTextLength();
	std::wstring text;
	if (len) {
		text.resize(len + 1);
		win.GetWindowText(text.data(), len + 1);
	}
	return text;
}

void CInstallServiceDlg::FillComboBox(UINT id, ComboData* data, int count, int selected) {
	CComboBox cb(GetDlgItem(id));

	for (int i = 0; i < count; i++) {
		auto& item = data[i];
		cb.SetItemData(cb.AddString(item.text), item.data);
	}
	cb.SetCurSel(selected);
	cb.Detach();
}
