#include "pch.h"
#include "DeviceManagerView.h"
#include "resource.h"
#include "ImageHelper.h"
#include <algorithm>
#include "SortHelper.h"

using namespace WinSys;

CDeviceManagerView::CDeviceManagerView(IMainFrame* frame) : CViewBase(frame) {
}

bool CDeviceManagerView::IsSortable(int col) const {
	return col == 0;
}

void CDeviceManagerView::DoSort(const SortInfo* si) {
	ATLASSERT(si->SortColumn == 0);

	if (m_SelectedDevice) {
		std::sort(m_Items.begin(), m_Items.end(), [&](const auto& i1, const auto& i2) {
			return CompareItems(i1.Name, i2.Name, si->SortAscending);
			});
	}
	else {
		std::sort(m_ClassItems.begin(), m_ClassItems.end(), [&](const auto& i1, const auto& i2) {
			return CompareItems(i1.Name, i2.Name, si->SortAscending);
			});
	}
}

LRESULT CDeviceManagerView::OnCreate(UINT, WPARAM, LPARAM, BOOL&) {
	auto hWndToolBar = m_Toolbar.Create(m_hWnd, nullptr, nullptr, ATL_SIMPLE_TOOLBAR_PANE_STYLE | TBSTYLE_LIST, 0, ATL_IDW_TOOLBAR);
	m_Toolbar.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS);
	//InitTreeToolbar(m_Toolbar);

	CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
	AddSimpleReBarBand(m_Toolbar);

	m_hWndClient = m_Splitter.Create(m_hWnd, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, WS_EX_CLIENTEDGE);

	m_List.Create(m_Splitter, rcDefault, nullptr, WS_CHILD | WS_VISIBLE |
		WS_CLIPCHILDREN | WS_CLIPSIBLINGS | LVS_REPORT | LVS_OWNERDATA | LVS_SINGLESEL);
	m_Tree.Create(m_Splitter, rcDefault, nullptr, WS_CHILD | WS_VISIBLE |
		WS_CLIPCHILDREN | WS_CLIPSIBLINGS | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_HASLINES | TVS_SHOWSELALWAYS);

	m_List.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_LABELTIP);
	m_List.InsertColumn(0, L"Property", LVCFMT_LEFT, 170);
	m_List.InsertColumn(1, L"Value", LVCFMT_LEFT, 350);
	m_List.InsertColumn(2, L"Details", LVCFMT_LEFT, 500);

	m_Splitter.SetSplitterExtendedStyle(SPLIT_FLATBAR | SPLIT_PROPORTIONAL);
	m_Splitter.SetSplitterPanes(m_Tree, m_List);
	m_Splitter.SetSplitterPosPct(25);

	m_DevMgr = DeviceManager::Create();
	if (m_DevMgr == nullptr) {
		AtlMessageBox(nullptr, L"Failed to create device manager", IDS_TITLE, MB_ICONERROR);
		return -1;
	}

	Refresh();

	return 0;
}

LRESULT CDeviceManagerView::OnTreeSelectionChanged(int, LPNMHDR, BOOL&) {
	UpdateList();
	return 0;
}

LRESULT CDeviceManagerView::OnListGetDispInfo(int, LPNMHDR hdr, BOOL&) {
	auto lv = (NMLVDISPINFO*)hdr;
	auto& item = lv->item;
	if (m_SelectedClass)
		return GetDeviceClassInfo(item);
	return GetDeviceInfo(item);
}

LRESULT CDeviceManagerView::OnRefresh(WORD, WORD, HWND, BOOL&) {
	return LRESULT();
}

LRESULT CDeviceManagerView::OnFind(WORD, WORD, HWND, BOOL&) {
	return LRESULT();
}

LRESULT CDeviceManagerView::OnFindNext(WORD, WORD, HWND, BOOL&) {
	return LRESULT();
}

void CDeviceManagerView::Refresh() {
	m_Tree.LockWindowUpdate(TRUE);
	m_Tree.DeleteAllItems();

	CImageList images = DeviceManager::GetClassImageList();
	m_ComputerIcon = images.AddIcon(ImageHelper::GetSystemIcon(SIID_DESKTOPPC));
	auto old = m_Tree.SetImageList(images, TVSIL_NORMAL);
	if (old)
		old.Destroy();

	m_Devices = m_DevMgr->EnumDevices();
	std::unordered_map<GUID, HTREEITEM> devClasses;
	devClasses.reserve(32);
	m_DeviceClasses.clear();
	m_DeviceClasses.reserve(32);

	auto root = m_Tree.InsertItem(L"This PC", m_ComputerIcon, m_ComputerIcon, TVI_ROOT, TVI_LAST);

	for (auto& di : m_Devices) {
		auto& guid = di.Data.ClassGuid;
		if (guid == GUID_NULL)
			continue;
		auto it = devClasses.find(guid);
		if (it == devClasses.end()) {
			// add new device class
			auto name = DeviceManager::GetDeviceClassDescription(&guid);
			int image = DeviceManager::GetClassImageIndex(&guid);
			auto hClass = m_Tree.InsertItem(name.c_str(), image, image, root, TVI_LAST);
			m_DeviceClasses.insert({ hClass, guid });
			devClasses.insert({ guid, hClass });
		}
		auto hClass = devClasses[guid];
		int image;
		auto hIcon = m_DevMgr->GetDeviceIcon(di);
		if (hIcon)
			image = m_Tree.GetImageList().AddIcon(hIcon);
		else
			m_Tree.GetItemImage(hClass, image, image);
		auto hItem = m_Tree.InsertItem(di.Description.c_str(), image, image, hClass, TVI_LAST);
		m_Tree.SortChildren(hClass);
		m_Tree.SetItemData(hItem, (DWORD_PTR)&di);
	}

	root.Expand(TVE_EXPAND);
	root.SortChildren();

	m_Tree.LockWindowUpdate(FALSE);
}

void CDeviceManagerView::UpdateList() {
	auto hSelected = m_Tree.GetSelectedItem();
	m_List.SetItemCount(0);

	if (hSelected == nullptr)
		return;

	// is it a class node
	auto it = m_DeviceClasses.find(hSelected);
	auto data = m_Tree.GetItemData(hSelected);
	if (it == m_DeviceClasses.end()) {
		if (data == 0)
			return;

		if (m_Items.empty()) {
			std::vector<ItemData> items{
				{ L"Class", ItemType::String, DeviceRegistryPropertyType::Class },
				{ L"Class GUID", ItemType::String, DeviceRegistryPropertyType::ClassGuid },
				{ L"Description", ItemType::String, DeviceRegistryPropertyType::Description },
				{ L"Friendly Name", ItemType::String, DeviceRegistryPropertyType::FriendlyName },
				{ L"Hardware IDs", ItemType::MultiString, DeviceRegistryPropertyType::HardwareId },
				{ L"Compatible IDs", ItemType::MultiString, DeviceRegistryPropertyType::CompatibleIds },
				{ L"Capabilities", ItemType::Dword, DeviceRegistryPropertyType::Capabilities },
				{ L"Manufacturer", ItemType::String, DeviceRegistryPropertyType::Mfg },
				{ L"Service", ItemType::String, DeviceRegistryPropertyType::Service },
				{ L"Driver", ItemType::String, DeviceRegistryPropertyType::Driver },
				{ L"Enumerator", ItemType::String, DeviceRegistryPropertyType::Enumerator },
				{ L"Location", ItemType::String, DeviceRegistryPropertyType::Location },
				{ L"Characteristics", ItemType::Dword, DeviceRegistryPropertyType::Characteristics },
				{ L"Device Type", ItemType::Dword, DeviceRegistryPropertyType::DeviceType },
				{ L"Exclusive", ItemType::Boolean, DeviceRegistryPropertyType::Exclusive },
				{ L"Device Name", ItemType::String, DeviceRegistryPropertyType::PdoName },
				{ L"Location Paths", ItemType::MultiString, DeviceRegistryPropertyType::LocationPaths },
				{ L"Lower Filters", ItemType::MultiString, DeviceRegistryPropertyType::LowerFilters },
				{ L"Upper Filters", ItemType::MultiString, DeviceRegistryPropertyType::UpperFilters },
				{ L"Removal Policy", ItemType::Dword, DeviceRegistryPropertyType::RemovalPolicy },
				{ L"Removal Policy (Default)", ItemType::Dword, DeviceRegistryPropertyType::RemovalPolicyHwDefault },
				{ L"Removal Policy (Override)", ItemType::Dword, DeviceRegistryPropertyType::RemovalPolicyOverride },
				{ L"Security Descriptor", ItemType::String, DeviceRegistryPropertyType::SecurityDescriptorString },
				{ L"UI Number", ItemType::Dword, DeviceRegistryPropertyType::UINumber },
				{ L"Install State", ItemType::Dword, DeviceRegistryPropertyType::InstallState },
				{ L"Legacy Bus Type", ItemType::Dword, DeviceRegistryPropertyType::LegacyBusType },
			};
			m_Items.swap(items);
		}
		m_SelectedClass = nullptr;
		m_SelectedDevice = (DeviceInfo*)data;
		for (int i = 0; i < m_Items.size(); i++) {
			m_Items[i].Value = GetDeviceProperty(*m_SelectedDevice, i);
		}

		m_List.SetItemCount(static_cast<int>(m_Items.size()));
	}
	else {
		if (m_ClassItems.empty()) {
			std::vector<ClassItemData> items{
				{ L"Class GUID", ItemType::Guid },
				{ L"Lower Filters", ItemType::MultiString, DeviceClassRegistryPropertyType::LowerFilters },
				{ L"Upper Filters", ItemType::MultiString, DeviceClassRegistryPropertyType::UpperFilters },
				{ L"Exclusive", ItemType::Boolean, DeviceClassRegistryPropertyType::Exclusive },
				{ L"Characteristics", ItemType::Dword, DeviceClassRegistryPropertyType::Characteristics },
				{ L"Security Descriptor", ItemType::String, DeviceClassRegistryPropertyType::SecurityDescriptionString },
			};
			m_ClassItems.swap(items);
		}

		m_SelectedClass = &it->second;
		m_SelectedDevice = nullptr;
		for (int i = 0; i < m_ClassItems.size(); i++) {
			m_ClassItems[i].Value = GetDeviceClassProperty(m_SelectedClass, i);
		}
		m_List.SetItemCount(static_cast<int>(m_ClassItems.size()));
	}
	auto si = GetSortInfo(m_List);
	if (si)
		DoSort(si);
}

LRESULT CDeviceManagerView::GetDeviceClassInfo(LVITEM& item) const {
	if (item.mask & LVIF_TEXT) {
		auto index = item.iItem;
		switch (item.iSubItem) {
			case 0:	
				item.pszText = (PWSTR)m_ClassItems[index].Name;
				break;

			case 1:
				::StringCchCopy(item.pszText, item.cchTextMax, GetDeviceClassProperty(m_SelectedClass, index));
				break;

		}
	}
	return 0;
}

CString CDeviceManagerView::GetDeviceClassProperty(const GUID* guid, int index) const {
	ATLASSERT(m_SelectedClass);

	CString result;
	auto& prop = m_ClassItems[index];
	switch (prop.Type) {
		case ItemType::Guid:		// GUID
			::StringFromGUID2(*guid, result.GetBufferSetLength(64), 64);
			break;

		case ItemType::MultiString:	
			for (auto& str : DeviceManager::GetDeviceClassRegistryPropertyMultiString(guid, prop.PropertyType))
				result += str.c_str() + CString(L", ");
			if (!result.IsEmpty())
				result = result.Left(result.GetLength() - 2);
			break;

		case ItemType::Dword:
			result.Format(L"0x%08X", DeviceManager::GetDeviceClassRegistryProperty<DWORD>(guid, prop.PropertyType));
			break;

		case ItemType::Boolean:
			result = DeviceManager::GetDeviceClassRegistryProperty<DWORD>(guid, prop.PropertyType) ? L"Yes" : L"No";
			break;

		case ItemType::String:
			result = DeviceManager::GetDeviceClassRegistryPropertyString(guid, prop.PropertyType).c_str();
			break;
	}
	return result;
}

LRESULT CDeviceManagerView::GetDeviceInfo(LVITEM& item) const {
	ATLASSERT(m_SelectedDevice);
	if (item.mask & LVIF_TEXT) {
		auto index = item.iItem;
		switch (item.iSubItem) {
			case 0:
				item.pszText = (PWSTR)m_Items[index].Name;
				break;

			case 1:
				::StringCchCopy(item.pszText, item.cchTextMax, m_Items[index].Value);
				break;

		}
	}
	return 0;
}

CString CDeviceManagerView::GetDeviceProperty(DeviceInfo& di, int index) const {

	CString result;
	auto& prop = m_Items[index];
	switch (prop.Type) {
		case ItemType::String:
			result = m_DevMgr->GetDeviceRegistryPropertyString(di, prop.PropertyType).c_str();
			break;

		case ItemType::MultiString:
			for (auto& str : m_DevMgr->GetDeviceRegistryPropertyMultiString(di, prop.PropertyType))
				result += str.c_str() + CString(L", ");
			if (!result.IsEmpty())
				result = result.Left(result.GetLength() - 2);
			break;

		case ItemType::Dword:
			result.Format(L"%08X", m_DevMgr->GetDeviceRegistryProperty<DWORD>(di, prop.PropertyType));
			break;

		case ItemType::Boolean:
			result = m_DevMgr->GetDeviceRegistryProperty<DWORD>(di, prop.PropertyType) ? L"Yes" : L"No";
			break;

	}
	return result;
}

bool CDeviceManagerView::CompareItems(const CString& i1, const CString& i2, bool asc) {
	return SortHelper::SortStrings(i1, i2, asc);
}
