#pragma once

#include "resource.h"

class CProgressDlg : public CDialogImpl<CProgressDlg> {
public:
	enum { IDD = IDD_PROGRESS };

	BEGIN_MSG_MAP(CProgressDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

	void ShowCancelButton(bool show);
	void UpdateProgress(int value);
	void SetProgressMarquee(bool set = true);
	void SetMessageText(PCWSTR text);
	void SetTimerCallback(std::function<void()> callback, int interval);
	void Close(UINT id = IDOK);

private:
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

private:
	CProgressBarCtrl m_Progress;
	CString m_Text;
	std::function<void()> m_Callback;
	int m_Interval = 0;
	bool m_ShowCancel = true;
	bool m_IsMarquee = false;
};

