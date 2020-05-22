#pragma once

#include "ColumnManager.h"

enum class ListViewRowCheck {
	None,
	Unchecked,
	Checked
};

template<typename T>
struct CVirtualListView {
	BEGIN_MSG_MAP(CVirtualListView)
		NOTIFY_CODE_HANDLER(LVN_COLUMNCLICK, OnColumnClick)
		NOTIFY_CODE_HANDLER(LVN_ODFINDITEM, OnFindItem)
		NOTIFY_CODE_HANDLER(LVN_GETDISPINFO, OnGetDispInfo)
	ALT_MSG_MAP(1)
		REFLECTED_NOTIFY_CODE_HANDLER(LVN_GETDISPINFO, OnGetDispInfo)
		REFLECTED_NOTIFY_CODE_HANDLER(LVN_COLUMNCLICK, OnColumnClick)
		REFLECTED_NOTIFY_CODE_HANDLER(LVN_ODFINDITEM, OnFindItem)
	END_MSG_MAP()

	struct SortInfo {
		int SortColumn = -1;
		UINT_PTR Id;
		HWND hWnd;
		bool SortAscending;
	private:
		friend struct CVirtualListView;
		int RealSortColumn = -1;
	};

	bool ClearSort(UINT_PTR id = 0) {
		auto si = FindById(id);
		if (si == nullptr)
			return false;

		auto header = CListViewCtrl(si->hWnd).GetHeader();
		HDITEM h;
		h.mask = HDI_FORMAT;
		header.GetItem(si->SortColumn, &h);
		h.fmt = (h.fmt & HDF_JUSTIFYMASK) | HDF_STRING;
		header.SetItem(si->SortColumn, &h);
		si->SortColumn = -1;
		return true;
	}

	bool ClearSort(HWND hWnd) {
		auto si = FindByHwnd(hWnd);
		if (si == nullptr)
			return false;

		auto header = CListViewCtrl(si->hWnd).GetHeader();
		HDITEM h;
		h.mask = HDI_FORMAT;
		header.GetItem(si->RealSortColumn, &h);
		h.fmt = (h.fmt & HDF_JUSTIFYMASK) | HDF_STRING;
		header.SetItem(si->RealSortColumn, &h);
		si->SortColumn = -1;
		return true;
	}

protected:
	ColumnManager* GetExistingColumnManager(HWND hListView) const {
		auto it = std::find_if(m_Columns.begin(), m_Columns.end(), [=](auto& cm) {
			return cm->GetListView() == hListView;
			});
		if (it != m_Columns.end())
			return (*it).get();
		return nullptr;
	}

	ColumnManager* GetColumnManager(HWND hListView) const {
		auto mgr = GetExistingColumnManager(hListView);
		if (mgr)
			return mgr;
		auto cm = std::make_unique<ColumnManager>(hListView);
		auto pcm = cm.get();
		m_Columns.push_back(std::move(cm));
		return pcm;
	}

	int GetRealColumn(HWND hListView, int column) const {
		auto cm = GetExistingColumnManager(hListView);
		return cm ? cm->GetRealColumn(column) : column;
	}

	LRESULT OnGetDispInfo(int /*idCtrl*/, LPNMHDR hdr, BOOL& /*bHandled*/) {
		auto lv = (NMLVDISPINFO*)hdr;
		auto& item = lv->item;
		auto col = GetRealColumn(hdr->hwndFrom, item.iSubItem);
		auto p = static_cast<T*>(this);
		if (item.mask & LVIF_TEXT)
			::StringCchCopy(item.pszText, item.cchTextMax, p->GetColumnText(hdr->hwndFrom, item.iItem, col));
		if (item.mask & LVIF_IMAGE)
			item.iImage = p->GetRowImage(item.iItem);
		if (item.mask & LVIF_INDENT)
			item.iIndent = p->GetRowIndent(item.iItem);
		if (item.mask & LVIF_STATE) {
			item.state = INDEXTOSTATEIMAGEMASK((int)p->IsRowChecked(item.iItem));
			item.stateMask = LVIS_STATEIMAGEMASK;
		}
		return 0;
	}

	LRESULT OnFindItem(int /*idCtrl*/, LPNMHDR hdr, BOOL& /*bHandled*/) {
		auto fi = (NMLVFINDITEM*)hdr;
		auto text = fi->lvfi.psz;
		auto len = ::wcslen(text);
		auto list = fi->hdr.hwndFrom;

		if (ListView_GetSelectedCount(list) == 0)
			return -1;

		int selected = ListView_GetNextItem(list, -1, LVIS_SELECTED);
		int start = selected + 1;
		int count = ListView_GetItemCount(list);
		WCHAR name[256];
		for (int i = start; i < count + start; i++) {
			ListView_GetItemText(list, i % count, 0, name, _countof(name));
			if (::_wcsnicmp(name, text, len) == 0)
				return i % count;
		}
		return -1;
	}

	LRESULT OnColumnClick(int /*idCtrl*/, LPNMHDR hdr, BOOL& /*bHandled*/) {
		auto lv = (NMLISTVIEW*)hdr;
		auto col = GetRealColumn(hdr->hwndFrom, lv->iSubItem);

		auto p = static_cast<T*>(this);
		if (!p->IsSortable(col))
			return 0;

		auto si = FindById(hdr->idFrom);
		if (si == nullptr) {
			SortInfo s;
			s.hWnd = hdr->hwndFrom;
			s.Id = hdr->idFrom;
			m_Controls.push_back(s);
			si = m_Controls.data() + m_Controls.size() - 1;
		}

		auto oldSortColumn = si->SortColumn;
		if (col == si->SortColumn)
			si->SortAscending = !si->SortAscending;
		else {
			si->SortColumn = col;
			si->SortAscending = true;
		}

		CListViewCtrl list(hdr->hwndFrom);
		auto header = list.GetHeader();

		HDITEM h;
		if (si->RealSortColumn >= 0) {
			h.mask = HDI_FORMAT;
			header.GetItem(si->RealSortColumn, &h);
			h.fmt = (h.fmt & HDF_JUSTIFYMASK) | HDF_STRING;
			header.SetItem(si->RealSortColumn, &h);
		}
		si->RealSortColumn = lv->iSubItem;

		h.mask = HDI_FORMAT;
		header.GetItem(lv->iSubItem, &h);
		h.fmt = (h.fmt & HDF_JUSTIFYMASK) | HDF_STRING | (si->SortAscending ? HDF_SORTUP : HDF_SORTDOWN);
		header.SetItem(lv->iSubItem, &h);

		//if (si->RealSortColumn >= 0) {
		//	h.mask = HDI_FORMAT;
		//	header.GetItem(oldSortColumn, &h);
		//	h.fmt = (h.fmt & HDF_JUSTIFYMASK) | HDF_STRING;
		//	header.SetItem(oldSortColumn, &h);
		//}

		static_cast<T*>(this)->DoSort(si);
		list.RedrawItems(list.GetTopIndex(), list.GetTopIndex() + list.GetCountPerPage());

		return 0;
	}

	bool IsSortable(int) const {
		return true;
	}

	int GetSortColumn(HWND hWnd, UINT_PTR id = 0) const {
		auto si = FindById(id);
		return si ? GetRealColumn(hWnd, si->SortColumn) : -1;
	}
	int IsSortAscending(UINT_PTR id) const {
		auto si = FindById(id);
		return si ? si->SortAscending : false;
	}

	//SortInfo* GetSortInfo(UINT_PTR id = 0) {
	//	if (id == 0 && m_Controls.empty())
	//		return nullptr;
	//	return id == 0 ? &m_Controls[0] : FindById(id);
	//}

	SortInfo* GetSortInfo(HWND h = nullptr) {
		if (h == nullptr && m_Controls.empty())
			return nullptr;
		return h == nullptr ? &m_Controls[0] : FindByHwnd(h);
	}

	void DoSort(const SortInfo*) {}
	CString GetColumnText(HWND hWnd, int row, int column) const {
		return L"";
	}
	int GetRowImage(int row) const {
		return 0;
	}

	int GetRowIndent(int row) const {
		return 0;
	}

	ListViewRowCheck IsRowChecked(int row) const {
		return ListViewRowCheck::None;
	}

private:
	SortInfo* FindById(UINT_PTR id) const {
		if (id == 0)
			return m_Controls.empty() ? nullptr : &m_Controls[0];
		for (auto& info : m_Controls)
			if (info.Id == id)
				return &info;
		return nullptr;
	}

	SortInfo* FindByHwnd(HWND h) const {
		if (h == nullptr)
			return m_Controls.empty() ? nullptr : &m_Controls[0];
		for (auto& info : m_Controls)
			if (info.hWnd == h)
				return &info;
		return nullptr;
	}

	mutable std::vector<SortInfo> m_Controls;
	mutable std::vector<std::unique_ptr<ColumnManager>> m_Columns;
};
