#include "stdafx.h"
#include "ModulesView.h"
#include <algorithm>
#include "SortHelper.h"
#include "resource.h"

CModulesView::CModulesView(DWORD pid, IMainFrame* frame) : m_Tracker(pid), m_pFrame(frame) {
}

CModulesView::CModulesView(HANDLE hProcess, IMainFrame* frame) : m_Tracker(hProcess), m_pFrame(frame) {
}

CString CModulesView::GetColumnText(HWND, int row, int col) const {
	const auto& mi = m_Modules[row];
	CString text;

	switch (col) {
		case 0: return mi->Name.c_str();
		case 1: return mi->Type == WinSys::MapType::Image ? L"Image" : L"Data";
		case 2: text.Format(L"0x%08X", mi->ModuleSize); break;
		case 3: text.Format(L"0x%p", mi->Base); break;
		case 4: 
			if (mi->Type == WinSys::MapType::Image)
				text.Format(L"0x%p", mi->ImageBase);
			else
				text = L"N/A";
			break;
		case 5: return mi->Path.c_str();
	}

	return text;
}

int CModulesView::GetRowImage(int row) const {
	auto& mi = m_Modules[row];

	if (mi->Type == WinSys::MapType::Data)
		return 1;

	return mi->ImageBase == mi->Base ? 0 : 2;
}

void CModulesView::DoSort(const SortInfo* si) {
	if (si == nullptr)
		return;

	std::sort(m_Modules.begin(), m_Modules.end(), [=](auto& m1, auto& m2) {
		switch (si->SortColumn) {
			case 0: return SortHelper::SortStrings(m1->Name, m2->Name, si->SortAscending);
			case 1: return SortHelper::SortNumbers(m1->Type, m2->Type, si->SortAscending);
			case 2: return SortHelper::SortNumbers(m1->ModuleSize, m2->ModuleSize, si->SortAscending);
			case 3: return SortHelper::SortNumbers(m1->Base, m2->Base, si->SortAscending);
			case 4: return SortHelper::SortNumbers(m1->ImageBase, m2->ImageBase, si->SortAscending);
			case 5: return SortHelper::SortStrings(m1->Path, m2->Path, si->SortAscending);
		}
		return false;
		});

}

LRESULT CModulesView::OnCreate(UINT, WPARAM, LPARAM, BOOL&) {
	m_hWndClient = m_List.Create(m_hWnd, rcDefault, nullptr, ListViewDefaultStyle);
	m_List.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_LABELTIP);

	CImageList images;
	images.Create(16, 16, ILC_COLOR32, 4, 4);
	UINT ids[] = { IDI_DLL, IDI_DLL_DB, IDI_DLL_RELOC };
	for (auto id : ids)
		images.AddIcon(AtlLoadIconImage(id, 0, 16, 16));
	m_List.SetImageList(images, LVSIL_SMALL);

	auto cm = GetColumnManager(m_List);
	cm->AddColumn(L"Name", LVCFMT_LEFT, 170, ColumnFlags::Visible | ColumnFlags::Mandatory);
	cm->AddColumn(L"Type", LVCFMT_LEFT, 60, ColumnFlags::Visible);
	cm->AddColumn(L"Size", LVCFMT_RIGHT, 100, ColumnFlags::Visible);
	cm->AddColumn(L"Base Address", LVCFMT_RIGHT, 130, ColumnFlags::Visible);
	cm->AddColumn(L"Image Base", LVCFMT_RIGHT, 130, ColumnFlags::Visible);
	cm->AddColumn(L"Path", LVCFMT_LEFT, 450, ColumnFlags::Visible);

	cm->UpdateColumns();

	Refresh();

	return 0;
}

LRESULT CModulesView::OnTimer(UINT, WPARAM, LPARAM, BOOL&) {
	return 0;
}

LRESULT CModulesView::OnDestroy(UINT, WPARAM, LPARAM, BOOL&) {
	return 0;
}

LRESULT CModulesView::OnActivate(UINT, WPARAM activate, LPARAM, BOOL&) {
	if (activate)
		Refresh();
	return 0;
}

LRESULT CModulesView::OnRefresh(WORD, WORD, HWND, BOOL&) {
	Refresh();

	return 0;
}

void CModulesView::Refresh() {
	auto first = m_Modules.empty();
	auto count = m_Tracker.EnumModules();
	m_Modules = m_Tracker.GetModules();
	DoSort(nullptr);
	m_List.SetItemCountEx(count, LVSICF_NOSCROLL | LVSICF_NOINVALIDATEALL);
	if (!first) {
		m_List.RedrawItems(m_List.GetTopIndex(), m_List.GetTopIndex() + m_List.GetCountPerPage());
	}
}
