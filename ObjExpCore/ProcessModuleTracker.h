#pragma once

#include <vector>
#include <memory>

namespace WinSys {
	enum class MapType {
		Image,
		Data
	};

	enum class DllCharacteristics {
		None = 0,
		HighEntropyVA = 0x20,
		DynamicBase = 0x40,
		ForceIntegrity = 0x80,
		NxCompat = 0x100,
		NoIsolation = 0x200,
		NoSEH = 0x400,
		NoBind = 0x800,
		AppContainer = 0x1000,
		WDMDriver = 0x2000,
		ControlFlowGuard = 0x4000,
		TerminalServerAware = 0x8000
	};
	DEFINE_ENUM_FLAG_OPERATORS(WinSys::DllCharacteristics);

	struct ModuleInfo {
		std::wstring Name;
		std::wstring Path;
		void* ImageBase;
		void* Base;
		uint32_t ModuleSize;
		DllCharacteristics Characteristics;
		MapType Type;
	};

	class ProcessModuleTracker final {
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
