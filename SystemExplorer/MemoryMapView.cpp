#include "pch.h"
#include "ImageHelper.h"
#include "MemoryMapView.h"
#include "DriverHelper.h"
#include "resource.h"
#include <algorithm>
#include <Psapi.h>
#include "ntdll.h"
#include <Helpers.h>
#include "ListViewColorHelper.h"

CMemoryMapView::CMemoryMapView(IMainFrame* frame, DWORD pid) : CViewBase(frame), m_Pid(pid) {
}

bool CMemoryMapView::IsUpdating() const {
	return false;
}

LRESULT CMemoryMapView::OnCreate(UINT, WPARAM, LPARAM, BOOL&) {
	m_hProcess = DriverHelper::OpenProcess(m_Pid, PROCESS_QUERY_INFORMATION);
	if (m_hProcess == nullptr)
		return -1;

	m_Tracker.reset(new WinSys::ProcessVMTracker(m_hProcess));
	if (m_Tracker == nullptr)
		return -1;

	m_hReadProcess.reset(DriverHelper::OpenProcess(m_Pid, PROCESS_VM_READ));

	m_hWndClient = m_Tree.Create(*this, rcDefault, nullptr, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS |
		TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | TVS_SHOWSELALWAYS);

	m_Tree.AddColumn(L"State", 200);
	m_Tree.AddColumn(L"Address", 140, HDF_RIGHT);
	m_Tree.AddColumn(L"Size", 120, HDF_RIGHT);
	m_Tree.AddColumn(L"Type", 70);
	m_Tree.AddColumn(L"Protection", 140);
	m_Tree.AddColumn(L"Alloc Protection", 140);
	m_Tree.AddColumn(L"Usage", 90);
	m_Tree.AddColumn(L"Details", 500);

	CImageList images;
	images.Create(16, 16, ILC_COLOR32, 4, 0);
	UINT icons[] = { IDI_FREE, IDI_RESERVED, IDI_COMMIT };
	for (auto icon : icons)
		ImageHelper::AddIcon(images, AtlLoadIconImage(icon, 0, 16, 16));
	m_Tree.GetTreeControl().SetImageList(KeepImageList(images), TVSIL_NORMAL);

	Refresh();

	return 0;
}

LRESULT CMemoryMapView::OnRefresh(WORD, WORD, HWND, BOOL&) {
	CWaitCursor wait;
	Refresh();

	return 0;
}

LRESULT CMemoryMapView::OnDarkModeChanged(UINT, WPARAM, LPARAM, BOOL& bHandled) {
	// The tree's own background is updated by CTreeListViewCtrl itself (it handles WTLHelper::ThemeChangedMessage,
	// which WTLHelper::SwitchToMode sends to every descendant). What's left are the items' colors, which are set
	// on each item from the current mode's colors (UsageToBackColor), so they're set again here, in place.
	for (size_t i = 0; i < m_Items.size() && i < m_ItemHandles.size(); i++)
		ApplyItemColors(m_ItemHandles[i], *m_Items[i]);
	m_Tree.GetTreeControl().Invalidate();

	bHandled = FALSE;
	return 0;
}

void CMemoryMapView::Refresh() {
	if (m_Details.empty())
		m_Details.reserve(128);

	m_Tracker->EnumRegions();
	m_Items = m_Tracker->GetRegions();
	m_Details.clear();
	m_Details.reserve(m_Items.size() / 2);
	m_NoDetails.clear();

	// enum threads
	m_ProcMgr.EnumProcessesAndThreads(m_Pid);
	m_Threads = m_ProcMgr.GetThreads();

	// their stacks
	m_Stacks.clear();
	if (m_hReadProcess) {
		m_Stacks.reserve(m_Threads.size());
		for (auto& t : m_Threads) {
			NT_TIB tib;
			if (::ReadProcessMemory(m_hReadProcess.get(), t->TebBase, &tib, sizeof(tib), nullptr))
				m_Stacks.push_back({ tib.StackLimit, tib.StackBase, t->Id });
		}
	}

	// enum heaps
	m_Heaps.clear();
	m_Heaps.reserve(8);
	// CreateToolhelp32Snapshot fails with INVALID_HANDLE_VALUE, hence unique_hfile
	wil::unique_hfile hSnapshot(::CreateToolhelp32Snapshot(TH32CS_SNAPHEAPLIST, m_Pid));
	HEAPLIST32 list;
	list.dwSize = sizeof(list);
	if (hSnapshot && ::Heap32ListFirst(hSnapshot.get(), &list)) {
		int index = 1;
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

	auto tree = m_Tree.GetTreeControl();
	tree.SetRedraw(FALSE);
	tree.DeleteAllItems();

	m_ItemHandles.clear();
	m_ItemHandles.reserve(m_Items.size());

	HTREEITEM hTop = TVI_ROOT;
	for (auto& item : m_Items) {
		bool isTopLevel = item->State == MEM_FREE || item->AllocationBase == item->BaseAddress;
		auto hItem = AddItem(isTopLevel ? TVI_ROOT : hTop, *item);
		m_ItemHandles.push_back(hItem);
		if (isTopLevel)
			hTop = hItem;
	}

	tree.SetRedraw(TRUE);
	tree.Invalidate();
}

HTREEITEM CMemoryMapView::AddItem(HTREEITEM hParent, const WinSys::MemoryRegionItem& item) {
	int image = item.State == MEM_COMMIT ? 2 : (item.State == MEM_RESERVE ? 1 : 0);
	CString address;
	address.Format(L"0x%p", item.AllocationBase);
	auto hItem = m_Tree.GetTreeControl().InsertItem(hParent == TVI_ROOT ? (PCWSTR)address : StateToString(item.State), image, image, hParent, TVI_LAST);

	address.Format(L"0x%0p", item.BaseAddress);
	m_Tree.SetSubItemText(hItem, 1, address);
	m_Tree.SetSubItemText(hItem, 2, FormatWithCommas(item.RegionSize >> 10) + L" KB");
	m_Tree.SetSubItemText(hItem, 3, item.State != MEM_COMMIT ? L"" : TypeToString(item.Type));
	m_Tree.SetSubItemText(hItem, 4, item.State != MEM_COMMIT ? CString() : ProtectionToString(item.Protect));
	m_Tree.SetSubItemText(hItem, 5, item.State == MEM_FREE ? CString() : ProtectionToString(item.AllocationProtect));
	m_Tree.SetSubItemText(hItem, 6, UsageToString(item));
	m_Tree.SetSubItemText(hItem, 7, GetDetails(item).Details);

	ApplyItemColors(hItem, item);

	return hItem;
}

void CMemoryMapView::ApplyItemColors(HTREEITEM hItem, const WinSys::MemoryRegionItem& item) {
	auto back = UsageToBackColor(item);
	if (back != CLR_INVALID) {
		// all the columns (the first one too, or the rows look half painted), with the mode's usual text color
		auto text = ::GetSysColor(COLOR_WINDOWTEXT);
		for (int col = 0; col < 8; col++)
			m_Tree.SetSubItemColor(hItem, col, text, back);
	}
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

	// no match for a region with no protection (e.g. reserved or free memory): it has no text then
	auto it = std::find_if(std::begin(prot), std::end(prot), [protection](auto& p) {
		return (p.Value & protection) != 0;
		});
	CString text = it == std::end(prot) ? L"" : it->Text;

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

PCWSTR CMemoryMapView::UsageToString(const WinSys::MemoryRegionItem& item) const {
	auto it = m_Details.find(item.AllocationBase ? item.AllocationBase : item.BaseAddress);
	MemoryUsage usage;
	if (it != m_Details.end())
		usage = it->second.Usage;
	else
		// as UsageToBackColor colors them: committed memory of no known use is private data; reserved memory isn't
		// anything yet
		usage = item.State == MEM_COMMIT ? MemoryUsage::PrivateData : MemoryUsage::Unknown;

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

//
// The color of a region is a tint of the window's background toward its usage's color, so the same colors work in
// light and dark mode: soft enough for the usual text color to read well on it (and for the selection to stand
// out), yet telling the usages apart.
//
COLORREF CMemoryMapView::UsageToBackColor(const WinSys::MemoryRegionItem& item) const {
	COLORREF color;
	switch (GetDetails(item).Usage) {
		case MemoryUsage::PrivateData: color = RGB(230, 180, 0); break;		// amber
		case MemoryUsage::Heap: color = RGB(170, 90, 220); break;			// purple
		case MemoryUsage::Image: color = RGB(230, 80, 80); break;			// red
		case MemoryUsage::Mapped: color = RGB(0, 150, 220); break;			// blue
		case MemoryUsage::ThreadStack: color = RGB(40, 180, 90); break;		// green
		case MemoryUsage::Unusable: color = RGB(128, 128, 128); break;		// gray
		default:
			// committed memory of no known use is private data (as the Usage column says); reserved and free
			// memory is left plain
			if (item.State != MEM_COMMIT)
				return CLR_INVALID;
			color = RGB(230, 180, 0);
			break;
	}

	return GetTintedBackground(color, 32, 24);
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
	if (m_NoDetails.contains(mi.BaseAddress))
		return details;
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
		else {
			// try the threads' stacks
			for (auto& stack : m_Stacks) {
				if (mi.BaseAddress >= stack.Limit && mi.BaseAddress < stack.Base) {
					details.Details.Format(L"Thread %u (0x%X) stack", stack.ThreadId, stack.ThreadId);
					details.Usage = MemoryUsage::ThreadStack;
					break;
				}
			}
		}
	}
	if (details.Usage == MemoryUsage::Unknown) {
		for (auto& heap : m_Heaps) {
			if (mi.AllocationBase <= (PVOID)heap.Address && mi.AllocationBase != nullptr && (BYTE*)mi.AllocationBase + mi.RegionSize > (PVOID)heap.Address) {
				details.Usage = MemoryUsage::Heap;
				details.Details.Format(L"Heap %u %s", heap.Id, (PCWSTR)HeapFlagsToString(heap.Flags));
				break;
			}
		}
	}

	if (details.Usage != MemoryUsage::Unknown)
		m_Details.insert({ mi.AllocationBase, details });
	else
		m_NoDetails.insert(mi.BaseAddress);

	return details;
}
