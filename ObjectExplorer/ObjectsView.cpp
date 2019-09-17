// View.cpp : implementation of the CView class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include <algorithm>
#include <execution>
#include "ObjectsView.h"
#include "ClipboardHelper.h"
#include "ProcessHelper.h"
#include "IObjectsView.h"

int CObjectsView::ColumnCount;

CObjectsView::CObjectsView(CUpdateUIBase* pUpdateUI, IMainFrame* pFrame, PCWSTR type)
	: m_pUpdateUI(pUpdateUI), m_pFrame(pFrame), m_Typename(type) {
	ATLASSERT(pFrame);
}

BOOL CObjectsView::PreTranslateMessage(MSG* pMsg) {
	return FALSE;
}

void CObjectsView::DoSort(const SortInfo* si) {
	std::sort(std::execution::seq, m_Objects.begin(), m_Objects.end(), [this, si](const auto& o1, const auto& o2) {
		return CompareItems(*o1.get(), *o2.get(), si);
		});

	RedrawItems(GetTopIndex(), GetTopIndex() + GetCountPerPage());
}

void CObjectsView::OnFinalMessage(HWND /*hWnd*/) {
	delete this;
}

std::shared_ptr<ObjectInfoEx>& CObjectsView::GetItem(int index) {
	return m_Objects[index];
}

bool CObjectsView::CompareItems(const ObjectInfoEx& o1, const ObjectInfoEx& o2, const SortInfo* si) {
	switch (si->SortColumn) {
		case 0:		// type
			return SortStrings(m_ObjMgr.GetType(o1.TypeIndex)->TypeName, m_ObjMgr.GetType(o2.TypeIndex)->TypeName, si->SortAscending);

		case 1:		// address
			return SortNumbers(o1.Object, o2.Object, si->SortAscending);

		case 2:		// name
			return SortStrings(o1.Name, o2.Name, si->SortAscending);

		case 3:		// handles
			return SortNumbers(o1.HandleCount, o2.HandleCount, si->SortAscending);

	}

	//ATLASSERT(false);
	return false;
}

CString CObjectsView::GetObjectDetails(ObjectInfoEx* info) const {
	auto h = ObjectManager::DupHandle(info);	// info->LocalHandle.get();
	if (!h)
		return L"";

	auto& type = m_ObjMgr.GetType(info->TypeIndex)->TypeDetails;
	CString details = type ? type->GetDetails(h) : L"";
	::CloseHandle(h);
	return details;
}

CString CObjectsView::GetProcessHandleInfo(const HandleInfo & hi) const {
	CString info;
	info.Format(L"H: %d, PID: %d (%s)",
		hi.HandleValue, hi.ProcessId, (PCWSTR)m_ObjMgr.GetProcessNameById(hi.ProcessId));
	return info;
}

LRESULT CObjectsView::OnActivatePage(UINT, WPARAM, LPARAM, BOOL&) {
	return LRESULT();
}

LRESULT CObjectsView::OnTimer(UINT, WPARAM id, LPARAM, BOOL&) {
	if (id == 1) {
		Refresh();
		auto si = GetSortInfo();
		if (si && si->SortColumn >= 0)
			DoSort(si);
		RedrawItems(GetTopIndex(), GetTopIndex() + GetCountPerPage());
	}
	return 0;
}

LRESULT CObjectsView::OnEditCopy(WORD, WORD, HWND, BOOL&) {
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

LRESULT CObjectsView::OnCreate(UINT, WPARAM, LPARAM, BOOL&) {
	DefWindowProc();

	SetExtendedListViewStyle(LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

	struct {
		PCWSTR Header;
		int Width;
		int Format = LVCFMT_LEFT;
	} columns[] = {
		{ L"Type", 140 },
		{ L"Address", 140, LVCFMT_RIGHT },
		{ L"Name", 330 },
		//{ L"Create Time", 120 },
		{ L"Handles", 100, LVCFMT_RIGHT },
		{ L"First Handle", 160, LVCFMT_LEFT },
		{ L"Details", 450 },
		//{ L"Non-Paged Pool", 100, LVCFMT_RIGHT },
	};

	ColumnCount = _countof(columns);

	int i = 0;
	for (auto& c : columns)
		InsertColumn(i++, c.Header, c.Format, c.Width);

	SetImageList(m_pFrame->GetImageList(), LVSIL_SMALL);

	Refresh();
	//SetTimer(1, 2000, nullptr);

	return 0;
}

LRESULT CObjectsView::OnDestroy(UINT, WPARAM, LPARAM, BOOL&) {
	return 0;
}

LRESULT CObjectsView::OnForwardMessage(UINT, WPARAM, LPARAM lParam, BOOL& handled) {
	auto msg = (MSG*)lParam;
	LRESULT result = 0;
	//handled = ProcessWindowMessage(msg->hwnd, msg->message, msg->wParam, msg->lParam, result, 2);
	return result;
}

LRESULT CObjectsView::OnGetDispInfo(int, LPNMHDR hdr, BOOL&) {
	auto lv = (NMLVDISPINFO*)hdr;
	auto& item = lv->item;
	auto& data = GetItem(item.iItem);

	if (item.mask & LVIF_TEXT) {
		switch (item.iSubItem) {
			case 0:	// type
				item.pszText = (PWSTR)(PCWSTR)m_ObjMgr.GetType(data->TypeIndex)->TypeName;
				break;

			case 1:	// address
				::StringCchPrintf(item.pszText, item.cchTextMax, L"0x%p", data->Object);
				break;

			case 2:	// name
				item.pszText = (PWSTR)(PCWSTR)data->Name;
				break;

				//case 3:	// create time
				//	if (data->CreateTime > 0)
				//		::StringCchCopy(item.pszText, item.cchTextMax, (PCWSTR)CTime(*(FILETIME*)&data->CreateTime).Format(L"%D %X"));
				//	break;

			case 3:	// handles
				::StringCchPrintf(item.pszText, item.cchTextMax, L"%u", data->HandleCount);
				break;

			case 4:	// first handle
				::StringCchCopy(item.pszText, item.cchTextMax, GetProcessHandleInfo(*data->Handles[0].get()));
				break;

			case 5:	// details
				::StringCchCopy(item.pszText, item.cchTextMax, GetObjectDetails(data.get()));

		}
	}
	if (item.mask & LVIF_IMAGE) {
		item.iImage = m_pFrame->GetIconIndexByType((PCWSTR)m_ObjMgr.GetType(data->TypeIndex)->TypeName);
	}
	return 0;
}

LRESULT CObjectsView::OnContextMenu(int, LPNMHDR hdr, BOOL &) {
	auto lv = (NMITEMACTIVATE*)hdr;
	if (m_pView) {
		UINT id;
		int index;
		if (!m_pView->GetContextMenu(id, index))
			return FALSE;

		CMenuHandle menu = AtlLoadMenu(id);
		if (!menu)
			return FALSE;
		return m_pFrame->TrackPopupMenu(menu.GetSubMenu(index), *this);
	}
	return FALSE;
}

void CObjectsView::Refresh() {
	m_ObjMgr.EnumProcesses();
	m_ObjMgr.EnumTypes();
	m_ObjMgr.EnumHandlesAndObjects(m_Typename);
	m_Objects = m_ObjMgr.GetObjects();
	SetItemCountEx(static_cast<int>(m_Objects.size()), LVSICF_NOINVALIDATEALL | LVSICF_NOSCROLL);
}
