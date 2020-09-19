#pragma once

#include "resource.h"
#include "VirtualListView.h"

class CJobProperties : 
	public CDialogImpl<CJobProperties>,
	public CVirtualListView<CJobProperties> {
public:
	enum { IDD = IDD_JOB };

	CJobProperties(const WinSys::ProcessManager& pm, HANDLE hJob, PCWSTR name) : m_pm(pm), m_hJob(hJob), m_Name(name) {}

	CString GetColumnText(HWND h, int row, int col) const;
	void DoSort(const SortInfo* si);

	BEGIN_MSG_MAP(CJobProperties)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_CTLCOLORDLG, OnDialogColor)
		MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnDialogColor)
		MESSAGE_HANDLER(WM_CTLCOLOREDIT, OnDialogColor)
		MESSAGE_HANDLER(WM_CTLCOLORSCROLLBAR, OnDialogColor)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
		COMMAND_ID_HANDLER(IDC_TERM, OnTerminate)
		CHAIN_MSG_MAP(CVirtualListView<CJobProperties>)
	END_MSG_MAP()

private:
	struct ProcessInfo {
		std::wstring Name;
		DWORD Id;
	};
	struct LimitInfo {
		CString Name, Value;
	};

	void UpdateJob();
	void DrawStats(CDC& dc);
	void BeginWriteInfo(CDC& dc, UINT id);
	template<typename T>
	void AddDataItem(CDC& dc, PCWSTR property, const T& value) {
		AddDataItem(dc, property, CString(std::to_wstring(value).c_str()));
	}
	void AddDataItem(CDC& dc, PCWSTR property, const CTimeSpan& ts);
	void AddDataItem(CDC& dc, PCWSTR property, const CString& value);

	template<typename T>
	void AddLimit(PCWSTR name, T const& value) {
		LimitInfo li = { name, CString(std::to_wstring(value).c_str()) };
		m_Limits.push_back(li);
	}
	void AddLimit(PCWSTR name, PCWSTR value);
	void AddLimit(PCWSTR name, const std::wstring& value);
	void AddLimit(PCWSTR name, const CString& value);

	LRESULT OnDialogColor(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnTerminate(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

private:
	HANDLE m_hJob;
	std::vector<ProcessInfo> m_Processes;
	std::vector<LimitInfo> m_Limits;
	const WinSys::ProcessManager& m_pm;
	CString m_Name;
	CRect m_InfoRectProperty, m_InfoRectValue;
	int m_Height;
	CListViewCtrl m_ProcList, m_LimitList;
};

