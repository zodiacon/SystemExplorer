#include "pch.h"
#include "SearchView.h"
#include "SortHelper.h"

CSearchView::CSearchView(IMainFrame* frame) : CViewBase(frame), m_SearchBar(this) {
}

CString CSearchView::GetColumnText(HWND, int row, int col) const {
	if (m_Items.empty()) {
		if (col == 2)
			return L"Searching...";
		return L"";
	}

	auto& item = m_Items[row];
	CString text;

	switch (col) {
		case 0: return item.Type;
		case 1: 
			if (item.Type == L"DLL")
				text.Format(L"0x%llX", item.Id);
			else
				text.Format(L"%llu (0x%llX)", item.Id, item.Id); 
			break;
		case 2: return item.Name;
		case 3: text.Format(L"%d (0x%X)", item.ProcessId, item.ProcessId); break;
		case 4: return item.Details;
	}

	return text;
}

int CSearchView::GetRowImage(HWND, int row) const {
	if (m_Items.empty() && row == 0)
		return m_FirstIcon;

	const auto& type = m_Items[row].Type;
	if (type == L"DLL")
		return m_FirstIcon + 1;

	auto icon = GetFrame()->GetIconIndexByType(type);
	return icon;
}

void CSearchView::DoSort(const SortInfo* si) {
	ATLASSERT(si);

	std::sort(m_Items.begin(), m_Items.end(), [&](const auto& i1, const auto& i2) {
		switch (si->SortColumn) {
			case 0: return SortHelper::SortStrings(i1.Type, i2.Type, si->SortAscending);
			case 1: return SortHelper::SortNumbers(i1.Id, i2.Id, si->SortAscending);
			case 2: return SortHelper::SortStrings(i1.Name, i2.Name, si->SortAscending);
			case 3: return SortHelper::SortNumbers(i1.ProcessId, i2.ProcessId, si->SortAscending);
			case 4: return SortHelper::SortStrings(i1.Details, i2.Details, si->SortAscending);
		}
		return false;
		});
}

void CSearchView::UpdateResults() {
	auto count = m_Items.size();
	m_Items = m_Searcher->GetResults();
	if (count != m_Items.size()) {
		m_List.SetItemCountEx((int)m_Items.size(), LVSICF_NOINVALIDATEALL | LVSICF_NOSCROLL);
		Sort(m_List, true);
	}
	else if (m_Items.empty())
		m_List.SetItemCount(0);
}

void CSearchView::StartSearch(PCWSTR text) {
	m_Items.clear();
	m_List.SetItemCount(1);

	m_Searcher.reset(new ObjectSearcher(*this));
	m_Searcher->SearchAsync(text);
	SetTimer(1, 1000, nullptr);
}

bool CSearchView::CancelSearch() {
	UpdateResults();
	return true;
}

LRESULT CSearchView::OnCreate(UINT, WPARAM, LPARAM, BOOL&) {
	CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
	//auto hWndToolBar = CreateToolBar();
	//AddSimpleReBarBand(hWndToolBar);

	m_SearchBar.Create(*this);
	AddSimpleReBarBand(m_SearchBar);

	m_hWndClient = m_List.Create(*this, rcDefault, nullptr, ListViewDefaultStyle);
	m_List.SetExtendedListViewStyle(LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP | LVS_EX_INFOTIP | LVS_EX_HEADERDRAGDROP);

	m_List.InsertColumn(0, L"Type", LVCFMT_LEFT, 80);
	m_List.InsertColumn(1, L"Handle", LVCFMT_RIGHT, 120);
	m_List.InsertColumn(2, L"Name", LVCFMT_LEFT, 250);
	m_List.InsertColumn(3, L"PID", LVCFMT_RIGHT, 120);
	m_List.InsertColumn(4, L"Details", LVCFMT_LEFT, 250);

	CImageList il = GetFrame()->GetImageList();
	CImageList images = il.Duplicate();
	m_FirstIcon = images.AddIcon(AtlLoadIconImage(IDI_FIND, 0, 16, 16));
	images.AddIcon(AtlLoadIconImage(IDI_DLL, 0, 16, 16));

	m_List.SetImageList(images, LVSIL_SMALL);

	return 0;
}

LRESULT CSearchView::OnSearchDone(UINT, WPARAM, LPARAM, BOOL&) {
	KillTimer(1);
	UpdateResults();
	m_SearchBar.Reset();

	return 0;
}

LRESULT CSearchView::OnTimer(UINT, WPARAM id, LPARAM, BOOL&) {
	if (id == 1)
		UpdateResults();

	return 0;
}
