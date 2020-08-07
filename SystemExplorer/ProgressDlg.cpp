#include "pch.h"
#include "ProgressDlg.h"
#include "DialogHelper.h"

void CProgressDlg::ShowCancelButton(bool show) {
	m_ShowCancel = show;
}

void CProgressDlg::UpdateProgress(int value) {
	if (m_Progress)
		m_Progress.SetPos(value);
}

void CProgressDlg::SetProgressMarquee(bool set) {
	if (m_Progress)
		m_Progress.SetMarquee(set);
	else
		m_IsMarquee = set;
}

void CProgressDlg::SetMessageText(PCWSTR text) {
	if (m_hWnd)
		SetDlgItemText(IDC_MESSAGE, text);
	else
		m_Text = text;
}

void CProgressDlg::SetTimerCallback(std::function<void()> callback, int interval) {
	m_Callback = callback;
	m_Interval = interval;
}

void CProgressDlg::Close() {
	EndDialog(IDCANCEL);
}

LRESULT CProgressDlg::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&) {
	DialogHelper::AdjustOKCancelButtons(this);

	m_Progress.Attach(GetDlgItem(IDC_PROGRESS));

	if (!m_ShowCancel)
		GetDlgItem(IDCANCEL).ShowWindow(SW_HIDE);

	if (m_IsMarquee)
		m_Progress.SetMarquee(true);
	SetDlgItemText(IDC_MESSAGE, m_Text);

	if (m_Interval)
		SetTimer(1, m_Interval, nullptr);

	return 0;
}

LRESULT CProgressDlg::OnCloseCmd(WORD, WORD wID, HWND, BOOL&) {
	if (m_ShowCancel)
		EndDialog(IDCANCEL);
	return 0;
}

LRESULT CProgressDlg::OnTimer(UINT, WPARAM id, LPARAM, BOOL&) {
	if (id == 1 && m_Callback)
		m_Callback();

	return 0;
}
