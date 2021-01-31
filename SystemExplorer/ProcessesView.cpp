#include "pch.h"
#include "ProcessesView.h"
#include "SortHelper.h"
#include <algorithm>
#include "Settings.h"
#include "StandardColors.h"
#include "FormatHelper.h"
#include "SelectColumnsDlg.h"
#include "DriverHelper.h"
#include "ProcessPropertiesDlg.h"
#include "ListViewHelper.h"
#include "ClipboardHelper.h"
#include "ImageIconCache.h"
#include <ProcessInfo.h>

using namespace WinSys;

CProcessesView::CProcessesView(IMainFrame* frame) : CViewBase(frame) {
}

CString CProcessesView::GetColumnText(HWND, int row, int col) const {
	auto& p = m_Processes[row];
	auto& px = GetProcessInfoEx(p.get());
	return FormatHelper::GetProcessColumnValue((ProcessColumn)col, m_ProcMgr, p.get(), px);
}

int CProcessesView::GetRowImage(HWND, int row) const {
	return ImageIconCache::Get().GetIcon(GetProcessInfoEx(m_Processes[row].get()).GetExecutablePath());	//GetImageIndex(m_Images);
}

void CProcessesView::DoSort(const SortInfo* si) {
	if (si == nullptr)
		return;

	auto col = si->SortColumn;
	auto asc = si->SortAscending;

	std::sort(m_Processes.begin(), m_Processes.end(), [=](const auto& p1, const auto& p2) {
		switch (static_cast<ProcessColumn>(col)) {
			case ProcessColumn::Name: return SortHelper::SortStrings(p1->GetImageName(), p2->GetImageName(), asc);
			case ProcessColumn::PackageFullName: return SortHelper::SortStrings(p1->GetPackageFullName(), p2->GetPackageFullName(), asc);
			case ProcessColumn::Id: return SortHelper::SortNumbers(p1->Id, p2->Id, asc);
			case ProcessColumn::UserName: return SortHelper::SortStrings(GetProcessInfoEx(p1.get()).UserName(), GetProcessInfoEx(p2.get()).UserName(), asc);
			case ProcessColumn::Session: return SortHelper::SortNumbers(p1->SessionId, p2->SessionId, asc);
			case ProcessColumn::PriorityClass: return SortHelper::SortNumbers(GetProcessInfoEx(p1.get()).GetPriorityClass(), GetProcessInfoEx(p2.get()).GetPriorityClass(), asc);
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
			case ProcessColumn::MemoryPriority: return SortHelper::SortNumbers(GetProcessInfoEx(p1.get()).GetMemoryPriority(), GetProcessInfoEx(p2.get()).GetMemoryPriority(), asc);
			case ProcessColumn::IoPriority: return SortHelper::SortNumbers(GetProcessInfoEx(p1.get()).GetIoPriority(), GetProcessInfoEx(p2.get()).GetIoPriority(), asc);
			case ProcessColumn::CommandLine: return SortHelper::SortStrings(GetProcessInfoEx(p1.get()).GetCommandLine(), GetProcessInfoEx(p2.get()).GetCommandLine(), asc);
			case ProcessColumn::IoReadBytes: return SortHelper::SortNumbers(p1->ReadTransferCount, p2->ReadTransferCount, asc);
			case ProcessColumn::IoWriteBytes: return SortHelper::SortNumbers(p1->WriteTransferCount, p2->WriteTransferCount, asc);
			case ProcessColumn::IoOtherBytes: return SortHelper::SortNumbers(p1->OtherTransferCount, p2->OtherTransferCount, asc);
			case ProcessColumn::IoReads: return SortHelper::SortNumbers(p1->ReadOperationCount, p2->ReadOperationCount, asc);
			case ProcessColumn::IoWrites: return SortHelper::SortNumbers(p1->WriteOperationCount, p2->WriteOperationCount, asc);
			case ProcessColumn::IoOther: return SortHelper::SortNumbers(p1->OtherOperationCount, p2->OtherOperationCount, asc);
			case ProcessColumn::GDIObjects: return SortHelper::SortNumbers(GetProcessInfoEx(p1.get()).GetGdiObjects(), GetProcessInfoEx(p2.get()).GetGdiObjects(), asc);
			case ProcessColumn::UserObjects: return SortHelper::SortNumbers(GetProcessInfoEx(p1.get()).GetUserObjects(), GetProcessInfoEx(p2.get()).GetUserObjects(), asc);
			case ProcessColumn::PeakGdiObjects: return SortHelper::SortNumbers(GetProcessInfoEx(p1.get()).GetPeakGdiObjects(), GetProcessInfoEx(p2.get()).GetPeakGdiObjects(), asc);
			case ProcessColumn::PeakUserObjects: return SortHelper::SortNumbers(GetProcessInfoEx(p1.get()).GetPeakUserObjects(), GetProcessInfoEx(p2.get()).GetPeakUserObjects(), asc);
			case ProcessColumn::KernelTime: return SortHelper::SortNumbers(p1->KernelTime, p2->KernelTime, asc);
			case ProcessColumn::UserTime: return SortHelper::SortNumbers(p1->UserTime, p2->UserTime, asc);
			case ProcessColumn::Elevated: return SortHelper::SortBoolean(GetProcessInfoEx(p1.get()).IsElevated(), GetProcessInfoEx(p2.get()).IsElevated(), asc);
			case ProcessColumn::Integrity: return SortHelper::SortNumbers(GetProcessInfoEx(p1.get()).GetIntegrityLevel(), GetProcessInfoEx(p2.get()).GetIntegrityLevel(), asc);
			case ProcessColumn::Virtualized: return SortHelper::SortNumbers(GetProcessInfoEx(p1.get()).GetVirtualizationState(), GetProcessInfoEx(p2.get()).GetVirtualizationState(), asc);
			case ProcessColumn::JobId: return SortHelper::SortNumbers(p1->JobObjectId, p2->JobObjectId, asc);
			case ProcessColumn::WindowTitle: return SortHelper::SortStrings(GetProcessInfoEx(p1.get()).GetWindowTitle(), GetProcessInfoEx(p2.get()).GetWindowTitle(), asc);
			case ProcessColumn::Platform: return SortHelper::SortNumbers(GetProcessInfoEx(p1.get()).GetBitness(), GetProcessInfoEx(p2.get()).GetBitness(), asc);
			case ProcessColumn::Description: return SortHelper::SortStrings(GetProcessInfoEx(p1.get()).GetDescription(), GetProcessInfoEx(p2.get()).GetDescription(), asc);
			case ProcessColumn::Company: return SortHelper::SortStrings(GetProcessInfoEx(p1.get()).GetCompanyName(), GetProcessInfoEx(p2.get()).GetCompanyName(), asc);
			case ProcessColumn::DpiAwareness: return SortHelper::SortNumbers(GetProcessInfoEx(p1.get()).GetDpiAwareness(), GetProcessInfoEx(p2.get()).GetDpiAwareness(), asc);
		}
		return false;
		});

}

bool CProcessesView::OnDoubleClickList(int row, int col, POINT& pt) {
	if (row < 0 || m_Processes[row]->Id == 0)
		return false;
	ShowProperties(row);

	return true;
}

DWORD CProcessesView::OnPrePaint(int, LPNMCUSTOMDRAW cd) {
	m_hFont = (HFONT)::GetCurrentObject(cd->hdc, OBJ_FONT);

	return CDRF_NOTIFYITEMDRAW;
}

DWORD CProcessesView::OnItemPrePaint(int, LPNMCUSTOMDRAW cd) {
	if (cd->hdr.hwndFrom == m_List) {
		auto lcd = (LPNMLVCUSTOMDRAW)cd;
		auto sub = lcd->iSubItem;
		lcd->clrTextBk = CLR_INVALID;
		lcd->clrText = CLR_INVALID;
		int index = (int)cd->dwItemSpec;
		auto& p = m_Processes[index];
		auto& px = GetProcessInfoEx(p.get());
		GetProcessColors(px, lcd->clrTextBk, lcd->clrText);

		return CDRF_NOTIFYSUBITEMDRAW;
	}
	return CDRF_DODEFAULT;
}

DWORD CProcessesView::OnSubItemPrePaint(int, LPNMCUSTOMDRAW cd) {
	auto lcd = (LPNMLVCUSTOMDRAW)cd;
	auto sub = lcd->iSubItem;
	int index = (int)cd->dwItemSpec;

	auto cm = GetColumnManager(m_List);
	auto real = static_cast<ProcessColumn>(cm->GetRealColumn(sub));
	if ((cm->GetColumn((int)real).Flags & ColumnFlags::Numeric) == ColumnFlags::Numeric)
		::SelectObject(cd->hdc, GetFrame()->GetMonoFont());
	else
		::SelectObject(cd->hdc, m_hFont);

	if (real == ProcessColumn::CPU) {
		auto& p = m_Processes[index];
		if (p->Id > 0) {
			lcd->clrTextBk = CLR_INVALID;
			lcd->clrText = CLR_INVALID;
			Settings::Get().GetCPUColors(p->CPU / 10000, lcd->clrTextBk, lcd->clrText);
			m_LastTimeCPU = true;
		}
	}
	else if(m_LastTimeCPU) {
		auto& p = m_Processes[index];
		auto& px = GetProcessInfoEx(p.get());
		GetProcessColors(px, lcd->clrTextBk, lcd->clrText);
		m_LastTimeCPU = false;
	}
	return CDRF_SKIPPOSTPAINT | CDRF_NEWFONT;
}

LRESULT CProcessesView::OnCreate(UINT, WPARAM, LPARAM, BOOL& bHandled) {
	ToolBarButtonInfo buttons[] = {
		{ ID_PROCESS_THREADS, IDI_THREAD, 0, L"Threads" },
		{ ID_HANDLES_SHOWHANDLEINPROCESS, IDI_HANDLES, 0, L"Handles" },
		{ ID_PROCESS_MODULES, IDI_DLL, 0, L"Modules" },
		{ ID_PROCESS_MEMORYMAP, IDI_DRAM, 0, L"Memory" },
		{ ID_PROCESS_HEAPS, IDI_HEAP, 0, L"Heaps" },
		{ 0 },
		{ ID_PROCESS_KILL, IDI_DELETE },
		{ 0 },
		{ ID_HEADER_COLUMNS, IDI_EDITCOLUMNS, 0, L"Columns" },
		{ 0 },
		{ ID_PROCESS_COLORS, IDI_COLORWHEEL, 0, L"Colors" },
	};
	CreateAndInitToolBar(buttons, _countof(buttons));

	m_hWndClient = m_List.Create(m_hWnd, rcDefault, nullptr, ListViewDefaultStyle);// &~LVS_SHAREIMAGELISTS);
	m_List.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_LABELTIP | LVS_EX_HEADERDRAGDROP);

	m_List.SetImageList(ImageIconCache::Get().GetImageList(), LVSIL_SMALL);

	auto cm = GetColumnManager(m_List);
	cm->AddColumn(L"Name", LVCFMT_LEFT, 200, ColumnFlags::Visible | ColumnFlags::Mandatory | ColumnFlags::Const);
	cm->AddColumn(L"Id", LVCFMT_RIGHT, 130, ColumnFlags::Visible | ColumnFlags::Mandatory | ColumnFlags::Numeric | ColumnFlags::Const);
	cm->AddColumn(L"User Name", LVCFMT_LEFT, 200, ColumnFlags::Visible | ColumnFlags::Const);
	cm->AddColumn(L"Session", LVCFMT_RIGHT, 60, ColumnFlags::Visible | ColumnFlags::Numeric | ColumnFlags::Const);
	cm->AddColumn(L"Performance\\CPU (%)", LVCFMT_RIGHT, 80, ColumnFlags::Visible | ColumnFlags::Numeric);
	cm->AddColumn(L"Performance\\CPU Time", LVCFMT_RIGHT, 120, ColumnFlags::Numeric);
	cm->AddColumn(L"Parent", LVCFMT_LEFT, 180, ColumnFlags::Const);
	cm->AddColumn(L"Performance\\Base Priority", LVCFMT_LEFT, 80, ColumnFlags::Numeric);
	cm->AddColumn(L"Performance\\Priority Class", LVCFMT_LEFT, 120, ColumnFlags::Visible);
	cm->AddColumn(L"Performance\\Threads", LVCFMT_RIGHT, 60, ColumnFlags::Visible | ColumnFlags::Numeric);
	cm->AddColumn(L"Performance\\Peak Threads", LVCFMT_RIGHT, 60, ColumnFlags::Numeric);
	cm->AddColumn(L"Performance\\Handles", LVCFMT_RIGHT, 70, ColumnFlags::Visible | ColumnFlags::Numeric);
	cm->AddColumn(L"Attributes", LVCFMT_LEFT, 100, ColumnFlags::Visible | ColumnFlags::Const);
	cm->AddColumn(L"Image Path", LVCFMT_LEFT, 300, ColumnFlags::Const);
	cm->AddColumn(L"Create Time", LVCFMT_LEFT, 160, ColumnFlags::Visible | ColumnFlags::Numeric | ColumnFlags::Const);
	cm->AddColumn(L"Memory\\Commit (K)", LVCFMT_RIGHT, 110, ColumnFlags::Visible | ColumnFlags::Numeric);
	cm->AddColumn(L"Memory\\Peak Commit (K)", LVCFMT_RIGHT, 120, ColumnFlags::Numeric);
	cm->AddColumn(L"Memory\\Working Set (K)", LVCFMT_RIGHT, 110, ColumnFlags::Visible | ColumnFlags::Numeric);
	cm->AddColumn(L"Memory\\Peak WS (K)", LVCFMT_RIGHT, 120, ColumnFlags::Numeric);
	cm->AddColumn(L"Memory\\Virtual Size (K)", LVCFMT_RIGHT, 110, ColumnFlags::Numeric);
	cm->AddColumn(L"Memory\\Peak Virtual Size (K)", LVCFMT_RIGHT, 120, ColumnFlags::Numeric);
	cm->AddColumn(L"Memory\\Paged Pool (K)", LVCFMT_RIGHT, 110, ColumnFlags::Numeric);
	cm->AddColumn(L"Memory\\Peak Paged (K)", LVCFMT_RIGHT, 110, ColumnFlags::Numeric);
	cm->AddColumn(L"Memory\\Non Paged (K)", LVCFMT_RIGHT, 110, ColumnFlags::Numeric);
	cm->AddColumn(L"Memory\\Peak NPaged (K)", LVCFMT_RIGHT, 120, ColumnFlags::Numeric);
	cm->AddColumn(L"Performance\\kernel Time", LVCFMT_RIGHT, 120, ColumnFlags::Numeric);
	cm->AddColumn(L"Performance\\User Time", LVCFMT_RIGHT, 120, ColumnFlags::Numeric);
	cm->AddColumn(L"Performance\\I/O Priority", LVCFMT_LEFT, 80, ColumnFlags::None);
	cm->AddColumn(L"Performance\\Memory Priority", LVCFMT_RIGHT, 80, ColumnFlags::Numeric);
	cm->AddColumn(L"Command Line", LVCFMT_LEFT, 250, ColumnFlags::Const);
	cm->AddColumn(L"Package Name", LVCFMT_LEFT, 250, ColumnFlags::Const);
	cm->AddColumn(L"Job Object Id", LVCFMT_RIGHT, 100, ColumnFlags::Numeric | ColumnFlags::Const);
	cm->AddColumn(L"I/O\\I/O Read Bytes", LVCFMT_RIGHT, 120, ColumnFlags::Numeric);
	cm->AddColumn(L"I/O\\I/O Write Bytes", LVCFMT_RIGHT, 120, ColumnFlags::Numeric);
	cm->AddColumn(L"I/O\\I/O Other Bytes", LVCFMT_RIGHT, 120, ColumnFlags::Numeric);
	cm->AddColumn(L"I/O\\I/O Reads", LVCFMT_RIGHT, 90, ColumnFlags::Numeric);
	cm->AddColumn(L"I/O\\I/O Writes", LVCFMT_RIGHT, 90, ColumnFlags::Numeric);
	cm->AddColumn(L"I/O\\I/O Other", LVCFMT_RIGHT, 90, ColumnFlags::Numeric);
	cm->AddColumn(L"GUI\\GDI Objects", LVCFMT_RIGHT, 90, ColumnFlags::Numeric);
	cm->AddColumn(L"GUI\\User Objects", LVCFMT_RIGHT, 90, ColumnFlags::Numeric);
	cm->AddColumn(L"GUI\\Peak GDI Objects", LVCFMT_RIGHT, 90, ColumnFlags::Numeric);
	cm->AddColumn(L"GUI\\Peak User Objects", LVCFMT_RIGHT, 90, ColumnFlags::Numeric);
	cm->AddColumn(L"Token\\Integrity", LVCFMT_LEFT, 70, ColumnFlags::None);
	cm->AddColumn(L"Token\\Elevated", LVCFMT_LEFT, 60, ColumnFlags::Const);
	cm->AddColumn(L"Token\\Virtualization", LVCFMT_LEFT, 70, ColumnFlags::Const);
	cm->AddColumn(L"Window Title", LVCFMT_LEFT, 200, ColumnFlags::None);
	cm->AddColumn(L"Platform", LVCFMT_LEFT, 60, ColumnFlags::Const);
	cm->AddColumn(L"Description", LVCFMT_LEFT, 250, ColumnFlags::Const | ColumnFlags::Visible);
	cm->AddColumn(L"Company Name", LVCFMT_LEFT, 150, ColumnFlags::Const | ColumnFlags::Visible);
	cm->AddColumn(L"DPI Awareness", LVCFMT_LEFT, 80, ColumnFlags::None);

	cm->UpdateColumns();

	Refresh();
	UpdateUI();

	return 0;
}

void CProcessesView::OnUpdate() {
	Refresh();
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

	auto tick = ::GetTickCount64();
	count = (int)m_Processes.size();
	for (int i = 0; i < count; i++) {
		auto& p = m_Processes[i];
		auto& px = GetProcessInfoEx(p.get());
		if (px.IsTerminated && tick > px.TargetTime) {
			m_ProcessesEx.erase(p.get());
			m_Processes.erase(m_Processes.begin() + i);
			i--;
			count--;
			continue;
		}
		if (px.IsNew && tick > px.TargetTime) {
			px.IsNew = false;
		}
	}

	for (auto& p : m_ProcMgr.GetNewProcesses()) {
		m_Processes.push_back(p);
		count++;
		auto& px = GetProcessInfoEx(p.get());
		px.IsNew = true;
		px.TargetTime = tick + 2000;
	}

	for (auto& p : m_ProcMgr.GetTerminatedProcesses()) {
		auto& px = GetProcessInfoEx(p.get());
		px.IsTerminated = true;
		px.TargetTime = tick + 2000;
	}

	auto si = GetSortInfo(m_List);
	if (!m_ProcMgr.GetNewProcesses().empty() || (si && ((GetColumnManager(m_List)->GetColumn(si->SortColumn).Flags & ColumnFlags::Const) != ColumnFlags::Const)))
		DoSort(si);
	m_List.SetItemCountEx(count, LVSICF_NOSCROLL | LVSICF_NOINVALIDATEALL);
	auto top = m_List.GetTopIndex();
	m_List.RedrawItems(top, top + m_List.GetCountPerPage());
}

void CProcessesView::UpdateUI() {
	int selected = m_List.GetSelectedIndex();

	auto ui = GetFrame()->GetUpdateUI();
	ui->UIEnable(ID_PROCESS_KILL, selected >= 0);
	ui->UIEnable(ID_PROCESS_KILLBYNAME, selected >= 0);
	ui->UIEnable(ID_HANDLES_SHOWHANDLEINPROCESS, selected >= 0);
	ui->UIEnable(ID_PROCESS_GOTOFILELOCATION, selected >= 0);
	ui->UIEnable(ID_EDIT_PROPERTIES, selected >= 0 && m_Processes[selected]->Id > 0);

	if (selected < 0)
		return;

	auto& p = m_Processes[selected];
	auto& px = GetProcessInfoEx(p.get());
	UINT id = 0;
	switch (px.GetPriorityClass()) {
		case ProcessPriorityClass::Idle: id = ID_PRIORITYCLASS_IDLE; break;
		case ProcessPriorityClass::BelowNormal: id = ID_PRIORITYCLASS_BELOWNORMAL; break;
		case ProcessPriorityClass::Normal: id = ID_PRIORITYCLASS_NORMAL; break;
		case ProcessPriorityClass::AboveNormal: id = ID_PRIORITYCLASS_ABOVENORMAL; break;
		case ProcessPriorityClass::High: id = ID_PRIORITYCLASS_HIGH; break;
		case ProcessPriorityClass::Realtime: id = ID_PRIORITYCLASS_REALTIME; break;
	}

	if (id)
		ui->UISetRadioMenuItem(id, ID_PRIORITYCLASS_IDLE, ID_PRIORITYCLASS_REALTIME);
}

void CProcessesView::ShowProperties(int row) {
	auto& process = m_Processes[row];
	auto& px = GetProcessInfoEx(process.get());
	auto dlg = new CProcessPropertiesDlg(m_ProcMgr, px);
	dlg->Create(::GetAncestor(m_hWnd, GA_ROOT));
	dlg->ShowWindow(SW_SHOW);
}

ProcessInfoEx& CProcessesView::GetProcessInfoEx(ProcessInfo* pi) const {
	auto it = m_ProcessesEx.find(pi);
	if (it != m_ProcessesEx.end())
		return it->second;

	ProcessInfoEx px(pi);
	auto pair = m_ProcessesEx.insert({ pi, std::move(px) });
	return pair.first->second;
}

void CProcessesView::GetProcessColors(const ProcessInfoEx& px, COLORREF& bk, COLORREF& text) const {
	auto& colors = Settings::Get().Processes.Colors;
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
	static const ProcessAttributes all[] = {
		ProcessAttributes::Managed,
		ProcessAttributes::Immersive,
		ProcessAttributes::Service,
		ProcessAttributes::Protected,
		ProcessAttributes::Secure,
		ProcessAttributes::InJob,
		ProcessAttributes::Wow64,
	};

	for (int i = 0; i < _countof(all); i++) {
		if (colors[i + 2].Enabled && ((attributes & all[i]) == all[i])) {
			bk = colors[i + 2].Color;
			text = colors[i + 2].TextColor;
			return;
		}
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

LRESULT CProcessesView::OnItemStateChanged(int, LPNMHDR hdr, BOOL& handled) {
	UpdateUI();
	handled = FALSE;

	return 0;
}

LRESULT CProcessesView::OnListRightClick(int, LPNMHDR hdr, BOOL&) {
	auto paused = IsPaused();
	if (!paused)
		Pause(true);

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
	bool headerClick = false;
	menu.LoadMenu(IDR_CONTEXT);
	if (index >= 0) {
		headerClick = true;
		// right-click header
		hSubMenu = menu.GetSubMenu(7);
		auto cm = GetColumnManager(m_List);
		m_SelectedHeader = cm->GetRealColumn(index);
		GetFrame()->GetUpdateUI()->UIEnable(ID_HEADER_HIDECOLUMN,
			(cm->GetColumn(m_SelectedHeader).Flags & ColumnFlags::Mandatory) == ColumnFlags::None);
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
		UpdateUI();
		auto id = (UINT)GetFrame()->TrackPopupMenu(hSubMenu, *this, &pt, TPM_RETURNCMD);
		if (id) {
			GetFrame()->SendFrameMessage(WM_COMMAND, id, headerClick ? 0 : reinterpret_cast<LPARAM>(m_Processes[index].get()));
		}
	}
	if (!paused)
		Pause(false);
	return 0;
}

void CProcessesView::OnPauseResume(bool paused) {
	GetFrame()->GetUpdateUI()->UISetCheck(ID_VIEW_PAUSE, IsPaused());
}

LRESULT CProcessesView::OnProcessKill(WORD, WORD, HWND, BOOL&) {
	int selected = m_List.GetSelectedIndex();
	ATLASSERT(selected >= 0);
	auto& p = m_Processes[selected];

	CString text;
	text.Format(L"Kill process %u (%ws)?", p->Id, p->GetImageName().c_str());
	if (AtlMessageBox(*this, (PCWSTR)text, IDS_TITLE, MB_ICONWARNING | MB_OKCANCEL | MB_DEFBUTTON2) == IDCANCEL)
		return 0;

	auto hProcess = DriverHelper::OpenProcess(p->Id, PROCESS_TERMINATE);
	BOOL ok = false;
	if (hProcess) {
		ok = ::TerminateProcess(hProcess, 0);
		::CloseHandle(hProcess);
	}
	if (!ok)
		AtlMessageBox(*this, L"Failed to kill process", IDS_TITLE, MB_ICONERROR);

	return 0;
}

LRESULT CProcessesView::OnProcessGoToFileLocation(WORD, WORD, HWND, BOOL&) {
	int selected = m_List.GetSelectedIndex();
	ATLASSERT(selected >= 0);
	auto& px = GetProcessInfoEx(m_Processes[selected].get());

	if ((INT_PTR)::ShellExecute(nullptr, L"open", L"explorer",
		(L"/select,\"" + px.GetExecutablePath() + L"\"").c_str(),
		nullptr, SW_SHOWDEFAULT) < 32)
		AtlMessageBox(*this, L"Failed to locate executable", IDS_TITLE, MB_ICONERROR);

	return 0;
}

LRESULT CProcessesView::OnPriorityClass(WORD, WORD id, HWND, BOOL&) {
	int selected = m_List.GetSelectedIndex();
	ATLASSERT(selected >= 0);

	Process process(DriverHelper::OpenProcess(m_Processes[selected]->Id, PROCESS_SET_INFORMATION));
	bool ok = FALSE;
	if (process.IsValid()) {
		auto pc = ProcessPriorityClass::Unknown;
		switch (id) {
			case ID_PRIORITYCLASS_ABOVENORMAL: pc = ProcessPriorityClass::AboveNormal; break;
			case ID_PRIORITYCLASS_BELOWNORMAL: pc = ProcessPriorityClass::BelowNormal; break;
			case ID_PRIORITYCLASS_NORMAL: pc = ProcessPriorityClass::Normal; break;
			case ID_PRIORITYCLASS_IDLE: pc = ProcessPriorityClass::Idle; break;
			case ID_PRIORITYCLASS_HIGH: pc = ProcessPriorityClass::High; break;
			case ID_PRIORITYCLASS_REALTIME: pc = ProcessPriorityClass::Realtime; break;
		}
		ok = process.SetPriorityClass(pc);
	}
	if (!ok) {
		AtlMessageBox(*this, L"Failed to change priority class", IDS_TITLE, MB_ICONERROR);
	}
	return 0;
}

LRESULT CProcessesView::OnProcessItem(WORD, WORD id, HWND, BOOL&) {
	return GetFrame()->SendFrameMessage(WM_COMMAND, id, reinterpret_cast<LPARAM>(m_Processes[m_List.GetSelectedIndex()].get()));
}

LRESULT CProcessesView::OnProperties(WORD, WORD, HWND, BOOL&) {
	auto selected = m_List.GetSelectedIndex();
	ATLASSERT(selected >= 0);
	ShowProperties(selected);

	return 0;
}

LRESULT CProcessesView::OnProcessColors(WORD, WORD id, HWND, BOOL&) {
	GetFrame()->SendFrameMessage(WM_COMMAND, id, 0);
	return 0;
}

LRESULT CProcessesView::OnProcessKillByName(WORD, WORD, HWND, BOOL&) {
	int selected = m_List.GetSelectedIndex();
	ATLASSERT(selected >= 0);
	auto& process = m_Processes[selected];

	std::vector<DWORD> pids;
	for (auto& p : m_Processes)
		if (p->GetImageName() == process->GetImageName())
			pids.push_back(p->Id);

	CString text;
	text.Format(L"Kill %u processes named %s?", (DWORD)pids.size(), process->GetImageName().c_str());
	if (AtlMessageBox(*this, (PCWSTR)text, IDS_TITLE, MB_ICONWARNING | MB_OKCANCEL | MB_DEFBUTTON2) == IDOK) {
		int killed = 0;
		for(auto pid : pids) {
			auto hProcess = DriverHelper::OpenProcess(pid, PROCESS_TERMINATE);
			if (hProcess && ::TerminateProcess(hProcess, 0))
				killed++;
			if (hProcess)
				::CloseHandle(hProcess);
		}
		text.Format(L"Successfully killed %d processes", killed);
		AtlMessageBox(*this, killed ? (PCWSTR)text : L"Failed to kill processes", IDS_TITLE, MB_ICONINFORMATION);
	}

	return 0;
}

LRESULT CProcessesView::OnFileSave(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	PauseResumeUpdates pr(this);

	CSimpleFileDialog dlg(FALSE, L"tsv", nullptr, OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_EXPLORER | OFN_ENABLESIZING,
		L"Tab Separated Values (*.tsv)\0*.tsv\0All Files\0*.*\0", *this);
	if (dlg.DoModal() == IDOK) {
		CWaitCursor wait;
		ListViewHelper::SaveAll(dlg.m_szFileName, m_List);
	}

	return 0;
}

LRESULT CProcessesView::OnCopyRow(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
	auto selected = m_List.GetSelectedIndex();
	if (selected >= 0)
		ClipboardHelper::CopyText(*this, ListViewHelper::GetRowAsString(m_List, selected));
	return 0;
}
