// View.cpp : implementation of the CView class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include <algorithm>
#include "AllObjectsView.h"
#include "ClipboardHelper.h"

int CAllObjectsView::ColumnCount;

CAllObjectsView::CAllObjectsView(ObjectManager& om) : m_ObjMgr(om) {
}

BOOL CAllObjectsView::PreTranslateMessage(MSG* pMsg) {
	return FALSE;
}

void CAllObjectsView::DoSort(const SortInfo* si) {
	std::sort(m_AllObjects.begin(), m_AllObjects.end(), [si](const auto& o1, const auto& o2) {
		return CompareItems(*o1.get(), *o2.get(), si);
		});

	RedrawItems(GetTopIndex(), GetTopIndex() + GetCountPerPage());
}

void CAllObjectsView::OnFinalMessage(HWND /*hWnd*/) {
	delete this;
}

std::shared_ptr<ObjectInfo>& CAllObjectsView::GetItem(int index) {
	return m_AllObjects[index];
}

bool CAllObjectsView::CompareItems(const ObjectInfo& o1, const ObjectInfo& o2, const SortInfo* si) {
	switch (si->SortColumn) {
		case 0:		// type
			return SortStrings(o1.Type->Name, o2.Type->Name, si->SortAscending);

		case 1:		// address
			return SortNumbers(o1.Object, o2.Object, si->SortAscending);

		case 2:		// name
			return SortStrings(o1.Name, o2.Name, si->SortAscending);

		case 3:		// creator PID
			return SortNumbers(o1.CreatorProcess, o2.CreatorProcess, si->SortAscending);

		case 4:		// creator name
			return SortStrings(o1.CreatorName, o2.CreatorName, si->SortAscending);

		case 5:		// handles
			return SortNumbers(o1.HandleCount, o2.HandleCount, si->SortAscending);

		case 6:		// pointers
			return SortNumbers(o1.PointerCount, o2.PointerCount, si->SortAscending);

		case 7:		// paged pool
			return SortNumbers(o1.PagedPoolCharge, o2.PagedPoolCharge, si->SortAscending);

		case 8:		// non-paged pool
			return SortNumbers(o1.NonPagedPoolCharge, o2.NonPagedPoolCharge, si->SortAscending);
	}

	ATLASSERT(false);
	return false;
}

ULONG CAllObjectsView::GetTrueRefCount(void* pObject) {
	return ULONG_MAX;
}

LRESULT CAllObjectsView::OnTimer(UINT, WPARAM id, LPARAM, BOOL&) {
	if (id == 1) {
		Refresh();
		auto si = GetSortInfo();
		if(si && si->SortColumn >= 0)
			DoSort(si);
		RedrawItems(GetTopIndex(), GetTopIndex() + GetCountPerPage());
	}
	return 0;
}

LRESULT CAllObjectsView::OnEditCopy(WORD, WORD, HWND, BOOL&) {
	auto selected = GetSelectedIndex();
	if (selected < 0)
		return 0;

	CString text;
	for (int i = 0; i < ColumnCount; i++) {
		CString temp;
		GetItemText(selected, i, temp);
		text += temp + ", ";
	}

	ClipboardHelper::CopyText(*this, text.Left(text.GetLength() - 2));

	return 0;
}

LRESULT CAllObjectsView::OnCreate(UINT, WPARAM, LPARAM, BOOL&) {
	DefWindowProc();

	SetExtendedListViewStyle(LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT);

	struct {
		PCWSTR Header;
		int Width;
		int Format = LVCFMT_LEFT;
	} columns[] = {
		{ L"Type", 130 },
		{ L"Address", 140, LVCFMT_RIGHT },
		{ L"Name", 300 },
		{ L"Creator PID", 100, LVCFMT_RIGHT },
		{ L"Creator Name", 150 },
		{ L"Handles", 100, LVCFMT_RIGHT },
		{ L"Pointers", 100, LVCFMT_RIGHT },
		{ L"Paged Pool", 100, LVCFMT_RIGHT },
		{ L"Non-Paged Pool", 100, LVCFMT_RIGHT },
		{ L"True Ref", 100, LVCFMT_RIGHT },
	};

	m_IsWindows81OrLater = ::IsWindows8Point1OrGreater();

	ColumnCount = _countof(columns);

	int i = 0;
	for(auto& c : columns)
		InsertColumn(i++, c.Header, c.Format, c.Width);

	Refresh();
	SetTimer(1, 1000, nullptr);

	return 0;
}

LRESULT CAllObjectsView::OnDestroy(UINT, WPARAM, LPARAM, BOOL&) {
	return 0;
}

LRESULT CAllObjectsView::OnForwardMessage(UINT, WPARAM, LPARAM lParam, BOOL& handled) {
	auto msg = (MSG*)lParam;
	LRESULT result = 0;
	//handled = ProcessWindowMessage(msg->hwnd, msg->message, msg->wParam, msg->lParam, result, 2);
	return result;
}

LRESULT CAllObjectsView::OnGetDispInfo(int, LPNMHDR hdr, BOOL&) {
	auto lv = (NMLVDISPINFO*)hdr;
	auto& item = lv->item;
	auto& data = GetItem(item.iItem);

	if (item.mask & LVIF_TEXT) {
		switch (item.iSubItem) {
			case 0:	// type
				item.pszText = (PWSTR)(PCWSTR)data->Type->Name;
				break;

			case 1:	// address
				::StringCchPrintf(item.pszText, item.cchTextMax, L"0x%p", data->Object);
				break;

			case 2:	// name
				item.pszText = (PWSTR)(PCWSTR)data->Name;
				break;

			case 3:	// creator PID
				::StringCchPrintf(item.pszText, item.cchTextMax, L"%d (0x%X)", 
					data->CreatorProcess, data->CreatorProcess);
				break;

			case 4: // creator name
				item.pszText = (PWSTR)(PCWSTR)data->CreatorName;
				break;

			case 5:	// handles
				::StringCchPrintf(item.pszText, item.cchTextMax, L"%u", data->HandleCount);
				break;

			case 6:	// pointers
				::StringCchPrintf(item.pszText, item.cchTextMax, L"%u", data->PointerCount);
				break;

			case 7:	// paged pool
				::StringCchPrintf(item.pszText, item.cchTextMax, L"%u", data->PagedPoolCharge);
				break;

			case 8:	// non-paged pool
				::StringCchPrintf(item.pszText, item.cchTextMax, L"%u", data->NonPagedPoolCharge);
				break;

			case 9:
				ULONG value = data->PointerCount;
				if (m_IsWindows81OrLater)
					value = GetTrueRefCount(data->Object);
				if(value != ULONG_MAX)
					::StringCchPrintf(item.pszText, item.cchTextMax, L"%u", value);
				break;

		}
	}
	return 0;
}

void CAllObjectsView::Refresh() {
	m_ObjMgr.EnumProcesses();
	m_ObjMgr.EnumObjects();
	m_AllObjects = m_ObjMgr.GetAllObjects();
	SetItemCountEx(static_cast<int>(m_AllObjects.size()), LVSICF_NOINVALIDATEALL | LVSICF_NOSCROLL);
}
