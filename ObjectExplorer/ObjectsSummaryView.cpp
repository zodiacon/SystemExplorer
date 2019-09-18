// View.cpp : implementation of the CObjectSummaryView class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include <algorithm>

#include "ObjectsSummaryView.h"

BOOL CObjectSummaryView::PreTranslateMessage(MSG* pMsg) {
	return FALSE;
}

bool CObjectSummaryView::TogglePause() {
	m_Paused = !m_Paused;
	if (m_Paused)
		KillTimer(1);
	else
		SetTimer(1, m_Interval, nullptr);
	return m_Paused;
}

void CObjectSummaryView::SetInterval(int interval) {
	m_Interval = interval;
}

DWORD CObjectSummaryView::OnPrePaint(int, LPNMCUSTOMDRAW) {
	return CDRF_NOTIFYITEMDRAW;
}

DWORD CObjectSummaryView::OnSubItemPrePaint(int, LPNMCUSTOMDRAW cd) {
	auto lcd = (LPNMLVCUSTOMDRAW)cd;
	auto sub = lcd->iSubItem;
	lcd->clrTextBk = CLR_INVALID;

	if (sub < 2 || sub > 5)
		return CDRF_DODEFAULT;

	auto index = (int)cd->dwItemSpec;
	auto& item = GetItem(index);
	auto& changes = m_ObjectManager.GetChanges();

	for (auto& change : changes) {
		if (std::get<0>(change) == item && MapChangeToColumn(std::get<1>(change)) == sub) {
			lcd->clrTextBk = std::get<2>(change) > 0 ? RGB(0, 255, 0) : RGB(255, 0, 0);
		}
	}
	return CDRF_NOTIFYITEMDRAW;
}

DWORD CObjectSummaryView::OnItemPrePaint(int, LPNMCUSTOMDRAW) {
	return CDRF_NOTIFYSUBITEMDRAW;
}

LRESULT CObjectSummaryView::OnActivatePage(UINT, WPARAM wParam, LPARAM, BOOL&) {
	if(wParam == 0)
		KillTimer(1);
	else {
		if (!m_Paused)
			SetTimer(1, m_Interval);
	}

	return 0;
}

LRESULT CObjectSummaryView::OnCreate(UINT, WPARAM, LPARAM, BOOL &) {
	DefWindowProc();

	InsertColumn(0, L"Name", LVCFMT_LEFT, 130);
	InsertColumn(1, L"Index", LVCFMT_RIGHT, 50);
	InsertColumn(2, L"Objects", LVCFMT_RIGHT, 100);
	InsertColumn(3, L"Handles", LVCFMT_RIGHT, 100);
	InsertColumn(4, L"Peak Objects", LVCFMT_RIGHT, 100);
	InsertColumn(5, L"Peak Handles", LVCFMT_RIGHT, 100);
	InsertColumn(6, L"Pool Type", LVCFMT_LEFT, 110);
	InsertColumn(7, L"Default Paged Charge", LVCFMT_RIGHT, 110);
	InsertColumn(8, L"Default Non-Paged Charge", LVCFMT_RIGHT, 110);
	InsertColumn(9, L"Valid Access Mask", LVCFMT_RIGHT, 110);

	SetExtendedListViewStyle(LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP | LVS_EX_HEADERDRAGDROP, 0);

	SetImageList(m_pFrame->GetImageList(), LVSIL_SMALL);

	auto count = m_ObjectManager.EnumTypes();
	SetItemCount(count);
	m_Items = m_ObjectManager.GetObjectTypes();

	SetTimer(1, m_Interval, nullptr);

	m_UIUpdate.UIEnable(ID_EDIT_COPY, FALSE);

	return 0;
}

LRESULT CObjectSummaryView::OnTimer(UINT, WPARAM wParam, LPARAM, BOOL &) {
	if (wParam == 1) {
		KillTimer(1);
		m_ObjectManager.EnumTypes();
		if (m_SortColumn >= 0)
			DoSort();
		LockWindowUpdate();
		RedrawItems(GetTopIndex(), GetCountPerPage() + GetTopIndex());
		LockWindowUpdate(FALSE);

		if (!m_Paused)
			SetTimer(1, m_Interval, nullptr);

		CString text;
		text.Format(L"Objects: %d", m_ObjectManager.GetTotalObjects());
		m_UIUpdate.UISetText(0, text);
		text.Format(L"Handles: %d", m_ObjectManager.GetTotalHandles());
		m_UIUpdate.UISetText(1, text);
	}
	return 0;
}

LRESULT CObjectSummaryView::OnGetDispInfo(int, LPNMHDR hdr, BOOL &) {
	auto lv = reinterpret_cast<NMLVDISPINFO*>(hdr);
	auto& item = lv->item;
	auto index = item.iItem;
	auto col = item.iSubItem;
	auto& data = GetItem(index);

	if (lv->item.mask & LVIF_TEXT) {
		switch (col) {
			case 0:		// name
				item.pszText = (PWSTR)(PCWSTR)data->TypeName;
				break;

			case 1:		// index
				::StringCchPrintf(item.pszText, item.cchTextMax, L"%d", data->TypeIndex);
				break;

			case 2:		// objects
				::StringCchPrintf(item.pszText, item.cchTextMax, L"%d", data->TotalNumberOfObjects);
				break;

			case 3:		// handles
				::StringCchPrintf(item.pszText, item.cchTextMax, L"%d", data->TotalNumberOfHandles);
				break;

			case 4:		// peak objects
				::StringCchPrintf(item.pszText, item.cchTextMax, L"%d", data->HighWaterNumberOfObjects);
				break;

			case 5:		// peak handles
				::StringCchPrintf(item.pszText, item.cchTextMax, L"%d", data->HighWaterNumberOfHandles);
				break;

			case 6:		// name
				item.pszText = (PWSTR)PoolTypeToString(data->PoolType);
				break;

			case 7:		// default non-paged charge
				::StringCchPrintf(item.pszText, item.cchTextMax, L"%d", data->DefaultNonPagedPoolCharge);
				break;

			case 8:		// default paged charge
				::StringCchPrintf(item.pszText, item.cchTextMax, L"%d", data->DefaultPagedPoolCharge);
				break;

			case 9:		// valid access mask
				::StringCchPrintf(item.pszText, item.cchTextMax, L"0x%08X", data->ValidAccessMask);
				break;
		}
	}
	if (lv->item.mask & LVIF_IMAGE) {
		item.iImage = m_pFrame->GetIconIndexByType(data->TypeName);
	}
	return 0;
}

LRESULT CObjectSummaryView::OnColumnClick(int, LPNMHDR hdr, BOOL &) {
	auto lv = (NMLISTVIEW*)hdr;
	auto col = lv->iSubItem;
	auto oldSortColumn = m_SortColumn;
	if (col == m_SortColumn)
		m_SortAscending = !m_SortAscending;
	else {
		m_SortColumn = col;
		m_SortAscending = true;
	}

	HDITEM h;
	h.mask = HDI_FORMAT;
	auto header = GetHeader();
	header.GetItem(m_SortColumn, &h);
	h.fmt = (h.fmt & HDF_JUSTIFYMASK) | HDF_STRING | (m_SortAscending ? HDF_SORTUP : HDF_SORTDOWN);
	header.SetItem(m_SortColumn, &h);

	if (oldSortColumn >= 0 && oldSortColumn != m_SortColumn) {
		h.mask = HDI_FORMAT;
		header.GetItem(oldSortColumn, &h);
		h.fmt = (h.fmt & HDF_JUSTIFYMASK) | HDF_STRING;
		header.SetItem(oldSortColumn, &h);
	}

	DoSort();
	RedrawItems(GetTopIndex(), GetTopIndex() + GetCountPerPage());

	return 0;
}

LRESULT CObjectSummaryView::OnFindItem(int, LPNMHDR hdr, BOOL &) {
	auto fi = (NMLVFINDITEM*)hdr;
	if (fi->lvfi.flags & (LVFI_PARTIAL | LVFI_SUBSTRING)) {
		int start = fi->iStart;
		auto count = GetItemCount();
		auto text = fi->lvfi.psz;
		auto len = ::wcslen(text);
		for (int i = start; i < start + count; i++) {
			auto index = i % count;
			if (::_wcsnicmp(m_Items[index]->TypeName, text, len) == 0)
				return index;
		}
	}
	return -1;
}

LRESULT CObjectSummaryView::OnEditCopy(WORD, WORD, HWND, BOOL &) {
	ATLASSERT(GetSelectedCount() > 0);

	auto first = -1;
	CString text, temp;
	for (UINT i = 0; i < GetSelectedCount(); i++) {
		first = GetNextItem(first, LVNI_SELECTED);
		ATLASSERT(first >= 0);
		for (int col = 0; col < ColumnCount; col++) {
			GetItemText(first, col, temp);
			text += temp;
			if (col < ColumnCount - 1)
				text += L",";
		}
		text += L"\r\n";
	}

	if (OpenClipboard()) {
		::EmptyClipboard();
		auto size = (text.GetLength() + 1) * sizeof(WCHAR);
		auto hData = ::GlobalAlloc(GMEM_MOVEABLE, size);
		if (hData) {
			auto p = ::GlobalLock(hData);
			if (p) {
				::memcpy(p, text.GetBuffer(), size);
				::GlobalUnlock(p);
				::SetClipboardData(CF_UNICODETEXT, hData);
			}
		}
		::CloseClipboard();
	}

	return 0;
}

LRESULT CObjectSummaryView::OnSelectionChanged(int, LPNMHDR, BOOL &) {
	m_UIUpdate.UIEnable(ID_EDIT_COPY, GetSelectedCount() > 0);

	return 0;
}

void CObjectSummaryView::OnViewActivated() {
	int zz = 9;
}

LRESULT CObjectSummaryView::OnExport(WORD, WORD, HWND, BOOL &) {
	CSimpleFileDialog dlg(FALSE, nullptr, nullptr,
		OFN_HIDEREADONLY | OFN_EXPLORER | OFN_OVERWRITEPROMPT,
		L"All Files\0*.*\0", m_hWnd);
	if (dlg.DoModal() == IDOK) {
		wil::unique_hfile hFile(::CreateFile(dlg.m_szFileName, GENERIC_WRITE, 0, nullptr, OPEN_ALWAYS, 0, nullptr));
		if (hFile) {
			CString text, temp;
			for (int i = 0; i < GetItemCount(); i++) {
				for (int col = 0; col < ColumnCount; col++) {
					GetItemText(i, col, temp);
					text += temp;
					if (col < 8)
						text += L",";
				}
				text += L"\r\n";
			}
			DWORD bytes;
			::WriteFile(hFile.get(), text.GetBuffer(), (text.GetLength() + 1) * sizeof(WCHAR), &bytes, nullptr);
		}
		else {
			MessageBox(L"Failed to open file.", L"Kernel Object Viewer");
		}
	}

	return 0;
}

std::shared_ptr<ObjectTypeInfoEx> CObjectSummaryView::GetItem(int index) const {
	return m_Items[index];
}

PCWSTR CObjectSummaryView::PoolTypeToString(PoolType type) {
	switch (type) {
		case PoolType::NonPagedPool:
			return L"Non Paged";
		case PoolType::PagedPool:
			return L"Paged";
		case PoolType::NonPagedPoolNx:
			return L"Non Paged NX";
		case PoolType::PagedPoolSessionNx:
			return L"Paged Session NX";
	}
	return L"Unknown";
}

bool CObjectSummaryView::CompareItems(const std::shared_ptr<ObjectTypeInfoEx>& item1, const std::shared_ptr<ObjectTypeInfoEx>& item2) const {
	bool result = false;
	switch (m_SortColumn) {
		case 0:		// name
			result = item2->TypeName.CompareNoCase(item1->TypeName) >= 0;
			break;

		case 1:		// index
			result = item2->TypeIndex > item1->TypeIndex;
			break;

		case 2:		// objects
			result = m_SortAscending ?
				(item2->TotalNumberOfObjects > item1->TotalNumberOfObjects) :
				(item2->TotalNumberOfObjects < item1->TotalNumberOfObjects);
			return result;

		case 3:		// handles
			result = m_SortAscending ?
				(item2->TotalNumberOfHandles > item1->TotalNumberOfHandles) :
				(item2->TotalNumberOfHandles < item1->TotalNumberOfHandles);
			return result;

		case 4:		// peak objects
			result = m_SortAscending ?
				(item2->HighWaterNumberOfObjects > item1->HighWaterNumberOfObjects) :
				(item2->HighWaterNumberOfObjects < item1->HighWaterNumberOfObjects);
			return result;

		case 5:		// peak handles
			result = m_SortAscending ?
				(item2->HighWaterNumberOfHandles > item1->HighWaterNumberOfHandles) :
				(item2->HighWaterNumberOfHandles < item1->HighWaterNumberOfHandles);
			return result;

		case 6:		// pool type
			result = m_SortAscending ?
				(item2->PoolType > item1->PoolType) : (item1->PoolType > item2->PoolType);
			return result;

		case 7:		// default non-paged charge
			result = m_SortAscending ?
				(item2->DefaultNonPagedPoolCharge > item1->DefaultNonPagedPoolCharge) :
				(item1->DefaultNonPagedPoolCharge > item2->DefaultNonPagedPoolCharge);
			return result;

		case 8:		// default paged charge
			result = m_SortAscending ?
				(item2->DefaultPagedPoolCharge > item1->DefaultPagedPoolCharge) :
				(item1->DefaultPagedPoolCharge > item2->DefaultPagedPoolCharge);
			return result;

		case 9:		// valid access mask
			result = m_SortAscending ?
				(item2->ValidAccessMask > item1->ValidAccessMask) :
				(item1->ValidAccessMask > item2->ValidAccessMask);
			return result;
	}
	return m_SortAscending ? result : !result;
}

void CObjectSummaryView::DoSort() {
	std::sort(m_Items.begin(), m_Items.end(),
		[this](auto& i1, auto& i2) { return CompareItems(i1, i2); });
}

int CObjectSummaryView::MapChangeToColumn(ObjectManager::ChangeType type) const {
	switch (type) {
		case ObjectManager::ChangeType::TotalHandles: return 3;
		case ObjectManager::ChangeType::TotalObjects: return 2;
		case ObjectManager::ChangeType::PeakHandles: return 5;
		case ObjectManager::ChangeType::PeakObjects: return 4;
	}
	return -1;
}
