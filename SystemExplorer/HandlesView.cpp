#include "pch.h"
#include "HandlesView.h"
#include <algorithm>
#include <execution>
#include "SortHelper.h"
#include <string>
#include "ObjectType.h"
#include "ObjectTypeFactory.h"
#include "DriverHelper.h"
#include "NtDll.h"
#include "ObjectHandlesDlg.h"
#include "AccessMaskDecoder.h"
#include "SecurityInfo.h"
#include "SecurityHelper.h"
#include "UndocListView.h"

using namespace WinSys;

CHandlesView::CHandlesView(IMainFrame* pFrame, PCWSTR type, DWORD pid) : CViewBase(pFrame),
	m_pUI(pFrame->GetUpdateUI()), m_HandleType(type), m_Pid(pid) {
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

	m_List.RedrawItems(m_List.GetTopIndex(), m_List.GetTopIndex() + m_List.GetCountPerPage());
}

bool CHandlesView::IsSortable(int col) const {
	// details column cannot be sorted
	return col != 9;
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
		case 8:		// decoded access mask
			return SortHelper::SortNumbers(h1.GrantedAccess, h2.GrantedAccess, si->SortAscending);
	}

	return false;
}

LRESULT CHandlesView::OnCreate(UINT, WPARAM, LPARAM, BOOL&) {
	CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
	auto hWndToolBar = CreateToolBar();
	AddSimpleReBarBand(hWndToolBar);
	m_hWndClient = m_List.Create(*this, rcDefault, nullptr, ListViewDefaultStyle);

	m_List.SetExtendedListViewStyle(LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP | LVS_EX_INFOTIP | LVS_EX_HEADERDRAGDROP);

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
		{ L"Decoded Access Mask", 200, LVCFMT_LEFT },
		{ L"Details", 500 }
	};

	m_ColumnCount = _countof(columns);

	int i = 0;
	for (auto& c : columns)
		m_List.InsertColumn(i++, c.Header, c.Format, c.Width);

	m_List.SetImageList(GetFrame()->GetImageList(), LVSIL_SMALL);

	Refresh();

	//CComPtr<IListViewFooter> spFooter;
	//m_List.SendMessage(LVM_QUERYINTERFACE, reinterpret_cast<WPARAM>(&__uuidof(IListViewFooter)), reinterpret_cast<WPARAM>(&spFooter));
	//if (spFooter) {
	//	spFooter->SetIntroText(L"Footer is working!!!");
	//	spFooter->Show(nullptr);
	//}

	return 0;
}

LRESULT CHandlesView::OnDestroy(UINT, WPARAM, LPARAM, BOOL&) {
	m_pUI->UIEnable(ID_HANDLES_CLOSEHANDLE, FALSE);
	m_pUI->UIEnable(ID_EDIT_SECURITY, FALSE);

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

			case 8:	// decoded access mask
				::StringCchCopy(item.pszText, item.cchTextMax, AccessMaskDecoder::DecodeAccessMask(m_ObjMgr.GetType(data->ObjectTypeIndex)->TypeName, data->GrantedAccess));
				break;

			case 9:	// details
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
		item.iImage = GetFrame()->GetIconIndexByType((PCWSTR)m_ObjMgr.GetType(data->ObjectTypeIndex)->TypeName);
	}
	return 0;
}

LRESULT CHandlesView::OnItemChanged(int, LPNMHDR, BOOL&) {
	auto index = m_List.GetSelectedIndex();
	m_pUI->UIEnable(ID_HANDLES_CLOSEHANDLE, index >= 0);
	m_pUI->UIEnable(ID_OBJECTS_ALLHANDLESFOROBJECT, index >= 0);
	m_pUI->UIEnable(ID_EDIT_SECURITY, index >= 0);

	return 0;
}

LRESULT CHandlesView::OnContextMenu(int, LPNMHDR, BOOL&) {
	CMenu menu;
	menu.LoadMenuW(IDR_CONTEXT);
	GetFrame()->TrackPopupMenu(menu.GetSubMenu(0), *this);

	return 0;
}

LRESULT CHandlesView::OnCloseHandle(WORD, WORD, HWND, BOOL&) {
	auto selected = m_List.GetSelectedIndex();
	ATLASSERT(selected >= 0);

	auto& item = m_Handles[selected];
	if (AtlMessageBox(*this, L"Closing a handle can potentially make the process unstable. Continue?",
		IDR_MAINFRAME, MB_OKCANCEL | MB_DEFBUTTON2 | MB_ICONWARNING) == IDCANCEL)
		return 0;

	auto hDup = m_ObjMgr.DupHandle(ULongToHandle(item->HandleValue), item->ProcessId, item->ObjectTypeIndex,
		0, DUPLICATE_SAME_ACCESS | DUPLICATE_CLOSE_SOURCE);
	if (!hDup) {
		AtlMessageBox(*this, L"Failed to close handle", IDR_MAINFRAME, MB_ICONERROR);
		return 0;
	}
	::CloseHandle(hDup);
	m_Handles.erase(m_Handles.begin() + selected);
	m_List.SetItemCountEx(static_cast<int>(m_Handles.size()), LVSICF_NOINVALIDATEALL | LVSICF_NOSCROLL);
	m_List.RedrawItems(selected, selected + m_List.GetCountPerPage());

	return 0;
}

LRESULT CHandlesView::OnRefresh(WORD, WORD, HWND, BOOL&) {
	CWaitCursor wait;
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
			auto hDup = ObjectManager::DupHandle(h.HandleValue, m_Pid, h.ObjectTypeIndex);
			auto name = hDup ? m_ObjMgr.GetObjectName(hDup, h.ObjectTypeIndex) : L"";
			if (m_NamedObjectsOnly && name.IsEmpty()) {
				::CloseHandle(hDup);
				continue;
			}
			auto hi = std::make_shared<HandleInfo>();
			hi->HandleValue = HandleToULong(h.HandleValue);
			hi->ProcessId = m_Pid;
			hi->ObjectTypeIndex = h.ObjectTypeIndex;
			if (hDup) {
				NT::OBJECT_BASIC_INFORMATION info;
				if (NT_SUCCESS(NT::NtQueryObject(hDup, NT::ObjectBasicInformation, &info, sizeof(info), nullptr))) {
					hi->GrantedAccess = info.GrantedAccess;
					hi->HandleAttributes = info.Attributes;
				}
				hi->Name = name;
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
		m_List.SetItemCountEx(static_cast<int>(m_Handles.size()), LVSICF_NOINVALIDATEALL | LVSICF_NOSCROLL);
		m_List.RedrawItems(m_List.GetTopIndex(), m_List.GetTopIndex() + m_List.GetCountPerPage());
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

LRESULT CHandlesView::OnShowAllHandles(WORD, WORD, HWND, BOOL&) {
	ATLASSERT(m_List.GetSelectedIndex() >= 0);
	auto& item = m_Handles[m_List.GetSelectedIndex()];
	ATLASSERT(item->ObjectInfo);
	CObjectHandlesDlg dlg(item->ObjectInfo, m_ProcMgr);
	CImageList il = GetFrame()->GetImageList();
	dlg.DoModal(*this, (LPARAM)il.GetIcon(GetFrame()->GetIconIndexByType(item->ObjectInfo->TypeName)));

	return 0;
}

LRESULT CHandlesView::OnShowNamedObjectsOnly(WORD, WORD, HWND, BOOL&) {
	m_NamedObjectsOnly = !m_NamedObjectsOnly;
	m_pUI->UISetCheck(ID_HANDLES_NAMEDOBJECTSONLY, m_NamedObjectsOnly);
	CWaitCursor wait;
	Refresh();

	return 0;
}

LRESULT CHandlesView::OnEditSecurity(WORD, WORD, HWND, BOOL&) {
	auto selected = m_List.GetSelectedIndex();
	ATLASSERT(selected >= 0);
	auto& item = m_Handles[selected];

	auto hDup = DriverHelper::DupHandle(UlongToHandle(item->HandleValue), item->ProcessId, READ_CONTROL | WRITE_DAC);
	if (!hDup)
		hDup = DriverHelper::DupHandle(UlongToHandle(item->HandleValue), item->ProcessId, READ_CONTROL);
	if (!hDup) {
		AtlMessageBox(*this, L"Error in handle duplication", IDR_MAINFRAME, MB_ICONERROR);
		return 0;
	}

	SecurityInfo info(hDup, item->Name);
	if (!::EditSecurity(*this, &info)) {
		AtlMessageBox(*this, L"Error launching security dialog box", IDR_MAINFRAME, MB_ICONERROR);
	}
	::CloseHandle(hDup);

	return 0;
}

void CHandlesView::Refresh() {
	if (m_hProcess && ::WaitForSingleObject(m_hProcess.get(), 0) == WAIT_OBJECT_0) {
		MessageBox((L"Process " + std::to_wstring(m_Pid) + L" is no longer running.").c_str(), L"Object Explorer", MB_OK);
		m_List.SetItemCount(0);
		return;
	}
	m_ObjMgr.EnumHandles(m_HandleType, m_Pid, m_NamedObjectsOnly);
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
	DoSort(GetSortInfo(m_List));
	m_List.SetItemCountEx(static_cast<int>(m_Handles.size()), LVSICF_NOINVALIDATEALL | LVSICF_NOSCROLL);
	m_List.RedrawItems(m_List.GetTopIndex(), m_List.GetTopIndex() + m_List.GetCountPerPage());
}

void CHandlesView::UpdateUI() {
	m_pUI->UISetCheck(ID_VIEW_PAUSE, m_Paused);
}

CString CHandlesView::AccessMaskToString(PCWSTR type, ACCESS_MASK access) {
	return CString();
}

CString CHandlesView::HandleAttributesToString(ULONG attributes) {
	CString result;
	if (attributes & 2)
		result += L", Inherit";
	if (attributes & 1)
		result += L", Protect";
	if (attributes & 4)
		result += L", Audit";

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

HWND CHandlesView::CreateToolBar() {
	CToolBarCtrl tb;
	auto hWndToolBar = tb.Create(m_hWnd, nullptr, nullptr, ATL_SIMPLE_TOOLBAR_PANE_STYLE | TBSTYLE_LIST, 0, ATL_IDW_TOOLBAR);
	tb.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS);

	CImageList tbImages;
	tbImages.Create(24, 24, ILC_COLOR32, 8, 4);
	tb.SetImageList(tbImages);

	struct {
		UINT id;
		int image;
		int style = BTNS_BUTTON;
		int state = TBSTATE_ENABLED;
		PCWSTR text = nullptr;
	} buttons[] = {
		{ ID_HANDLES_NAMEDOBJECTSONLY, IDI_FONT, BTNS_CHECK | BTNS_SHOWTEXT, TBSTATE_ENABLED, L"Named Objects Only" },
	};
	for (auto& b : buttons) {
		if (b.id == 0)
			tb.AddSeparator(0);
		else {
			int image = b.image == 0 ? I_IMAGENONE : tbImages.AddIcon(AtlLoadIconImage(b.image, 0, 24, 24));
			tb.AddButton(b.id, b.style, b.state, image, b.text, 0);
		}
	}

	return hWndToolBar;
}
