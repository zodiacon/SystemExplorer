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
		_handle.reset(::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid));
		if(_handle)
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

	std::shared_ptr<ModuleInfo> FillModule(const MEMORY_BASIC_INFORMATION& mbi) {
		auto mi = std::make_shared<ModuleInfo>();
		mi->ModuleSize = 0;
		WCHAR name[MAX_PATH];
		mi->ImageBase = 0;
		if (::GetMappedFileName(_handle.get(), mbi.AllocationBase, name, _countof(name))) {
			mi->Path = Helpers::GetDosNameFromNtName(name);
			mi->Name = ::wcsrchr(name, L'\\') + 1;
			BYTE buffer[1 << 12];
			if (::ReadProcessMemory(_handle.get(), mbi.BaseAddress, buffer, sizeof(buffer), nullptr)) {
				auto nt = ::ImageNtHeader(buffer);
				if (nt) {
					auto machine = nt->FileHeader.Machine;
					if(machine == IMAGE_FILE_MACHINE_ARM || machine == IMAGE_FILE_MACHINE_I386)
						mi->ImageBase = ULongToPtr(((IMAGE_OPTIONAL_HEADER32*)&nt->OptionalHeader)->ImageBase);
					else
						mi->ImageBase = (PVOID)nt->OptionalHeader.ImageBase;
				}
			}
		}
		else {
			mi->Name = mi->Path = L"<pagefile backed>";
		}
		mi->Base = mbi.AllocationBase;
		mi->Type = mbi.Type == MEM_MAPPED ? MapType::Data : MapType::Image;
		return mi;
	}

	std::shared_ptr<ModuleInfo> FillModule(HMODULE h) {
		MODULEINFO m;
		::GetModuleInformation(_handle.get(), h, &m, sizeof(m));
		WCHAR name[MAX_PATH];
		::GetModuleBaseName(_handle.get(), h, name, _countof(name));
		auto mi = std::make_shared<ModuleInfo>();
		mi->Name = name;
		::GetModuleFileNameEx(_handle.get(), h, name, _countof(name));
		mi->Path = name;
		mi->Base = m.lpBaseOfDll;
		mi->ImageBase = 0;
		mi->ModuleSize = m.SizeOfImage;
		return mi;
	}

	uint32_t EnumModules() {
		return _handle ? EnumModulesWithVirtualQuery() : EnumModulesWithToolHelp();
	}

	uint32_t EnumModulesWithPsapi() {
		HMODULE hModule[4096];
		DWORD needed;
		if (!::EnumProcessModulesEx(_handle.get(), hModule, sizeof(hModule), &needed, LIST_MODULES_ALL))
			return 0;

		WCHAR name[MAX_PATH];
		bool first = _modules.empty();
		if (first) {
			_modules.reserve(128);
			_newModules.reserve(8);
			_unloadedModules.reserve(8);
		}
		else {
			_newModules.clear();
			_unloadedModules.clear();
		}

		auto existing = _moduleMap;

		for (int i = 0; i < needed / sizeof(HMODULE); i++) {
			auto& h = hModule[i];
			if (first) {
				auto mi = FillModule(h);
				_moduleMap.insert({ mi->Path, mi });
				_modules.push_back(std::move(mi));
			}
			else {
				::GetModuleFileNameEx(_handle.get(), h, name, _countof(name));
				auto it = _moduleMap.find(name);
				if (it == _moduleMap.end()) {
					// new module
					auto mi = FillModule(h);
					_modules.push_back(mi);
					_newModules.push_back(mi);
				}
				else {
					existing.erase(name);
				}
			}
		}
		return static_cast<uint32_t>(_modules.size());
	}

	uint32_t EnumModulesWithVirtualQuery() {
		bool first = _modules.empty();
		if (first) {
			_modules.reserve(128);
			_newModules.reserve(8);
			_unloadedModules.reserve(8);
		}
		else {
			_newModules.clear();
			_unloadedModules.clear();
		}

		auto existing = _moduleMap;

		MEMORY_BASIC_INFORMATION mbi;
		BYTE* address = nullptr;
		ModuleInfo* pmi = nullptr;
		while (::VirtualQueryEx(_handle.get(), address, &mbi, sizeof(mbi)) > 0) {
			if (mbi.State == MEM_COMMIT && mbi.Type != MEM_PRIVATE) {
				if (mbi.AllocationBase == mbi.BaseAddress) {
					auto mi = FillModule(mbi);
					pmi = mi.get();
					auto key = mi->Name + L"#" + std::to_wstring((size_t)mi->Base);
					if (first) {
						_moduleMap.insert({ key, mi });
						_modules.push_back(std::move(mi));
					}
					else {
						auto it = _moduleMap.find(key);
						if (it == _moduleMap.end()) {
							// new module
							_modules.push_back(mi);
							_newModules.push_back(mi);
						}
						else {
							existing.erase(key);
						}
					}
				}
				pmi->ModuleSize += (ULONG)mbi.RegionSize;
			}
			address += mbi.RegionSize;
		}
		return static_cast<uint32_t>(_modules.size());
	}

	uint32_t EnumModulesWithToolHelp() {
		wil::unique_handle hSnapshot(::CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, _pid));
		if (!hSnapshot)
			return 0;

		bool first = _modules.empty();
		if (first) {
			_modules.reserve(128);
			_newModules.reserve(8);
			_unloadedModules.reserve(8);
		}
		else {
			_newModules.clear();
			_unloadedModules.clear();
		}

		MODULEENTRY32 me;
		me.dwSize = sizeof(me);
		::Module32First(hSnapshot.get(), &me);

		auto existing = _moduleMap;
		do {
			if (first) {
				auto mi = FillModule(me);
				_moduleMap.insert({ mi->Path, mi });
				_modules.push_back(std::move(mi));
			}
			else {
				auto it = _moduleMap.find(me.szExePath);
				if (it == _moduleMap.end()) {
					// new module
					auto mi = FillModule(me);
					_modules.push_back(mi);
					_newModules.push_back(mi);
				}
				else {
					existing.erase(me.szExePath);
				}
			}
		} while (::Module32Next(hSnapshot.get(), &me));

		for (auto& [key, mi] : existing)
			_unloadedModules.push_back(mi);

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
