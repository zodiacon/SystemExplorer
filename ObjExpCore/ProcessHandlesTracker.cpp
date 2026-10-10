#include "pch.h"
#include "ProcessHandlesTracker.h"
#include <unordered_set>
#include <assert.h>

using namespace WinSys;

bool HandleEntryInfo::operator==(const HandleEntryInfo& other) const {
	// the type too, as the hash uses it (a handle value reused for an object of another type is another handle)
	return HandleValue == other.HandleValue && ObjectTypeIndex == other.ObjectTypeIndex;
}

template<>
struct std::hash<HandleEntryInfo> {
	size_t operator()(const HandleEntryInfo& key) const {
		return (size_t)key.HandleValue ^ ((int32_t)key.ObjectTypeIndex << 16);
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

	bool IsValid() const {
		return _hProcess.is_valid();
	}

private:
	wil::unique_handle _hProcess;
	std::vector<HandleEntryInfo> _closedHandles, _newHandles;
	std::unordered_set<HandleEntryInfo> _handles;
	// the snapshot buffer, kept between refreshes
	std::unique_ptr<BYTE[]> _buffer;
	ULONG _bufferSize{ 0 };
};

ProcessHandlesTracker::Impl::Impl(uint32_t pid) : 
	Impl::Impl(::OpenProcess(PROCESS_QUERY_INFORMATION | SYNCHRONIZE, FALSE, pid)) {
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

	auto rc = ::WaitForSingleObject(_hProcess.get(), 0);
	assert(rc != WAIT_FAILED);

	if (rc == WAIT_OBJECT_0) {
		_closedHandles.clear();
		_closedHandles.insert(_closedHandles.begin(), _handles.begin(), _handles.end());
		_newHandles.clear();
		_handles.clear();
		return 0;
	}
	if (!_buffer) {
		// most processes have a few hundred handles at most; the buffer grows as needed
		_bufferSize = 1 << 16;
		_buffer.reset(new BYTE[_bufferSize]);
	}
	for (int i = 0;; i++) {
		ULONG len = 0;
		auto status = ::NtQueryInformationProcess(_hProcess.get(), ProcessHandleInformation, _buffer.get(), _bufferSize, &len);
		if (status == STATUS_SUCCESS)
			break;
		if ((status == STATUS_INFO_LENGTH_MISMATCH || status == STATUS_BUFFER_TOO_SMALL) && i < 8) {
			// handles come and go, so leave some room
			_bufferSize = (std::max)(len, _bufferSize);
			_bufferSize += _bufferSize / 4;
			_buffer.reset(new BYTE[_bufferSize]);
			continue;
		}
		return 0;
	}

	auto info = reinterpret_cast<PROCESS_HANDLE_SNAPSHOT_INFORMATION*>(_buffer.get());
	if (clearHostory)
		_handles.clear();

	_newHandles.clear();
	_closedHandles.clear();
	if (_handles.empty()) {
		_newHandles.reserve(1024);
		_closedHandles.reserve(32);
	}

	if (_handles.empty()) {
		_handles.reserve(info->NumberOfHandles);
		for (ULONG i = 0; i < info->NumberOfHandles; i++) {
			const auto& entry = info->Handles[i];
			HandleEntryInfo key = { entry.HandleValue, (uint16_t)entry.ObjectTypeIndex };
			_handles.insert(key);
		}
	}
	else {
		// the handles seen now move from _handles to current; what's left in _handles are the closed ones
		std::unordered_set<HandleEntryInfo> current;
		current.reserve(info->NumberOfHandles);
		for (ULONG i = 0; i < info->NumberOfHandles; i++) {
			const auto& entry = info->Handles[i];
			HandleEntryInfo key = { entry.HandleValue, (uint16_t)entry.ObjectTypeIndex };
			if (auto node = _handles.extract(key); !node.empty()) {
				// existing handle
				current.insert(std::move(node));
			}
			else {
				// new handle
				_newHandles.push_back(key);
				current.insert(key);
			}
		}
		_closedHandles.insert(_closedHandles.end(), _handles.begin(), _handles.end());
		_handles = std::move(current);
	}

	return static_cast<uint32_t>(_handles.size());
}

ProcessHandlesTracker::ProcessHandlesTracker(uint32_t pid) : _impl(new Impl(pid)) {
}

ProcessHandlesTracker::ProcessHandlesTracker(HANDLE hProcess) : _impl(new Impl(hProcess)) {
}

ProcessHandlesTracker::~ProcessHandlesTracker() = default;

bool WinSys::ProcessHandlesTracker::IsValid() const {
	return _impl->IsValid();
}

uint32_t ProcessHandlesTracker::EnumHandles(bool clearHistory) {
	return _impl->EnumHandles(clearHistory);
}

const std::vector<HandleEntryInfo>& ProcessHandlesTracker::GetNewHandles() const {
	return _impl->GetNewHandles();
}

const std::vector<HandleEntryInfo>& ProcessHandlesTracker::GetClosedHandles() const {
	return _impl->GetClosedHandles();
}
