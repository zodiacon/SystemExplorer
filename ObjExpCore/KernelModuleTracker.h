#pragma once

#include <string>

namespace WinSys {
	struct KernelModuleInfo {
		std::wstring Name;
		std::string FullPath;
		HANDLE hSection;
		void* MappedBase;
		void* ImageBase;
		uint32_t ImageSize;
		uint32_t Flags;
		uint16_t LoadOrderIndex;
		uint16_t InitOrderIndex;
		uint16_t LoadCount;
		void* DefaultBase;
		uint32_t ImageChecksum;
		uint32_t TimeDateStamp;
	};

	class KernelModuleTracker final {
	public:
		uint32_t EnumModules();

		const std::vector<std::shared_ptr<KernelModuleInfo>>& GetModules() const;
		const std::vector<std::shared_ptr<KernelModuleInfo>>& GetNewModules() const;
		const std::vector<std::shared_ptr<KernelModuleInfo>>& GetUnloadedModules() const;

	private:
		std::vector<std::shared_ptr<KernelModuleInfo>> _modules, _newModules, _unloadedModules;
		const std::unordered_map<void*, std::shared_ptr<KernelModuleInfo>> _moduleMap;
	};
}
