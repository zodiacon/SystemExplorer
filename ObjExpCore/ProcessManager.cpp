#include "pch.h"
#include "ProcessManager.h"
#include "Keys.h"
#include "ProcessInfo.h"
#include "ThreadInfo.h"
#include "Processes.h"
#include <VersionHelpers.h>

using namespace WinSys;

struct ProcessManager::Impl {
	using ProcessMap = std::unordered_map<ProcessOrThreadKey, std::shared_ptr<ProcessInfo>>;
	using ThreadMap = std::unordered_map<ProcessOrThreadKey, std::shared_ptr<ThreadInfo>>;

	// processes

	std::unordered_map<uint32_t, std::shared_ptr<ProcessInfo>> _processesById;
	std::vector<std::shared_ptr<ProcessInfo>> _processes;
	std::vector<std::shared_ptr<ProcessInfo>> _terminatedProcesses;
	std::vector<std::shared_ptr<ProcessInfo>> _newProcesses;
	ProcessMap _processesByKey;

	// threads

	std::vector<std::shared_ptr<ThreadInfo>> _threads;
	std::vector<std::shared_ptr<ThreadInfo>> _newThreads;
	std::vector<std::shared_ptr<ThreadInfo>> _terminatedThreads;
	std::unordered_map<uint32_t, std::shared_ptr<ThreadInfo>> _threadsById;
	ThreadMap _threadsByKey;

	LARGE_INTEGER _prevTicks{};
	// the snapshot buffer, kept between refreshes: it's several MB, and a snapshot is taken every second or so
	wil::unique_virtualalloc_ptr<BYTE> _buffer;
	ULONG _bufferSize{ 0 };
	static uint32_t _totalProcessors;
	static int64_t _qpcFrequency;

	//
	// CPU usage in units of 1/10,000 of a percent (1,000,000 is 100%): cpuTime is in 100ns units, ticks is the elapsed
	// time in QueryPerformanceCounter ticks, which run at _qpcFrequency (10 MHz on many systems, but not all: assuming
	// that scaled the usage wrongly elsewhere). processors: 1 for 100% to mean one processor all the time.
	//
	static int32_t CpuUsage(int64_t cpuTime, int64_t ticks, uint32_t processors) {
		if (ticks <= 0 || cpuTime <= 0)
			return 0;
		// the elapsed time in 100ns units is ticks * 10,000,000 / frequency
		return (int32_t)((double)cpuTime * _qpcFrequency / ((double)ticks * 10.0 * processors));
	}
	static bool _isElevated;

	Impl() {
		if (_totalProcessors == 0) {
			LARGE_INTEGER frequency;
			::QueryPerformanceFrequency(&frequency);
			_qpcFrequency = frequency.QuadPart;
			_totalProcessors = ::GetActiveProcessorCount(ALL_PROCESSOR_GROUPS);
			_isElevated = Process::OpenById(::GetCurrentProcessId())->IsElevated();
		}
	}

	size_t EnumProcesses(bool includeThreads, uint32_t pid);
	NTSTATUS QuerySnapshot(SYSTEM_INFORMATION_CLASS infoClass, LARGE_INTEGER& ticks);
	std::shared_ptr<ProcessInfo> BuildProcessInfo(const SYSTEM_PROCESS_INFORMATION* info, bool includeThreads, ThreadMap
		& threadsByKey, int64_t delta, std::shared_ptr<ProcessInfo> pi, bool extended);
	std::vector<std::shared_ptr<ProcessInfo>>& GetProcesses() {
		return _processes;
	}

	const std::vector<std::shared_ptr<ProcessInfo>>& GetProcesses() const {
		return _processes;
	}

	std::shared_ptr<ProcessInfo> GetProcessInfo(int index) const;
	std::shared_ptr<ProcessInfo> GetProcessById(uint32_t pid) const {
		auto it = _processesById.find(pid);
		return it == _processesById.end() ? nullptr : it->second;
	}

	std::shared_ptr<ProcessInfo> GetProcessByKey(const ProcessOrThreadKey& key) const;
	const std::vector<std::shared_ptr<ProcessInfo>>& GetTerminatedProcesses() const;
	const std::vector<std::shared_ptr<ProcessInfo>>& GetNewProcesses() const;
	std::vector<std::shared_ptr<ThreadInfo>>& GetThreads() {
		return _threads;
	}
	const std::vector<std::shared_ptr<ThreadInfo>>& GetThreads() const {
		return _threads;
	}

	size_t GetProcessCount() const {
		return _processes.size();
	}

	std::shared_ptr<ThreadInfo> GetThreadInfo(int index) const {
		return _threads[index];
	}

	std::shared_ptr<ThreadInfo> GetThreadByKey(const ProcessOrThreadKey& key) const {
		auto it = _threadsByKey.find(key);
		return it == _threadsByKey.end() ? nullptr : it->second;
	}

	const std::vector<std::shared_ptr<ThreadInfo>>& GetTerminatedThreads() const {
		return _terminatedThreads;
	}

	const std::vector<std::shared_ptr<ThreadInfo>>& GetNewThreads() const {
		return _newThreads;
	}

	size_t GetThreadCount() const {
		return _threads.size();
	}

	std::vector<std::pair<std::shared_ptr<ProcessInfo>, int>> BuildProcessTree() {
		std::vector<std::pair<std::shared_ptr<ProcessInfo>, int>> tree;
		auto count = EnumProcesses(false, 0);
		tree.reserve(count);

		auto map = _processesById;
		for (auto& p : _processes) {
			auto it = _processesById.find(p->ParentId);
			if (p->ParentId == 0 || it == _processesById.end() || (it != _processesById.end() && it->second->CreateTime > p->CreateTime)) {
				// root
				DbgPrint((PSTR)"Root: %ws (%u) (Parent: %u)\n", p->GetImageName().c_str(), p->Id, p->ParentId);
				tree.push_back(std::make_pair(p, 0));
				map.erase(p->Id);
				if (p->Id == 0)
					continue;
				auto children = FindChildren(map, p.get(), 1);
				for (auto& child : children)
					tree.push_back(std::make_pair(_processesById[child.first], child.second));
			}
		}
		return tree;
	}

	std::vector<std::pair<uint32_t, int>> FindChildren(decltype(_processesById)& map, ProcessInfo* parent, int indent) {
		std::vector<std::pair<uint32_t, int>> children;
		for (auto& p : _processes) {
			if (p->ParentId == parent->Id && p->CreateTime > parent->CreateTime) {
				children.push_back(std::make_pair(p->Id, indent));
				map.erase(p->Id);
				auto children2 = FindChildren(map, p.get(), indent + 1);
				children.insert(children.end(), children2.begin(), children2.end());
			}
		}
		return children;
	}
};

uint32_t ProcessManager::Impl::_totalProcessors;
int64_t ProcessManager::Impl::_qpcFrequency;
bool ProcessManager::Impl::_isElevated;

ProcessManager::ProcessManager() : _impl(std::make_unique<Impl>()) {}
ProcessManager::~ProcessManager() = default;


std::shared_ptr<ProcessInfo> ProcessManager::GetProcessInfo(int index) const {
	return _impl->GetProcessInfo(index);
}

std::shared_ptr<ProcessInfo> ProcessManager::GetProcessById(uint32_t pid) const {
	return _impl->GetProcessById(pid);
}

std::shared_ptr<ProcessInfo> ProcessManager::GetProcessByKey(const ProcessOrThreadKey & key) const {
	return _impl->GetProcessByKey(key);
}

const std::vector<std::shared_ptr<ProcessInfo>>& ProcessManager::GetTerminatedProcesses() const {
	return _impl->GetTerminatedProcesses();
}
const std::vector<std::shared_ptr<ProcessInfo>>& ProcessManager::GetNewProcesses() const {
	return _impl->GetNewProcesses();
}

std::vector<std::shared_ptr<ProcessInfo>>& ProcessManager::GetProcesses() {
	return _impl->GetProcesses();
}

const std::vector<std::shared_ptr<ProcessInfo>>& ProcessManager::GetProcesses() const {
	return _impl->GetProcesses();
}

std::vector<std::shared_ptr<ThreadInfo>>& ProcessManager::GetThreads() {
	return _impl->GetThreads();
}
const std::vector<std::shared_ptr<ThreadInfo>>& ProcessManager::GetThreads() const {
	return _impl->GetThreads();
}

size_t ProcessManager::EnumProcesses() {
	return _impl->EnumProcesses(false, 0);
}

std::shared_ptr<ThreadInfo> ProcessManager::GetThreadInfo(int index) const {
	return _impl->GetThreadInfo(index);
}

std::shared_ptr<ThreadInfo> ProcessManager::GetThreadByKey(const ProcessOrThreadKey & key) const {
	return _impl->GetThreadByKey(key);
}

const std::vector<std::shared_ptr<ThreadInfo>>& ProcessManager::GetTerminatedThreads() const {
	return _impl->GetTerminatedThreads();
}

const std::vector<std::shared_ptr<ThreadInfo>>& ProcessManager::GetNewThreads() const {
	return _impl->GetNewThreads();
}

size_t ProcessManager::GetThreadCount() const {
	return _impl->GetThreadCount();
}

size_t WinSys::ProcessManager::GetProcessCount() const {
	return _impl->GetProcesses().size();
}

std::wstring ProcessManager::GetProcessNameById(uint32_t pid) const {
	if (pid == 0)
		return L"";
	auto pi = GetProcessById(pid);
	return pi ? pi->GetImageName() : L"";
}

std::vector<std::pair<std::shared_ptr<ProcessInfo>, int>> ProcessManager::BuildProcessTree() {
	return _impl->BuildProcessTree();
}

size_t ProcessManager::EnumProcessesAndThreads(uint32_t pid) {
	return _impl->EnumProcesses(true, pid);
}

NTSTATUS ProcessManager::Impl::QuerySnapshot(SYSTEM_INFORMATION_CLASS infoClass, LARGE_INTEGER& ticks) {
	if (!_buffer) {
		_bufferSize = 1 << 22;
		_buffer.reset((BYTE*)::VirtualAlloc(nullptr, _bufferSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
		if (!_buffer) {
			_bufferSize = 0;
			return STATUS_NO_MEMORY;
		}
	}
	for (int i = 0; i < 8; i++) {
		// get timing info as close as possible to the API call
		::QueryPerformanceCounter(&ticks);
		ULONG len = 0;
		auto status = NtQuerySystemInformation(infoClass, _buffer.get(), _bufferSize, &len);
		if (status != STATUS_INFO_LENGTH_MISMATCH && status != STATUS_BUFFER_TOO_SMALL)
			return status;

		// processes and threads come and go, so leave some room
		_bufferSize = (std::max)(len, _bufferSize);
		_bufferSize = (_bufferSize + _bufferSize / 4 + 0xffff) & ~0xffff;
		_buffer.reset((BYTE*)::VirtualAlloc(nullptr, _bufferSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
		if (!_buffer) {
			_bufferSize = 0;
			return STATUS_NO_MEMORY;
		}
	}
	return STATUS_INFO_LENGTH_MISMATCH;
}

size_t ProcessManager::Impl::EnumProcesses(bool includeThreads, uint32_t pid) {
	auto extended = _isElevated && IsWindows8OrGreater();
	LARGE_INTEGER ticks;
	auto status = QuerySnapshot(extended ? SystemFullProcessInformation : SystemExtendedProcessInformation, ticks);
	if (!NT_SUCCESS(status)) {
		// keep the last snapshot, rather than report every process as terminated
		_newProcesses.clear();
		_terminatedProcesses.clear();
		if (includeThreads) {
			_newThreads.clear();
			_terminatedThreads.clear();
		}
		return _processes.size();
	}
	auto delta = ticks.QuadPart - _prevTicks.QuadPart;

	std::vector<std::shared_ptr<ProcessInfo>> processes;
	processes.reserve(_processes.empty() ? 512 : _processes.size() + 10);
	ProcessMap processesByKey;
	processesByKey.reserve(_processes.size() == 0 ? 512 : _processes.size() + 10);
	_processesById.clear();
	_processesById.reserve(_processes.capacity());

	_newProcesses.clear();

	ThreadMap threadsByKey;
	if (includeThreads) {
		threadsByKey.reserve(4096);
		_newThreads.clear();
		if (_threads.empty())
			_newThreads.reserve(4096);
		_threads.clear();
		_threadsById.clear();
	}

	{
		auto p = reinterpret_cast<SYSTEM_PROCESS_INFORMATION*>(_buffer.get());

		for (;;) {
			if (pid == 0 || pid == HandleToULong(p->UniqueProcessId)) {
				ProcessOrThreadKey key = { p->CreateTime.QuadPart, HandleToULong(p->UniqueProcessId) };
				std::shared_ptr<ProcessInfo> pi;
				if (auto it = _processesByKey.find(key); it == _processesByKey.end()) {
					// new process
					pi = BuildProcessInfo(p, includeThreads, threadsByKey, delta, pi, extended);
					_newProcesses.push_back(pi);
					pi->CPU = 0;
				}
				else {
					const auto& pi2 = it->second;
					// a share of all the processors' time
					auto cpu = CpuUsage(p->KernelTime.QuadPart + p->UserTime.QuadPart - pi2->UserTime - pi2->KernelTime, delta, _totalProcessors);
					pi = BuildProcessInfo(p, includeThreads, threadsByKey, delta, pi2, extended);
					pi->CPU = cpu;

					// remove from known processes
					_processesByKey.erase(key);
				}
				processes.push_back(pi);
				//
				// add process to maps
				//
				processesByKey.insert({ key, pi });
				_processesById.insert({ pi->Id, pi });
			}
			if (p->NextEntryOffset == 0)
				break;
			p = reinterpret_cast<SYSTEM_PROCESS_INFORMATION*>((BYTE*)p + p->NextEntryOffset);
		}
	}
	_processes = std::move(processes);

	//
	// remaining processes are terminated ones
	//
	_terminatedProcesses.clear();
	_terminatedProcesses.reserve(_processesByKey.size());
	for (const auto& [key, pi] : _processesByKey)
		_terminatedProcesses.push_back(pi);

	_processesByKey = std::move(processesByKey);

	if (includeThreads) {
		_terminatedThreads.clear();
		_terminatedThreads.reserve(_threadsByKey.size());
		for (const auto& [key, ti] : _threadsByKey)
			_terminatedThreads.push_back(ti);

		_threadsByKey = std::move(threadsByKey);
	}

	_prevTicks = ticks;

	return static_cast<uint32_t>(_processes.size());
}

#ifdef __cplusplus
#if _MSC_VER >= 1300
#define TYPE_ALIGNMENT( t ) __alignof(t)
#endif
#else
#define TYPE_ALIGNMENT( t ) \
	FIELD_OFFSET( struct { char x; t test; }, test )
#endif

std::shared_ptr<ProcessInfo> ProcessManager::Impl::BuildProcessInfo(
	const SYSTEM_PROCESS_INFORMATION * info, bool includeThreads, ThreadMap & threadsByKey, int64_t delta, std::shared_ptr<ProcessInfo> pi, bool extended) {
	if (pi == nullptr) {
		pi = std::make_shared<ProcessInfo>();
		pi->Id = HandleToULong(info->UniqueProcessId);
		pi->SessionId = info->SessionId;
		pi->CreateTime = info->CreateTime.QuadPart;
		pi->Key.Created = pi->CreateTime;
		pi->Key.Id = pi->Id;
		pi->ParentId = HandleToULong(info->InheritedFromUniqueProcessId);
		pi->ClearThreads();
		auto name = info->UniqueProcessId == 0 ? L"(Idle)" : std::wstring(info->ImageName.Buffer, info->ImageName.Length / sizeof(WCHAR));
		if (extended && info->UniqueProcessId) {
			auto ext = (SYSTEM_PROCESS_INFORMATION_EXTENSION*)((BYTE*)info +
				FIELD_OFFSET(SYSTEM_PROCESS_INFORMATION, Threads) + sizeof(SYSTEM_EXTENDED_THREAD_INFORMATION) * info->NumberOfThreads);
			pi->JobObjectId = ext->JobObjectId;
			auto index = name.rfind(L'\\');
			::memcpy(pi->UserSid, (BYTE*)info + ext->UserSidOffset, sizeof(pi->UserSid));
			pi->_processName = index == std::wstring::npos ? name : name.substr(index + 1);
			pi->_nativeImagePath = name;
			if (ext->PackageFullNameOffset > 0) {
				pi->_packageFullName = (const wchar_t*)((BYTE*)ext + ext->PackageFullNameOffset);
			}
		}
		else {
			pi->_processName = name;
			pi->JobObjectId = 0;
		}
	}

	pi->ThreadCount = info->NumberOfThreads;
	pi->BasePriority = info->BasePriority;
	pi->UserTime = info->UserTime.QuadPart;
	pi->KernelTime = info->KernelTime.QuadPart;
	pi->HandleCount = info->HandleCount;
	pi->PageFaultCount = info->PageFaultCount;
	pi->PeakThreads = info->NumberOfThreadsHighWatermark;
	pi->PeakVirtualSize = info->PeakVirtualSize;
	pi->VirtualSize = info->VirtualSize;
	pi->WorkingSetSize = info->WorkingSetSize;
	pi->PeakWorkingSetSize = info->PeakWorkingSetSize;
	pi->PagefileUsage = info->PagefileUsage;
	pi->OtherOperationCount = info->OtherOperationCount.QuadPart;
	pi->ReadOperationCount = info->ReadOperationCount.QuadPart;
	pi->WriteOperationCount = info->WriteOperationCount.QuadPart;
	pi->HardFaultCount = info->HardFaultCount;
	pi->OtherTransferCount = info->OtherTransferCount.QuadPart;
	pi->ReadTransferCount = info->ReadTransferCount.QuadPart;
	pi->WriteTransferCount = info->WriteTransferCount.QuadPart;
	pi->PeakPagefileUsage = info->PeakPagefileUsage;
	pi->CycleTime = info->CycleTime;
	pi->NonPagedPoolUsage = info->QuotaNonPagedPoolUsage;
	pi->PagedPoolUsage = info->QuotaPagedPoolUsage;
	pi->PeakNonPagedPoolUsage = info->QuotaPeakNonPagedPoolUsage;
	pi->PeakPagedPoolUsage = info->QuotaPeakPagedPoolUsage;
	pi->PrivatePageCount = info->PrivatePageCount;

	if (includeThreads && pi->Id > 0) {
		auto threadCount = info->NumberOfThreads;
		// the process' threads as of this snapshot (an existing process still has the previous snapshot's)
		pi->ClearThreads();
		for (ULONG i = 0; i < threadCount; i++) {
			auto tinfo = (SYSTEM_EXTENDED_THREAD_INFORMATION*)info->Threads + i;
			const auto& baseInfo = tinfo->ThreadInfo;
			ProcessOrThreadKey key = { baseInfo.CreateTime.QuadPart, HandleToULong(baseInfo.ClientId.UniqueThread) };
			std::shared_ptr<ThreadInfo> thread;
			std::shared_ptr<ThreadInfo> ti2;
			bool newobject = true;
			int64_t cpuTime;
			if (auto it = _threadsByKey.find(key); it != _threadsByKey.end()) {
				thread = it->second;
				cpuTime = thread->UserTime + thread->KernelTime;
				newobject = false;
			}
			if (newobject) {
				thread = std::make_shared<ThreadInfo>();
				thread->_processName = pi->GetImageName();
				thread->Id = HandleToULong(baseInfo.ClientId.UniqueThread);
				thread->ProcessId = HandleToULong(baseInfo.ClientId.UniqueProcess);
				thread->CreateTime = baseInfo.CreateTime.QuadPart;
				thread->StartAddress = baseInfo.StartAddress;
				thread->StackBase = tinfo->StackBase;
				thread->StackLimit = tinfo->StackLimit;
				thread->Win32StartAddress = tinfo->Win32StartAddress;
				thread->TebBase = tinfo->TebBase;
				thread->Key = key;
			}
			thread->KernelTime = baseInfo.KernelTime.QuadPart;
			thread->UserTime = baseInfo.UserTime.QuadPart;
			thread->Priority = baseInfo.Priority;
			thread->BasePriority = baseInfo.BasePriority;
			thread->ThreadState = (ThreadState)baseInfo.ThreadState;
			thread->WaitReason = (WaitReason)baseInfo.WaitReason;
			thread->WaitTime = baseInfo.WaitTime;
			thread->ContextSwitches = baseInfo.ContextSwitches;

			pi->AddThread(thread);

			_threads.push_back(thread);
			if (newobject) {
				// new thread
				thread->CPU = 0;
				_newThreads.push_back(thread);
			}
			else {
				// a share of one processor's time: 100% is the thread running all the time
				thread->CPU = CpuUsage(thread->KernelTime + thread->UserTime - cpuTime, delta, 1);
				_threadsByKey.erase(thread->Key);
			}
			threadsByKey.insert(std::make_pair(thread->Key, thread));
			_threadsById.insert(std::make_pair(thread->Id, thread));
		}
	}
	return pi;
}

std::shared_ptr<ProcessInfo> ProcessManager::Impl::GetProcessInfo(int index) const {
	return _processes[index];
}

std::shared_ptr<ProcessInfo> ProcessManager::Impl::GetProcessByKey(const ProcessOrThreadKey & key) const {
	auto it = _processesByKey.find(key);
	return it == _processesByKey.end() ? nullptr : it->second;
}

const std::vector<std::shared_ptr<ProcessInfo>>& ProcessManager::Impl::GetTerminatedProcesses() const {
	return _terminatedProcesses;
}

const std::vector<std::shared_ptr<ProcessInfo>>& ProcessManager::Impl::GetNewProcesses() const {
	return _newProcesses;
}

