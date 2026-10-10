#include "pch.h"
#include "ProcessModuleTracker.h"
#include <TlHelp32.h>
#include <Psapi.h>
#include <ImageHlp.h>
#include "Helpers.h"

#pragma comment(lib, "imagehlp")

using namespace WinSys;

struct ProcessModuleTracker::Impl {
	std::vector<std::shared_ptr<ModuleInfo>> _modules, _newModules, _unloadedModules;
	std::unordered_map<std::wstring, std::shared_ptr<ModuleInfo>> _moduleMap;
	DWORD _pid;
	wil::unique_handle _handle;
	BOOL _isWow64;

	explicit Impl(DWORD pid) : _pid(pid) {
		_handle.reset(::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | SYNCHRONIZE, FALSE, pid));
		if (_handle)
			::IsWow64Process(_handle.get(), &_isWow64);
	}

	explicit Impl(HANDLE hProcess) : _handle(hProcess) {
		::IsWow64Process(_handle.get(), &_isWow64);
	}

	const std::vector<std::shared_ptr<ModuleInfo>>& GetModules() const {
		return _modules;
	}

	const std::vector<std::shared_ptr<ModuleInfo>>& GetNewModules() const {
		return _newModules;
	}

	const std::vector<std::shared_ptr<ModuleInfo>>& GetUnloadedModules() const {
		return _unloadedModules;
	}

	std::shared_ptr<ModuleInfo> FillModule(const MODULEENTRY32& me) {
		auto mi = std::make_shared<ModuleInfo>();
		mi->Name = me.szModule;
		mi->Path = me.szExePath;
		mi->Base = (void*)me.hModule;
		mi->ModuleSize = me.modBaseSize;
		return mi;
	}

	// ntName: the module's mapped file name (NT path), empty if unknown
	std::shared_ptr<ModuleInfo> FillModule(const MEMORY_BASIC_INFORMATION& mbi, const WCHAR* ntName) {
		auto mi = std::make_shared<ModuleInfo>();
		mi->ModuleSize = 0;
		mi->ImageBase = 0;
		if (*ntName) {
			mi->Path = Helpers::GetDosNameFromNtName(ntName);
			auto bs = ::wcsrchr(ntName, L'\\');
			mi->Name = bs ? bs + 1 : ntName;
			BYTE buffer[1 << 12];
			if (::ReadProcessMemory(_handle.get(), mbi.BaseAddress, buffer, sizeof(buffer), nullptr)) {
				auto nt = ::ImageNtHeader(buffer);
				if (nt) {
					auto machine = nt->FileHeader.Machine;
					if (machine == IMAGE_FILE_MACHINE_ARM || machine == IMAGE_FILE_MACHINE_I386) {
						auto oh = (IMAGE_OPTIONAL_HEADER32*)&nt->OptionalHeader;
						mi->ImageBase = UlongToPtr(oh->ImageBase);
						mi->Characteristics = (DllCharacteristics)oh->DllCharacteristics;
					}
					else {
						mi->ImageBase = (PVOID)nt->OptionalHeader.ImageBase;
						mi->Characteristics = (DllCharacteristics)nt->OptionalHeader.DllCharacteristics;
					}
				}
			}
		}
		mi->Base = mbi.AllocationBase;
		mi->Type = mbi.Type == MEM_MAPPED ? MapType::Data : MapType::Image;
		return mi;
	}

	uint32_t EnumModules() {
		return _handle ? EnumModulesWithVirtualQuery() : EnumModulesWithToolHelp();
	}

	void PrepareEnum(bool first) {
		if (first) {
			_modules.reserve(128);
			_moduleMap.reserve(128);
			_newModules.reserve(8);
			_unloadedModules.reserve(8);
		}
		else {
			_newModules.clear();
			_unloadedModules.clear();
		}
	}

	//
	// The modules seen in this enumeration move from _moduleMap to current, so what's left in _moduleMap afterwards
	// are the unloaded ones. A module keeps its ModuleInfo object for as long as it's loaded.
	//
	void EndEnum(std::unordered_map<std::wstring, std::shared_ptr<ModuleInfo>>& current, std::vector<std::shared_ptr<ModuleInfo>>& modules) {
		for (auto& [key, mi] : _moduleMap)
			_unloadedModules.push_back(mi);
		_moduleMap = std::move(current);
		_modules = std::move(modules);
	}

	uint32_t EnumModulesWithVirtualQuery() {
		bool first = _modules.empty();
		PrepareEnum(first);

		std::unordered_map<std::wstring, std::shared_ptr<ModuleInfo>> current;
		current.reserve(_moduleMap.size() + 16);
		std::vector<std::shared_ptr<ModuleInfo>> modules;
		modules.reserve(_modules.size() + 16);

		MEMORY_BASIC_INFORMATION mbi;
		const BYTE* address = nullptr;
		ModuleInfo* pmi = nullptr;
		WCHAR name[MAX_PATH];
		while (::VirtualQueryEx(_handle.get(), address, &mbi, sizeof(mbi)) > 0) {
			if (mbi.State == MEM_COMMIT && mbi.Type != MEM_PRIVATE) {
				if (mbi.AllocationBase == mbi.BaseAddress) {
					// the mapped name and base identify a module; the rest of what FillModule reads is needed only for a new one
					if (!::GetMappedFileName(_handle.get(), mbi.AllocationBase, name, _countof(name)))
						name[0] = 0;
					auto bs = ::wcsrchr(name, L'\\');
					auto key = std::wstring(bs ? bs + 1 : name) + L"#" + std::to_wstring((size_t)mbi.AllocationBase);
					std::shared_ptr<ModuleInfo> mi;
					if (auto node = _moduleMap.extract(key); !node.empty()) {
						mi = node.mapped();
						current.insert(std::move(node));
					}
					else if (auto it = current.find(key); it != current.end()) {
						mi = it->second;
					}
					else {
						mi = FillModule(mbi, name);
						if (!first)
							_newModules.push_back(mi);
						current.insert({ key, mi });
					}
					modules.push_back(mi);
					// the size is the sum of the module's committed regions, counted again on each enumeration
					mi->ModuleSize = 0;
					pmi = mi.get();
				}
				if (pmi && pmi->Base == mbi.AllocationBase)
					pmi->ModuleSize += (ULONG)mbi.RegionSize;
			}
			address += mbi.RegionSize;
		}

		EndEnum(current, modules);
		return static_cast<uint32_t>(_modules.size());
	}

	uint32_t EnumModulesWithToolHelp() {
		// CreateToolhelp32Snapshot fails with INVALID_HANDLE_VALUE, hence unique_hfile
		wil::unique_hfile hSnapshot(::CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, _pid));
		if (!hSnapshot)
			return static_cast<uint32_t>(_modules.size());

		MODULEENTRY32 me;
		me.dwSize = sizeof(me);
		if (!::Module32First(hSnapshot.get(), &me))
			return static_cast<uint32_t>(_modules.size());

		bool first = _modules.empty();
		PrepareEnum(first);

		std::unordered_map<std::wstring, std::shared_ptr<ModuleInfo>> current;
		current.reserve(_moduleMap.size() + 16);
		std::vector<std::shared_ptr<ModuleInfo>> modules;
		modules.reserve(_modules.size() + 16);

		do {
			std::shared_ptr<ModuleInfo> mi;
			if (auto node = _moduleMap.extract(me.szExePath); !node.empty()) {
				mi = node.mapped();
				current.insert(std::move(node));
			}
			else if (auto it = current.find(me.szExePath); it != current.end()) {
				mi = it->second;
			}
			else {
				mi = FillModule(me);
				if (!first)
					_newModules.push_back(mi);
				current.insert({ mi->Path, mi });
			}
			modules.push_back(mi);
		} while (::Module32Next(hSnapshot.get(), &me));

		EndEnum(current, modules);
		return static_cast<uint32_t>(_modules.size());
	}
};

WinSys::ProcessModuleTracker::~ProcessModuleTracker() = default;

WinSys::ProcessModuleTracker::ProcessModuleTracker(DWORD pid) : _impl(new Impl(pid)) {
}

WinSys::ProcessModuleTracker::ProcessModuleTracker(HANDLE hProcess) : _impl(new Impl(hProcess)) {
}

uint32_t WinSys::ProcessModuleTracker::EnumModules() {
	return _impl->EnumModules();
}

const std::vector<std::shared_ptr<ModuleInfo>>& WinSys::ProcessModuleTracker::GetModules() const {
	return _impl->GetModules();
}

const std::vector<std::shared_ptr<ModuleInfo>>& WinSys::ProcessModuleTracker::GetNewModules() const {
	return _impl->GetNewModules();
}

const std::vector<std::shared_ptr<ModuleInfo>>& WinSys::ProcessModuleTracker::GetUnloadedModules() const {
	return _impl->GetUnloadedModules();
}

bool ProcessModuleTracker::IsRunning() const {
	return ::WaitForSingleObject(_impl->_handle.get(), 0) == WAIT_TIMEOUT;
}
