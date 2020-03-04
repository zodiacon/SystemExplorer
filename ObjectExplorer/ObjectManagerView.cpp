#include "stdafx.h"
#include "ObjectManagerView.h"
#include "ObjectManager.h"
#include "resource.h"
#include <algorithm>
#include "SortHelper.h"
#include "NtDll.h"

CObjectManagerView::CObjectManagerView(IMainFrame* frame) : m_pFrame(frame) {
}

CString CObjectManagerView::GetDirectoryPath() const {
	auto item = m_Tree.GetSelectedItem();
	CString path;
	while(item.GetParent()) {
		CString text;
		item.GetText(text);
		path = L"\\" + text + path;
		item = item.GetParent();
	}

	if (path.IsEmpty())
		path = L"\\";

	return path;
}

void CObjectManagerView::OnFinalMessage(HWND) {
	delete this;
}

void CObjectManagerView::DoSort(const SortInfo* si) {
	std::sort(m_Objects.begin(), m_Objects.end(), [=](const auto& data1, const auto& data2) {
		return CompareItems(data1, data2, si->SortColumn, si->SortAscending);
		});
}

bool CObjectManagerView::IsSortable(int column) const {
	return column != 2;
}

LRESULT CObjectManagerView::OnCreate(UINT, WPARAM, LPARAM, BOOL&) {
	m_Splitter.Create(*this, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
	m_List.Create(m_Splitter, rcDefault, nullptr, WS_CHILD | WS_VISIBLE |  
		WS_CLIPCHILDREN | WS_CLIPSIBLINGS | LVS_REPORT | LVS_OWNERDATA | LVS_SINGLESEL);
	m_Tree.Create(m_Splitter, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | 
		WS_CLIPCHILDREN | WS_CLIPSIBLINGS | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_HASLINES);
	m_List.SetImageList(m_pFrame->GetImageList(), LVSIL_SMALL);
	CImageList images;
	images.Create(16, 16, ILC_COLOR | ILC_COLOR32, 2, 0);
	images.AddIcon(AtlLoadIcon(IDI_FOLDER));
	images.AddIcon(AtlLoadIcon(IDI_FOLDER_CLOSED));
	m_Tree.SetImageList(images, TVSIL_NORMAL);

	m_List.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
	m_List.InsertColumn(0, L"Name", LVCFMT_LEFT, 300);
	m_List.InsertColumn(1, L"Type", LVCFMT_LEFT, 150);
	m_List.InsertColumn(2, L"Details", LVCFMT_LEFT, 500);

	m_Splitter.SetSplitterExtendedStyle(SPLIT_FLATBAR | SPLIT_PROPORTIONAL);
	m_Splitter.SetSplitterPanes(m_Tree, m_List);
	m_Splitter.SetSplitterPosPct(20);
	m_Splitter.UpdateSplitterLayout();

	InitTree();

	return 0;
}

LRESULT CObjectManagerView::OnSize(UINT, WPARAM, LPARAM lParam, BOOL&) {
	auto x = GET_X_LPARAM(lParam), y = GET_Y_LPARAM(lParam);
	if (m_Splitter)
		m_Splitter.MoveWindow(0, 0, x, y);
	return 0;
}

LRESULT CObjectManagerView::OnTreeSelectionChanged(int, LPNMHDR, BOOL&) {
	auto item = m_Tree.GetSelectedItem();
	ATLASSERT(item);
	UpdateList(true);
	return 0;
}

LRESULT CObjectManagerView::OnListGetDispInfo(int, LPNMHDR hdr, BOOL&) {
	auto lv = (NMLVDISPINFO*)hdr;
	auto& item = lv->item;
	auto& data = m_Objects[item.iItem];

	if (item.mask & LVIF_TEXT) {
		switch (item.iSubItem) {
		case 0:		// name
			item.pszText = (PWSTR)(PCWSTR)data.Name;
			break;

		case 1:		// type
			item.pszText = (PWSTR)(PCWSTR)data.Type;
			break;

		case 2:		// details
			auto type = ObjectManager::GetType(data.Type);
			if (type && type->TypeDetails) {
				HANDLE hObject;
				auto status = ObjectManager::OpenObject(
					data.FullName, data.Type, &hObject,
					data.Type == L"File" ? FILE_READ_ATTRIBUTES : GENERIC_READ);
				if (hObject) {
					::StringCchCopy(item.pszText, item.cchTextMax, type->TypeDetails->GetDetails(hObject));
					::CloseHandle(hObject);
				}
				else if (status == STATUS_ACCESS_DENIED)
					item.pszText = L"<access denied>";
				else if (status == STATUS_UNSUCCESSFUL)
					item.pszText = L"<unavailable>";
			}
			break;
		}
	}
	if (item.mask & LVIF_IMAGE) {
		item.iImage = m_pFrame->GetIconIndexByType(data.Type);
	}
	return 0;
}

LRESULT CObjectManagerView::OnRefresh(WORD, WORD, HWND, BOOL&) {
	UpdateList(false);
	return 0;
}

void CObjectManagerView::InitTree() {
	m_Tree.LockWindowUpdate();
	m_Tree.DeleteAllItems();

	auto root = m_Tree.InsertItem(L"\\", 1, 0, TVI_ROOT, TVI_LAST);
	EnumDirectory(root, L"\\");
	root.SortChildren(TRUE);
	root.Expand(TVE_EXPAND);
	root.Select(TVGN_CARET);

	m_Tree.LockWindowUpdate(FALSE);
}

void CObjectManagerView::UpdateList(bool newNode) {
	auto path = GetDirectoryPath();
	m_Objects.clear();
	m_Objects.reserve(128);
	for (auto& item : ObjectManager::EnumDirectoryObjects(path)) {
		if (item.TypeName != L"Directory") {
			ObjectData data;
			data.Name = item.Name.c_str();
			data.Type = item.TypeName.c_str();
			data.FullName = path.Right(1) == L"\\" ? path + data.Name : path + L"\\" + data.Name;
			m_Objects.push_back(std::move(data));
		}
	}
	if (newNode) {
		m_List.SetItemCount(static_cast<int>(m_Objects.size()));
		ClearSort();
	}
	else {
		auto si = GetSortInfo(0);
		if (si) {
			if (si->SortColumn >= 0)
				DoSort(si);
		}
		m_List.SetItemCountEx(static_cast<int>(m_Objects.size()), LVSICF_NOINVALIDATEALL | LVSICF_NOSCROLL);
		m_List.RedrawItems(m_List.GetTopIndex(), m_List.GetCountPerPage() + m_List.GetTopIndex());
	}
}

void CObjectManagerView::EnumDirectory(CTreeItem root, const CString& path) {
	for (auto& dir : ObjectManager::EnumDirectoryObjects(path)) {
		if (dir.TypeName == L"Directory") {
			auto node = m_Tree.InsertItem(dir.Name.c_str(), 1, 0, root, TVI_LAST);
			EnumDirectory(node, path.Right(1) == L"\\" ? path + dir.Name.c_str() : path + L"\\" + dir.Name.c_str());
		}
	}
}

bool CObjectManagerView::CompareItems(const ObjectData& data1, const ObjectData& data2, int col, bool asc) {
	switch (col) {
		case 0: return SortHelper::SortStrings(data1.Name, data2.Name, asc);
		case 1: return SortHelper::SortStrings(data1.Type, data2.Type, asc);
	}
	return false;
}
