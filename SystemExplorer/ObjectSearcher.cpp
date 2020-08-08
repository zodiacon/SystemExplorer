#include "pch.h"
#include "ObjectSearcher.h"
#include "ObjectManager.h"

ObjectSearcher::ObjectSearcher(HWND hWnd, ObjectSearchType searchType, DWORD pid, const CString& filter)
	: _hWnd(hWnd), _type(searchType), _pid(pid), _filter(filter) {
}

void ObjectSearcher::SearchAsync(const CString& text) {
	_text = text;
	::QueueUserWorkItem([](PVOID p) -> DWORD {
		((ObjectSearcher*)p)->DoSearch();
		return 0;
		}, this, WT_EXECUTEDEFAULT);
}

std::vector<SearchResultItem> ObjectSearcher::GetResults() {
	std::lock_guard locker(_lock);
	auto results = _items;
	return results;
}

void ObjectSearcher::DoSearch() {
	_running = true;
	_cancelRequested = false;
	CString stext(_text);
	stext.MakeLower();
	_items.clear();
	_items.reserve(64);

	auto type = _type;
	if (type == ObjectSearchType::Default)
		type = ObjectSearchType::Handles | ObjectSearchType::DLLs;

	if ((type & ObjectSearchType::DLLs) == ObjectSearchType::DLLs)
		SearchDLLs(stext);

	if ((type & ObjectSearchType::Handles) == ObjectSearchType::Handles)
		SearchHandles(stext);

	// when done, post message
	::PostMessage(_hWnd, SearchDoneMessage, _cancelRequested ? 1 : 0, 0);
	_running = false;
}

void ObjectSearcher::CancelSearch() {
	_cancelRequested = true;
}

void ObjectSearcher::SearchHandles(const CString& stext) {
	ObjectManager om;
	WinSys::ProcessManager pm;
	pm.EnumProcesses();

	om.EnumHandles(_filter, _pid, true);

	for (auto& h : om.GetHandles()) {
		if (_cancelRequested)
			break;

		CString name(h->Name);
		name.MakeLower();
		if (name.Find(stext) >= 0) {
			SearchResultItem item;
			item.Id = h->HandleValue;
			item.Name = h->Name;
			item.Type = om.GetType(h->ObjectTypeIndex)->TypeName;
			item.ProcessId = h->ProcessId;
			item.Details = pm.GetProcessNameById(h->ProcessId).c_str();

			std::lock_guard locker(_lock);
			_items.emplace_back(std::move(item));
		}
	}
}

void ObjectSearcher::SearchDLLs(const CString& text) {
	wil::unique_handle hSnapshot(::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0));
	if (!hSnapshot)
		return;

	PROCESSENTRY32 pe;
	pe.dwSize = sizeof(pe);
	::Process32First(hSnapshot.get(), &pe);

	MODULEENTRY32 me;
	me.dwSize = sizeof(me);

	while (::Process32Next(hSnapshot.get(), &pe)) {
		wil::unique_handle hModules(::CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pe.th32ProcessID));
		if (!hModules)
			continue;

		if (::Module32First(hModules.get(), &me)) {
			do {
				CString name(me.szModule);
				name.MakeLower();
				if (name.Find(text) >= 0) {
					SearchResultItem item;
					item.Id = (DWORD64)me.modBaseAddr;
					item.Name = me.szModule;
					item.Type = L"DLL";
					item.ProcessId = pe.th32ProcessID;
					item.Details = me.szExePath;

					std::lock_guard locker(_lock);
					_items.emplace_back(std::move(item));
				}
			} while (::Module32Next(hModules.get(), &me));
		}

	}
}
