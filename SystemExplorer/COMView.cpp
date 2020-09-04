#include "pch.h"
#include "COMView.h"
#include "SortHelper.h"
#include <algorithm>
#include "resource.h"

using namespace WinSys;

CComView::CComView(IMainFrame* frame) : CViewBase(frame) {
}

DWORD CComView::OnPrePaint(int, LPNMCUSTOMDRAW cd) {
	m_hFont = (HFONT)::GetCurrentObject(cd->hdc, OBJ_FONT);

	return CDRF_NOTIFYITEMDRAW;
}

DWORD CComView::OnItemPrePaint(int, LPNMCUSTOMDRAW) {
	return CDRF_NOTIFYSUBITEMDRAW;
}

DWORD CComView::OnSubItemPrePaint(int, LPNMCUSTOMDRAW cd) {
	auto lcd = (LPNMLVCUSTOMDRAW)cd;
	auto sub = lcd->iSubItem;
	int index = (int)cd->dwItemSpec;

	auto cm = GetColumnManager(m_List);
	auto real = cm->GetRealColumn(sub);
	if ((cm->GetColumn((int)real).Flags & ColumnFlags::Numeric) == ColumnFlags::Numeric)
		::SelectObject(cd->hdc, GetFrame()->GetMonoFont());
	else
		::SelectObject(cd->hdc, m_hFont);

	return CDRF_DODEFAULT | CDRF_NEWFONT;
}

CString CComView::GetColumnText(HWND, int row, int col) const {
	auto f = m_GetColumnTextFunctions[m_SelectedNode.GetData()];
	if (f)
		return f(row, col);

	return L"";
}

int CComView::GetRowImage(int row) const {
	if (m_SelectedNode.GetData() == (DWORD_PTR)NodeType::Classes)
		return (int)m_Classes[row].ServerType;

	return 0;
}

void CComView::DoSort(const SortInfo* si) {
	if (si == nullptr)
		return;

	auto sorter = m_Sorter[m_SelectedNode.GetData()];
	if (sorter)
		sorter(si);
}

LRESULT CComView::OnCreate(UINT, WPARAM, LPARAM, BOOL&) {
	m_hWndClient = m_Splitter.Create(*this, rcDefault, nullptr,
		WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);

	m_List.Create(m_Splitter, rcDefault, nullptr, ListViewDefaultStyle & ~LVS_SHAREIMAGELISTS);
	m_List.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_LABELTIP | LVS_EX_HEADERDRAGDROP);
	CImageList images;
	images.Create(16, 16, ILC_COLOR32, 4, 4);
	m_List.SetImageList(images, LVSIL_SMALL);

	m_Tree.Create(m_Splitter, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | TVS_LINESATROOT | TVS_HASLINES | TVS_HASBUTTONS | TVS_SHOWSELALWAYS);

	m_Splitter.SetSplitterExtendedStyle(SPLIT_FLATBAR | SPLIT_PROPORTIONAL);

	m_Splitter.SetSplitterPanes(m_Tree, m_List);

	images.Detach();
	images.Create(16, 16, ILC_COLOR32, 4, 4);
	UINT ids[] = { IDI_COMPONENT, IDI_CLASS, IDI_INTERFACE, IDI_APPID, IDI_LIBRARY };
	for (auto id : ids)
		images.AddIcon(AtlLoadIconImage(id));
	m_Tree.SetImageList(images, TVSIL_NORMAL);

	InitTree();

	m_GetColumnTextFunctions[(int)NodeType::Classes] = [this](auto row, auto col) { return GetColumnTextClass(row, col); };
	m_GetColumnTextFunctions[(int)NodeType::Interfaces] = [this](auto row, auto col) { return GetColumnTextInterface(row, col); };
	m_GetColumnTextFunctions[(int)NodeType::Typelibs] = [this](auto row, auto col) { return GetColumnTextTypeLibrary(row, col); };

	m_InitListView[(int)NodeType::Classes] = [this]() { InitListViewClasses(); };
	m_InitListView[(int)NodeType::Interfaces] = [this]() { InitListViewInterfaces(); };
	m_InitListView[(int)NodeType::Typelibs] = [this]() { InitListViewTypeLibraries(); };

	m_Sorter[(int)NodeType::Classes] = [this](auto si) { DoSortClasses(si); };
	m_Sorter[(int)NodeType::Interfaces] = [this](auto si) { DoSortInterfaces(si); };
	m_Sorter[(int)NodeType::Typelibs] = [this](auto si) { DoSortTypeLibraries(si); };

	m_Splitter.SetSplitterPosPct(15);

	return 0;
}

LRESULT CComView::OnSelectedTreeItemChanged(int, LPNMHDR hdr, BOOL&) {
	auto selected = m_Tree.GetSelectedItem();
	if (selected == nullptr)
		return 0;

	UpdateList(selected);

	return 0;
}

void CComView::InitTree() {
	auto item = CreateRootItem(L"Default", ComStore::Default);
	CreateRootItem(L"Machine", ComStore::User);
	CreateRootItem(L"User", ComStore::Machine);
	CreateRootItem(L"Default (32 bit)", ComStore::Default32);
	CreateRootItem(L"Machine (32 bit)", ComStore::User32);
	CreateRootItem(L"User (32 bit)", ComStore::Machine32);
	item.Expand(TVE_EXPAND);
	item.Select();
	UpdateList(item);
}

CTreeItem CComView::CreateRootItem(PCWSTR name, ComStore store) {
	auto root = m_Tree.InsertItem(name, 0, 0, TVI_ROOT, TVI_LAST);
	root.SetData((DWORD_PTR)store);
	CreateStoreItems(root);

	return root;
}

void CComView::CreateStoreItems(CTreeItem root) {
	root.InsertAfter(L"Classes", root, 1).SetData((DWORD_PTR)NodeType::Classes);
	root.InsertAfter(L"Interfaces", root, 2).SetData((DWORD_PTR)NodeType::Interfaces);
	root.InsertAfter(L"Type Libraries", root, 4).SetData((DWORD_PTR)NodeType::Typelibs);
	root.InsertAfter(L"App IDs", root, 3).SetData((DWORD_PTR)NodeType::AppIds);
}

void CComView::UpdateList(CTreeItem item) {
	CWaitCursor wait;

	m_Classes.clear();
	m_Interfaces.clear();

	auto parent = item.GetParent();
	if (!parent) {
		// sub node
		if (m_ComExplorer.Open((ComStore)item.GetData())) {

		}
		m_List.SetItemCount(0);
		GetColumnManager(m_List)->Clear();

		return;
	}

	int count = 0;
	m_SelectedNode = item;
	auto init = m_InitListView[item.GetData()];
	while (m_List.DeleteColumn(0))
		;
	GetColumnManager(m_List)->Clear();
	m_List.GetImageList(LVSIL_SMALL).RemoveAll();
	if (init)
		init();

	switch ((NodeType)item.GetData()) {
		case NodeType::Classes:
			m_Classes = m_ComExplorer.EnumClasses();
			count = static_cast<int>(m_Classes.size());
			break;

		case NodeType::Interfaces:
			m_Interfaces = m_ComExplorer.EnumInterfaces();
			count = static_cast<int>(m_Interfaces.size());
			break;

		case NodeType::Typelibs:
			m_TypeLibs = m_ComExplorer.EnumTypeLibraries();
			count = static_cast<int>(m_TypeLibs.size());
			break;
	}

	m_List.SetItemCount(count);
}

CString CComView::GetColumnTextClass(int row, int col) const {
	auto& data = m_Classes[row];

	CString text;

	switch (col) {
		case 0: return data.FriendlyName.c_str();
		case 1: return ClsidToString(data.Clsid);
		case 2: return ServerTypeToString(data.ServerType);
		case 3: return data.ModulePath.c_str();
		case 4: return data.ThreadingModel.c_str();
		case 5: 
			if (data.AppId != GUID_NULL)
				return ClsidToString(data.AppId);
			break;
	}
	return text;
}

CString CComView::GetColumnTextInterface(int row, int col) const {
	auto& data = m_Interfaces[row];

	CString text;

	switch (col) {
		case 0: return data.FriendlyName.c_str();
		case 1: return ClsidToString(data.Iid);
		case 2: return data.ProxyStub == GUID_NULL ? L"" : ClsidToString(data.ProxyStub);
		case 3: return data.TypeLib == GUID_NULL ? L"" : ClsidToString(data.TypeLib);
	}
	return text;
}

CString CComView::GetColumnTextTypeLibrary(int row, int col) const {
	auto& item = m_TypeLibs[row];
	CString text;

	switch (col) {
		case 0:
			::StringFromGUID2(item.TypeLibId, text.GetBufferSetLength(64), 64); 
			break;
		case 1: return item.Win32Path.c_str();
		case 2: return item.Win64Path.c_str();
	}
	return text;
}

void CComView::InitListViewClasses() {
	auto cm = GetColumnManager(m_List);

	cm->AddColumn(L"Friendly Name", LVCFMT_LEFT, 250);
	cm->AddColumn(L"CLSID", LVCFMT_CENTER, 300, ColumnFlags::Visible | ColumnFlags::Numeric);
	cm->AddColumn(L"Server Type", LVCFMT_LEFT, 80);
	cm->AddColumn(L"Path / Service", LVCFMT_LEFT, 250);
	cm->AddColumn(L"Threading Model", LVCFMT_LEFT, 90);
	cm->AddColumn(L"App ID", LVCFMT_CENTER, 300, ColumnFlags::Visible | ColumnFlags::Numeric);
	cm->UpdateColumns();

	auto images = m_List.GetImageList(LVSIL_SMALL);
	UINT ids[] = { IDI_DLL_SERVER, IDI_APP_SERVER, IDI_SERVICE };
	for(auto id : ids)
		images.AddIcon(AtlLoadIconImage(id, 0, 16, 16));

}

void CComView::InitListViewInterfaces() {
	auto cm = GetColumnManager(m_List);

	cm->AddColumn(L"Friendly Name", LVCFMT_LEFT, 250);
	cm->AddColumn(L"IID", LVCFMT_CENTER, 300, ColumnFlags::Visible | ColumnFlags::Numeric);
	cm->AddColumn(L"Proxy/Stub", LVCFMT_CENTER, 300, ColumnFlags::Visible | ColumnFlags::Numeric);
	cm->AddColumn(L"Type Library", LVCFMT_CENTER, 300, ColumnFlags::Visible | ColumnFlags::Numeric);

	auto images = m_List.GetImageList(LVSIL_SMALL);
	images.AddIcon(AtlLoadIconImage(IDI_INTERFACE, 0, 16, 16));
}

void CComView::InitListViewTypeLibraries() {
	auto cm = GetColumnManager(m_List);

	cm->AddColumn(L"GUID", LVCFMT_LEFT, 300, ColumnFlags::Visible | ColumnFlags::Numeric);
	cm->AddColumn(L"Win32 Path", LVCFMT_LEFT, 300);
	cm->AddColumn(L"Win64 Path", LVCFMT_LEFT, 300);
	cm->UpdateColumns();

	auto images = m_List.GetImageList(LVSIL_SMALL);
	UINT ids[] = { IDI_GENERIC };
	for (auto id : ids)
		images.AddIcon(AtlLoadIconImage(id, 0, 16, 16));
}

void CComView::DoSortClasses(const SortInfo* si) {
	auto asc = si->SortAscending;
	auto col = si->SortColumn;

	std::sort(m_Classes.begin(), m_Classes.end(), [=](auto& c1, auto& c2) {
		switch (col) {
			case 0: return SortHelper::SortStrings(c1.FriendlyName, c2.FriendlyName, asc);
			case 1: return SortHelper::SortStrings(ClsidToString(c1.Clsid), ClsidToString(c2.Clsid), asc);
			case 2: return SortHelper::SortNumbers(c1.ServerType, c2.ServerType, asc);
			case 3: return SortHelper::SortStrings(c1.ModulePath, c2.ModulePath, asc);
			case 4: return SortHelper::SortStrings(c1.ThreadingModel, c2.ThreadingModel, asc);
			case 5: return SortHelper::SortStrings(ClsidToString(c1.AppId), ClsidToString(c2.AppId), asc);
		}
		return false;
		});
}

void CComView::DoSortInterfaces(const SortInfo* si) {
	auto asc = si->SortAscending;
	auto col = si->SortColumn;

	std::sort(m_Interfaces.begin(), m_Interfaces.end(), [=](auto& c1, auto& c2) {
		switch (col) {
			case 0: return SortHelper::SortStrings(c1.FriendlyName, c2.FriendlyName, asc);
			case 1: return SortHelper::SortStrings(ClsidToString(c1.Iid), ClsidToString(c2.Iid), asc);
			case 2: return SortHelper::SortStrings(ClsidToString(c1.ProxyStub), ClsidToString(c2.ProxyStub), asc);
			case 3: return SortHelper::SortStrings(ClsidToString(c1.TypeLib), ClsidToString(c2.TypeLib), asc);
		}
		return false;
		});
}

void CComView::DoSortTypeLibraries(const SortInfo* si) {
	auto asc = si->SortAscending;
	auto col = si->SortColumn;

	std::sort(m_TypeLibs.begin(), m_TypeLibs.end(), [=](auto& t1, auto& t2) {
		switch (col) {
			case 0: return SortHelper::SortStrings(ClsidToString(t1.TypeLibId), ClsidToString(t2.TypeLibId), asc);
			case 1: return SortHelper::SortStrings(t1.Win32Path, t2.Win32Path, asc);
			case 2: return SortHelper::SortStrings(t1.Win64Path, t2.Win64Path, asc);
		}
		return false;
		});
}

CString CComView::ClsidToString(const GUID& guid) {
	WCHAR text[64];
	if (::StringFromGUID2(guid, text, _countof(text)))
		return text;
	return L"";
}

PCWSTR CComView::ServerTypeToString(ComServerType type) {
	switch (type) {
		case ComServerType::InProc: return L"In Proc";
		case ComServerType::OutOfProc: return L"Out of Proc";
		case ComServerType::Service: return L"Service";
	}
	return L"";
}
