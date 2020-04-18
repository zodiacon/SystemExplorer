#pragma once

#include <memory>

namespace WinSys {
	struct MemoryRegionItem : MEMORY_BASIC_INFORMATION {
		bool operator==(const MemoryRegionItem&) const;
	};

	class ProcessVMTracker {
	public:
		ProcessVMTracker(HANDLE hProcess);
		ProcessVMTracker(DWORD pid);
		~ProcessVMTracker();

		bool IsValid() const;
		const std::vector<std::shared_ptr<MemoryRegionItem>>& GetRegions() const;
		const std::vector<std::shared_ptr<MemoryRegionItem>>& GetNewRegions() const;
		const std::vector<std::shared_ptr<MemoryRegionItem>>& GetOldRegions() const;
		size_t EnumRegions();

	private:
		struct Impl;
		std::unique_ptr<Impl> _impl;
	};
}

template<>
struct std::hash<WinSys::MemoryRegionItem> {
	size_t operator()(const WinSys::MemoryRegionItem& key) const {
		return (size_t)key.BaseAddress ^ key.Protect ^ key.State;
	}
};
