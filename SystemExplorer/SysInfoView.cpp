#include "pch.h"
#include "SysInfoView.h"
#include "FormatHelper.h"
#include "SortHelper.h"

using namespace WinSys;

CSysInfoView::CSysInfoView(IMainFrame* frame) : CViewBase(frame), m_Items{
	ItemData(RowType::Version, L"Windows Version", ItemDataFlags::Const | ItemDataFlags::Special),
	ItemData(RowType::BootTime, L"Boot Time", ItemDataFlags::Const | ItemDataFlags::Time | ItemDataFlags::Special),
	ItemData(RowType::TotalRAM, L"Usable RAM", ItemDataFlags::Const | ItemDataFlags::Special),
	ItemData(RowType::TotalCPUs, L"Processor Count", &m_BasicSysInfo.NumberOfProcessors, &m_BasicSysInfo.NumberOfProcessors, ItemDataFlags::Bits32 | ItemDataFlags::Const),
	ItemData(RowType::Processes, L"Processes", &m_PerfInfo.ProcessCount, &m_OldPerfInfo.ProcessCount, ItemDataFlags::Bits32),
	ItemData(RowType::Threads, L"Threads", &m_PerfInfo.ThreadCount, &m_OldPerfInfo.ThreadCount, ItemDataFlags::Bits32),
	ItemData(RowType::Handles, L"Handles", &m_PerfInfo.HandleCount, &m_OldPerfInfo.HandleCount, ItemDataFlags::Bits32),
	ItemData(RowType::IdleTime, L"Idle Time", &m_SysPerfInfo.IdleProcessTime, &m_OldSysPerfInfo.IdleProcessTime, ItemDataFlags::TimeSpan),
	ItemData(RowType::AvailableVM, L"Available VM", &m_SysPerfInfo.AvailablePages, &m_OldSysPerfInfo.AvailablePages, ItemDataFlags::Bits32 | ItemDataFlags::MemoryInPages),
	ItemData(RowType::CommitSize, L"Commit Size", &m_SysPerfInfo.CommittedPages, &m_OldSysPerfInfo.CommittedPages, ItemDataFlags::Special),
	ItemData(RowType::CommitLimit, L"Commit Limit", &m_SysPerfInfo.CommitLimit, &m_OldSysPerfInfo.CommitLimit, ItemDataFlags::MemoryInPages | ItemDataFlags::Bits32),
	ItemData(RowType::PeakCommit, L"Peak Commit", &m_SysPerfInfo.PeakCommitment, &m_OldSysPerfInfo.PeakCommitment, ItemDataFlags::MemoryInPages | ItemDataFlags::Bits32),
	ItemData(RowType::PagedPoolSize, L"Paged Pool Size", &m_SysPerfInfo.PagedPoolPages, &m_OldSysPerfInfo.PagedPoolPages, ItemDataFlags::Bits32 | ItemDataFlags::MemoryInPages),
	ItemData(RowType::ResidentPagedPoolSize, L"Resident Paged Pool", &m_SysPerfInfo.ResidentPagedPoolPage, &m_OldSysPerfInfo.ResidentPagedPoolPage, ItemDataFlags::Special | ItemDataFlags::Bits32),
	ItemData(RowType::AvailabePagePool, L"Available Paged Pool", &m_SysPerfInfo.AvailablePagedPoolPages, &m_OldSysPerfInfo.AvailablePagedPoolPages, ItemDataFlags::Bits32 | ItemDataFlags::MemoryInPages),
	ItemData(RowType::NonPagedPoolSize, L"Non-Paged Pool Size", &m_SysPerfInfo.NonPagedPoolPages, &m_OldSysPerfInfo.NonPagedPoolPages, ItemDataFlags::Bits32 | ItemDataFlags::MemoryInPages),

	ItemData(RowType::IoReadTransferCount, L"I/O Read Transfer (Bytes)", &m_SysPerfInfo.IoReadTransferCount, &m_OldSysPerfInfo.IoReadTransferCount),
	ItemData(RowType::IoWriteTransferCount, L"I/O Write Transfer (Bytes)", &m_SysPerfInfo.IoWriteTransferCount, &m_OldSysPerfInfo.IoWriteTransferCount),
	ItemData(RowType::IoOtherTransferCount, L"I/O Other Transfer (Bytes)", &m_SysPerfInfo.IoOtherTransferCount, &m_OldSysPerfInfo.IoOtherTransferCount),

	ItemData(RowType::IoReadOperationCount, L"I/O Read Operations", &m_SysPerfInfo.IoReadOperationCount, &m_OldSysPerfInfo.IoReadOperationCount, ItemDataFlags::Bits32),
	ItemData(RowType::IoWriteOperationCount, L"I/O Write Operations", &m_SysPerfInfo.IoWriteOperationCount, &m_OldSysPerfInfo.IoWriteOperationCount, ItemDataFlags::Bits32),
	ItemData(RowType::IoOtherOperationCount, L"I/O Other Operations", &m_SysPerfInfo.IoOtherOperationCount, &m_OldSysPerfInfo.IoOtherOperationCount, ItemDataFlags::Bits32),

	ItemData(RowType::SystemCalls, L"System Calls", &m_SysPerfInfo.SystemCalls, &m_OldSysPerfInfo.SystemCalls, ItemDataFlags::Bits32),
	ItemData(RowType::PageFaultCount, L"Page Fault Count", &m_SysPerfInfo.PageFaultCount, &m_OldSysPerfInfo.PageFaultCount, ItemDataFlags::Bits32),
	ItemData(RowType::PageReadCount, L"Page Read Count", &m_SysPerfInfo.PageReadCount, &m_OldSysPerfInfo.PageReadCount, ItemDataFlags::Bits32),
	ItemData(RowType::PageReadIoCount, L"Page Read I/O Count", &m_SysPerfInfo.PageReadIoCount, &m_OldSysPerfInfo.PageReadIoCount, ItemDataFlags::Bits32),
	ItemData(RowType::DemandZeroCount, L"Demand Zero Count", &m_SysPerfInfo.DemandZeroCount, &m_OldSysPerfInfo.DemandZeroCount, ItemDataFlags::Bits32),
	ItemData(RowType::CopyOnWriteCount, L"Copy On Write Count", &m_SysPerfInfo.CopyOnWriteCount, &m_OldSysPerfInfo.CopyOnWriteCount, ItemDataFlags::Bits32),
	ItemData(RowType::TransitionCount, L"Transition Count", &m_SysPerfInfo.TransitionCount, &m_OldSysPerfInfo.TransitionCount, ItemDataFlags::Bits32),
	ItemData(RowType::FreeSystemPtes, L"Free System PTEs", &m_SysPerfInfo.FreeSystemPtes, &m_OldSysPerfInfo.FreeSystemPtes, ItemDataFlags::Bits32),
} {}

CString CSysInfoView::GetColumnText(HWND, int row, int col) const {
	auto& item = m_Items[row];
	if (col == 0) {
		return item.Name;
	}
	CString text;
	auto flags = item.Flags;

	if ((flags & ItemDataFlags::Special) == ItemDataFlags::None) {

		if ((flags & ItemDataFlags::MemoryInPages) == ItemDataFlags::MemoryInPages) {
			return FormatHelper::FormatWithCommas(*item.Current.Value32 >> 8) + L" MB";
		}
		if ((flags & ItemDataFlags::Bits32) == ItemDataFlags::Bits32)
			return FormatHelper::FormatWithCommas(*item.Current.Value32);
		if ((flags & ItemDataFlags::TimeSpan) == ItemDataFlags::TimeSpan)
			return FormatHelper::TimeSpanToString(*item.Current.Value64);
		if ((flags & ItemDataFlags::Bits32) == ItemDataFlags::None)
			return FormatHelper::FormatWithCommas(*item.Current.Value64);
	}

	switch (item.Type) {
		case RowType::BootTime: return FormatHelper::TimeToString(SystemInformation::GetBootTime());

		case RowType::TotalRAM:
			text = FormatHelper::FormatWithCommas(SystemInformation::GetBasicSystemInfo().TotalPhysicalInPages >> 8) + L" MB";
			text.Format(L"%s (%u GB)", text, (ULONG)((SystemInformation::GetBasicSystemInfo().TotalPhysicalInPages + (1 << 17)) >> 18));
			break;

		case RowType::CommitSize:
			text = FormatHelper::FormatWithCommas(m_SysPerfInfo.CommittedPages >> 8) + L" MB";
			text.Format(L"%s (%d %%)", text, m_SysPerfInfo.CommittedPages * 100 / m_SysPerfInfo.CommitLimit);
			break;

		case RowType::ResidentPagedPoolSize:
			text = FormatHelper::FormatWithCommas(m_SysPerfInfo.ResidentPagedPoolPage >> 8) + L" MB";
			text.Format(L"%s (%d %%)", text, m_SysPerfInfo.ResidentPagedPoolPage * 100 / m_SysPerfInfo.PagedPoolPages);
			break;

		case RowType::Version:
		{
			auto& ver = SystemInformation::GetWindowsVersion();
			text.Format(L"%u.%u.%u", ver.Major, ver.Minor, ver.Build);
			break;
		}

		default:
			ATLASSERT(false);
			break;
	}
	return text;
}

DWORD CSysInfoView::OnPrePaint(int, LPNMCUSTOMDRAW) {
	return CDRF_NOTIFYITEMDRAW;
}

DWORD CSysInfoView::OnItemPrePaint(int, LPNMCUSTOMDRAW cd) {
	auto lv = (NMLVCUSTOMDRAW*)cd;

	lv->clrTextBk = CLR_INVALID;
	auto row = (int)lv->nmcd.dwItemSpec;
	auto& item = m_Items[row];
	if ((item.Flags & ItemDataFlags::Const) == ItemDataFlags::Const)
		return CDRF_DODEFAULT;

	if ((item.Flags & ItemDataFlags::Bits32) == ItemDataFlags::Bits32) {
		if (*item.Current.Value32 > *item.Previous.Value32)
			lv->clrTextBk = RGB(0, 255, 0);
		else if (*item.Current.Value32 < *item.Previous.Value32)
			lv->clrTextBk = RGB(255, 64, 0);
	}
	else {
		if (*item.Current.Value64 > *item.Previous.Value64)
			lv->clrTextBk = RGB(0, 255, 0);
		else if (*item.Current.Value64 < *item.Previous.Value64)
			lv->clrTextBk = RGB(255, 64, 0);
	}
	return CDRF_DODEFAULT;
}

void CSysInfoView::DoSort(const SortInfo* si) {
	std::sort(m_Items.begin(), m_Items.end(), [&](auto& i1, auto& i2) {
		return SortHelper::SortStrings(i1.Name, i2.Name, si->SortAscending);
		});
}

bool CSysInfoView::IsSortable(int row) const {
	return row == 0;
}

void CSysInfoView::Refresh() {
	m_OldSysPerfInfo = m_SysPerfInfo;
	m_OldPerfInfo = m_PerfInfo;
	::GetPerformanceInfo(&m_PerfInfo, sizeof(m_PerfInfo));
	m_SysPerfInfo = SystemInformation::GetPerformanceInformation();
}

LRESULT CSysInfoView::OnCreate(UINT, WPARAM, LPARAM, BOOL&) {
	m_hWndClient = m_List.Create(*this, rcDefault, nullptr, ListViewDefaultStyle & ~LVS_SHAREIMAGELISTS);
	m_List.SetExtendedListViewStyle(LVS_EX_DOUBLEBUFFER | LVS_EX_INFOTIP | LVS_EX_FULLROWSELECT);

	m_List.InsertColumn(0, L"Name", LVCFMT_LEFT, 150);
	m_List.InsertColumn(1, L"Value", LVCFMT_LEFT, 250);

	m_BasicSysInfo = SystemInformation::GetBasicSystemInfo();
	int count = (int)m_Items.size();
	m_List.SetItemCount(count);

	Refresh();
	SetTimer(1, 1000, nullptr);

	return 0;
}

void CSysInfoView::OnUpdate() {
	Refresh();
	m_List.RedrawItems(m_List.GetTopIndex(), m_List.GetTopIndex() + m_List.GetCountPerPage());
}

CSysInfoView::ItemData::ItemData(RowType type, PCWSTR name, uint32_t* pCurrent, uint32_t* pPrevious, ItemDataFlags flags)
	: Type(type), Name(name), Flags(flags) {
	Current.Value32 = pCurrent;
	Previous.Value32 = pPrevious;
}

CSysInfoView::ItemData::ItemData(RowType type, PCWSTR name, DWORD* pCurrent, DWORD* pPrevious, ItemDataFlags flags)
	: ItemData(type, name, (uint32_t*)pCurrent, (uint32_t*)pPrevious, flags) {
}

CSysInfoView::ItemData::ItemData(RowType type, PCWSTR name, uint64_t* pCurrent, uint64_t* pPrevious, ItemDataFlags flags)
	: Type(type), Name(name), Flags(flags) {
	Current.Value64 = pCurrent;
	Previous.Value64 = pPrevious;
}

CSysInfoView::ItemData::ItemData(RowType type, PCWSTR name, int64_t* pCurrent, int64_t* pPrevious, ItemDataFlags flags)
	: ItemData(type, name, (uint64_t*)pCurrent, (uint64_t*)pPrevious, flags) {
}

CSysInfoView::ItemData::ItemData(RowType type, PCWSTR name, ItemDataFlags flags) : Type(type), Name(name), Flags(flags) {
	Current.Value64 = Previous.Value64 = nullptr;
}
