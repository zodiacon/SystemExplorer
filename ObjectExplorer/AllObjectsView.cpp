// View.cpp : implementation of the CView class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include <algorithm>
#include <execution>
#include "AllObjectsView.h"
#include "ClipboardHelper.h"
#include "ProcessHelper.h"
#include "NtDll.h"

int CAllObjectsView::ColumnCount;

CAllObjectsView::CAllObjectsView(ObjectManager& om, CUpdateUIBase* pUpdateUI) : m_ObjMgr(om), m_pUpdateUI(pUpdateUI) {
}

BOOL CAllObjectsView::PreTranslateMessage(MSG* pMsg) {
	return FALSE;
}

void CAllObjectsView::DoSort(const SortInfo* si) {
	std::sort(std::execution::par, m_AllObjects.begin(), m_AllObjects.end(), [this, si](const auto& o1, const auto& o2) {
		return CompareItems(*o1.get(), *o2.get(), si);
		});

	RedrawItems(GetTopIndex(), GetTopIndex() + GetCountPerPage());
}

void CAllObjectsView::OnFinalMessage(HWND /*hWnd*/) {
	delete this;
}

std::shared_ptr<ObjectInfoEx>& CAllObjectsView::GetItem(int index) {
	return m_AllObjects[index];
}

bool CAllObjectsView::CompareItems(const ObjectInfoEx& o1, const ObjectInfoEx& o2, const SortInfo* si) {
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

	ATLASSERT(false);
	return false;
}

CString CAllObjectsView::GetObjectDetails(ObjectInfoEx* info) const {
	auto h = info->LocalHandle.get();//m_ObjMgr.DupHandle(info);
	if (!h)
		return L"";

	auto& type = m_ObjMgr.GetType(info->TypeIndex)->TypeDetails;
	return type ? type->GetDetails(h) : L"";
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

	SetExtendedListViewStyle(LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

	struct {
		PCWSTR Header;
		int Width;
		int Format = LVCFMT_LEFT;
	} columns[] = {
		{ L"Type", 130 },
		{ L"Address", 140, LVCFMT_RIGHT },
		{ L"Name", 300 },
		{ L"Handles", 100, LVCFMT_RIGHT },
		{ L"Details", 500 },
		//{ L"Non-Paged Pool", 100, LVCFMT_RIGHT },
	};

	ColumnCount = _countof(columns);

	int i = 0;
	for(auto& c : columns)
		InsertColumn(i++, c.Header, c.Format, c.Width);

	// icons
	struct {
		UINT icon;
		PCWSTR name;
	} icons[] = {
		{ IDI_GENERIC,		L"" },
		{ IDI_PROCESS,		L"Process" },
		{ IDI_THREAD,		L"Thread" },
		{ IDI_JOB,			L"Job" },
		{ IDI_MUTEX,		L"Mutant" },
		{ IDI_EVENT,		L"Event" },
		{ IDI_SEMAPHORE,	L"Semaphore" },
		{ IDI_DESKTOP,		L"Desktop" },
		{ IDI_WINSTATION,	L"WindowStation" },
		{ IDI_PORT,			L"ALPC Port" },
		{ IDI_KEY,			L"Key" },
		{ IDI_DEVICE,		L"Device" },
		{ IDI_FILE,			L"File" },
		{ IDI_SYMLINK,		L"SymbolicLink" },
		{ IDI_SECTION,		L"Section" },
		{ IDI_DIRECTORY,	L"Directory" },
		{ IDI_TIMER,		L"Timer" },
		{ IDI_TOKEN,		L"Token" },
	};

	m_Images.Create(16, 16, ILC_COLOR32 | ILC_COLOR, 8, 8);
	int index = 0;
	for (auto& icon : icons) {
		m_Images.AddIcon(AtlLoadIcon(icon.icon));
		_iconMap.insert({ icon.name, index++ });
	}
	SetImageList(m_Images, LVSIL_SMALL);

	Refresh();
	//SetTimer(1, 2000, nullptr);

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
				item.pszText = (PWSTR)(PCWSTR)m_ObjMgr.GetType(data->TypeIndex)->TypeName;
				break;

			case 1:	// address
				::StringCchPrintf(item.pszText, item.cchTextMax, L"0x%p", data->Object);
				break;

			case 2:	// name
				item.pszText = (PWSTR)(PCWSTR)data->Name;
				break;

			case 3:	// handles
				::StringCchPrintf(item.pszText, item.cchTextMax, L"%u", data->HandleCount);
				break;

			case 4:	// details
				::StringCchCopy(item.pszText, item.cchTextMax, GetObjectDetails(data.get()));

		}
	}
	if (item.mask & LVIF_IMAGE) {
		auto it = _iconMap.find((PCWSTR)m_ObjMgr.GetType(data->TypeIndex)->TypeName);
		item.iImage = it == _iconMap.end() ? 0 : it->second;
	}
	return 0;
}

void CAllObjectsView::Refresh() {
	m_ObjMgr.EnumProcesses();
	m_ObjMgr.EnumHandlesAndObjects();
	m_AllObjects = m_ObjMgr.GetObjects();
	SetItemCountEx(static_cast<int>(m_AllObjects.size()), LVSICF_NOINVALIDATEALL | LVSICF_NOSCROLL);
}
