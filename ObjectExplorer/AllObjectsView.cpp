// View.cpp : implementation of the CView class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include <algorithm>
#include <execution>
#include "AllObjectsView.h"
#include "ClipboardHelper.h"
#include "ProcessHelper.h"
#include "NtDll.h"

int CAllObjectsView::ColumnCount;

CAllObjectsView::CAllObjectsView(ObjectManager& om, CUpdateUIBase* pUpdateUI) : m_ObjMgr(om), m_pUpdateUI(pUpdateUI) {
}

BOOL CAllObjectsView::PreTranslateMessage(MSG* pMsg) {
	return FALSE;
}

void CAllObjectsView::DoSort(const SortInfo* si) {
	std::sort(std::execution::par, m_AllObjects.begin(), m_AllObjects.end(), [this, si](const auto& o1, const auto& o2) {
		return CompareItems(*o1.get(), *o2.get(), si);
		});

	RedrawItems(GetTopIndex(), GetTopIndex() + GetCountPerPage());
}

void CAllObjectsView::OnFinalMessage(HWND /*hWnd*/) {
	delete this;
}

std::shared_ptr<ObjectInfoEx>& CAllObjectsView::GetItem(int index) {
	return m_AllObjects[index];
}

bool CAllObjectsView::CompareItems(const ObjectInfoEx& o1, const ObjectInfoEx& o2, const SortInfo* si) {
	switch (si->SortColumn) {
		case 0:		// type
			return SortStrings(m_ObjMgr.GetType(o1.TypeIndex)->TypeName, m_ObjMgr.GetType(o2.TypeIndex)->TypeName, si->SortAscending);

		case 1:		// address
			return SortNumbers(o1.Object, o2.Object, si->SortAscending);

		case 2:		// name
			return SortStrings(o1.Name, o2.Name, si->SortAscending);

		case 3:		// handles
			return SortNumbers(o1.HandleCount, o2.HandleCount, si->SortAscending);

	}

	ATLASSERT(false);
	return false;
}

CString CAllObjectsView::GetObjectDetails(ObjectInfoEx * info) const {
	auto h = info->LocalHandle.get();//m_ObjMgr.DupHandle(info);
	if (!h)
		return L"";

	if (info->TypeIndex == m_ObjMgr.GetProcessTypeIndex())
		return GetProcessObjectDetails(h);
	if (info->TypeIndex == m_ObjMgr.GetThreadTypeIndex())
		return GetThreadObjectDetails(h);
	if (info->TypeIndex == m_ObjMgr.GetEventTypeIndex())
		return GetEventObjectDetails(h);
	if (info->TypeIndex == m_ObjMgr.GetMutexTypeIndex())
		return GetMutexObjectDetails(h);
	if (info->TypeIndex == m_ObjMgr.GetSymbolicLinkTypeIndex())
		return GetSymLinkObjectDetails(h);
	if (info->TypeIndex == m_ObjMgr.GetSectionTypeIndex())
		return GetSectionObjectDetails(h);
	if (info->TypeIndex == m_ObjMgr.GetKeyTypeIndex())
		return GetKeyObjectDetails(h);

	return L"";
}

CString CAllObjectsView::GetProcessObjectDetails(HANDLE hProcess) const {
	CString details;
	auto pid = ::GetProcessId(hProcess);
	auto name = ProcessHelper::GetProcessName(hProcess);
	if (name.IsEmpty())
		name = m_ObjMgr.GetProcessNameById(pid);
	FILETIME create, exit{}, dummy;
	if (::GetProcessTimes(hProcess, &create, &exit, &dummy, &dummy)) {
		details.Format(L"PID: %d (%s) Created: %s Exited: %s", pid, name,
			CTime(create).Format(L"%D %X"),
			exit.dwHighDateTime + exit.dwLowDateTime == 0 ? L"(running)" : CTime(exit).Format(L"%D %X"));
	}
	else {
		details.Format(L"PID: %d (%s)", pid, name);
	}
	return details;
}

CString CAllObjectsView::GetThreadObjectDetails(HANDLE hThread) const {
	CString details;
	auto tid = ::GetThreadId(hThread);
	if (tid == 0)
		return details;

	auto pid = ::GetProcessIdOfThread(hThread);
	FILETIME created{}, exited{}, kernel{}, user{};
	ATLVERIFY(::GetThreadTimes(hThread, &created, &exited, &kernel, &user));
	details.Format(L"TID: %d, PID: %d (%s) Created: %s, Exited: %s, CPU Time: %s",
		tid, pid, m_ObjMgr.GetProcessNameById(pid),
		CTime(created).Format(L"%D %X"), 
		exited.dwHighDateTime + exited.dwLowDateTime == 0 ? L"(running)" : CTime(exited).Format(L"%D %X"),
		CTimeSpan((*(int64_t*)&kernel + *(int64_t*)&user) / 10000000).Format(L"%D:%H:%M:%S"));

	return details;
}

CString CAllObjectsView::GetEventObjectDetails(HANDLE hEvent) const {
	NT::EVENT_BASIC_INFORMATION info;
	CString details;
	if (NT_SUCCESS(NT::NtQueryEvent(hEvent, NT::EventBasicInformation, &info, sizeof(info), nullptr))) {
		details.Format(L"Type: %s, Signaled: %s", info.EventType == NT::SynchronizationEvent ? L"Synchronization" : L"Notification",
			info.EventState ? L"True" : L"False");
	}

	return details;
}

CString CAllObjectsView::GetMutexObjectDetails(HANDLE hMutex) const {
	NT::MUTANT_BASIC_INFORMATION info;
	NT::MUTANT_OWNER_INFORMATION owner;
	CString details;
	if (NT_SUCCESS(NT::NtQueryMutant(hMutex, NT::MutantBasicInformation, &info, sizeof(info), nullptr))
		&& NT_SUCCESS(NT::NtQueryMutant(hMutex, NT::MutantOwnerInformation, &owner, sizeof(owner), nullptr))) {
		details.Format(L"Owner TID: %d, Count: %d, Abandoned: %s", 
			HandleToULong(owner.ClientId.UniqueThread),
			info.CurrentCount, info.AbandonedState ? L"True" : L"False");
	}
	return details;
}

CString CAllObjectsView::GetSectionObjectDetails(HANDLE hSection) const {
	CString details;
	NT::SECTION_BASIC_INFORMATION bi;
	if (NT_SUCCESS(NT::NtQuerySection(hSection, NT::SectionBasicInformation, &bi, sizeof(bi), nullptr))) {
		details.Format(L"Size: %d KiB, Attributes: 0x%X (%s)", 
			bi.MaximumSize.QuadPart >> 10, bi.AllocationAttributes, SectionAttributesToString(bi.AllocationAttributes));
	}
	return details;
}

CString CAllObjectsView::GetSymLinkObjectDetails(HANDLE hLink) const {
	WCHAR buffer[1024];
	UNICODE_STRING str;
	str.MaximumLength = sizeof(buffer);
	str.Buffer = buffer;
	if(NT_SUCCESS(NT::NtQuerySymbolicLinkObject(hLink, &str, nullptr)))
		return CString(buffer, str.Length / sizeof(WCHAR));
	return L"";
}

CString CAllObjectsView::GetKeyObjectDetails(HANDLE hKey) const {
	CString details;
	BYTE buffer[1 << 12];
	if (NT_SUCCESS(NT::NtQueryKey(hKey, NT::KeyBasicInformation, buffer, sizeof(buffer), nullptr))) {
		auto info = (NT::KEY_BASIC_INFORMATION*)buffer;
		details.Format(L"%s, LastWrite: %s", info->Name, CTime(*(FILETIME*)info->LastWriteTime.QuadPart).Format(L"%D %X"));
	}
	return details;
}

CString CAllObjectsView::SectionAttributesToString(DWORD value) {
	CString text;
	struct {
		DWORD attribute;
		PCWSTR text;
	} attributes[] = {
		{ SEC_COMMIT, L"Commit" },
		{ SEC_RESERVE, L"Reserve" },
		{ SEC_IMAGE, L"Image" },
		{ SEC_NOCACHE, L"No Cache" },
		{ SEC_FILE, L"File" },
		{ SEC_WRITECOMBINE, L"Write Combine" },
		{ SEC_PROTECTED_IMAGE, L"Protected Image" },
		{ SEC_LARGE_PAGES, L"Large Pages" },
		{ SEC_IMAGE_NO_EXECUTE, L"No Execute" },
	};

	for (auto& item : attributes)
		if (value & item.attribute)
			(text += item.text) += L", ";
	if (text.GetLength() == 0)
		text = L"None";
	else
		text = text.Left(text.GetLength() - 2);
	return text;
}

LRESULT CAllObjectsView::OnTimer(UINT, WPARAM id, LPARAM, BOOL&) {
	if (id == 1) {
		Refresh();
		auto si = GetSortInfo();
		if(si && si->SortColumn >= 0)
			DoSort(si);
		RedrawItems(GetTopIndex(), GetTopIndex() + GetCountPerPage());
	}
	return 0;
}

LRESULT CAllObjectsView::OnEditCopy(WORD, WORD, HWND, BOOL&) {
	auto selected = GetSelectedIndex();
	if (selected < 0)
		return 0;

	CString text;
	for (int i = 0; i < ColumnCount; i++) {
		CString temp;
		GetItemText(selected, i, temp);
		text += temp + ", ";
	}

	ClipboardHelper::CopyText(*this, text.Left(text.GetLength() - 2));

	return 0;
}

LRESULT CAllObjectsView::OnCreate(UINT, WPARAM, LPARAM, BOOL&) {
	DefWindowProc();

	SetExtendedListViewStyle(LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);

	struct {
		PCWSTR Header;
		int Width;
		int Format = LVCFMT_LEFT;
	} columns[] = {
		{ L"Type", 130 },
		{ L"Address", 140, LVCFMT_RIGHT },
		{ L"Name", 300 },
		{ L"Handles", 100, LVCFMT_RIGHT },
		{ L"Details", 500 },
		//{ L"Non-Paged Pool", 100, LVCFMT_RIGHT },
//		{ L"True Ref", 100, LVCFMT_RIGHT },
	};

	ColumnCount = _countof(columns);

	int i = 0;
	for(auto& c : columns)
		InsertColumn(i++, c.Header, c.Format, c.Width);

	Refresh();
	//SetTimer(1, 2000, nullptr);

	return 0;
}

LRESULT CAllObjectsView::OnDestroy(UINT, WPARAM, LPARAM, BOOL&) {
	return 0;
}

LRESULT CAllObjectsView::OnForwardMessage(UINT, WPARAM, LPARAM lParam, BOOL& handled) {
	auto msg = (MSG*)lParam;
	LRESULT result = 0;
	//handled = ProcessWindowMessage(msg->hwnd, msg->message, msg->wParam, msg->lParam, result, 2);
	return result;
}

LRESULT CAllObjectsView::OnGetDispInfo(int, LPNMHDR hdr, BOOL&) {
	auto lv = (NMLVDISPINFO*)hdr;
	auto& item = lv->item;
	auto& data = GetItem(item.iItem);

	if (item.mask & LVIF_TEXT) {
		switch (item.iSubItem) {
			case 0:	// type
				item.pszText = (PWSTR)(PCWSTR)m_ObjMgr.GetType(data->TypeIndex)->TypeName;
				break;

			case 1:	// address
				::StringCchPrintf(item.pszText, item.cchTextMax, L"0x%p", data->Object);
				break;

			case 2:	// name
				item.pszText = (PWSTR)(PCWSTR)data->Name;
				break;

			case 3:	// handles
				::StringCchPrintf(item.pszText, item.cchTextMax, L"%u", data->HandleCount);
				break;

			case 4:	// details
				::StringCchCopy(item.pszText, item.cchTextMax, GetObjectDetails(data.get()));

		}
	}
	return 0;
}

void CAllObjectsView::Refresh() {
	m_ObjMgr.EnumProcesses();
	m_ObjMgr.EnumHandlesAndObjects();
	m_AllObjects = m_ObjMgr.GetObjects();
	SetItemCountEx(static_cast<int>(m_AllObjects.size()), LVSICF_NOINVALIDATEALL | LVSICF_NOSCROLL);
}
