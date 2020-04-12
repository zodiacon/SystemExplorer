#include "stdafx.h"
#include "HandlesView.h"
#include <algorithm>
#include <execution>
#include "SortHelper.h"
#include <string>
#include "ObjectType.h"
#include "ObjectTypeFactory.h"
#include "DriverHelper.h"
#include "NtDll.h"

using namespace WinSys;

CHandlesView::CHandlesView(IMainFrame* pFrame, PCWSTR type, DWORD pid) :
	m_pFrame(pFrame), m_pUI(pFrame->GetUpdateUI()), m_HandleType(type), m_Pid(pid) {
	m_hProcess.reset(DriverHelper::OpenProcess(pid, SYNCHRONIZE));
	if (pid) {
		auto hProcess = DriverHelper::OpenProcess(pid, SYNCHRONIZE | PROCESS_QUERY_INFORMATION);
		m_HandleTracker.reset(new ProcessHandlesTracker(hProcess));
		if (!m_HandleTracker->IsValid()) {
			AtlMessageBox(nullptr, (L"Failed to open handle to process " + std::to_wstring(pid)).c_str(), IDS_TITLE, MB_ICONERROR);
			m_HandleTracker.reset();
		}
	}
}

void CHandlesView::DoSort(const SortInfo* si) {
	if (si == nullptr)
		return;
	CWaitCursor wait(m_Pid == 0 && m_HandleType.IsEmpty());

	std::sort(std::execution::par_unseq, m_Handles.begin(), m_Handles.end(), [this, si](const auto& o1, const auto& o2) {
		return CompareItems(*o1.get(), *o2.get(), si);
		});

	RedrawItems(GetTopIndex(), GetTopIndex() + GetCountPerPage());
}

bool CHandlesView::IsSortable(int col) const {
	// details column cannot be sorted
	return col != 8;
}

bool CHandlesView::CompareItems(HandleInfo& h1, HandleInfo& h2, const SortInfo* si) {
	switch (si->SortColumn) {
		case 0:		// type
			return SortHelper::SortStrings(m_ObjMgr.GetType(h1.ObjectTypeIndex)->TypeName, m_ObjMgr.GetType(h2.ObjectTypeIndex)->TypeName, si->SortAscending);

		case 1:		// address
			return SortHelper::SortNumbers(h1.Object, h2.Object, si->SortAscending);

		case 2:		// name
			if ((h1.HandleAttributes & 0x8000) == 0) {
				h1.Name = m_ObjMgr.GetObjectName(ULongToHandle(h1.HandleValue), h1.ProcessId, h1.ObjectTypeIndex);
				h1.HandleAttributes |= 0x8000;
			}
			if ((h2.HandleAttributes & 0x8000) == 0) {
				h2.Name = m_ObjMgr.GetObjectName(ULongToHandle(h2.HandleValue), h2.ProcessId, h2.ObjectTypeIndex);
				h2.HandleAttributes |= 0x8000;
			}
			return SortHelper::SortStrings(h1.Name, h2.Name, si->SortAscending);

		case 3:		// handle
			return SortHelper::SortNumbers(h1.HandleValue, h2.HandleValue, si->SortAscending);

		case 4:		// process name
			return SortHelper::SortStrings(m_ProcMgr.GetProcessNameById(h1.ProcessId), m_ProcMgr.GetProcessNameById(h2.ProcessId), si->SortAscending);

		case 5:		// PID
			return SortHelper::SortNumbers(h1.ProcessId, h2.ProcessId, si->SortAscending);

		case 6:		// attributes
			return SortHelper::SortNumbers(h1.HandleAttributes & 0x7fff, h2.HandleAttributes & 0x7fff, si->SortAscending);

		case 7:		// access mask
			return SortHelper::SortNumbers(h1.GrantedAccess, h2.GrantedAccess, si->SortAscending);
	}

	return false;
}

LRESULT CHandlesView::OnCreate(UINT, WPARAM, LPARAM, BOOL&) {
	DefWindowProc();

	SetExtendedListViewStyle(LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP | LVS_EX_HEADERDRAGDROP);

	struct {
		PCWSTR Header;
		int Width;
		int Format = LVCFMT_LEFT;
	} columns[] = {
		{ L"Type", 140 },
		{ L"Address", 140, LVCFMT_RIGHT },
		{ L"Name", 330 },
		{ L"Handle", 100, LVCFMT_RIGHT },
		{ L"Process Name", 160 },
		{ L"PID", m_Pid == 0 ? 100 : 1, LVCFMT_RIGHT | (m_Pid == 0 ? 0 : LVCFMT_FIXED_WIDTH) },
		{ L"Attributes", 100 },
		{ L"Access Mask", 100, LVCFMT_RIGHT },
		{ L"Details", 500 }
	};

	m_ColumnCount = _countof(columns);

	int i = 0;
	for (auto& c : columns)
		InsertColumn(i++, c.Header, c.Format, c.Width);

	SetImageList(m_pFrame->GetImageList(), LVSIL_SMALL);

	Refresh();

	return 0;
}

LRESULT CHandlesView::OnDestroy(UINT, WPARAM, LPARAM, BOOL&) {
	m_pUI->UIEnable(ID_HANDLES_CLOSEHANDLE, FALSE);

	return DefWindowProc();
}

LRESULT CHandlesView::OnGetDispInfo(int, LPNMHDR hdr, BOOL&) {
	auto lv = (NMLVDISPINFO*)hdr;
	auto& item = lv->item;
	auto& data = m_Handles[item.iItem];

	if (item.mask & LVIF_TEXT) {
		switch (item.iSubItem) {
			case 0:	// type
				item.pszText = (PWSTR)(PCWSTR)m_ObjMgr.GetType(data->ObjectTypeIndex)->TypeName;
				break;

			case 1:	// address
				::StringCchPrintf(item.pszText, item.cchTextMax, L"0x%p", data->Object);
				break;

			case 2:	// name
				if (data->HandleAttributes & 0x8000)
					item.pszText = (PWSTR)(PCWSTR)data->Name;
				else {
					CString name = m_ObjMgr.GetObjectName(ULongToHandle(data->HandleValue), data->ProcessId, data->ObjectTypeIndex);
					if (!name.IsEmpty())
						::StringCchCopy(item.pszText, item.cchTextMax, name);
					data->Name = name;
					data->HandleAttributes |= 0x8000;
				}
				break;

			case 3:	// handle
				::StringCchPrintf(item.pszText, item.cchTextMax, L"%d (0x%X)", data->HandleValue, data->HandleValue);
				break;

			case 4:	// process name
				::StringCchCopy(item.pszText, item.cchTextMax, m_ProcMgr.GetProcessNameById(data->ProcessId).c_str());
				break;

			case 5:	// PID
				::StringCchPrintf(item.pszText, item.cchTextMax, L"%d (0x%X)", data->ProcessId, data->ProcessId);
				break;

			case 6:	// attributes
				::StringCchPrintf(item.pszText, item.cchTextMax, L"%s (%d)", HandleAttributesToString(data->HandleAttributes), data->HandleAttributes & 0x7fff);
				break;

			case 7:	// access mask
				::StringCchPrintf(item.pszText, item.cchTextMax, L"0x%08X", data->GrantedAccess);
				break;

			case 8:	// details
				if (::GetTickCount() > m_TargetUpdateTime || m_DetailsCache.find(data.get()) == m_DetailsCache.end()) {
					auto h = m_ObjMgr.DupHandle(ULongToHandle(data->HandleValue), data->ProcessId, data->ObjectTypeIndex);
					if (h) {
						auto& type = ObjectTypeFactory::CreateObjectType(data->ObjectTypeIndex, ObjectManager::GetType(data->ObjectTypeIndex)->TypeName);
						CString details = type ? type->GetDetails(h) : L"";
						m_DetailsCache[data.get()] = details;
						if (!details.IsEmpty())
							::StringCchCopy(item.pszText, item.cchTextMax, details);
						::CloseHandle(h);
					}
					m_TargetUpdateTime = ::GetTickCount() + 5000;
				}
				else {
					::StringCchCopy(item.pszText, item.cchTextMax, m_DetailsCache[data.get()]);
				}
				break;
		}
	}
	if (item.mask & LVIF_IMAGE) {
		item.iImage = m_pFrame->GetIconIndexByType((PCWSTR)m_ObjMgr.GetType(data->ObjectTypeIndex)->TypeName);
	}
	return 0;
}

LRESULT CHandlesView::OnItemChanged(int, LPNMHDR, BOOL&) {
	m_pUI->UIEnable(ID_HANDLES_CLOSEHANDLE, GetSelectedIndex() >= 0);

	return 0;
}

LRESULT CHandlesView::OnContextMenu(int, LPNMHDR, BOOL&) {
	CMenu menu;
	menu.LoadMenuW(IDR_CONTEXT);
	m_pFrame->TrackPopupMenu(menu.GetSubMenu(0), *this);

	return 0;
}

LRESULT CHandlesView::OnCloseHandle(WORD, WORD, HWND, BOOL&) {
	auto selected = GetSelectedIndex();
	ATLASSERT(selected >= 0);

	auto& item = m_Handles[selected];
	if (MessageBox(L"Closing a handle can potentially make the process unstable. Continue?", L"Object Explorer",
		MB_OKCANCEL | MB_DEFBUTTON2 | MB_ICONWARNING) == IDCANCEL)
		return 0;

	auto hDup = m_ObjMgr.DupHandle(ULongToHandle(item->HandleValue), item->ProcessId, item->ObjectTypeIndex,
		0, DUPLICATE_SAME_ACCESS | DUPLICATE_CLOSE_SOURCE);
	if (!hDup) {
		AtlMessageBox(*this, L"Failed to close handle", L"Object Explorer", MB_ICONERROR);
		return 0;
	}
	::CloseHandle(hDup);
	m_Handles.erase(m_Handles.begin() + selected);
	SetItemCountEx(static_cast<int>(m_Handles.size()), LVSICF_NOINVALIDATEALL | LVSICF_NOSCROLL);
	RedrawItems(selected, selected + GetCountPerPage());

	return 0;
}

LRESULT CHandlesView::OnRefresh(WORD, WORD, HWND, BOOL&) {
	Refresh();
	return 0;
}

LRESULT CHandlesView::OnTimer(UINT, WPARAM id, LPARAM, BOOL&) {
	if (id == 1 && m_HandleTracker) {
		m_HandleTracker->EnumHandles();
		ATLTRACE(L"New handles: %u, Closed handles: %u\n", m_HandleTracker->GetNewHandles().size(), m_HandleTracker->GetClosedHandles().size());

		for (auto& h : m_HandleTracker->GetClosedHandles()) {
			Change change;
			change.Color = RGB(255, 0, 0);
			change.Handle = HandleToUlong(h.HandleValue);
			change.TargetTime = ::GetTickCount() + 2000;
			change.IsNewHandle = false;
			m_Changes.push_back(change);
		}

		for (auto& h : m_HandleTracker->GetNewHandles()) {
			auto hi = std::make_shared<HandleInfo>();
			hi->HandleValue = HandleToULong(h.HandleValue);
			hi->ProcessId = m_Pid;
			hi->ObjectTypeIndex = h.ObjectTypeIndex;
			auto hDup = ObjectManager::DupHandle(h.HandleValue, m_Pid, h.ObjectTypeIndex);
			if (hDup) {
				NT::OBJECT_BASIC_INFORMATION info;
				if (NT_SUCCESS(NT::NtQueryObject(hDup, NT::ObjectBasicInformation, &info, sizeof(info), nullptr))) {
					hi->GrantedAccess = info.GrantedAccess;
					hi->HandleAttributes = info.Attributes;
				}
				hi->Name = m_ObjMgr.GetObjectName(hDup, h.ObjectTypeIndex);
				hi->Object = DriverHelper::GetObjectAddress(hDup);
				::CloseHandle(hDup);
			}
			Change change;
			change.Color = RGB(0, 255, 0);
			change.Handle = hi->HandleValue;
			change.TargetTime = ::GetTickCount() + 2000;
			change.IsNewHandle = true;
			m_Changes.push_back(change);
			m_Handles.push_back(hi);
		}

		// process changes
		for (int i = 0; i < m_Changes.size(); i++) {
			auto& change = m_Changes[i];
			if (change.TargetTime < ::GetTickCount()) {
				if (!change.IsNewHandle) {
					auto it = std::find_if(m_Handles.begin(), m_Handles.end(), [&](auto& hi) {
						return hi->HandleValue == change.Handle;
						});
					if (it != m_Handles.end()) {
						m_Handles.erase(it);
					}
				}
				m_Changes.erase(m_Changes.begin() + i);
				i--;
			}
		}

		auto si = GetSortInfo(*this);
		if (si)
			DoSort(si);
		SetItemCountEx(static_cast<int>(m_Handles.size()), LVSICF_NOINVALIDATEALL | LVSICF_NOSCROLL);
		RedrawItems(GetTopIndex(), GetTopIndex() + GetCountPerPage());
	}
	return 0;
}

LRESULT CHandlesView::OnActivate(UINT, WPARAM activate, LPARAM, BOOL&) {
	if (m_HandleTracker) {
		if (activate) {
			if (!m_Paused) {
				Refresh();
				SetTimer(1, 1000, nullptr);
			}
			UpdateUI();
		}
		else {
			KillTimer(1);
		}
	}
	return 0;
}

LRESULT CHandlesView::OnPauseResume(WORD, WORD, HWND, BOOL&) {
	m_Paused = !m_Paused;
	if (m_HandleTracker) {
		if (m_Paused)
			KillTimer(1);
		else
			SetTimer(1, 1000, nullptr);
	}
	UpdateUI();
	return 0;
}

void CHandlesView::Refresh() {
	if (m_hProcess && ::WaitForSingleObject(m_hProcess.get(), 0) == WAIT_OBJECT_0) {
		MessageBox((L"Process " + std::to_wstring(m_Pid) + L" is no longer running.").c_str(), L"Object Explorer", MB_OK);
		SetItemCount(0);
		return;
	}
	m_ObjMgr.EnumHandles(m_HandleType, m_Pid);
	if (m_HandleTracker) {
		m_DetailsCache.clear();
		m_DetailsCache.reserve(1024);
		m_Changes.clear();
		m_Changes.reserve(8);
		m_HandleTracker->EnumHandles(true);
		if (!m_Paused)
			SetTimer(1, 1000, nullptr);
	}
	m_ProcMgr.EnumProcesses();
	m_Handles = m_ObjMgr.GetHandles();
	DoSort(GetSortInfo(*this));
	SetItemCountEx(static_cast<int>(m_Handles.size()), LVSICF_NOINVALIDATEALL | LVSICF_NOSCROLL);
}

void CHandlesView::UpdateUI() {
	m_pUI->UISetCheck(ID_VIEW_PAUSE, m_Paused);
}

CString CHandlesView::HandleAttributesToString(ULONG attributes) {
	CString result;
	if (attributes & HANDLE_FLAG_INHERIT)
		result += L", Inherit";
	if (attributes & HANDLE_FLAG_PROTECT_FROM_CLOSE)
		result += L", Protect";

	if (result.IsEmpty())
		result = L"None";
	else
		result = result.Mid(2);
	return result;
}

DWORD CHandlesView::OnPrePaint(int, LPNMCUSTOMDRAW) {
	return CDRF_NOTIFYITEMDRAW;
}

DWORD CHandlesView::OnSubItemPrePaint(int, LPNMCUSTOMDRAW cd) {
	auto lcd = (LPNMLVCUSTOMDRAW)cd;
	auto sub = lcd->iSubItem;
	lcd->clrTextBk = CLR_INVALID;
	int index = (int)cd->dwItemSpec;
	auto& hi = m_Handles[index];

	for (auto& change : m_Changes) {
		if (change.Handle == hi->HandleValue) {
			lcd->clrTextBk = change.Color;
			break;
		}
	}

	return CDRF_DODEFAULT;
}

DWORD CHandlesView::OnItemPrePaint(int, LPNMCUSTOMDRAW) {
	return CDRF_NOTIFYITEMDRAW;
}
