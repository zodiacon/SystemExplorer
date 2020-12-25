#include "pch.h"
#include "SystemModulesView.h"
#include "resource.h"
#include "SortHelper.h"

CString CSystemModulesView::GetColumnText(HWND, int row, int col) const {
	auto& m = m_Modules[row];
	CString text;

	switch (col) {
		case 0: return m->Name.c_str();
		case 1: return m->FullPath.c_str();
		case 2: text.Format(L"0x%p", m->ImageBase); break;
		case 3: text.Format(L"0x%X", m->ImageSize); break;
		case 4: text.Format(L"0x%p", m->MappedBase); break;
		case 5: text.Format(L"%u", m->LoadOrderIndex); break;
		//case 6: text.Format(L"%u", m->InitOrderIndex); break;
	}
	return text;
}

int CSystemModulesView::GetRowImage(HWND, int row) const {
	return 0;
}

void CSystemModulesView::DoSort(const SortInfo* si) {
	if (si == nullptr)
		return;

	std::sort(m_Modules.begin(), m_Modules.end(), [&si](auto& m1, auto& m2) -> bool {
		switch (si->SortColumn) {
			case 0: return SortHelper::SortStrings(m1->Name, m2->Name, si->SortAscending);
			case 1: return SortHelper::SortStrings(m1->FullPath, m2->FullPath, si->SortAscending);
			case 2: return SortHelper::SortNumbers(m1->ImageBase, m2->ImageBase, si->SortAscending);
			case 3: return SortHelper::SortNumbers(m1->ImageSize, m2->ImageSize, si->SortAscending);
			case 4: return SortHelper::SortNumbers(m1->MappedBase, m2->MappedBase, si->SortAscending);
			case 5: return SortHelper::SortNumbers(m1->LoadOrderIndex, m2->LoadOrderIndex, si->SortAscending);
		}
		return false;
		});
}

void CSystemModulesView::DoRefresh() {
	auto count = m_Tracker.EnumModules();
	m_Modules = m_Tracker.GetModules();
	DoSort(GetSortInfo(m_List));
	m_List.SetItemCountEx(count, LVSICF_NOINVALIDATEALL | LVSICF_NOSCROLL);
	auto top = m_List.GetTopIndex();
	m_List.RedrawItems(top, top + m_List.GetCountPerPage());
}

LRESULT CSystemModulesView::OnCreate(UINT, WPARAM, LPARAM, BOOL&) {
	m_hWndClient = m_List.Create(m_hWnd, rcDefault, nullptr, ListViewDefaultStyle & ~LVS_SHAREIMAGELISTS);
	m_List.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_LABELTIP | LVS_EX_HEADERDRAGDROP);

	CImageList images;
	images.Create(16, 16, ILC_COLOR32, 2, 1);
	images.AddIcon(AtlLoadIcon(IDI_DLL));

	m_List.SetImageList(images, LVSIL_SMALL);

	auto cm = GetColumnManager(m_List);
	cm->AddColumn(L"Name", LVCFMT_LEFT, 200, ColumnFlags::Visible | ColumnFlags::Mandatory | ColumnFlags::Const);
	cm->AddColumn(L"Full Path", LVCFMT_LEFT, 320, ColumnFlags::Visible | ColumnFlags::Const);
	cm->AddColumn(L"Image Base", LVCFMT_RIGHT, 150, ColumnFlags::Visible | ColumnFlags::Numeric | ColumnFlags::Const);
	cm->AddColumn(L"Image Size", LVCFMT_RIGHT, 100, ColumnFlags::Visible | ColumnFlags::Numeric | ColumnFlags::Const);
	cm->AddColumn(L"Mapped Base", LVCFMT_RIGHT, 120, ColumnFlags::Numeric | ColumnFlags::Const);
	cm->AddColumn(L"Load Order", LVCFMT_RIGHT, 60, ColumnFlags::Visible | ColumnFlags::Numeric | ColumnFlags::Const);
	//cm->AddColumn(L"Init Order", LVCFMT_RIGHT, 60, ColumnFlags::Visible | ColumnFlags::Numeric | ColumnFlags::Const);

	cm->UpdateColumns();

	DoRefresh();

	return 0;
}
