#pragma once

#include "ColumnManager.h"
#include "ListViewHelper.h"

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
		NOTIFY_CODE_HANDLER(NM_RCLICK, OnRightClick)
		NOTIFY_CODE_HANDLER(NM_DBLCLK, OnDoubleClick)

		//NOTIFY_CODE_HANDLER(LVN_ITEMCHANGED, OnItemChanged)
		ALT_MSG_MAP(1)
		REFLECTED_NOTIFY_CODE_HANDLER(LVN_GETDISPINFO, OnGetDispInfo)
		REFLECTED_NOTIFY_CODE_HANDLER(LVN_COLUMNCLICK, OnColumnClick)
		REFLECTED_NOTIFY_CODE_HANDLER(LVN_ODFINDITEM, OnFindItem)
		REFLECTED_NOTIFY_CODE_HANDLER(NM_RCLICK, OnRightClick)
		REFLECTED_NOTIFY_CODE_HANDLER(NM_DBLCLK, OnDoubleClick)
		//REFLECTED_NOTIFY_CODE_HANDLER(LVN_ITEMCHANGED, OnItemChanged)
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

	LRESULT OnDoubleClick(int, LPNMHDR hdr, BOOL& handled) {
		CListViewCtrl lv(hdr->hwndFrom);
		POINT pt;
		::GetCursorPos(&pt);
		POINT pt2(pt);
		lv.ScreenToClient(&pt);
		LVHITTESTINFO info{};
		info.pt = pt;
		lv.SubItemHitTest(&info);
		auto pT = static_cast<T*>(this);
		handled = pT->OnDoubleClickList(info.iItem, info.iSubItem, pt2);
		return 0;
	}

	LRESULT OnRightClick(int, LPNMHDR hdr, BOOL& handled) {
		WCHAR className[16];
		if (!::GetClassName(hdr->hwndFrom, className, _countof(className))) {
			handled = FALSE;
			return 0;
		}
		if (::wcscmp(className, WC_LISTVIEW)) {
			handled = FALSE;
			return 0;
		}
		CListViewCtrl lv(hdr->hwndFrom);
		POINT pt;
		::GetCursorPos(&pt);
		POINT pt2(pt);
		auto header = lv.GetHeader();
		ATLASSERT(header);
		header.ScreenToClient(&pt);
		HDHITTESTINFO hti;
		hti.pt = pt;
		auto pT = static_cast<T*>(this);
		int index = header.HitTest(&hti);
		if (index >= 0) {
			handled = pT->OnRightClickHeader(index, pt2);
		}
		else {
			LVHITTESTINFO info{};
			info.pt = pt;
			lv.SubItemHitTest(&info);
			handled = pT->OnRightClickList(info.iItem, info.iSubItem, pt2);
		}
		return 0;
	}

	bool OnRightClickHeader(int index, POINT& pt) {
		return false;
	}

	bool OnRightClickList(int row, int col, POINT& pt) {
		return false;
	}

	bool OnDoubleClickList(int row, int col, POINT& pt) {
		return false;
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
		if (item.mask & LVIF_IMAGE) {
			item.iImage = p->GetRowImage(hdr->hwndFrom, item.iItem);
		}
		if (item.mask & LVIF_INDENT)
			item.iIndent = p->GetRowIndent(item.iItem);
		if ((ListView_GetExtendedListViewStyle(hdr->hwndFrom) & LVS_EX_CHECKBOXES) && item.iSubItem == 0 && (item.mask & LVIF_STATE)) {
			item.state = INDEXTOSTATEIMAGEMASK((int)p->IsRowChecked(item.iItem));
			item.stateMask = LVIS_STATEIMAGEMASK;

			if (item.iItem == m_Selected) {
				item.state |= LVIS_SELECTED;
				item.stateMask |= LVIS_SELECTED;
			}
		}
		return 0;
	}

	int m_Selected = -1;

	LRESULT OnItemChanged(int /*idCtrl*/, LPNMHDR hdr, BOOL& bHandled) {
		auto lv = (NMLISTVIEW*)hdr;
		if (lv->uChanged & LVIF_STATE) {
			if (lv->uNewState & LVIS_SELECTED)
				m_Selected = lv->iItem;
		}
		return 0;
	}

	LRESULT OnFindItem(int /*idCtrl*/, LPNMHDR hdr, BOOL& /*bHandled*/) {
		auto fi = (NMLVFINDITEM*)hdr;
		auto text = fi->lvfi.psz;
		auto len = ::wcslen(text);
		auto list = fi->hdr.hwndFrom;

		int selected = fi->iStart;
		int start = selected + 1;
		int count = ListView_GetItemCount(list);
		WCHAR name[128]{};
		if (len >= _countof(name))
			len = _countof(name) - 1;
		int end = (fi->lvfi.flags & LVFI_WRAP) ? (count + start) : count;
		bool partial = fi->lvfi.flags & (LVFI_PARTIAL | LVFI_SUBSTRING);
		for (int i = start; i < end; i++) {
			ListView_GetItemText(list, i % count, 0, name, _countof(name));
			if (partial) {
				if (::_wcsnicmp(name, text, len) == 0)
					return i % count;
			}
			else {
				if (::_wcsicmp(name, text) == 0)
					return i % count;
			}
		}
		return -1;
	}

	void Sort(SortInfo const* si, bool ensureVisible = false) {
		ATLASSERT(si);

		CListViewCtrl list(si->hWnd);
		auto header = list.GetHeader();

		auto selected = list.GetSelectedIndex();
		int count = header.GetItemCount();
		std::vector<CString> selectedText;
		if (selected >= 0) {
			//list.SetItemState(selected, LVIS_SELECTED, 0);
			CString text;
			for (int i = 0; i < count; i++) {
				list.GetItemText(selected, i, text);
				selectedText.push_back(text);
			}
		}
		static_cast<T*>(this)->DoSort(si);
		if (selected >= 0) {
			selected = -1;
			int start = -1, i, n;
			CString text;
			while ((n = list.FindItem(selectedText[0], false, false, start)) >= 0) {
				for (i = 1; i < count; i++) {
					if (list.GetItemText(n, i, text) && text != selectedText[i]) {
						break;
					}
				}
				if (i == count) {
					selected = n;
					break;
				}
				start = n;
			}

		}
		if (selected >= 0) {
			if (ensureVisible) {
				list.EnsureVisible(selected, FALSE);
				//list.SetSelectionMark(selected);
			}
			//list.SetItemState(-1, 0, LVIS_SELECTED);
			list.SetItemState(selected, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		}
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
		CListViewCtrl list(hdr->hwndFrom);
		auto header = list.GetHeader();

		auto oldSortColumn = si->SortColumn;
		if (col == si->SortColumn)
			si->SortAscending = !si->SortAscending;
		else {
			si->SortColumn = col;
			si->SortAscending = true;
		}

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

		Sort(si);
		list.RedrawItems(list.GetTopIndex(), list.GetTopIndex() + list.GetCountPerPage());

		return 0;
	}

	void Sort(CListViewCtrl list, bool redraw = false) {
		auto si = GetSortInfo(list);
		Sort(si, false);
		if (redraw)
			list.RedrawItems(list.GetTopIndex(), list.GetTopIndex() + list.GetCountPerPage());
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

	int GetRowImage(HWND hWnd, int row) const {
		return -1;
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
