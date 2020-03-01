#include "stdafx.h"
#include "ObjectHandlesDlg.h"
#include "ObjectManager.h"
#include "HandlesView.h"
#include "SortHelper.h"
#include <algorithm>

CObjectHandlesDlg::CObjectHandlesDlg(ObjectInfo* info, ObjectManager& om) : m_pObject(info), m_ObjMgr(om) {
}

void CObjectHandlesDlg::DoSort(const SortInfo* si) {
	std::sort(m_Handles.begin(), m_Handles.end(), [=](const auto& h1, const auto& h2) {
		return CompareItems(h1.get(), h2.get(), si->SortColumn, si->SortAscending);
		});

	int n = m_List.GetTopIndex();
	m_List.RedrawItems(n, n + m_List.GetCountPerPage());
}

bool CObjectHandlesDlg::CompareItems(const HandleInfo* h1, const HandleInfo* h2, int col, bool asc) {
	switch (col) {
		case 0: return SortHelper::SortStrings(m_ObjMgr.GetProcessNameById(h1->ProcessId), m_ObjMgr.GetProcessNameById(h2->ProcessId), asc);
		case 1: return SortHelper::SortNumbers(h1->ProcessId, h2->ProcessId, asc);
		case 2: return SortHelper::SortNumbers(h1->HandleValue, h2->HandleValue, asc);
		case 3: return SortHelper::SortNumbers(h1->GrantedAccess, h2->GrantedAccess, asc);
		case 4: return SortHelper::SortNumbers(h1->HandleAttributes & 0x7fff, h2->HandleAttributes & 0x7fff, asc);
	}
	return false;
}

LRESULT CObjectHandlesDlg::OnInitDialog(UINT, WPARAM, LPARAM lParam, BOOL&) {
	m_List.Attach(GetDlgItem(IDC_HANDLES));
	m_List.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);

	m_List.InsertColumn(0, L"Process Name", LVCFMT_LEFT, 180);
	m_List.InsertColumn(1, L"PID", LVCFMT_RIGHT, 100);
	m_List.InsertColumn(2, L"Handle", LVCFMT_RIGHT, 100);
	m_List.InsertColumn(3, L"Access", LVCFMT_RIGHT, 100);
	m_List.InsertColumn(4, L"Attributes", LVCFMT_RIGHT, 100);

	m_Handles = m_pObject->Handles;
	m_List.SetItemCount(m_pObject->HandleCount);

	SetIcon((HICON)lParam, FALSE);

	CString text;
	GetWindowText(text);
	text.Format(L"%s 0x%p (%s)", text, m_pObject->Object, m_pObject->TypeName);
	SetWindowText(text);

	DlgResize_Init(true);

	return 0;
}

LRESULT CObjectHandlesDlg::OnCloseCmd(WORD, WORD wID, HWND, BOOL&) {
	EndDialog(wID);

	return 0;
}

LRESULT CObjectHandlesDlg::OnGetDispInfo(int, LPNMHDR hdr, BOOL&) {
	auto lv = (NMLVDISPINFO*)hdr;
	auto& item = lv->item;
	auto& data = m_Handles[item.iItem];

	if (item.mask & LVIF_TEXT) {
		switch (item.iSubItem) {
		case 0:		// process name
			::StringCchCopy(item.pszText, item.cchTextMax, m_ObjMgr.GetProcessNameById(data->ProcessId));
			break;

		case 1:		// PID
			::StringCchPrintf(item.pszText, item.cchTextMax, L"%d", data->ProcessId);
			break;

		case 2:		// handle
			::StringCchPrintf(item.pszText, item.cchTextMax, L"0x%X", data->HandleValue);
			break;

		case 3:		// access
			::StringCchPrintf(item.pszText, item.cchTextMax, L"0x%08X", data->GrantedAccess);
			break;

		case 4:		// attributes
			::StringCchCopy(item.pszText, item.cchTextMax, CHandlesView::HandleAttributesToString(data->HandleAttributes));
			break;
		}
	}
	return 0;
}
