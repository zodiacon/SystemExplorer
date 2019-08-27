// View.cpp : implementation of the CView class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "AllObjectsView.h"

CAllObjectsView::CAllObjectsView(ObjectManager& om) : m_ObjMgr(om) {
}

BOOL CAllObjectsView::PreTranslateMessage(MSG* pMsg) {
	pMsg;
	return FALSE;
}

void CAllObjectsView::OnFinalMessage(HWND /*hWnd*/) {
	delete this;
}

std::shared_ptr<ObjectInfo>& CAllObjectsView::GetItem(int index) {
	return m_AllObjects[index];
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
	};

	int i = 0;
	for(auto& c : columns)
		InsertColumn(i++, c.Header, c.Format, c.Width);

	Refresh();

	return 0;
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
				item.pszText = (PWSTR)(PCWSTR)m_ObjMgr.GetProcessNameById(data->CreatorProcess);
				break;

			case 5:	// handles
				::StringCchPrintf(item.pszText, item.cchTextMax, L"%d", data->HandleCount);
				break;

			case 6:	// pointers
				::StringCchPrintf(item.pszText, item.cchTextMax, L"%d", data->PointerCount);
				break;

			case 7:	// paged pool
				::StringCchPrintf(item.pszText, item.cchTextMax, L"%d", data->PagedPoolCharge);
				break;

			case 8:	// non-paged pool
				::StringCchPrintf(item.pszText, item.cchTextMax, L"%d", data->NonPagedPoolCharge);
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