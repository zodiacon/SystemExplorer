#include "stdafx.h"
#include "MemoryMapView.h"
#include "DriverHelper.h"
#include "resource.h"
#include "SortHelper.h"
#include <algorithm>
#include <Psapi.h>
#include "ntdll.h"

CMemoryMapView::CMemoryMapView(IMainFrame* frame, DWORD pid) : CViewBase(frame), m_Pid(pid) {
}

CString CMemoryMapView::GetColumnText(HWND h, int row, int column) const {
	auto& item = *m_Items[row];
	CString text;

	switch (column) {
		case 0: return StateToString(item.State);
		case 1: text.Format(L"0x%0p", item.BaseAddress); break;
		case 2: return FormatWithCommas(item.RegionSize >> 10) + L" KB";
		case 3: return item.State != MEM_COMMIT ? L"" : TypeToString(item.Type);
		case 4: return item.State != MEM_COMMIT ? L"" : ProtectionToString(item.Protect);
		case 5: return item.State == MEM_FREE ? L"" : ProtectionToString(item.AllocationProtect);
		case 6: return UsageToString(item);
		case 7: return GetDetails(item).Details;
	}
	return text;
}

int CMemoryMapView::GetRowImage(int row) const {
	switch (m_Items[row]->State) {
		case MEM_COMMIT: return 2;
		case MEM_RESERVE: return 1;
	}
	return 0;
}

int CMemoryMapView::GetRowIndent(int row) const {
	auto& item = *m_Items[row];
	return item.State == MEM_FREE || item.AllocationBase == item.BaseAddress ? 0 : 1;
}

void CMemoryMapView::DoSort(const SortInfo* si) {
	if (si == nullptr)
		return;

	std::sort(m_Items.begin(), m_Items.end(), [=](auto& i1, auto& i2) {
		return CompareItems(*i1, *i2, si->SortColumn, si->SortAscending);
		});
}

DWORD CMemoryMapView::OnPrePaint(int, LPNMCUSTOMDRAW cd) {
	if (cd->hdr.hwndFrom != m_List)
		return CDRF_DODEFAULT;
	return CDRF_NOTIFYITEMDRAW;
}

DWORD CMemoryMapView::OnSubItemPrePaint(int, LPNMCUSTOMDRAW cd) {
	auto lcd = (LPNMLVCUSTOMDRAW)cd;
	auto sub = lcd->iSubItem;
	lcd->clrTextBk = CLR_INVALID;
	int index = (int)cd->dwItemSpec;
	auto& item = m_Items[index];

	lcd->clrTextBk = UsageToBackColor(*item);

	return CDRF_DODEFAULT;
}

DWORD CMemoryMapView::OnItemPrePaint(int, LPNMCUSTOMDRAW) {
	return CDRF_NOTIFYITEMDRAW;
}

LRESULT CMemoryMapView::OnCreate(UINT, WPARAM, LPARAM, BOOL&) {
	m_hProcess = DriverHelper::OpenProcess(m_Pid, PROCESS_QUERY_INFORMATION);
	if (m_hProcess == nullptr)
		return -1;

	m_Tracker.reset(new WinSys::ProcessVMTracker(m_hProcess));
	if (m_Tracker == nullptr)
		return -1;

	m_hReadProcess.reset(DriverHelper::OpenProcess(m_Pid, PROCESS_VM_READ));

	m_hWndClient = m_List.Create(*this, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | LVS_SINGLESEL | LVS_REPORT | LVS_OWNERDATA | LVS_SHOWSELALWAYS);
	m_List.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_LABELTIP);

	m_List.InsertColumn(0, L"State", 0, 110);
	m_List.InsertColumn(1, L"Address", LVCFMT_RIGHT, 140);
	m_List.InsertColumn(2, L"Size", LVCFMT_RIGHT, 120);
	m_List.InsertColumn(3, L"Type", LVCFMT_LEFT, 70);
	m_List.InsertColumn(4, L"Protection", 0, 140);
	m_List.InsertColumn(5, L"Alloc Protection", 0, 140);
	m_List.InsertColumn(6, L"Usage", 0, 90);
	m_List.InsertColumn(7, L"Details", 0, 500);

	CImageList images;
	images.Create(16, 16, ILC_COLOR32, 4, 0);
	UINT icons[] = { IDI_FREE, IDI_RESERVED, IDI_COMMIT };
	for (auto icon : icons)
		images.AddIcon(AtlLoadIconImage(icon, 0, 16, 16));
	m_List.SetImageList(images, LVSIL_SMALL);

	Refresh();

	return 0;
}

LRESULT CMemoryMapView::OnRefresh(WORD, WORD, HWND, BOOL&) {
	CWaitCursor wait;
	Refresh();

	return 0;
}

void CMemoryMapView::Refresh() {
	if (m_Details.empty())
		m_Details.reserve(128);

	m_Tracker->EnumRegions();
	m_Items = m_Tracker->GetRegions();
	m_Details.clear();
	m_Details.reserve(m_Items.size() / 2);

	// enum threads
	m_ProcMgr.EnumProcessesAndThreads(m_Pid);
	m_Threads = m_ProcMgr.GetThreads();

	// enum heaps
	m_Heaps.clear();
	m_Heaps.reserve(8);
	wil::unique_handle hSnapshot(::CreateToolhelp32Snapshot(TH32CS_SNAPHEAPLIST, m_Pid));
	if (hSnapshot) {
		HEAPLIST32 list;
		list.dwSize = sizeof(list);
		HEAPENTRY32 entry;
		entry.dwSize = sizeof(entry);
		int index = 1;
		::Heap32ListFirst(hSnapshot.get(), &list);
		do {
			HeapInfo hi;
			hi.Address = list.th32HeapID;
			hi.Flags = list.dwFlags;
			hi.Id = index++;
			m_Heaps.push_back(hi);
		} while (::Heap32ListNext(hSnapshot.get(), &list));
	}

	for (auto& mi : m_Items) {
		GetDetails(*mi);
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

CString CMemoryMapView::HeapFlagsToString(DWORD flags) {
	CString text;
	if (flags & HF32_DEFAULT)
		text += L" [Default]";
	if (flags & HF32_SHARED)
		text += L" [Shared]";
	return text;
}

bool CMemoryMapView::CompareItems(WinSys::MemoryRegionItem& m1, WinSys::MemoryRegionItem& m2, int col, bool asc) {
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
		case 6: return SortHelper::SortNumbers(GetDetails(m1).Usage, GetDetails(m2).Usage, asc);
		case 7: return SortHelper::SortStrings(GetDetails(m1).Details, GetDetails(m2).Details, asc);
	}
	return false;
}

PCWSTR CMemoryMapView::UsageToString(const WinSys::MemoryRegionItem& item) const {
	auto it = m_Details.find(item.AllocationBase ? item.AllocationBase : item.BaseAddress);
	MemoryUsage usage;
	if (it != m_Details.end())
		usage = it->second.Usage;
	else
		usage = item.State == MEM_FREE ? MemoryUsage::Unknown : MemoryUsage::PrivateData;

	switch (usage) {
		case MemoryUsage::ThreadStack: return L"Stack";
		case MemoryUsage::Image: return L"Image File";
		case MemoryUsage::Mapped: return L"Mapped File";
		case MemoryUsage::Heap: return L"Heap";
		case MemoryUsage::PrivateData: return L"Data";
		case MemoryUsage::Unusable: return L"Unusable";
	}
	return L"";
}

COLORREF CMemoryMapView::UsageToBackColor(const WinSys::MemoryRegionItem& item) const {
	switch (GetDetails(item).Usage) {
		case MemoryUsage::PrivateData: return RGB(255, 255, 0);
		case MemoryUsage::ThreadStack: return RGB(0, 255, 128);
		case MemoryUsage::Image: return RGB(255, 128, 128);
		case MemoryUsage::Mapped: return RGB(128, 255, 255);
		case MemoryUsage::Unusable: return RGB(192, 192, 192);
		case MemoryUsage::Heap: return RGB(192, 128, 192);
	}
	if(item.State != MEM_FREE)
		return RGB(255, 255, 0);
	return CLR_INVALID;
}

CString CMemoryMapView::FormatWithCommas(long long size) {
	CString result;
	result.Format(L"%lld", size);
	int i = 3;
	while (result.GetLength() - i > 0) {
		result = result.Left(result.GetLength() - i) + L"," + result.Right(i);
		i += 4;
	}
	return result;
}

CMemoryMapView::ItemDetails CMemoryMapView::GetDetails(const WinSys::MemoryRegionItem& mi) const {
	if (auto it = m_Details.find(mi.AllocationBase ? mi.AllocationBase : mi.BaseAddress); it != m_Details.end()) {
		return it->second;
	}

	ItemDetails details;
	details.Usage = MemoryUsage::Unknown;
	if (mi.State == MEM_FREE) {
		if (mi.RegionSize < (1 << 16)) {
			details.Usage = MemoryUsage::Unusable;
		}
		m_Details.insert({ mi.BaseAddress, details });
		return details;
	}

	if (mi.State == MEM_COMMIT) {
		if (mi.Type == MEM_IMAGE || mi.Type == MEM_MAPPED) {
			WCHAR filename[MAX_PATH];
			if (::GetMappedFileName(m_hProcess, mi.BaseAddress, filename, MAX_PATH) > 0) {
				details.Details = WinSys::Helpers::GetDosNameFromNtName(filename).c_str();
				details.Usage = mi.Type == MEM_IMAGE ? MemoryUsage::Image : MemoryUsage::Mapped;
			}
		}
		else if (m_hReadProcess) {
			// try threads
			for (auto& t : m_Threads) {
				NT_TIB tib;
				if (::ReadProcessMemory(m_hReadProcess.get(), t->TebBase, &tib, sizeof(tib), nullptr)) {
					if (mi.BaseAddress >= tib.StackLimit && mi.BaseAddress < tib.StackBase) {
						details.Details.Format(L"Thread %u (0x%X) stack", t->Id, t->Id);
						details.Usage = MemoryUsage::ThreadStack;
						break;
					}
				}
			}
		}
	}
	if (details.Usage == MemoryUsage::Unknown) {
		for (auto& heap : m_Heaps) {
			if (mi.AllocationBase <= (PVOID)heap.Address && mi.AllocationBase != nullptr && (BYTE*)mi.AllocationBase + mi.RegionSize > (PVOID)heap.Address) {
				details.Usage = MemoryUsage::Heap;
				details.Details.Format(L"Heap %u %s", heap.Id, HeapFlagsToString(heap.Flags));
				break;
			}
		}
	}

	if (details.Usage != MemoryUsage::Unknown)
		m_Details.insert({ mi.AllocationBase, details });

	return details;
}
