#include "stdafx.h"
#include "HandlesView.h"

CHandlesView::CHandlesView(CUpdateUIBase* pUpdateUI, IMainFrame* pFrame, PCWSTR type) :
	m_pUI(pUpdateUI), m_pFrame(pFrame), m_HandleType(type) {
}

LRESULT CHandlesView::OnCreate(UINT, WPARAM, LPARAM, BOOL&) {
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
		{ L"Handle", 100, LVCFMT_RIGHT },
		{ L"Process Name", 160 },
		{ L"PID", 450 },
		{ L"Attributes", 100 },
		//{ L"Non-Paged Pool", 100, LVCFMT_RIGHT },
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

		}
	}
	if (item.mask & LVIF_IMAGE) {
		item.iImage = m_pFrame->GetIconIndexByType((PCWSTR)m_ObjMgr.GetType(data->ObjectTypeIndex)->TypeName);
	}
	return 0;
}

LRESULT CHandlesView::OnContextMenu(int, LPNMHDR, BOOL&) {
	return LRESULT();
}

void CHandlesView::Refresh() {
	m_ObjMgr.EnumHandles(m_HandleType);
	m_Handles = m_ObjMgr.GetHandles();
	SetItemCountEx(static_cast<int>(m_Handles.size()), LVSICF_NOINVALIDATEALL | LVSICF_NOSCROLL);
}
