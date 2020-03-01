#include "stdafx.h"
#include "ObjectHandlesDlg.h"
#include "ObjectManager.h"
#include "HandlesView.h"

CObjectHandlesDlg::CObjectHandlesDlg(ObjectInfo* info, ObjectManager& om) : m_pObject(info), m_ObjMgr(om) {
}

LRESULT CObjectHandlesDlg::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&) {
	m_List.Attach(GetDlgItem(IDC_HANDLES));
	m_List.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);

	m_List.InsertColumn(0, L"Process Name", LVCFMT_LEFT, 150);
	m_List.InsertColumn(1, L"PID", LVCFMT_RIGHT, 100);
	m_List.InsertColumn(2, L"Handle", LVCFMT_RIGHT, 100);
	m_List.InsertColumn(3, L"Access", LVCFMT_RIGHT, 100);
	m_List.InsertColumn(4, L"Attributes", LVCFMT_RIGHT, 100);

	m_Handles = m_pObject->Handles;
	m_List.SetItemCount(m_pObject->HandleCount);
	
	CString text;
	GetWindowText(text);
	text.Format(L"%s 0x%p (%s)", text, m_pObject->Object, m_pObject->TypeName);
	SetWindowText(text);

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
			::StringCchPrintf(item.pszText, item.cchTextMax, L"0x%X", data->GrantedAccess);
			break;

		case 4:		// attributes
			::StringCchCopy(item.pszText, item.cchTextMax, CHandlesView::HandleAttributesToString(data->HandleAttributes));
			break;
		}
	}
	return 0;
}
