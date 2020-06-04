#pragma once

#include <map>
#include <vector>

enum class ColumnFlags {
	None = 0,
	Visible = 1,
	Fixed = 2,
	Const = 4,		// currently unused
	Mandatory = 8,
	Numeric = 0x10,
	Modified = 0x80
};
DEFINE_ENUM_FLAG_OPERATORS(ColumnFlags);

class ColumnManager {
public:
	struct ColumnInfo {
		int DefaultWidth;
		int Format;
		CString Name;
		ColumnFlags Flags;
		CString Category;

		bool IsVisible() const;
		bool IsMandatory() const;
		void SetVisible(bool);
	};

	ColumnManager(HWND hListView) : m_ListView(hListView) {}
	~ColumnManager();
	ColumnManager(const ColumnManager&) = default;
	ColumnManager& operator=(const ColumnManager&) = delete;

	HWND GetListView() const {
		return m_ListView;
	}

	bool CopyTo(ColumnManager& other) const;
	void AddFromControl(HWND hList = nullptr);
	void SetVisible(int column, bool visible);
	bool IsVisible(int column) const;
	bool IsModified(int column) const;
	void SetModified(int column, bool modified);
	bool IsConst(int column) const;
	int AddColumn(PCWSTR name, int format, int width, ColumnFlags flags = ColumnFlags::Visible);
	const ColumnInfo& GetColumn(int index) const;
	const std::vector<int>& GetColumnsByCategory(PCWSTR category) const;
	const std::vector<CString>& GetCategories() const;

	void UpdateColumns();
	int GetRealColumn(int index) const;

	int GetCount() const {
		return static_cast<int>(m_Columns.size());
	}

protected:
	void SetColumn(int i, const ColumnInfo& info);

private:
	CListViewCtrl m_ListView;
	std::vector<ColumnInfo> m_Columns;
	std::map<CString, std::vector<int>> m_ColumnsByCategory;
	std::vector<CString> m_Categories;
};

inline bool ColumnManager::ColumnInfo::IsVisible() const {
	return (Flags & ColumnFlags::Visible) == ColumnFlags::Visible;
}

inline bool ColumnManager::ColumnInfo::IsMandatory() const {
	return (Flags & ColumnFlags::Mandatory) == ColumnFlags::Mandatory;
}

inline void ColumnManager::ColumnInfo::SetVisible(bool visible) {
	bool old = (Flags & ColumnFlags::Visible) == ColumnFlags::Visible;
	if (old == visible)
		return;

	if (visible)
		Flags |= ColumnFlags::Visible;
	else
		Flags &= ~ColumnFlags::Visible;
	Flags |= ColumnFlags::Modified;
}

