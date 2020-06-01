#pragma once

#include <vector>
#include <memory>

namespace WinSys {
	enum class MapType {
		Image,
		Data
	};

	struct ModuleInfo {
		std::wstring Name;
		std::wstring Path;
		void* ImageBase;
		void* Base;
		uint32_t ModuleSize;
		MapType Type;
	};

	class ProcessModuleTracker {
	public:
		explicit ProcessModuleTracker(DWORD pid);
		explicit ProcessModuleTracker(HANDLE hProcess);

		~ProcessModuleTracker();

		uint32_t EnumModules();
		const std::vector<std::shared_ptr<ModuleInfo>>& GetModules() const;
		const std::vector<std::shared_ptr<ModuleInfo>>& GetNewModules() const;
		const std::vector<std::shared_ptr<ModuleInfo>>& GetUnloadedModules() const;

	private:
		struct Impl;
		std::unique_ptr<Impl> _impl;
	};
}
