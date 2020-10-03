#include "pch.h"
#include "SysInfoView.h"
#include "FormatHelper.h"
#include "SortHelper.h"

using namespace WinSys;

static PCWSTR names[] = {
    L"Version", L"RAM", L"Processors", L"Commit Limit", L"Processes", L"Threads", L"Handles",
};

CString CSysInfoView::GetColumnText(HWND, int row, int col) const {
    if (col == 0) {
        return names[(int)m_Items[row]];
    }
    CString text;
    switch (m_Items[row]) {
        case RowType::TotalCPUs:
            text.Format(L"%u", SystemInformation::GetBasicSystemInfo().NumberOfProcessors);
            break;
        case RowType::TotalRAM:
            text = FormatHelper::FormatWithCommas(SystemInformation::GetBasicSystemInfo().TotalPhysicalInPages >> 8) + L" MB";
            text.Format(L"%s (%u GB)", text, (ULONG)((SystemInformation::GetBasicSystemInfo().TotalPhysicalInPages + (1 << 17)) >> 18));
            break;
        case RowType::CommitLimit:
            return FormatHelper::FormatWithCommas(SystemInformation::GetBasicSystemInfo().CommitLimitInPages >> 8) + L" MB";

        case RowType::Processes:
            return FormatHelper::FormatWithCommas(m_PerfInfo.ProcessCount);
    }
    return text;
}

void CSysInfoView::Refresh() {
    ::GetPerformanceInfo(&m_PerfInfo, sizeof(m_PerfInfo));
}

LRESULT CSysInfoView::OnCreate(UINT, WPARAM, LPARAM, BOOL&) {
    m_hWndClient = m_List.Create(*this, rcDefault, nullptr, ListViewDefaultStyle & ~LVS_SHAREIMAGELISTS);
    m_List.SetExtendedListViewStyle(LVS_EX_DOUBLEBUFFER | LVS_EX_INFOTIP | LVS_EX_FULLROWSELECT);

    m_List.InsertColumn(0, L"Name", LVCFMT_LEFT, 150);
    m_List.InsertColumn(1, L"Value", LVCFMT_LEFT, 450);

    int count = _countof(names);
    m_Items.reserve(count);
    for (int i = 0; i < count; i++)
        m_Items.push_back((RowType)i);

    m_List.SetItemCount(count);

    Refresh();
    SetTimer(1, 1000, nullptr);

    return 0;
}

LRESULT CSysInfoView::OnTimer(UINT, WPARAM id, LPARAM, BOOL&) {
    if (id == 1) {
        Refresh();
        m_List.RedrawItems(m_List.GetTopIndex(), m_List.GetTopIndex() + m_List.GetCountPerPage());
    }
    return 0;
}
