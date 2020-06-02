#include "stdafx.h"
#include "ProcessesView.h"
#include "SortHelper.h"
#include <algorithm>
#include "Settings.h"
#include "StandardColors.h"
#include "FormatHelper.h"
#include "SelectColumnsDlg.h"

CProcessesView::CProcessesView(IMainFrame* frame) : m_pFrame(frame) {
}

CString CProcessesView::GetColumnText(HWND, int row, int col) const {
	auto& p = m_Processes[row];
	auto& px = GetProcessInfoEx(p.get());
	CString text;

	switch (static_cast<ProcessColumn>(col)) {
		case ProcessColumn::Name: return p->GetImageName().c_str();
		case ProcessColumn::UserName: return px.UserName().c_str();
		case ProcessColumn::Id: text.Format(L"%6u (0x%05X)", p->Id, p->Id); break;
		case ProcessColumn::Handles: text.Format(L"%u ", p->HandleCount); break;
		case ProcessColumn::Threads: text.Format(L"%u ", p->ThreadCount); break;
		case ProcessColumn::PeakThreads: text.Format(L"%u ", p->PeakThreads); break;
		case ProcessColumn::CPU:
			if (p->CPU > 0 && !px.IsTerminated) {
				auto value = p->CPU / 10000.0f;
				text.Format(L"%7.2f ", value);
			}
			break;
		case ProcessColumn::CreateTime: return CTime(*(FILETIME*)&p->CreateTime).Format(L"%x %X");
		case ProcessColumn::Session: text.Format(L"%2d  ", p->SessionId); break;
		case ProcessColumn::Attributes: return ProcessAttributesToString(px.GetAttributes(m_ProcMgr));
		case ProcessColumn::CPUTime: return FormatHelper::TimeSpanToString(p->UserTime + p->KernelTime);
		case ProcessColumn::CommitSize: return FormatHelper::FormatWithCommas(p->PagefileUsage >> 10);
		case ProcessColumn::PeakCommitSize: return FormatHelper::FormatWithCommas(p->PeakPagefileUsage >> 10);
		case ProcessColumn::WorkingSet: return FormatHelper::FormatWithCommas(p->WorkingSetSize >> 10);
		case ProcessColumn::PeakWorkingSet: return FormatHelper::FormatWithCommas(p->PeakWorkingSetSize >> 10);
		case ProcessColumn::VirtualSize: return FormatHelper::FormatWithCommas(p->VirtualSize >> 10);
		case ProcessColumn::PeakVirtualSize: return FormatHelper::FormatWithCommas(p->PeakVirtualSize >> 10);
		case ProcessColumn::PagedPool: return FormatHelper::FormatWithCommas(p->PagedPoolUsage >> 10);
		case ProcessColumn::NonPagedPool: return FormatHelper::FormatWithCommas(p->NonPagedPoolUsage >> 10);
		case ProcessColumn::PeakPagedPool: return FormatHelper::FormatWithCommas(p->PeakPagedPoolUsage >> 10);
		case ProcessColumn::PeakNonPagedPool: return FormatHelper::FormatWithCommas(p->PeakNonPagedPoolUsage >> 10);
		case ProcessColumn::Parent:
			if (p->ParentId > 0) {
				auto parent = m_ProcMgr.GetProcessById(p->ParentId);
				if (parent && parent->CreateTime < p->CreateTime) {
					text.Format(L"%6d (%s)", parent->Id, parent->GetImageName().c_str());
				}
				else {
					text.Format(L"%6d", p->ParentId);
				}
			}
			break;
	}

	return text;
}

int CProcessesView::GetRowImage(int row) const {
	return GetProcessInfoEx(m_Processes[row].get()).GetImageIndex(m_Images);
}

void CProcessesView::DoSort(const SortInfo* si) {
	if (si == nullptr)
		return;

	auto col = si->SortColumn;
	auto asc = si->SortAscending;

	std::sort(m_Processes.begin(), m_Processes.end(), [=](const auto& p1, const auto& p2) {
		switch (static_cast<ProcessColumn>(col)) {
			case ProcessColumn::Name: return SortHelper::SortStrings(p1->GetImageName(), p2->GetImageName(), asc);
			case ProcessColumn::Id: return SortHelper::SortNumbers(p1->Id, p2->Id, asc);
			case ProcessColumn::UserName: return SortHelper::SortStrings(GetProcessInfoEx(p1.get()).UserName(), GetProcessInfoEx(p2.get()).UserName(), asc);
			case ProcessColumn::Session: return SortHelper::SortNumbers(p1->SessionId, p2->SessionId, asc);
			case ProcessColumn::CPU: return SortHelper::SortNumbers(p1->CPU, p2->CPU, asc);
			case ProcessColumn::Parent: return SortHelper::SortNumbers(p1->ParentId, p2->ParentId, asc);
			case ProcessColumn::CreateTime: return SortHelper::SortNumbers(p1->CreateTime, p2->CreateTime, asc);
			case ProcessColumn::CommitSize: return SortHelper::SortNumbers(p1->PagefileUsage, p2->PagefileUsage, asc);
			case ProcessColumn::PeakCommitSize: return SortHelper::SortNumbers(p1->PeakPagefileUsage, p2->PeakPagefileUsage, asc);
			case ProcessColumn::Priority: return SortHelper::SortNumbers(p1->BasePriority, p2->BasePriority, asc);
			case ProcessColumn::Threads: return SortHelper::SortNumbers(p1->ThreadCount, p2->ThreadCount, asc);
			case ProcessColumn::Handles: return SortHelper::SortNumbers(p1->HandleCount, p2->HandleCount, asc);
			case ProcessColumn::WorkingSet: return SortHelper::SortNumbers(p1->WorkingSetSize, p2->WorkingSetSize, asc);
			case ProcessColumn::ExePath: return SortHelper::SortStrings(GetProcessInfoEx(p1.get()).GetExecutablePath(), GetProcessInfoEx(p2.get()).GetExecutablePath(), asc);
			case ProcessColumn::CPUTime: return SortHelper::SortNumbers(p1->KernelTime + p1->UserTime, p2->KernelTime + p2->UserTime, asc);
			case ProcessColumn::PeakThreads: return SortHelper::SortNumbers(p1->PeakThreads, p2->PeakThreads, asc);
			case ProcessColumn::VirtualSize: return SortHelper::SortNumbers(p1->VirtualSize, p2->VirtualSize, asc);
			case ProcessColumn::PeakWorkingSet: return SortHelper::SortNumbers(p1->PeakWorkingSetSize, p2->PeakWorkingSetSize, asc);
			case ProcessColumn::Attributes: return SortHelper::SortNumbers(GetProcessInfoEx(p1.get()).GetAttributes(m_ProcMgr), GetProcessInfoEx(p2.get()).GetAttributes(m_ProcMgr), asc);
			case ProcessColumn::PagedPool: return SortHelper::SortNumbers(p1->PagedPoolUsage, p2->PagedPoolUsage, asc);
			case ProcessColumn::PeakPagedPool: return SortHelper::SortNumbers(p1->PeakPagedPoolUsage, p2->PeakPagedPoolUsage, asc);
			case ProcessColumn::NonPagedPool: return SortHelper::SortNumbers(p1->NonPagedPoolUsage, p2->NonPagedPoolUsage, asc);
			case ProcessColumn::PeakNonPagedPool: return SortHelper::SortNumbers(p1->PeakNonPagedPoolUsage, p2->PeakNonPagedPoolUsage, asc);

		}
		return false;
		});

}

DWORD CProcessesView::OnPrePaint(int, LPNMCUSTOMDRAW cd) {
	m_hFont = (HFONT)::GetCurrentObject(cd->hdc, OBJ_FONT);

	return CDRF_NOTIFYITEMDRAW;
}

DWORD CProcessesView::OnItemPrePaint(int, LPNMCUSTOMDRAW) {
	return CDRF_NOTIFYSUBITEMDRAW;
}

DWORD CProcessesView::OnSubItemPrePaint(int, LPNMCUSTOMDRAW cd) {
	auto lcd = (LPNMLVCUSTOMDRAW)cd;
	auto sub = lcd->iSubItem;
	lcd->clrTextBk = CLR_INVALID;
	lcd->clrText = CLR_INVALID;
	int index = (int)cd->dwItemSpec;

	auto cm = GetColumnManager(m_List);
	auto real = cm->GetRealColumn(sub);
	if ((cm->GetColumn(real).Flags & ColumnFlags::Numeric) == ColumnFlags::Numeric)
		::SelectObject(cd->hdc, m_pFrame->GetMonoFont());
	else
		::SelectObject(cd->hdc, m_hFont);

	auto& p = m_Processes[index];
	auto& px = GetProcessInfoEx(p.get());
	if (real < 2) {
		GetProcessColors(px, lcd->clrTextBk, lcd->clrText);
	}
	else if(p->Id > 0 && (ProcessColumn)real == ProcessColumn::CPU)
		GetCPUColors(p->CPU / 10000, lcd->clrTextBk, lcd->clrText);

	return CDRF_SKIPPOSTPAINT | CDRF_NEWFONT;
}

LRESULT CProcessesView::OnCreate(UINT, WPARAM, LPARAM, BOOL&) {
	m_hWndClient = m_List.Create(m_hWnd, rcDefault, nullptr, ListViewDefaultStyle);
	m_List.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_LABELTIP | LVS_EX_HEADERDRAGDROP);

	m_Images.Create(16, 16, ILC_COLOR32, 64, 32);
	m_Images.AddIcon(AtlLoadSysIcon(IDI_APPLICATION));

	m_List.SetImageList(m_Images, LVSIL_SMALL);

	auto cm = GetColumnManager(m_List);
	cm->AddColumn(L"Name", LVCFMT_LEFT, 200, ColumnFlags::Visible | ColumnFlags::Mandatory);
	cm->AddColumn(L"Id", LVCFMT_RIGHT, 130, ColumnFlags::Visible | ColumnFlags::Mandatory | ColumnFlags::Numeric);
	cm->AddColumn(L"User Name", LVCFMT_LEFT, 200, ColumnFlags::Visible);
	cm->AddColumn(L"Session", LVCFMT_RIGHT, 60, ColumnFlags::Visible | ColumnFlags::Numeric);
	cm->AddColumn(L"CPU (%)", LVCFMT_RIGHT, 90, ColumnFlags::Visible | ColumnFlags::Numeric);
	cm->AddColumn(L"CPU Time", LVCFMT_RIGHT, 120, ColumnFlags::Numeric);
	cm->AddColumn(L"Parent", LVCFMT_LEFT, 180, ColumnFlags::None);
	cm->AddColumn(L"Base Priority", LVCFMT_LEFT, 80, ColumnFlags::Numeric);
	cm->AddColumn(L"Priority Class", LVCFMT_LEFT, 80, ColumnFlags::Visible);
	cm->AddColumn(L"Threads", LVCFMT_RIGHT, 60, ColumnFlags::Visible | ColumnFlags::Numeric);
	cm->AddColumn(L"Peak Threads", LVCFMT_RIGHT, 60, ColumnFlags::Numeric);
	cm->AddColumn(L"Handles", LVCFMT_RIGHT, 70, ColumnFlags::Visible | ColumnFlags::Numeric);
	cm->AddColumn(L"Attributes", LVCFMT_LEFT, 100, ColumnFlags::Visible);
	cm->AddColumn(L"Path", LVCFMT_LEFT, 200, ColumnFlags::None);
	cm->AddColumn(L"Create Time", LVCFMT_LEFT, 130, ColumnFlags::Visible);
	cm->AddColumn(L"Commit (K)", LVCFMT_RIGHT, 110, ColumnFlags::Visible | ColumnFlags::Numeric);
	cm->AddColumn(L"Peak Commit (K)", LVCFMT_RIGHT, 120, ColumnFlags::Numeric);
	cm->AddColumn(L"Working Set (K)", LVCFMT_RIGHT, 110, ColumnFlags::Visible | ColumnFlags::Numeric);
	cm->AddColumn(L"Peak WS (K)", LVCFMT_RIGHT, 120, ColumnFlags::Numeric);
	cm->AddColumn(L"Virtual Size (K)", LVCFMT_RIGHT, 110, ColumnFlags::Visible | ColumnFlags::Numeric);
	cm->AddColumn(L"Peak Virtual Size (K)", LVCFMT_RIGHT, 120, ColumnFlags::Numeric);
	cm->AddColumn(L"Paged Pool (K)", LVCFMT_RIGHT, 110, ColumnFlags::Numeric);
	cm->AddColumn(L"Peak Paged (K)", LVCFMT_RIGHT, 110, ColumnFlags::Numeric);
	cm->AddColumn(L"Non Paged (K)", LVCFMT_RIGHT, 110, ColumnFlags::Numeric);
	cm->AddColumn(L"Peak NPaged (K)", LVCFMT_RIGHT, 120, ColumnFlags::Numeric);

	cm->UpdateColumns();

	Refresh();
	SetTimer(1, m_UpdateInterval, nullptr);
	
	return 0;
}

LRESULT CProcessesView::OnTimer(UINT, WPARAM id, LPARAM, BOOL&) {
	if (id == 1)
		Refresh();
	return 0;
}

LRESULT CProcessesView::OnDestroy(UINT, WPARAM, LPARAM, BOOL&) {
	return LRESULT();
}

LRESULT CProcessesView::OnActivate(UINT, WPARAM activate, LPARAM, BOOL&) {
	if (activate) {
		SetTimer(1, m_UpdateInterval, nullptr);
	}
	else {
		KillTimer(1);
	}
	return 0;
}

LRESULT CProcessesView::OnRefresh(WORD, WORD, HWND, BOOL&) {
	Refresh();
	return 0;
}

void CProcessesView::Refresh() {
	bool first = m_Processes.empty();
	auto count = (int)m_ProcMgr.EnumProcesses();
	if (first) {
		m_Processes = m_ProcMgr.GetProcesses();
		m_List.SetItemCount(count);
		return;
	}

	count = (int)m_Processes.size();
	for(int i = 0; i < count; i++) {
		auto& p = m_Processes[i];
		auto& px = GetProcessInfoEx(p.get());
		if (px.IsTerminated && ::GetTickCount64() > px.TargetTime) {
			m_ProcessesEx.erase(p.get());
			m_Processes.erase(m_Processes.begin() + i);
			i--;
			count--;
			continue;
		}
		if (px.IsNew && ::GetTickCount64() > px.TargetTime) {
			px.IsNew = false;
		}
	}

	for (auto& p : m_ProcMgr.GetNewProcesses()) {
		m_Processes.push_back(p);
		count++;
		auto& px = GetProcessInfoEx(p.get());
		px.IsNew = true;
		px.TargetTime = ::GetTickCount64() + 2000;
	}

	for (auto& p : m_ProcMgr.GetTerminatedProcesses()) {
		auto& px = GetProcessInfoEx(p.get());
		px.IsTerminated = true;
		px.TargetTime = ::GetTickCount64() + 2000;
	}

	DoSort(GetSortInfo(m_List));
	m_List.SetItemCountEx(count, LVSICF_NOSCROLL | LVSICF_NOINVALIDATEALL);
	auto top = m_List.GetTopIndex();
	m_List.RedrawItems(top , top + m_List.GetCountPerPage());
}

CString CProcessesView::ProcessAttributesToString(ProcessAttributes attributes) {
	CString text;

	static const struct {
		ProcessAttributes Attribute;
		PCWSTR Text;
	} attribs[] = {
		{ ProcessAttributes::Managed, L"Managed" },
		{ ProcessAttributes::Immersive, L"Immersive" },
		{ ProcessAttributes::Protected, L"Protected" },
		{ ProcessAttributes::Secure, L"Secure" },
		{ ProcessAttributes::Service, L"Service" },
		{ ProcessAttributes::InJob, L"In Job" },
	};

	for (auto& item : attribs)
		if ((item.Attribute & attributes) == item.Attribute)
			text += CString(item.Text) + ", ";
	if (!text.IsEmpty())
		text = text.Mid(0, text.GetLength() - 2);
	return text;
}

void CProcessesView::GetCPUColors(int cpu, COLORREF& bk, COLORREF& text) {
	if (cpu > 90) {
		bk = StandardColors::Black;
		text = StandardColors::White;
	}
	else if (cpu > 80) {
		bk = StandardColors::DarkRed;
		text = StandardColors::White;
	}
	else if (cpu > 70) {
		bk = StandardColors::DarkOrange;
		text = StandardColors::Black;
	}
	else if (cpu > 60) {
		bk = StandardColors::Orange;
		text = StandardColors::Black;
	}
	else if (cpu > 50) {
		bk = StandardColors::Yellow;
		text = StandardColors::Black;
	}
	else if (cpu > 40) {
		bk = StandardColors::DarkBlue;
		text = StandardColors::White;
	}
	else if (cpu > 30) {
		bk = StandardColors::Blue;
		text = StandardColors::Black;
	}
	else if (cpu > 20) {
		bk = StandardColors::LightBlue;
		text = StandardColors::Black;
	}
	else if (cpu > 15) {
		bk = StandardColors::Cyan;
		text = StandardColors::Black;
	}
	else if (cpu > 10) {
		bk = StandardColors::LightCyan;
		text = StandardColors::Black;
	}
	else if (cpu > 5) {
		bk = StandardColors::LimeGreen;
		text = StandardColors::Black;
	}
}


ProcessInfoEx& CProcessesView::GetProcessInfoEx(WinSys::ProcessInfo* pi) const {
	auto it = m_ProcessesEx.find(pi);
	if (it != m_ProcessesEx.end())
		return it->second;

	ProcessInfoEx px(pi);
	auto pair = m_ProcessesEx.insert({ pi, std::move(px) });
	return pair.first->second;
}

void CProcessesView::GetProcessColors(const ProcessInfoEx& px, COLORREF& bk, COLORREF& text) const {
	auto& colors = m_pFrame->GetSettings().Processes.Colors;
	int i;

	if (colors[i = (int)ProcessColorIndex::DeletedObjects].Enabled && px.IsTerminated) {
		bk = colors[i].Color;
		text = colors[i].TextColor;
		return;
	}
	if (colors[i = (int)ProcessColorIndex::NewObjects].Enabled && px.IsNew) {
		bk = colors[i].Color;
		text = colors[i].TextColor;
		return;
	}

	auto attributes = px.GetAttributes(m_ProcMgr);
	if (colors[i = (int)ProcessColorIndex::Manageed].Enabled && (attributes & ProcessAttributes::Managed) == ProcessAttributes::Managed) {
		bk = colors[i].Color;
		text = colors[i].TextColor;
		return;
	}

	if (colors[i = (int)ProcessColorIndex::Immersive].Enabled && (attributes & ProcessAttributes::Immersive) == ProcessAttributes::Immersive) {
		bk = colors[i].Color;
		text = colors[i].TextColor;
		return;
	}
	if (colors[i = (int)ProcessColorIndex::Secure].Enabled && (attributes & ProcessAttributes::Secure) == ProcessAttributes::Secure) {
		bk = colors[i].Color;
		text = colors[i].TextColor;
		return;
	}
	if (colors[i = (int)ProcessColorIndex::Protected].Enabled && (attributes & ProcessAttributes::Protected) == ProcessAttributes::Protected) {
		bk = colors[i].Color;
		text = colors[i].TextColor;
		return;
	}
	if (colors[i = (int)ProcessColorIndex::Services].Enabled && (attributes & ProcessAttributes::Service) == ProcessAttributes::Service) {
		bk = colors[i].Color;
		text = colors[i].TextColor;
		return;
	}
	if (colors[i = (int)ProcessColorIndex::InJob].Enabled && (attributes & ProcessAttributes::InJob) == ProcessAttributes::InJob) {
		bk = colors[i].Color;
		text = colors[i].TextColor;
		return;
	}

	bk = text = CLR_INVALID;
}

LRESULT CProcessesView::OnHideColumn(WORD, WORD, HWND, BOOL&) {
	auto cm = GetColumnManager(m_List);
	cm->SetVisible(m_SelectedHeader, false);
	cm->UpdateColumns();
	m_List.RedrawItems(m_List.GetTopIndex(), m_List.GetTopIndex() + m_List.GetCountPerPage());

	return 0;
}

LRESULT CProcessesView::OnSelectColumns(WORD, WORD, HWND, BOOL&) {
	CSelectColumnsDlg dlg(GetColumnManager(m_List));
	if (IDOK == dlg.DoModal()) {
		m_List.RedrawItems(m_List.GetTopIndex(), m_List.GetTopIndex() + m_List.GetCountPerPage());
	}

	return 0;
}

LRESULT CProcessesView::OnListRightClick(int, LPNMHDR hdr, BOOL&) {
	POINT pt;
	::GetCursorPos(&pt);
	CPoint pt2(pt);
	auto header = m_List.GetHeader();
	header.ScreenToClient(&pt);
	HDHITTESTINFO hti;
	hti.pt = pt;
	int index = header.HitTest(&hti);
	CMenuHandle hSubMenu;
	CMenu menu;
	menu.LoadMenu(IDR_CONTEXT);
	if (index >= 0) {
		// right-click header
		hSubMenu = menu.GetSubMenu(7);
		m_SelectedHeader = index;
		m_pFrame->GetUpdateUI()->UIEnable(ID_HEADER_HIDECOLUMN,
			(GetColumnManager(m_List)->GetColumn(index).Flags & ColumnFlags::Mandatory) == ColumnFlags::None);
		pt = pt2;
	}
	else {
		index = m_List.GetSelectedIndex();
		if (index >= 0) {
			auto item = (NMITEMACTIVATE*)hdr;
			hSubMenu = menu.GetSubMenu(8);
			pt = item->ptAction;
			m_List.ClientToScreen(&pt);
		}
	}
	if (hSubMenu) {
		auto id = (UINT)m_pFrame->TrackPopupMenu(hSubMenu, *this, &pt, TPM_RETURNCMD);
		if (id) {
			m_pFrame->SendFrameMessage(WM_COMMAND, id, reinterpret_cast<LPARAM>(m_Processes[index].get()));
		}
	}
	return 0;
}

LRESULT CProcessesView::OnPause(WORD, WORD, HWND, BOOL&) {
	if (m_UpdateInterval > 0) {
		m_OldInterval = m_UpdateInterval;
		m_UpdateInterval = 0;
		KillTimer(1);
	}
	else {
		m_UpdateInterval = m_OldInterval;
		SetTimer(1, m_UpdateInterval, nullptr);
	}

	m_pFrame->GetUpdateUI()->UISetCheck(ID_VIEW_PAUSE, m_UpdateInterval == 0);
	return 0;
}
