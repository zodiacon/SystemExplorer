#include "pch.h"
#include "JobPropertiesDlg.h"
#include "DialogHelper.h"
#include "SortHelper.h"
#include "FormatHelper.h"
#include "DriverHelper.h"

namespace {
	long long ToSeconds(long long ticks) {
		return ticks / 10000000LL;
	}
}

CString CJobPropertiesDlg::GetColumnText(HWND h, int row, int col) const {
	CString text;
	if (h == m_ProcList) {
		auto& p = m_Processes[row];
		switch (col) {
			case 0: return p.Name.c_str();
			case 1: text.Format(L"%u (0x%X)", p.Id, p.Id); break;
		}
	}
	else {
		auto& limit = m_Limits[row];
		switch (col) {
			case 0: return limit.Name;
			case 1: return limit.Value;
		}
	}
	return text;
}

void CJobPropertiesDlg::DoSort(const SortInfo* si) {
	if (si->hWnd == m_ProcList) {
		auto comparer = [&](auto const& p1, auto const& p2) {
			switch (si->SortColumn) {
				case 0: return SortHelper::SortStrings(p1.Name, p2.Name, si->SortAscending);
				case 1: return SortHelper::SortNumbers(p1.Id, p2.Id, si->SortAscending);
			}
			return false;
		};
		std::sort(m_Processes.begin(), m_Processes.end(), comparer);
	}
}

void CJobPropertiesDlg::UpdateJob() {
	CClientDC dc(*this);
	DrawStats(dc);

	// update process list
	BYTE buffer[1 << 11];
	if (::QueryInformationJobObject(m_hJob, JobObjectBasicProcessIdList, buffer, sizeof(buffer), nullptr)) {
		auto info = reinterpret_cast<JOBOBJECT_BASIC_PROCESS_ID_LIST*>(buffer);

		m_Processes.clear();
		auto count = info->NumberOfProcessIdsInList;
		m_Processes.reserve(count);
		for (DWORD i = 0; i < count; i++) {
			auto pid = static_cast<DWORD>(info->ProcessIdList[i]);
			ProcessInfo pi;
			pi.Id = pid;
			pi.Name = m_pm.GetProcessNameById(pi.Id);
			m_Processes.emplace_back(std::move(pi));
		}
		m_ProcList.SetItemCountEx(static_cast<int>(count), LVSICF_NOSCROLL);
	}

	m_Limits.clear();
	m_Limits.reserve(8);
	{
		JOBOBJECT_EXTENDED_LIMIT_INFORMATION info;
		if (::QueryInformationJobObject(m_hJob, JobObjectExtendedLimitInformation, &info, sizeof(info), nullptr)) {
			auto flags = info.BasicLimitInformation.LimitFlags;
			if (flags & JOB_OBJECT_LIMIT_ACTIVE_PROCESS)
				AddLimit(L"Active Processes", info.BasicLimitInformation.ActiveProcessLimit);
			if (flags & JOB_OBJECT_LIMIT_BREAKAWAY_OK)
				AddLimit(L"Breakaway OK", L"");
			if (flags & JOB_OBJECT_LIMIT_DIE_ON_UNHANDLED_EXCEPTION)
				AddLimit(L"Die on Unhandled Exception", L"");
			if (flags & JOB_OBJECT_LIMIT_SILENT_BREAKAWAY_OK)
				AddLimit(L"Silent Breakaway OK", L"");
			if (flags & JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE)
				AddLimit(L"Kill on Job Close", L"");
			if (flags & JOB_OBJECT_LIMIT_PRIORITY_CLASS)
				AddLimit(L"Priority Class", CString(FormatHelper::PriorityClassToString((WinSys::ProcessPriorityClass)info.BasicLimitInformation.PriorityClass)));
			if (flags & JOB_OBJECT_LIMIT_SCHEDULING_CLASS)
				AddLimit(L"Scheduling Class ", info.BasicLimitInformation.SchedulingClass);
			if (flags & JOB_OBJECT_LIMIT_AFFINITY)
				AddLimit(L"Affinity: ", info.BasicLimitInformation.Affinity);
			if (flags & JOB_OBJECT_LIMIT_JOB_MEMORY)
				AddLimit(L"Job Memory", (std::to_wstring(info.JobMemoryLimit >> 20) + L" MB").c_str());
			if (flags & JOB_OBJECT_LIMIT_PROCESS_MEMORY)
				AddLimit(L"Process Memory", std::to_wstring(info.ProcessMemoryLimit >> 20) + L" MB");
			if (flags & JOB_OBJECT_LIMIT_WORKINGSET)
				AddLimit(L"Maximum Working Set", (std::to_wstring(info.BasicLimitInformation.MaximumWorkingSetSize >> 20) + L" MB").c_str());
		}
	}
	{
		JOBOBJECT_BASIC_UI_RESTRICTIONS info;
		if (::QueryInformationJobObject(m_hJob, JobObjectBasicUIRestrictions, &info, sizeof(info), nullptr)) {
			static struct {
				CString Name;
				DWORD Value;
			} uitypes[] = {
				{ L"Desktop", JOB_OBJECT_UILIMIT_DESKTOP },
				{ L"Read Clipboard", JOB_OBJECT_UILIMIT_READCLIPBOARD },
				{ L"Write Clipboard", JOB_OBJECT_UILIMIT_WRITECLIPBOARD },
				{ L"Global Atoms", JOB_OBJECT_UILIMIT_GLOBALATOMS },
				{ L"Exit Windows", JOB_OBJECT_UILIMIT_EXITWINDOWS },
				{ L"Display Settings", JOB_OBJECT_UILIMIT_DISPLAYSETTINGS },
				{ L"System Parameters", JOB_OBJECT_UILIMIT_SYSTEMPARAMETERS },
				{ L"Handles", JOB_OBJECT_UILIMIT_HANDLES }
			};
			for (auto& ui : uitypes) {
				if (ui.Value & info.UIRestrictionsClass)
					AddLimit(L"User Interface", ui.Name);
			}
		}
	}
	{
		JOBOBJECT_CPU_RATE_CONTROL_INFORMATION info;
		if (::QueryInformationJobObject(m_hJob, JobObjectCpuRateControlInformation, &info, sizeof(info), nullptr)
			&& (info.ControlFlags & JOB_OBJECT_CPU_RATE_CONTROL_ENABLE)) {
			CString text;
			AddLimit(L"CPU Rate Limit Flags", FormatHelper::JobCpuRateControlFlagsToString(info.ControlFlags));
			if (info.ControlFlags & JOB_OBJECT_CPU_RATE_CONTROL_MIN_MAX_RATE) {
				text.Format(L"Min: %u Max: %u", info.MinRate, info.MaxRate);
				AddLimit(L"CPU Rate Limit", text);
			}
			else if (info.ControlFlags & JOB_OBJECT_CPU_RATE_CONTROL_WEIGHT_BASED) {
				text.Format(L"Weight: %u", info.Weight);
				AddLimit(L"CPU Rate Limit", text);
			}
			else {
				text.Format(L"%u", info.CpuRate);
				AddLimit(L"CPU Rate Limit", text);
			}
		}
	}
	m_LimitList.SetItemCountEx(static_cast<int>(m_Limits.size()), LVSICF_NOSCROLL);
}

void CJobPropertiesDlg::DrawStats(CDC& dc) {
	JOBOBJECT_BASIC_AND_IO_ACCOUNTING_INFORMATION info;
	if (::QueryInformationJobObject(m_hJob, JobObjectBasicAndIoAccountingInformation, &info, sizeof(info), nullptr)) {
		dc.SetBkMode(TRANSPARENT);
		dc.SelectFont(GetFont());

		BeginWriteInfo(dc, IDC_BASIC_INFO);
		AddDataItem(dc, L"Active Processes:", info.BasicInfo.ActiveProcesses);
		AddDataItem(dc, L"Total Processes:", info.BasicInfo.TotalProcesses);
		AddDataItem(dc, L"Terminated Processes:", info.BasicInfo.TotalTerminatedProcesses);
		AddDataItem(dc, L"Total User Time:", CTimeSpan(ToSeconds(info.BasicInfo.TotalUserTime.QuadPart)));
		AddDataItem(dc, L"Total Kernel Time:", CTimeSpan(ToSeconds(info.BasicInfo.TotalKernelTime.QuadPart)));
		AddDataItem(dc, L"Total CPU Time:", CTimeSpan(ToSeconds(info.BasicInfo.TotalUserTime.QuadPart + info.BasicInfo.TotalKernelTime.QuadPart)));

		BeginWriteInfo(dc, IDC_IO);
		AddDataItem(dc, L"Read Transfer:", CString(std::to_wstring(info.IoInfo.ReadTransferCount >> 10).c_str()) + L" KB");
		AddDataItem(dc, L"Write Transfer:", CString(std::to_wstring(info.IoInfo.WriteTransferCount >> 10).c_str()) + L" KB");
		AddDataItem(dc, L"Other Transfer:", CString(std::to_wstring(info.IoInfo.OtherTransferCount >> 10).c_str()) + L" KB");
		AddDataItem(dc, L"Read Operations:", info.IoInfo.ReadOperationCount);
		AddDataItem(dc, L"Write Operations:", info.IoInfo.WriteOperationCount);
		AddDataItem(dc, L"Other Operations:", info.IoInfo.OtherOperationCount);
	}
}

void CJobPropertiesDlg::AddDataItem(CDC& dc, PCWSTR property, const CTimeSpan& ts) {
	AddDataItem(dc, property, ts.Format(L"%H:%M:%S"));
}

void CJobPropertiesDlg::AddDataItem(CDC& dc, PCWSTR property, const CString& value) {
	dc.DrawText(property, -1, m_InfoRectProperty, DT_RIGHT | DT_SINGLELINE);
	dc.DrawText(value, -1, m_InfoRectValue, DT_SINGLELINE | DT_LEFT);
	m_InfoRectProperty.OffsetRect(0, m_Height);
	m_InfoRectValue.OffsetRect(0, m_Height);
}

void CJobPropertiesDlg::BeginWriteInfo(CDC& dc, UINT id) {
	CRect rc;
	GetDlgItem(id).GetWindowRect(&rc);
	ScreenToClient(&rc);
	dc.SelectFont(GetFont());
	CSize size;
	dc.GetTextExtent(L"A", 1, &size);
	rc.top += size.cy;
	rc.DeflateRect(2, 4);
	dc.FillSolidRect(&rc, ::GetSysColor(COLOR_WINDOW));

	rc.bottom = rc.top + size.cy;
	CRect rc2(rc);
	rc.right = rc.left + 120;
	rc2.left = rc.right + 10;
	m_InfoRectProperty = rc;
	m_InfoRectValue = rc2;
	m_Height = size.cy + 4;
}

void CJobPropertiesDlg::AddLimit(PCWSTR name, PCWSTR value) {
	LimitInfo limit = { name, value };
	m_Limits.push_back(limit);
}

void CJobPropertiesDlg::AddLimit(PCWSTR name, const std::wstring& value) {
	AddLimit(name, value.c_str());
}

void CJobPropertiesDlg::AddLimit(PCWSTR name, const CString& value) {
	AddLimit(name, (PCWSTR)value);
}

LRESULT CJobPropertiesDlg::OnDialogColor(UINT, WPARAM, LPARAM, BOOL&) {
	return (LRESULT)::GetSysColorBrush(COLOR_WINDOW);
}

LRESULT CJobPropertiesDlg::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&) {
	DialogHelper::SetDialogIcon(this, IDI_JOB);
	DialogHelper::AddIconToButton(this, IDC_TERM, IDI_DELETE);

	m_ProcList.Attach(GetDlgItem(IDC_PROCLIST));
	m_ProcList.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_INFOTIP);
	m_ProcList.InsertColumn(0, L"Image Name", LVCFMT_LEFT, 150);
	m_ProcList.InsertColumn(1, L"PID", LVCFMT_RIGHT, 120);

	m_LimitList.Attach(GetDlgItem(IDC_LIMITS));
	m_LimitList.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_INFOTIP);
	m_LimitList.InsertColumn(0, L"Name", LVCFMT_LEFT, 160);
	m_LimitList.InsertColumn(1, L"Value", LVCFMT_LEFT, 130);

	SILOOBJECT_BASIC_INFORMATION silo;
	CString text(L"Silo: No");
	if (::QueryInformationJobObject(m_hJob, JobObjectSiloBasicInformation, &silo, sizeof(silo), nullptr)) {
		text.Format(L"%s Silo\t Silo ID: %u (0x%X)", silo.IsInServerSilo ? L"Server" : L"App", silo.SiloId, silo.SiloId);
	}
	SetDlgItemText(IDC_SILO, text);
	SetDlgItemText(IDC_NAME, m_Name);

	UpdateJob();
	SetTimer(1, 2000, nullptr);

	return 0;
}

LRESULT CJobPropertiesDlg::OnCloseCmd(WORD, WORD wID, HWND, BOOL&) {
	EndDialog(wID);

	return 0;
}

LRESULT CJobPropertiesDlg::OnTimer(UINT, WPARAM id, LPARAM, BOOL&) {
	if (id == 1)
		UpdateJob();
	return 0;
}

LRESULT CJobPropertiesDlg::OnPaint(UINT, WPARAM, LPARAM, BOOL&) {
	CPaintDC dc(*this);
	DrawStats(dc);
	return 0;
}

LRESULT CJobPropertiesDlg::OnTerminate(WORD, WORD wID, HWND, BOOL&) {
	if (AtlMessageBox(*this, L"Are you sure?", L"Kill Job", MB_OKCANCEL | MB_DEFBUTTON2 | MB_ICONWARNING) == IDCANCEL)
		return 0;

	auto hTerm = DriverHelper::DupHandle(m_hJob, ::GetCurrentProcessId(), JOB_OBJECT_TERMINATE, 0);
	BOOL ok = FALSE;
	if (hTerm)
		ok = ::TerminateJobObject(hTerm, 0);
	if (!hTerm || !ok) {
		AtlMessageBox(*this, L"Failed to terminate job", IDS_TITLE, MB_ICONEXCLAMATION);
		return 0;
	}
	if (hTerm)
		::CloseHandle(hTerm);
	return 0;
}
