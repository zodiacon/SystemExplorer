#include "stdafx.h"
#include "SelectColumnsDlg.h"
#include "SortHelper.h"
#include <algorithm>
#include "DialogHelper.h"
#include <algorithm>

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

	m_Tree.Attach(GetDlgItem(IDC_TREE));
	m_Tree.ModifyStyle(0, TVS_CHECKBOXES);
	ATLASSERT(m_Tree.GetStyle() & TVS_CHECKBOXES);
	m_Tree.SetExtendedStyle(TVS_EX_DIMMEDCHECKBOXES, TVS_EX_DIMMEDCHECKBOXES);

	UpdateVisibility();

	// init list

	auto count = m_ColMgr->GetCount();
	m_Items.reserve(count);
	m_TreeItems.reserve(count / 2);

	for (int i = 0; i < count; i++) {
		const auto& col = m_ColMgr->GetColumn(i);
		if (std::find_if(m_TreeItems.begin(), m_TreeItems.end(), [&](auto& item) {
			return item.Text == col.Category;
			}) == m_TreeItems.end()) {

			TreeItem t;
			t.Text = col.Category;
			t.hItem = m_Tree.InsertItem(col.Category, TVI_ROOT, TVI_LAST);
			m_Tree.SetItemData(t.hItem, m_TreeItems.size() + 0x100);
			m_TreeItems.push_back(t);
		}
		auto it = std::find_if(m_TreeItems.begin(), m_TreeItems.end(), [&](auto& item) {
			return item.Text == col.Category;
			});
		ATLASSERT(it != m_TreeItems.end());

		Item item;
		item.Name = col.Name;
		item.Category = col.Category;
		item.Index = i;
		item.Visible = m_ColMgr->IsVisible(i);

		auto hItem = m_Tree.InsertItem(col.Name, it->hItem, TVI_LAST);
		item.hItem = hItem;
		m_Tree.SetItemData(hItem, i);
		if ((col.Flags & ColumnFlags::Mandatory) == ColumnFlags::Mandatory)
			m_Tree.SetItemState(hItem, 0, TVIS_STATEIMAGEMASK);
		else
			m_Tree.SetCheckState(hItem, item.Visible);
		m_Items.push_back(item);
	}

	for (auto& item : m_TreeItems)
		m_Tree.SortChildren(item.hItem);

	m_List.SetItemCount(count);

	m_Init = false;

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
		if (selected <= 1)
			m_List.SetItemState(index, LVIS_SELECTED, LVIS_SELECTED);

		if (flags & LVHT_ONITEMSTATEICON)
			ToggleSelected();
	}
	return 0;
}

LRESULT CSelectColumnsDlg::OnSelectList(WORD code, WORD id, HWND, BOOL&) {
	if (code != BN_CLICKED)
		return 0;

	s_UseList = id == IDC_TYPE;
	UpdateVisibility();

	return 0;
}

LRESULT CSelectColumnsDlg::OnTreeItemChanged(int, LPNMHDR hdr, BOOL&) {
	if (m_Init)
		return 0;

	auto tv = (NMTVITEMCHANGE*)hdr;
	if (((tv->uStateNew ^ tv->uStateOld) & TVIS_STATEIMAGEMASK) == 0)
		return 0;

	auto checked = m_Tree.GetCheckState(tv->hItem);
	auto index = (int)m_Tree.GetItemData(tv->hItem);
	CString text;
	m_Tree.GetItemText(tv->hItem, text);
	if (index & 0x100) {
		// category
		CheckItemCategory(text, checked);
		CheckTreeChildren(tv->hItem, checked);
	}
	else {
		auto it = std::find_if(m_Items.begin(), m_Items.end(), [&](auto& item) {
			return item.Name == text;
			});

		it->Visible = checked;
	}
	return 0;
}

void CSelectColumnsDlg::ToggleSelected() {
	int i = -1;
	while ((i = m_List.GetNextItem(i, LVNI_SELECTED)) >= 0) {
		if (GetColumn(i).IsMandatory())
			continue;
		m_Items[i].Visible = !m_Items[i].Visible;

		m_Tree.SetCheckState(m_Items[i].hItem, m_Items[i].Visible);

		m_List.RedrawItems(i, i);
	}
}

void CSelectColumnsDlg::UpdateVisibility() {
	if (s_UseList) {
		CheckRadioButton(IDC_TYPE, IDC_TYPE2, IDC_TYPE);
		m_List.ShowWindow(SW_SHOW);
		m_Tree.ShowWindow(SW_HIDE);
	}
	else {
		CheckRadioButton(IDC_TYPE, IDC_TYPE2, IDC_TYPE2);
		m_List.ShowWindow(SW_HIDE);
		m_Tree.ShowWindow(SW_SHOW);
	}
}

void CSelectColumnsDlg::CheckItemCategory(const CString& category, bool check) {
	for (auto& item : m_Items)
		if (item.Category == category)
			item.Visible = check;
}

void CSelectColumnsDlg::CheckTreeChildren(HTREEITEM hParent, bool check) {
	auto hChild = m_Tree.GetChildItem(hParent);
	while (hChild) {
		if (m_Tree.GetItemState(hChild, TVIS_STATEIMAGEMASK))
			m_Tree.SetCheckState(hChild, check);
		hChild = m_Tree.GetNextSiblingItem(hChild);
	}
}

const ColumnManager::ColumnInfo& CSelectColumnsDlg::GetColumn(int index) const {
	return m_ColMgr->GetColumn(m_Items[index].Index);
}
