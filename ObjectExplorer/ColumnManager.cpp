#include "stdafx.h"
#include "ColumnManager.h"

bool ColumnManager::CopyTo(ColumnManager& other) const {
	if (other.GetCount() != GetCount())
		return false;

	int i = 0;
	for (const auto& column : m_Columns) {
		other.SetColumn(i, column);
		i++;
	}
	return true;
}

void ColumnManager::AddFromControl(HWND hWnd) {
	CHeaderCtrl header(hWnd == nullptr ? m_ListView.GetHeader() : CListViewCtrl(hWnd).GetHeader());
	ATLASSERT(header);
	auto count = header.GetItemCount();
	HDITEM item;
	WCHAR text[64];
	item.pszText = text;
	item.mask = HDI_FORMAT | HDI_WIDTH | HDI_TEXT;
	for (int i = 0; i < count; i++) {
		ATLVERIFY(header.GetItem(i, &item));
		ColumnInfo info;
		info.DefaultWidth = item.cxy;
		info.Flags |= (info.DefaultWidth <= 1 && ((item.fmt & HDF_FIXEDWIDTH) > 0) ? ColumnFlags::Visible : ColumnFlags::None);
		info.Name = item.pszText;
		m_Columns.push_back(info);
	}
}

void ColumnManager::SetVisible(int column, bool visible) {
	m_Columns[column].SetVisible(visible);
}

bool ColumnManager::IsVisible(int column) const {
	ATLASSERT(column >= 0 && column < GetCount());
	return (m_Columns[column].Flags & ColumnFlags::Visible) == ColumnFlags::Visible;
}

bool ColumnManager::IsConst(int column) const {
	return (m_Columns[column].Flags & ColumnFlags::Const) == ColumnFlags::Const;
}

int ColumnManager::AddColumn(PCWSTR name, int format, int width, ColumnFlags flags) {
	auto category = ::wcschr(name, L'\\');
	CString categoryName;
	if (category) {
		categoryName = CString(name, static_cast<int>(category - name));
		name = category + 1;
	}
	ColumnInfo info;
	info.Format = format;
	info.DefaultWidth = width;
	info.Flags = flags;
	info.Name = name;
	info.Category = categoryName;

	if (m_ListView && ((flags & ColumnFlags::Visible) == ColumnFlags::Visible)) {
		auto header = m_ListView.GetHeader();
		int i = m_ListView.InsertColumn(header.GetItemCount(), name, format, width);
		HDITEM hdi;
		hdi.mask = HDI_LPARAM;
		hdi.lParam = m_Columns.size();
		header.SetItem(i, &hdi);
	}

	m_Columns.push_back(info);
	if (!categoryName.IsEmpty()) {
		if (std::find(m_Categories.begin(), m_Categories.end(), categoryName) == m_Categories.end())
			m_Categories.push_back(categoryName);
		m_ColumnsByCategory[categoryName].push_back(static_cast<int>(m_Columns.size() - 1));
	}

	return static_cast<int>(m_Columns.size());
}

const ColumnManager::ColumnInfo& ColumnManager::GetColumn(int index) const {
	return m_Columns[index];
}

const std::vector<int>& ColumnManager::GetColumnsByCategory(PCWSTR category) const {
	return m_ColumnsByCategory.at(category);
}

const std::vector<CString>& ColumnManager::GetCategories() const {
	return m_Categories;
}

void ColumnManager::SetColumn(int i, const ColumnInfo& info) {
	ATLASSERT(i >= 0 && i < GetCount());
	if ((info.Flags & ColumnFlags::Visible) != (m_Columns[i].Flags & ColumnFlags::Visible)) {
		SetVisible(i, (info.Flags & ColumnFlags::Visible) == ColumnFlags::Visible);
	}
}
