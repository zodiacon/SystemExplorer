#pragma once

#include "resource.h"
#include "ProcessInfoEx.h"
#include "ProcessManager.h"

class CProcessPropertiesDlg :
	public CDialogImpl<CProcessPropertiesDlg>,
	public CDynamicDialogLayout<CProcessPropertiesDlg> {
public:
	enum { IDD = IDD_PROCESS_PROPERTIES };

	CProcessPropertiesDlg(const WinSys::ProcessManager& pm, ProcessInfoEx& px) : m_pm(pm), m_px(px) {}
	void SetModal(bool modal) {
		m_Modal = modal;
	}

	void OnFinalMessage(HWND) override;

	BEGIN_MSG_MAP(CProcessPropertiesDlg)
		MESSAGE_HANDLER(WM_GETMINMAXINFO, OnGetMinMaxInfo)
		MESSAGE_HANDLER(WM_CTLCOLORDLG, OnDialogColor)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnDialogColor)
		MESSAGE_HANDLER(WM_CTLCOLOREDIT, OnDialogColor)
		MESSAGE_HANDLER(WM_CTLCOLORSCROLLBAR, OnDialogColor)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		COMMAND_ID_HANDLER(IDC_COPY, OnCopy)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDC_EXPLORE, OnExplore)
		COMMAND_ID_HANDLER(IDC_EXPLORE_DIR, OnExploreDirectory)
		COMMAND_ID_HANDLER(IDC_TOKEN, OnShowToken)
		COMMAND_ID_HANDLER(IDC_JOB, OnShowJob)
		COMMAND_ID_HANDLER(IDC_ENV, OnShowEnvironment)
		CHAIN_MSG_MAP(CDynamicDialogLayout<CProcessPropertiesDlg>)
	END_MSG_MAP()

private:
	void InitProcess();

	LRESULT OnDialogColor(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnExplore(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnGetMinMaxInfo(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnExploreDirectory(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnCopy(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnShowToken(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnShowEnvironment(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnShowJob(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

private:
	ProcessInfoEx& m_px;
	const WinSys::ProcessManager& m_pm;
	bool m_Modal{ false };
};

