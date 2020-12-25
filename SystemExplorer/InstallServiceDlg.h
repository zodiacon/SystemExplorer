#pragma once

#include "ServiceManager.h"

class CInstallServiceDlg : public CDialogImpl<CInstallServiceDlg> {
public:
	enum { IDD = IDD_INSTALLSVC };

	const WinSys::ServiceInstallParams& GetInstallParams() const;

	BEGIN_MSG_MAP(CInstallServiceDlg)
		COMMAND_HANDLER(IDC_NAME, EN_CHANGE, OnNameChanged)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		COMMAND_ID_HANDLER(IDC_BROWSE, OnBrowseFile)
	END_MSG_MAP()

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

private:
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnNameChanged(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnBrowseFile(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	std::wstring GetDlgItemText(UINT id);

	struct ComboData {
		PCWSTR text;
		ULONG data;
	};

	void FillComboBox(UINT id, ComboData* data, int count, int selected = 0);

	WinSys::ServiceInstallParams m_InstallParams;
};
