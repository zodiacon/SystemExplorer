#include "stdafx.h"
#include "HandlesView.h"
#include <algorithm>
#include <execution>

CHandlesView::CHandlesView(CUpdateUIBase* pUpdateUI, IMainFrame* pFrame, PCWSTR type, DWORD pid) :
	m_pUI(pUpdateUI), m_pFrame(pFrame), m_HandleType(type), m_Pid(pid) {
}

void CHandlesView::DoSort(const SortInfo* si) {
	if (si == nullptr)
		return;
	CWaitCursor wait;
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
			return SortStrings(m_ObjMgr.GetType(h1.ObjectTypeIndex)->TypeName, m_ObjMgr.GetType(h2.ObjectTypeIndex)->TypeName, si->SortAscending);

		case 1:		// address
			return SortNumbers(h1.Object, h2.Object, si->SortAscending);

		case 2:		// name
			if ((h1.HandleAttributes & 0x8000) == 0) {
				h1.Name = m_ObjMgr.GetObjectName(ULongToHandle(h1.HandleValue), h1.ProcessId, h1.ObjectTypeIndex);
				h1.HandleAttributes |= 0x8000;
			}
			if ((h2.HandleAttributes & 0x8000) == 0) {
				h2.Name = m_ObjMgr.GetObjectName(ULongToHandle(h2.HandleValue), h2.ProcessId, h2.ObjectTypeIndex);
				h2.HandleAttributes |= 0x8000;
			}
			return SortStrings(h1.Name, h2.Name, si->SortAscending);

		case 3:		// handle
			return SortNumbers(h1.HandleValue, h2.HandleValue, si->SortAscending);

		case 4:		// process name
			return SortStrings(m_ObjMgr.GetProcessNameById(h1.ProcessId), m_ObjMgr.GetProcessNameById(h2.ProcessId), si->SortAscending);

		case 5:		// PID
			return SortNumbers(h1.ProcessId, h2.ProcessId, si->SortAscending);

		case 6:		// attributes
			return SortNumbers(h1.HandleAttributes & 0x7fff, h2.HandleAttributes & 0x7fff, si->SortAscending);

		case 7:		// access mask
			return SortNumbers(h1.GrantedAccess, h2.GrantedAccess, si->SortAscending);
	}

	return false;
}

LRESULT CHandlesView::OnCreate(UINT, WPARAM, LPARAM, BOOL&) {
	DefWindowProc();

	SetExtendedListViewStyle(LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_HEADERDRAGDROP);

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
		{ L"Details", 300 }
	};

	m_ColumnCount = _countof(columns);

	int i = 0;
	for (auto& c : columns)
		InsertColumn(i++, c.Header, c.Format, c.Width);

	SetImageList(m_pFrame->GetImageList(), LVSIL_SMALL);

	Refresh();

	return 0;
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
				item.pszText = (PWSTR)(PCWSTR)m_ObjMgr.GetProcessNameById(data->ProcessId);
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
				auto h = m_ObjMgr.DupHandle(ULongToHandle(data->HandleValue), data->ProcessId, data->ObjectTypeIndex);
				if (h) {
					auto& type = m_ObjMgr.GetType(data->ObjectTypeIndex)->TypeDetails;
					CString details = type ? type->GetDetails(h) : L"";
					if (!details.IsEmpty())
						::StringCchCopy(item.pszText, item.cchTextMax, details);
					::CloseHandle(h);
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
	m_pUI->UIEnable(ID_HANDLES_CLOSEHANDLE, GetSelectedCount() > 0);

	return 0;
}

LRESULT CHandlesView::OnContextMenu(int, LPNMHDR, BOOL&) {
	return 0;
}

LRESULT CHandlesView::OnCloseHandle(WORD, WORD, HWND, BOOL&) {
	auto selected = GetSelectedIndex();
	ATLASSERT(selected >= 0);

	auto& item = m_Handles[selected];
	if (MessageBox(L"Closing a handle can potentially make the process unstable. Continue?", L"Object Explorer",
		MB_OKCANCEL | MB_DEFBUTTON2 | MB_ICONWARNING) == IDNO)
		return 0;

	auto hDup = m_ObjMgr.DupHandle(ULongToHandle(item->HandleValue), item->ProcessId, item->ObjectTypeIndex, 
		0, DUPLICATE_SAME_ACCESS | DUPLICATE_CLOSE_SOURCE);
	if (!hDup) {
		AtlMessageBox(*this, L"Failed to close handle", L"Object Explorer", MB_ICONERROR);
		return 0;
	}
	::CloseHandle(hDup);

	return 0;
}

LRESULT CHandlesView::OnRefresh(WORD, WORD, HWND, BOOL&) {
	Refresh();
	return 0;
}

void CHandlesView::Refresh() {
	m_ObjMgr.EnumHandles(m_HandleType, m_Pid);
	m_Handles = m_ObjMgr.GetHandles();
	DoSort(GetSortInfo());
	SetItemCountEx(static_cast<int>(m_Handles.size()), LVSICF_NOINVALIDATEALL | LVSICF_NOSCROLL);
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
