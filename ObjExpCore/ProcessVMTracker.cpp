#include "pch.h"
#include "ProcessVMTracker.h"
#include <unordered_map>
#include "Helpers.h"

using namespace WinSys;

bool MemoryRegionItem::operator==(const MemoryRegionItem& other) const {
	return BaseAddress == other.BaseAddress && Protect == other.Protect && State == other.State;
}

struct ProcessVMTracker::Impl {
	Impl(HANDLE hProcess) : _hProcess(hProcess) {
	}

	Impl(DWORD pid) : Impl(::OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid)) {
	}

	bool IsValid() const {
		return _hProcess != nullptr;
	}

	~Impl() {
		if (_hProcess)
			::CloseHandle(_hProcess);
	}

	const std::vector<std::shared_ptr<MemoryRegionItem>>& GetRegions() const {
		return _items;
	}
	const std::vector<std::shared_ptr<MemoryRegionItem>>& GetNewRegions() const {
		return _newItems;
	}
	const std::vector<std::shared_ptr<MemoryRegionItem>>& GetOldRegions() const {
		return _deletedItems;
	}

	size_t EnumRegions();

private:
	HANDLE _hProcess;
	std::vector<std::shared_ptr<MemoryRegionItem>> _items, _newItems, _deletedItems;
	std::unordered_map<MemoryRegionItem, std::shared_ptr<MemoryRegionItem>> _set;
};

ProcessVMTracker::ProcessVMTracker(HANDLE hProcess) : _impl(new Impl(hProcess)) {
}

ProcessVMTracker::ProcessVMTracker(DWORD pid) : _impl(new Impl(pid)) {
}

ProcessVMTracker::~ProcessVMTracker() = default;

bool ProcessVMTracker::IsValid() const {
	return _impl->IsValid();
}

const std::vector<std::shared_ptr<MemoryRegionItem>>& ProcessVMTracker::GetRegions() const {
	return _impl->GetRegions();
}

const std::vector<std::shared_ptr<MemoryRegionItem>>& ProcessVMTracker::GetNewRegions() const {
	return _impl->GetNewRegions();
}

const std::vector<std::shared_ptr<MemoryRegionItem>>& ProcessVMTracker::GetOldRegions() const {
	return _impl->GetOldRegions();
}

size_t ProcessVMTracker::EnumRegions() {
	return _impl->EnumRegions();
}

size_t ProcessVMTracker::Impl::EnumRegions() {
	if (!IsValid())
		return 0;

	auto count = _items.size();
	_items.clear();
	_items.reserve(count == 0 ? 1024 : count + 32);
	_newItems.clear();
	_newItems.reserve(count == 0 ? 1024 : 32);
	_deletedItems.clear();
	_deletedItems.reserve(16);

	const BYTE* address = nullptr;
	auto oldItems = _set;
	for (;;) {
		auto mi = std::make_shared<MemoryRegionItem>();
		if (0 == ::VirtualQueryEx(_hProcess, address, mi.get(), sizeof(MEMORY_BASIC_INFORMATION)))
			break;

		if (count == 0) {
			_set.insert({ *mi, mi });
		}
		else {
			if (_set.find(*mi) == _set.end()) {
				// new item
				_newItems.push_back(mi);
				_set.insert({ *mi, mi });
			}
			else {
				oldItems.erase(*mi);
			}

			for (auto& [key, mi] : oldItems)
				_deletedItems.push_back(mi);
		}
		_items.push_back(mi);
		address += mi->RegionSize;
	}

	return _items.size();
}
