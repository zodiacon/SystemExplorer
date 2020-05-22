#include "stdafx.h"
#include "SelectColumnsDlg.h"
#include "SortHelper.h"
#include <algorithm>
#include "DialogHelper.h"

CSelectColumnsDlg::CSelectColumnsDlg(ColumnManager* cm) : m_ColMgr(cm) {
	ATLASSERT(cm);
}

CString CSelectColumnsDlg::GetColumnText(HWND, int row, int col) const {
	auto& column = m_Items[row];
	switch (col) {
		case 0: return column.Name;
		case 1: return column.Category;
	}

	ATLASSERT(false);
	return L"";
}

ListViewRowCheck CSelectColumnsDlg::IsRowChecked(int row) const {
	auto& col = GetColumn(row);
	if (col.IsMandatory())
		return ListViewRowCheck::None;

	return m_Items[row].Visible ? ListViewRowCheck::Checked : ListViewRowCheck::Unchecked;
}

void CSelectColumnsDlg::DoSort(const SortInfo* si) {
	std::sort(m_Items.begin(), m_Items.end(), [=](const auto& item1, const auto& item2) {
		switch (si->SortColumn) {
			case 0: return SortHelper::SortStrings(item1.Name, item2.Name, si->SortAscending);
			case 1: return SortHelper::SortStrings(item1.Category, item2.Category, si->SortAscending);
		}
		return false;
		});
}

LRESULT CSelectColumnsDlg::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&) {
	DialogHelper::AdjustOKCancelButtons(this);

	m_List.Attach(GetDlgItem(IDC_COLUMNS));
	m_List.SetExtendedListViewStyle(LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_LABELTIP);

	m_List.InsertColumn(0, L"Name", LVCFMT_LEFT, 150);
	m_List.InsertColumn(1, L"Category", LVCFMT_LEFT, 150);

	auto count = m_ColMgr->GetCount();
	m_Items.reserve(count);
	for (int i = 0; i < count; i++) {
		const auto& col = m_ColMgr->GetColumn(i);
		Item item;
		item.Name = col.Name;
		item.Category = col.Category;
		item.Index = i;
		item.Visible = m_ColMgr->IsVisible(i);
		m_Items.push_back(item);
	}

	m_List.SetItemCount(count);

	return TRUE;
}

LRESULT CSelectColumnsDlg::OnCloseCmd(WORD, WORD wID, HWND, BOOL&) {
	if (wID == IDOK) {
		// update column manager
		for (const auto& item : m_Items)
			m_ColMgr->SetVisible(item.Index, item.Visible);
		m_ColMgr->UpdateColumns();
	}

	EndDialog(wID);
	return 0;
}

LRESULT CSelectColumnsDlg::OnListKeyDown(int, LPNMHDR hdr, BOOL&) {
	auto lv = (LV_KEYDOWN*)hdr;
	if (lv->wVKey != VK_SPACE)
		return 0;

	int count = m_List.GetSelectedCount();
	if (count == 0)
		return 0;

	ToggleSelected();

	return 0;
}

LRESULT CSelectColumnsDlg::OnListClick(int, LPNMHDR hdr, BOOL&) {
	auto lv = (NMLISTVIEW*)hdr;
	
	UINT flags;
	int index = m_List.HitTest(lv->ptAction, &flags);
	if (index >= 0) {
		auto selected = m_List.GetSelectedCount();
		if (selected == 1)
			m_List.SetItemState(m_List.GetNextItem(-1, LVNI_SELECTED), 0, LVIS_SELECTED);
		if(selected <= 1)
			m_List.SetItemState(index, LVIS_SELECTED, LVIS_SELECTED);
		
		if (flags & LVHT_ONITEMSTATEICON)
			ToggleSelected();
	}
	return 0;
}

void CSelectColumnsDlg::ToggleSelected() {
	int i = -1;
	while ((i = m_List.GetNextItem(i, LVNI_SELECTED)) >= 0) {
		if (GetColumn(i).IsMandatory())
			continue;
		m_Items[i].Visible = !m_Items[i].Visible;
		m_List.RedrawItems(i, i);
	}
}

const ColumnManager::ColumnInfo& CSelectColumnsDlg::GetColumn(int index) const {
	return m_ColMgr->GetColumn(m_Items[index].Index);
}
