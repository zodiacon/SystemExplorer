#include "stdafx.h"
#include "MemoryMapView.h"
#include "DriverHelper.h"
#include "resource.h"
#include "SortHelper.h"
#include <algorithm>
#include <Psapi.h>

CMemoryMapView::CMemoryMapView(DWORD pid) : m_Pid(pid) {
}

CString CMemoryMapView::GetColumnText(HWND h, int row, int column) const {
	auto& item = m_Items[row];
	CString text;

	switch (column) {
		case 0: return StateToString(item.State);
		case 1: text.Format(L"0x%016X", item.BaseAddress); break;
		case 2: text.Format(L"0x%llX", item.RegionSize); break;
		case 3: return item.State != MEM_COMMIT ? L"" : TypeToString(item.Type);
		case 4: return item.State != MEM_COMMIT ? L"" : ProtectionToString(item.Protect);
		case 5: return item.State == MEM_FREE ? L"" : ProtectionToString(item.AllocationProtect);
		case 6: return GetDetails(item);
		case 7: text.Format(L"0x%016X", item.AllocationBase); break;
	}
	return text;
}

int CMemoryMapView::GetRowImage(int row) const {
	switch (m_Items[row].State) {
		case MEM_COMMIT: return 2;
		case MEM_RESERVE: return 1;
	}
	return 0;
}

void CMemoryMapView::DoSort(const SortInfo* si) {
	if (si == nullptr)
		return;

	std::sort(m_Items.begin(), m_Items.end(), [=](auto& i1, auto& i2) {
		return CompareItems(i1, i2, si->SortColumn, si->SortAscending);
		});
}

LRESULT CMemoryMapView::OnCreate(UINT, WPARAM, LPARAM, BOOL&) {
	m_hProcess.reset(DriverHelper::OpenProcess(m_Pid, PROCESS_QUERY_INFORMATION));
	if (!m_hProcess)
		return -1;

	::IsWow64Process(m_hProcess.get(), &m_Is32Bit);

	m_hWndClient = m_List.Create(*this, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | LVS_SINGLESEL | LVS_REPORT | LVS_OWNERDATA | LVS_SHOWSELALWAYS);
	m_List.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_LABELTIP);

	m_List.InsertColumn(0, L"State", 0, 90);
	m_List.InsertColumn(1, L"Address", LVCFMT_RIGHT, 140);
	m_List.InsertColumn(2, L"Size", LVCFMT_RIGHT, 120);
	m_List.InsertColumn(3, L"Type", LVCFMT_LEFT, 70);
	m_List.InsertColumn(4, L"Protection", 0, 140);
	m_List.InsertColumn(5, L"Alloc Protection", 0, 140);
	m_List.InsertColumn(6, L"Details", 0, 400);
//	m_List.InsertColumn(7, L"Base Address", LVCFMT_RIGHT, 140);

	CImageList images;
	images.Create(16, 16, ILC_COLOR32, 4, 0);
	UINT icons[] = { IDI_FREE, IDI_RESERVED, IDI_COMMIT };
	for (auto icon : icons)
		images.AddIcon(AtlLoadIconImage(icon, 0, 16, 16));
	m_List.SetImageList(images, LVSIL_SMALL);

	Refresh();

	return 0;
}

void CMemoryMapView::Refresh() {
	m_Items.clear();
	m_Items.reserve(128);

	BYTE* address = nullptr;
	MemoryItem mi;
	for (;;) {
		if (0 == ::VirtualQueryEx(m_hProcess.get(), address, &mi, sizeof(MEMORY_BASIC_INFORMATION)))
			break;

		m_Items.push_back(mi);
		address += mi.RegionSize;
	}
	DoSort(GetSortInfo(m_List));

	m_List.SetItemCountEx(static_cast<int>(m_Items.size()), LVSICF_NOSCROLL | LVSICF_NOINVALIDATEALL);
}

PCWSTR CMemoryMapView::StateToString(DWORD state) {
	switch (state) {
		case MEM_COMMIT: return L"Committed";
		case MEM_FREE: return L"Free";
		case MEM_RESERVE: return L"Reserved";
	}
	ATLASSERT(false);
	return nullptr;
}

CString CMemoryMapView::ProtectionToString(DWORD protection) {
	static const struct {
		PCWSTR Text;
		DWORD Value;
	} prot[] = {
		{ L"", 0 },
		{ L"Execute", PAGE_EXECUTE },
		{ L"Execute/Read", PAGE_EXECUTE_READ },
		{ L"Execute/Read/Write", PAGE_EXECUTE_READWRITE },
		{ L"WriteCopy", PAGE_WRITECOPY },
		{ L"Execute/WriteCopy", PAGE_EXECUTE_WRITECOPY },
		{ L"No Access", PAGE_NOACCESS },
		{ L"Read", PAGE_READONLY },
		{ L"Read/Write", PAGE_READWRITE },
	};

	CString text = std::find_if(std::begin(prot), std::end(prot), [protection](auto& p) {
		return (p.Value & protection) != 0;
		})->Text;

	static const struct {
		PCWSTR Text;
		DWORD Value;
	} extra[] = {
		{ L"Guard", PAGE_GUARD },
		{ L"No Cache", PAGE_NOCACHE },
		{ L"Write Combine", PAGE_WRITECOMBINE },
		{ L"Targets Invalid", PAGE_TARGETS_INVALID },
		{ L"Targets No Update", PAGE_TARGETS_NO_UPDATE },
	};

	std::for_each(std::begin(extra), std::end(extra), [&text, protection](auto& p) {
		if (p.Value & protection) ((text += L"/") += p.Text);
		});

	return text;
}

PCWSTR CMemoryMapView::TypeToString(DWORD type) {
	switch (type) {
		case MEM_IMAGE: return L"Image";
		case MEM_PRIVATE: return L"Private";
		case MEM_MAPPED: return L"Mapped";
	}
	return L"";
}

bool CMemoryMapView::CompareItems(MemoryItem& m1, MemoryItem& m2, int col, bool asc) {
	switch (col) {
		case 0: return SortHelper::SortNumbers(m1.State, m2.State, asc);
		case 1: return SortHelper::SortNumbers(m1.BaseAddress, m2.BaseAddress, asc);
		case 2: return SortHelper::SortNumbers(m1.RegionSize, m2.RegionSize, asc);
		case 3: return SortHelper::SortNumbers(m1.Type, m2.Type, asc);
		case 4: 
			if (m1.State != MEM_COMMIT)
				return false;
			if (m2.State != MEM_COMMIT)
				return true;
			return SortHelper::SortNumbers(m1.Protect, m2.Protect, asc);
		case 5: 
			if (m1.State == MEM_FREE)
				return false;
			if (m2.State == MEM_FREE)
				return true;
			return SortHelper::SortNumbers(m1.AllocationProtect, m2.AllocationProtect, asc);
		case 6: return SortHelper::SortStrings(GetDetails(m1), GetDetails(m2), asc);
	}
	return false;
}

const CString& CMemoryMapView::GetDetails(const MemoryItem& mi) const {
	if (mi.AttemptDetails)
		return mi.Details;

	mi.AttemptDetails = true;
	if (mi.State != MEM_COMMIT)
		return mi.Details;

	if (mi.Type == MEM_IMAGE) {
		WCHAR filename[MAX_PATH];
		if (::GetMappedFileName(m_hProcess.get(), mi.BaseAddress, filename, MAX_PATH) > 0) {
			return mi.Details = filename;
		}
	}
	return mi.Details;
}
