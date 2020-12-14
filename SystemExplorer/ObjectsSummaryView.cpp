// View.cpp : implementation of the CObjectSummaryView class
//
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "resource.h"
#include <algorithm>
#include "ClipboardHelper.h"
#include "ObjectsSummaryView.h"
#include "SortHelper.h"

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

bool CObjectSummaryView::IsSortable(int col) const {
	return col != 10;
}

DWORD CObjectSummaryView::OnPrePaint(int, LPNMCUSTOMDRAW) {
	return CDRF_NOTIFYITEMDRAW;
}

DWORD CObjectSummaryView::OnSubItemPrePaint(int, LPNMCUSTOMDRAW cd) {
	auto lcd = (LPNMLVCUSTOMDRAW)cd;
	auto sub = lcd->iSubItem;
	lcd->clrTextBk = CLR_INVALID;
	if (sub == 0) {
		return CDRF_DODEFAULT;
	}

	if((GetColumnManager(m_List)->GetColumn(GetRealColumn(m_List, sub)).Flags & ColumnFlags::Numeric) == ColumnFlags::Numeric)
		::SelectObject(cd->hdc, (HFONT)GetFrame()->GetMonoFont());
	else
		::SelectObject(cd->hdc, m_hFont);

	if (sub < 2 || sub > 5)
		return CDRF_DODEFAULT | CDRF_NEWFONT;

	auto index = (int)cd->dwItemSpec;
	auto item = GetItem(index);
	auto& changes = m_ObjectManager.GetChanges();
	lcd->clrText = CLR_INVALID;

	for (auto& change : changes) {
		if (std::get<0>(change) == item && MapChangeToColumn(std::get<1>(change)) == sub) {
			lcd->clrTextBk = std::get<2>(change) >= 0 ? RGB(0, 255, 0) : RGB(255, 0, 0);
			lcd->clrText = std::get<2>(change) >= 0 ? RGB(0, 0, 0) : RGB(255, 255, 255);
		}
	}
	return CDRF_DODEFAULT | CDRF_NEWFONT;
}

DWORD CObjectSummaryView::OnItemPrePaint(int, LPNMCUSTOMDRAW cd) {
	m_hFont = (HFONT)::GetCurrentObject(cd->hdc, OBJ_FONT);
	return CDRF_NOTIFYSUBITEMDRAW;
}

LRESULT CObjectSummaryView::OnActivatePage(UINT, WPARAM activate, LPARAM, BOOL&) {
	if(activate == 0)
		KillTimer(1);
	else {
		if (!m_Paused)
			SetTimer(1, m_Interval);
		UpdateUI();
	}

	return 0;
}

LRESULT CObjectSummaryView::OnCreate(UINT, WPARAM, LPARAM, BOOL &) {
	m_hWndClient = m_List.Create(*this, rcDefault, nullptr, ListViewDefaultStyle);
	auto cm = GetColumnManager(m_List);

	cm->AddColumn(L"Name", LVCFMT_LEFT, 180, ColumnFlags::Visible);
	cm->AddColumn(L"Index", LVCFMT_RIGHT, 50, ColumnFlags::Visible | ColumnFlags::Numeric);
	cm->AddColumn(L"Objects", LVCFMT_RIGHT, 100, ColumnFlags::Visible | ColumnFlags::Numeric);
	cm->AddColumn(L"Handles", LVCFMT_RIGHT, 100, ColumnFlags::Visible | ColumnFlags::Numeric);
	cm->AddColumn(L"Peak Objects", LVCFMT_RIGHT, 100, ColumnFlags::Visible | ColumnFlags::Numeric);
	cm->AddColumn(L"Peak Handles", LVCFMT_RIGHT, 100, ColumnFlags::Visible | ColumnFlags::Numeric);
	cm->AddColumn(L"Pool Type", LVCFMT_LEFT, 110);
	cm->AddColumn(L"Default Paged Charge", LVCFMT_RIGHT, 130, ColumnFlags::Visible | ColumnFlags::Numeric);
	cm->AddColumn(L"Default NP Charge", LVCFMT_RIGHT, 130, ColumnFlags::Visible | ColumnFlags::Numeric);
	cm->AddColumn(L"Valid Access Mask", LVCFMT_RIGHT, 120, ColumnFlags::Visible | ColumnFlags::Numeric);
	cm->AddColumn(L"Generic Mapping", LVCFMT_LEFT, 450);

	cm->UpdateColumns();

	m_List.SetExtendedListViewStyle(LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP | LVS_EX_HEADERDRAGDROP);

	m_List.SetImageList(GetFrame()->GetImageList(), LVSIL_SMALL);

	auto count = m_ObjectManager.EnumTypes();
	m_List.SetItemCount(count);
	m_Items = m_ObjectManager.GetObjectTypes();

	SetTimer(1, m_Interval, nullptr);

	GetFrame()->GetUpdateUI()->UIEnable(ID_EDIT_COPY, FALSE);

	return 0;
}

LRESULT CObjectSummaryView::OnTimer(UINT, WPARAM wParam, LPARAM, BOOL &) {
	if (wParam == 1) {
		KillTimer(1);
		m_ObjectManager.EnumTypes();
		auto si = GetSortInfo(*this);
		if (si)
			DoSort(si);
		LockWindowUpdate();
		auto top = m_List.GetTopIndex();
		m_List.RedrawItems(top, m_List.GetCountPerPage() + top);
		m_List.LockWindowUpdate(FALSE);

		if (!m_Paused)
			SetTimer(1, m_Interval, nullptr);

	}
	return 0;
}

LRESULT CObjectSummaryView::OnGetDispInfo(int, LPNMHDR hdr, BOOL &) {
	auto lv = reinterpret_cast<NMLVDISPINFO*>(hdr);
	auto& item = lv->item;
	auto index = item.iItem;
	auto col = item.iSubItem;
	auto data = GetItem(index);

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

			case 7:		// default paged charge
				::StringCchPrintf(item.pszText, item.cchTextMax, L"%d", data->DefaultPagedPoolCharge);
				break;

			case 8:		// default non-paged charge
				::StringCchPrintf(item.pszText, item.cchTextMax, L"%d", data->DefaultNonPagedPoolCharge);
				break;

			case 9:		// valid access mask
				::StringCchPrintf(item.pszText, item.cchTextMax, L"0x%08X", data->ValidAccessMask);
				break;

			case 10:	// generic mapping
				::StringCchPrintf(item.pszText, item.cchTextMax, L"Read: 0x%08X, Write: 0x%08X, Execute: 0x%08X, All: 0x%08X",
					data->GenericMapping.GenericRead, data->GenericMapping.GenericWrite, data->GenericMapping.GenericExecute, data->GenericMapping.GenericAll);
				break;
		}
	}
	if (lv->item.mask & LVIF_IMAGE) {
		item.iImage = GetFrame()->GetIconIndexByType(data->TypeName);
	}
	return 0;
}

LRESULT CObjectSummaryView::OnEditCopy(WORD, WORD, HWND, BOOL &) {
	ATLASSERT(m_List.GetSelectedCount() > 0);

	auto first = -1;
	CString text, temp;
	auto columns = GetColumnManager(m_List)->GetCount();
	for (UINT i = 0; i < m_List.GetSelectedCount(); i++) {
		first = m_List.GetNextItem(first, LVNI_SELECTED);
		ATLASSERT(first >= 0);
		for (int col = 0; col < columns; col++) {
			m_List.GetItemText(first, col, temp);
			text += temp;
			if (col < columns - 1)
				text += L",";
		}
		text += L"\r\n";
	}

	ClipboardHelper::CopyText(*this, text);

	return 0;
}

LRESULT CObjectSummaryView::OnSelectionChanged(int, LPNMHDR, BOOL &) {
	UpdateUI();

	return 0;
}

void CObjectSummaryView::UpdateUI() {
	auto ui = GetFrame()->GetUpdateUI();
	auto count = m_List.GetSelectedCount();
	ui->UIEnable(ID_EDIT_COPY, count > 0);
	ui->UIEnable(ID_TYPE_ALLHANDLES, count == 1);
	ui->UIEnable(ID_TYPE_ALLOBJECTS, count == 1);
	ui->UISetCheck(ID_VIEW_PAUSE, m_Paused);
}

LRESULT CObjectSummaryView::OnExport(WORD, WORD, HWND, BOOL &) {
	CSimpleFileDialog dlg(FALSE, L"csv", nullptr,
		OFN_HIDEREADONLY | OFN_EXPLORER | OFN_OVERWRITEPROMPT,
		L"CSV Files (*.csv)\0*.csv\0All Files\0*.*\0", m_hWnd);
	if (dlg.DoModal() == IDOK) {
		wil::unique_hfile hFile(::CreateFile(dlg.m_szFileName, GENERIC_WRITE, 0, nullptr, OPEN_ALWAYS, 0, nullptr));
		if (hFile) {
			CString text, temp;
			auto columns = GetColumnManager(m_List)->GetCount();
			for (int i = 0; i < m_List.GetItemCount(); i++) {
				for (int col = 0; col < columns; col++) {
					m_List.GetItemText(i, col, temp);
					text += temp;
					if (col < columns - 1)
						text += L",";
				}
				text += L"\r\n";
			}
			DWORD bytes;
			::WriteFile(hFile.get(), text.GetBuffer(), text.GetLength() * sizeof(WCHAR), &bytes, nullptr);
		}
		else {
			AtlMessageBox(*this, L"Failed to create file.", IDR_MAINFRAME);
		}
	}

	return 0;
}

LRESULT CObjectSummaryView::OnForwardMessage(UINT, WPARAM, LPARAM lParam, BOOL& handled) {
	auto pMsg = reinterpret_cast<MSG*>(lParam);
	LRESULT result;
	handled = ProcessWindowMessage(*this, pMsg->message, pMsg->wParam, pMsg->lParam, result, 1);
	return result;
}

LRESULT CObjectSummaryView::OnContextMenu(UINT, WPARAM, LPARAM lParam, BOOL&) {
	UpdateUI();

	CMenu menu;
	menu.LoadMenu(IDR_CONTEXT);
	GetFrame()->TrackPopupMenu(menu.GetSubMenu(3), *this);

	return 0;
}

LRESULT CObjectSummaryView::OnShowAllHandles(WORD, WORD, HWND, BOOL&) {
	auto item = GetItem(m_List.GetSelectedIndex());
	GetFrame()->ShowAllHandles(item->TypeName);

	return 0;
}

LRESULT CObjectSummaryView::OnShowAllObjects(WORD, WORD, HWND, BOOL&) {
	auto item = GetItem(m_List.GetSelectedIndex());
	GetFrame()->ShowAllObjects(item->TypeName);
	
	return 0;
}

LRESULT CObjectSummaryView::OnPause(WORD, WORD, HWND, BOOL&) {
	auto paused = TogglePause();
	GetFrame()->GetUpdateUI()->UISetCheck(ID_VIEW_PAUSE, paused);

	return 0;
}

std::shared_ptr<ObjectTypeInfo> CObjectSummaryView::GetItem(int index) const {
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

bool CObjectSummaryView::CompareItems(const std::shared_ptr<ObjectTypeInfo>& item1, const std::shared_ptr<ObjectTypeInfo>& item2, int col, bool asc) const {
	switch (col) {
		case 0: return SortHelper::SortStrings(item1->TypeName, item2->TypeName, asc);
		case 1: return SortHelper::SortNumbers(item1->TypeIndex, item2->TypeIndex, asc);
		case 2:	return SortHelper::SortNumbers(item1->TotalNumberOfObjects, item2->TotalNumberOfObjects, asc);
		case 3:	return SortHelper::SortNumbers(item1->TotalNumberOfHandles, item2->TotalNumberOfHandles, asc);
		case 4:	return SortHelper::SortNumbers(item1->HighWaterNumberOfObjects, item2->HighWaterNumberOfObjects, asc);
		case 5:	return SortHelper::SortNumbers(item1->HighWaterNumberOfHandles, item2->HighWaterNumberOfHandles, asc);
		case 6: return SortHelper::SortNumbers(item1->PoolType, item2->PoolType, asc);
		case 7:	return SortHelper::SortNumbers(item1->DefaultNonPagedPoolCharge, item2->DefaultNonPagedPoolCharge, asc);
		case 8:	return SortHelper::SortNumbers(item1->DefaultPagedPoolCharge, item2->DefaultPagedPoolCharge, asc);
		case 9:	return SortHelper::SortNumbers(item1->ValidAccessMask, item2->ValidAccessMask, asc);
	}
	return false;
}

void CObjectSummaryView::DoSort(const SortInfo* si) {
	std::sort(m_Items.begin(), m_Items.end(),
		[&](auto& i1, auto& i2) { return CompareItems(i1, i2, si->SortColumn, si->SortAscending); });
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
