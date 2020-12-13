#include "pch.h"
#include "ObjectManagerView.h"
#include "ObjectManager.h"
#include "resource.h"
#include <algorithm>
#include "SortHelper.h"
#include "NtDll.h"
#include "ObjectType.h"
#include "ObjectTypeFactory.h"
#include "SecurityInfo.h"
#include "ClipboardHelper.h"

CObjectManagerView::CObjectManagerView(IMainFrame* frame) : CViewBase(frame) {
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

CString CObjectManagerView::GetColumnText(HWND, int row, int col) {
	auto& data = m_Objects[row];
	CString text;
	switch (col) {
		case 0:	return data.Name;
		case 1:	return data.Type;
		case 2:		// details
			auto type = ObjectManager::GetType(data.Type);
			if (type) {
				if (type) {
					HANDLE hObject;
					auto status = ObjectManager::OpenObject(
						data.FullName, data.Type, &hObject,
						data.Type == L"File" ? FILE_READ_ATTRIBUTES : GENERIC_READ);
					if (hObject) {
						auto details = ObjectTypeFactory::CreateObjectType(type->TypeIndex, type->TypeName);
						if (details) {
							text = details->GetDetails(hObject);
							::CloseHandle(hObject);
							return text;
						}
					}
					if (status == STATUS_ACCESS_DENIED)
						text = L"<access denied>";
					else
						text = L"<unavailable>";
				}
			}
			break;
	}
	return text;
}

int CObjectManagerView::GetRowImage(HWND, int row) const {
	return GetFrame()->GetIconIndexByType(m_Objects[row].Type);
}

void CObjectManagerView::DoFind(const CString& text, DWORD flags) {
	auto down = flags & FR_DOWN;
	auto dir = down ? 1 : -1;
	auto index = m_List.GetSelectedIndex() + dir;
	auto count = m_List.GetItemCount();
	if (index < 0)
		index = 0;
	else if (index >= count)
		index = count - 1;

	auto matchCase = flags & FR_MATCHCASE;
	CString search(text);
	if (!matchCase)
		search.MakeLower();

	auto start = index;
	do {
		CString value(m_Objects[start].Name);
		if (!matchCase)
			value.MakeLower();
		if (value.Find(search) >= 0)
			break;
		start += dir + count;
		start %= count;
	} while (start != index);

	m_List.SelectItem(start);
	m_List.SetFocus();
}

LRESULT CObjectManagerView::OnCreate(UINT, WPARAM, LPARAM, BOOL&) {
	m_hWndClient = m_Splitter.Create(*this, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
	m_List.Create(m_Splitter, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | 
		WS_CLIPCHILDREN | WS_CLIPSIBLINGS | LVS_REPORT | LVS_OWNERDATA | LVS_SINGLESEL | LVS_SHAREIMAGELISTS | LVS_SHOWSELALWAYS);
	m_Tree.Create(m_Splitter, rcDefault, nullptr, WS_CHILD | WS_VISIBLE |  
		WS_CLIPCHILDREN | WS_CLIPSIBLINGS | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_HASLINES | TVS_SHOWSELALWAYS);
	m_List.SetImageList(GetFrame()->GetImageList(), LVSIL_SMALL);
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

	ObjectManager::EnumTypes();

	InitTree();

	return 0;
}

LRESULT CObjectManagerView::OnTreeSelectionChanged(int, LPNMHDR, BOOL&) {
	auto item = m_Tree.GetSelectedItem();
	ATLASSERT(item);
	UpdateList(true);
	return 0;
}

LRESULT CObjectManagerView::OnRefresh(WORD, WORD, HWND, BOOL&) {
	InitTree();
	return 0;
}

LRESULT CObjectManagerView::OnEditSecurity(WORD, WORD, HWND, BOOL&) {
	auto index = m_List.GetSelectedIndex();
	ATLASSERT(index >= 0);
	auto& item = m_Objects[index];

	return 0;
}

LRESULT CObjectManagerView::OnEditCopy(WORD, WORD, HWND, BOOL&) {
	auto hFocus = ::GetFocus();
	if (m_List == hFocus) {
		auto index = m_List.GetSelectedIndex();
		if (index >= 0)
			ClipboardHelper::CopyText(*this, m_Objects[index].Name);
	}
	else if (m_Tree == hFocus) {
		auto hSelected = m_Tree.GetSelectedItem();
		if (hSelected) {
			CString text;
			hSelected.GetText(text);
			ClipboardHelper::CopyText(*this, text);
		}
	}
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
		auto si = GetSortInfo(m_List);
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
