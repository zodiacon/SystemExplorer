#include "pch.h"
#include "ThreadsView.h"
#include "SortHelper.h"
#include <algorithm>
#include "FormatHelper.h"
#include "SelectColumnsDlg.h"

CThreadsView::CThreadsView(IMainFrame* frame, DWORD pid) : CViewBase(frame), m_Pid(pid) {
}

CString CThreadsView::GetColumnText(HWND, int row, int col) {
	CString text;
	auto& t = m_Threads[row];
	auto& tx = GetThreadInfoEx(t.get());
	if (tx.IsTerminated) {
		m_ThreadsEx.erase(t.get());
		m_Threads.erase(m_Threads.begin() + row);
		m_List.SetItemCountEx((int)m_Threads.size(), LVSICF_NOINVALIDATEALL | LVSICF_NOSCROLL);
		m_List.RedrawItems(row, row);
		return L"";
	}

	switch (static_cast<ThreadColumn>(col)) {
		case ThreadColumn::State: return ThreadStateToString(t->ThreadState);
		case ThreadColumn::Id: text.Format(L"%d (0x%05X)", t->Id, t->Id); break;
		case ThreadColumn::ProcessId: text.Format(L"%d (0x%05X)", t->ProcessId, t->ProcessId); break;
		case ThreadColumn::CPU:
			if (t->CPU > 0 && !tx.IsTerminating) {
				auto value = t->CPU / 10000.0f;
				if (value > 100)
					value = 100;
				text.Format(L"%.2f ", value);
			}
			break;

		case ThreadColumn::ProcessName: return t->GetProcessImageName().c_str();
		case ThreadColumn::CreateTime:
			return t->CreateTime < (1LL << 32) ? CString() : FormatHelper::TimeToString(t->CreateTime);
		case ThreadColumn::WaitReason: return t->ThreadState == WinSys::ThreadState::Waiting ? WaitReasonToString(t->WaitReason) : L"";
		case ThreadColumn::Priority: text.Format(L"%d ", t->Priority); break;
		case ThreadColumn::BasePriority: text.Format(L"%d ", t->BasePriority); break;
		case ThreadColumn::ContextSwitches: return FormatHelper::FormatWithCommas(t->ContextSwitches);
		case ThreadColumn::Teb: 
			if(t->TebBase != nullptr)
				text.Format(L"0x%p", t->TebBase); 
			break;
		case ThreadColumn::StartAddress: 
			if(t->StartAddress)
				text.Format(L"0x%p", t->StartAddress); 
			break;
		case ThreadColumn::Win32StartAddress: 
			if(t->Win32StartAddress != t->StartAddress)
				text.Format(L"0x%p", t->TebBase); 
			break;
		case ThreadColumn::StackBase: text.Format(L"0x%p", t->StackBase); break;
		case ThreadColumn::StackLimit: text.Format(L"0x%p", t->StackLimit); break;
		case ThreadColumn::CPUTime: return FormatHelper::TimeSpanToString(t->UserTime + t->KernelTime);
		case ThreadColumn::KernelTime: return FormatHelper::TimeSpanToString(t->KernelTime);
		case ThreadColumn::UserTime: return FormatHelper::TimeSpanToString(t->UserTime);
		case ThreadColumn::WaitTime: text.Format(L"%u.%03d", t->WaitTime / 1000, t->WaitTime % 1000); break;
	}
	return text;
}

int CThreadsView::GetRowImage(HWND, int row) const {
	auto& t = m_Threads[row];
	return (int)t->ThreadState;
}

void CThreadsView::DoSort(const SortInfo* si) {
	if (si == nullptr)
		return;

	auto col = si->SortColumn;
	auto asc = si->SortAscending;

	std::sort(m_Threads.begin(), m_Threads.end(), [=](const auto& t1, const auto& t2) {
		switch (static_cast<ThreadColumn>(col)) {
			case ThreadColumn::State: return SortHelper::SortNumbers(t1->ThreadState, t2->ThreadState, asc);
			case ThreadColumn::Id: return SortHelper::SortNumbers(t1->Id, t2->Id, asc);
			case ThreadColumn::ProcessId: return SortHelper::SortNumbers(t1->ProcessId, t2->ProcessId, asc);
			case ThreadColumn::ProcessName: return SortHelper::SortStrings(t1->GetProcessImageName(), t2->GetProcessImageName(), asc);
			case ThreadColumn::CPU: return SortHelper::SortNumbers(t1->CPU, t2->CPU, asc);
			case ThreadColumn::CreateTime: return SortHelper::SortNumbers(t1->CreateTime, t2->CreateTime, asc);
			case ThreadColumn::Priority: return SortHelper::SortNumbers(t1->Priority, t2->Priority, asc);
			case ThreadColumn::BasePriority: return SortHelper::SortNumbers(t1->BasePriority, t2->BasePriority, asc);
			case ThreadColumn::CPUTime: return SortHelper::SortNumbers(t1->KernelTime + t1->UserTime, t2->KernelTime + t2->UserTime, asc);
			case ThreadColumn::UserTime: return SortHelper::SortNumbers(t1->UserTime, t2->UserTime, asc);
			case ThreadColumn::KernelTime: return SortHelper::SortNumbers(t1->KernelTime, t2->KernelTime, asc);
			case ThreadColumn::Teb: return SortHelper::SortNumbers(t1->TebBase, t2->TebBase, asc);
			case ThreadColumn::WaitReason: return SortHelper::SortNumbers(t1->WaitReason, t2->WaitReason, asc);
			case ThreadColumn::StartAddress: return SortHelper::SortNumbers(t1->StartAddress, t2->StartAddress, asc);
			case ThreadColumn::Win32StartAddress: return SortHelper::SortNumbers(t1->Win32StartAddress, t2->Win32StartAddress, asc);
			case ThreadColumn::StackBase: return SortHelper::SortNumbers(t1->StackBase, t2->StackBase, asc);
			case ThreadColumn::StackLimit: return SortHelper::SortNumbers(t1->StackLimit, t2->StackLimit, asc);
			case ThreadColumn::ContextSwitches: return SortHelper::SortNumbers(t1->ContextSwitches, t2->ContextSwitches, asc);
			case ThreadColumn::WaitTime: return SortHelper::SortNumbers(t1->WaitTime, t2->WaitTime, asc);
		}
		return false;
		});
}

DWORD CThreadsView::OnPrePaint(int, LPNMCUSTOMDRAW cd) {
	m_hFont = (HFONT)::GetCurrentObject(cd->hdc, OBJ_FONT);

	return CDRF_NOTIFYITEMDRAW;
}

DWORD CThreadsView::OnItemPrePaint(int, LPNMCUSTOMDRAW) {
	return CDRF_NOTIFYSUBITEMDRAW;
}

DWORD CThreadsView::OnSubItemPrePaint(int, LPNMCUSTOMDRAW cd) {
	auto lcd = (LPNMLVCUSTOMDRAW)cd;
	auto sub = lcd->iSubItem;
	lcd->clrTextBk = CLR_INVALID;
	lcd->clrText = CLR_INVALID;
	int index = (int)cd->dwItemSpec;
	if (index >= m_Threads.size())
		return CDRF_DODEFAULT;

	auto cm = GetColumnManager(m_List);
	auto real = static_cast<ThreadColumn>(cm->GetRealColumn(sub));
	if ((cm->GetColumn((int)real).Flags & ColumnFlags::Numeric) == ColumnFlags::Numeric)
		::SelectObject(cd->hdc, GetFrame()->GetMonoFont());
	else
		::SelectObject(cd->hdc, m_hFont);

	auto& t = m_Threads[index];
	auto& tx = GetThreadInfoEx(t.get());
	switch (real) {
		case ThreadColumn::State:
		case ThreadColumn::Id:
			if (tx.IsTerminating)
				lcd->clrTextBk = StandardColors::Red;
			else if (tx.IsNew)
				lcd->clrTextBk = StandardColors::Lime;
			break;

		case ThreadColumn::CPU:
			Settings::Get().GetCPUColors(t->CPU / 10000, lcd->clrTextBk, lcd->clrText);
			break;
	}
	return CDRF_NEWFONT;
}

void CThreadsView::Refresh() {
	auto first = m_Threads.empty();
	m_ProcMgr.EnumProcessesAndThreads(m_Pid);
	auto count = (int)m_ProcMgr.GetThreadCount();

	if (first) {
		m_Threads = m_ProcMgr.GetThreads();
		m_List.SetItemCount(count);
		m_TermThreads.reserve(32);
		m_NewThreads.reserve(32);
		return;
	}

	count = (int)m_TermThreads.size();
	auto tick = ::GetTickCount64();

	for (int i = 0; i < count; i++) {
		auto& t = m_TermThreads[i];
		auto& tx = GetThreadInfoEx(t.get());
		if (tick > tx.TargetTime) {
			tx.IsTerminated = true;
			m_TermThreads.erase(m_TermThreads.begin() + i);
			i--;
			count--;
		}
	}

	count = (int)m_NewThreads.size();
	for (int i = 0; i < count; i++) {
		auto& t = m_NewThreads[i];
		auto& tx = GetThreadInfoEx(t.get());
		if (tick > tx.TargetTime) {
			tx.IsNew = false;
			m_NewThreads.erase(m_NewThreads.begin() + i);
			i--;
			count--;
		}
	}

	for (auto& t : m_ProcMgr.GetTerminatedThreads()) {
		auto& tx = GetThreadInfoEx(t.get());
		t->ThreadState = WinSys::ThreadState::Terminated;
		tx.TargetTime = ::GetTickCount64() + 1000;
		tx.IsTerminating = true;
		m_TermThreads.push_back(t);
	}

	for (auto& t : m_ProcMgr.GetNewThreads()) {
		auto& tx = GetThreadInfoEx(t.get());
		if (tx.IsTerminated || tx.IsTerminating)
			continue;
		tx.IsNew = true;
		tx.TargetTime = ::GetTickCount64() + 2000;
		m_NewThreads.push_back(t);
		m_Threads.push_back(t);
	}


	DoSort(GetSortInfo(m_List));
	count = (int)m_Threads.size();
	m_List.SetItemCountEx(count, LVSICF_NOSCROLL | LVSICF_NOINVALIDATEALL);
	auto top = m_List.GetTopIndex();
	m_List.RedrawItems(top, top + m_List.GetCountPerPage());

}

void CThreadsView::UpdateUI() {
}

LRESULT CThreadsView::OnCreate(UINT, WPARAM, LPARAM, BOOL&) {
	ToolBarButtonInfo buttons[] = {
		{ ID_HEADER_COLUMNS, IDI_EDITCOLUMNS, 0, L"Columns" },
	};
	CreateAndInitToolBar(buttons, _countof(buttons));

	m_hWndClient = m_List.Create(m_hWnd, rcDefault, nullptr, ListViewDefaultStyle & ~LVS_SHAREIMAGELISTS);
	m_List.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_LABELTIP | LVS_EX_HEADERDRAGDROP);

	CImageList images;
	images.Create(16, 16, ILC_COLOR32, 8, 4);
	UINT ids[] = {
		IDI_INIT, IDI_READY, IDI_RUNNING, IDI_STANDBY, IDI_DELETE, IDI_PAUSE, IDI_TRANSITION, IDI_READY, IDI_TRANSITION, IDI_PAUSE
	};

	for (auto id : ids)
		images.AddIcon(AtlLoadIconImage(id, 0, 16, 16));

	m_List.SetImageList(images, LVSIL_SMALL);

	auto cm = GetColumnManager(m_List);
	cm->AddColumn(L"State", LVCFMT_LEFT, 110, ColumnFlags::Visible | ColumnFlags::Mandatory);
	cm->AddColumn(L"Id", LVCFMT_RIGHT, 130, ColumnFlags::Visible | ColumnFlags::Mandatory | ColumnFlags::Numeric);
	cm->AddColumn(L"Process Id", LVCFMT_RIGHT, 130, ColumnFlags::Visible | ColumnFlags::Mandatory | ColumnFlags::Numeric);
	cm->AddColumn(L"Process Name", LVCFMT_LEFT, 200, ColumnFlags::Visible | ColumnFlags::Mandatory);
	cm->AddColumn(L"Performance\\CPU (%)", LVCFMT_RIGHT, 80, ColumnFlags::Visible | ColumnFlags::Numeric);
	cm->AddColumn(L"Performance\\CPU Time", LVCFMT_RIGHT, 120, ColumnFlags::Numeric);
	cm->AddColumn(L"Create Time", LVCFMT_LEFT, 160, ColumnFlags::Visible | ColumnFlags::Numeric);
	cm->AddColumn(L"Priority", LVCFMT_RIGHT, 60, ColumnFlags::Visible | ColumnFlags::Numeric);
	cm->AddColumn(L"Base Priority", LVCFMT_RIGHT, 70, ColumnFlags::Visible | ColumnFlags::Numeric);
	cm->AddColumn(L"TEB", LVCFMT_RIGHT, 140, ColumnFlags::Numeric);
	cm->AddColumn(L"Wait Reason", LVCFMT_LEFT, 90, ColumnFlags::None);
	cm->AddColumn(L"Start Address", LVCFMT_RIGHT, 140, ColumnFlags::Numeric);
	cm->AddColumn(L"Win32 Start Address", LVCFMT_RIGHT, 140, ColumnFlags::Numeric);
	cm->AddColumn(L"Stack Base", LVCFMT_RIGHT, 140, ColumnFlags::Numeric);
	cm->AddColumn(L"Stack Limit", LVCFMT_RIGHT, 140, ColumnFlags::Numeric);
	cm->AddColumn(L"Performance\\Context Switches", LVCFMT_RIGHT, 100, ColumnFlags::Numeric);
	cm->AddColumn(L"Performance\\Kernel Time", LVCFMT_RIGHT, 130, ColumnFlags::Numeric);
	cm->AddColumn(L"Performance\\User Time", LVCFMT_RIGHT, 130, ColumnFlags::Numeric);

	Refresh();
	SetTimer(1, m_UpdateInterval, nullptr);

	return 0;
}

LRESULT CThreadsView::OnTimer(UINT, WPARAM id, LPARAM, BOOL&) {
	if (id == 1)
		Refresh();

	return 0;
}

LRESULT CThreadsView::OnActivate(UINT, WPARAM activate, LPARAM, BOOL&) {
	if (m_UpdateInterval == 0)
		return 0;

	if (activate) {
		if (m_UpdateInterval)
			SetTimer(1, m_UpdateInterval, nullptr);
	}
	else {
		KillTimer(1);
	}
	return 0;
}

LRESULT CThreadsView::OnRefresh(WORD, WORD, HWND, BOOL&) {
	Refresh();

	return 0;
}

LRESULT CThreadsView::OnHideColumn(WORD, WORD, HWND, BOOL&) {
	auto cm = GetColumnManager(m_List);
	cm->SetVisible(m_SelectedHeader, false);
	cm->UpdateColumns();
	m_List.RedrawItems(m_List.GetTopIndex(), m_List.GetTopIndex() + m_List.GetCountPerPage());

	return 0;
}

LRESULT CThreadsView::OnSelectColumns(WORD, WORD, HWND, BOOL&) {
	CSelectColumnsDlg dlg(GetColumnManager(m_List));
	if (IDOK == dlg.DoModal()) {
		m_List.RedrawItems(m_List.GetTopIndex(), m_List.GetTopIndex() + m_List.GetCountPerPage());
	}

	return 0;
}

LRESULT CThreadsView::OnItemStateChanged(int, LPNMHDR hdr, BOOL&) {
	return LRESULT();
}

LRESULT CThreadsView::OnListRightClick(int, LPNMHDR hdr, BOOL&) {
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
	auto cm = GetColumnManager(m_List);
	ATLASSERT(cm);
	bool headerClick = false;
	if (index >= 0) {
		// right-click header
		hSubMenu = menu.GetSubMenu(7);
		m_SelectedHeader = cm->GetRealColumn(index);
		GetFrame()->GetUpdateUI()->UIEnable(ID_HEADER_HIDECOLUMN,
			(cm->GetColumn(m_SelectedHeader).Flags & ColumnFlags::Mandatory) == ColumnFlags::None);
		pt = pt2;
		headerClick = true;
	}
	else {
		//index = m_List.GetSelectedIndex();
		//if (index >= 0) {
		//	auto item = (NMITEMACTIVATE*)hdr;
		//	hSubMenu = menu.GetSubMenu(8);
		//	pt = item->ptAction;
		//	m_List.ClientToScreen(&pt);
		//}
	}
	if (hSubMenu) {
		UpdateUI();
		auto id = (UINT)GetFrame()->TrackPopupMenu(hSubMenu, *this, &pt, TPM_RETURNCMD);
		if (id) {
			GetFrame()->SendFrameMessage(WM_COMMAND, id, headerClick ? 0 : reinterpret_cast<LPARAM>(m_Threads[index].get()));
		}
	}
	return 0;
}

LRESULT CThreadsView::OnPause(WORD, WORD, HWND, BOOL&) {
	if (m_UpdateInterval > 0) {
		m_OldInterval = m_UpdateInterval;
		m_UpdateInterval = 0;
		KillTimer(1);
	}
	else {
		m_UpdateInterval = m_OldInterval;
		SetTimer(1, m_UpdateInterval, nullptr);
	}

	GetFrame()->GetUpdateUI()->UISetCheck(ID_VIEW_PAUSE, m_UpdateInterval == 0);
	return 0;
}

PCWSTR CThreadsView::ThreadStateToString(WinSys::ThreadState state) {
	static PCWSTR states[] = {
		L"Initialized (0)",
		L"Ready (1)",
		L"Running (2)",
		L"Standby (3)",
		L"Terminated (4)",
		L"Waiting (5)",
		L"Transition (6)",
		L"DeferredReady (7)",
		L"GateWaitObsolete (8)",
		L"WaitingForProcessInSwap (9)"
	};
	ATLASSERT(state >= WinSys::ThreadState::Initialized && state <= WinSys::ThreadState::WaitingForProcessInSwap);

	return states[(int)state];
}

CThreadsView::ThreadInfoEx& CThreadsView::GetThreadInfoEx(WinSys::ThreadInfo* ti) const {
	auto it = m_ThreadsEx.find(ti);
	if (it != m_ThreadsEx.end())
		return it->second;

	ThreadInfoEx tx;
	auto pair = m_ThreadsEx.insert({ ti, std::move(tx) });
	return pair.first->second;
}

PCWSTR CThreadsView::WaitReasonToString(WinSys::WaitReason reason) {
	static PCWSTR reasons[] = {
		L"Executive",
		L"FreePage",
		L"PageIn",
		L"PoolAllocation",
		L"DelayExecution",
		L"Suspended",
		L"UserRequest",
		L"WrExecutive",
		L"WrFreePage",
		L"WrPageIn",
		L"WrPoolAllocation",
		L"WrDelayExecution",
		L"WrSuspended",
		L"WrUserRequest",
		L"WrEventPair",
		L"WrQueue",
		L"WrLpcReceive",
		L"WrLpcReply",
		L"WrVirtualMemory",
		L"WrPageOut",
		L"WrRendezvous",
		L"WrKeyedEvent",
		L"WrTerminated",
		L"WrProcessInSwap",
		L"WrCpuRateControl",
		L"WrCalloutStack",
		L"WrKernel",
		L"WrResource",
		L"WrPushLock",
		L"WrMutex",
		L"WrQuantumEnd",
		L"WrDispatchInt",
		L"WrPreempted",
		L"WrYieldExecution",
		L"WrFastMutex",
		L"WrGuardedMutex",
		L"WrRundown",
		L"WrAlertByThreadId",
		L"WrDeferredPreempt"
	};

	return reasons[(int)reason];
}
