#include "pch.h"
#include "ProcessHandlesTracker.h"
#include <unordered_set>

using namespace WinSys;

bool HandleEntryInfo::operator==(const HandleEntryInfo& other) const {
	return HandleValue == other.HandleValue;
}

template<>
struct std::hash<HandleEntryInfo> {
	size_t operator()(const HandleEntryInfo& key) const {
		return (size_t)key.HandleValue;
	}
};

struct ProcessHandlesTracker::Impl {
	Impl(uint32_t pid);
	Impl(HANDLE hProcess);

	uint32_t EnumHandles(bool clearHistory);
	const std::vector<HandleEntryInfo>& GetNewHandles() const {
		return _newHandles;
	}
	const std::vector<HandleEntryInfo>& GetClosedHandles() const {
		return _closedHandles;
	}

private:
	wil::unique_handle _hProcess;
	std::vector<HandleEntryInfo> _closedHandles, _newHandles;
	std::unordered_set<HandleEntryInfo> _handles;
};

ProcessHandlesTracker::Impl::Impl(uint32_t pid) : Impl::Impl(::OpenProcess(PROCESS_QUERY_INFORMATION | SYNCHRONIZE, FALSE, pid)) {
}

ProcessHandlesTracker::Impl::Impl(HANDLE hProcess) : _hProcess(hProcess) {
	if (_hProcess) {
		_newHandles.reserve(16);
		_closedHandles.reserve(16);
	}
}

uint32_t ProcessHandlesTracker::Impl::EnumHandles(bool clearHostory) {
	if (!_hProcess)
		return 0;

	if (::WaitForSingleObject(_hProcess.get(), 0) == WAIT_OBJECT_0) {
		_closedHandles.clear();
		_closedHandles.insert(_closedHandles.begin(), _handles.begin(), _handles.end());
		_newHandles.clear();
		_handles.clear();
		return 0;
	}
	auto size = 1 << 22;
	ULONG len;
	std::unique_ptr<BYTE[]> buffer;
	for(;;) {
		buffer = std::make_unique<BYTE[]>(size);
		auto status = ::NtQueryInformationProcess(_hProcess.get(), ProcessHandleInformation, buffer.get(), size, &len);
		if (status == STATUS_SUCCESS)
			break;
		if (STATUS_BUFFER_TOO_SMALL == status) {
			size = len + (1 << 12);
			continue;
		}
		return 0;
	}

	auto info = reinterpret_cast<PROCESS_HANDLE_SNAPSHOT_INFORMATION*>(buffer.get());
	if (clearHostory)
		_handles.clear();

	_newHandles.clear();
	_closedHandles.clear();

	if (_handles.empty()) {
		_handles.reserve(info->NumberOfHandles);
		for (ULONG i = 0; i < info->NumberOfHandles; i++) {
			auto& entry = info->Handles[i];
			HandleEntryInfo key = { entry.HandleValue, entry.ObjectTypeIndex };
			_handles.insert(key);
		}
	}
	else {
		auto oldHandles = _handles;
		for (ULONG i = 0; i < info->NumberOfHandles; i++) {
			auto& entry = info->Handles[i];
			HandleEntryInfo key = { entry.HandleValue, entry.ObjectTypeIndex };
			if (_handles.find(key) == _handles.end()) {
				// new handle
				_newHandles.push_back(key);
				_handles.insert(key);
			}
			else {
				// existing handle
				oldHandles.erase(key);
			}
		}
		for (auto& hi : oldHandles) {
			_closedHandles.push_back(hi);
			_handles.erase(hi);
		}
	}

	return static_cast<uint32_t>(_handles.size());
}

ProcessHandlesTracker::ProcessHandlesTracker(uint32_t pid) : _impl(new Impl(pid)) {
}

ProcessHandlesTracker::ProcessHandlesTracker(HANDLE hProcess) : _impl(new Impl(hProcess)) {
}

WinSys::ProcessHandlesTracker::~ProcessHandlesTracker() = default;

uint32_t ProcessHandlesTracker::EnumHandles(bool clearHistory) {
	return _impl->EnumHandles(clearHistory);
}

const std::vector<HandleEntryInfo>& ProcessHandlesTracker::GetNewHandles() const {
	return _impl->GetNewHandles();
}

const std::vector<HandleEntryInfo>& ProcessHandlesTracker::GetClosedHandles() const {
	return _impl->GetClosedHandles();
}
