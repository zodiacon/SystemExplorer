#pragma once

namespace WinSys {
	struct HandleEntryInfo {
		HANDLE HandleValue;
		uint16_t ObjectTypeIndex;

		bool operator==(const HandleEntryInfo& other) const;
	};

	class ProcessHandlesTracker {
	public:
		ProcessHandlesTracker(uint32_t pid);
		ProcessHandlesTracker(HANDLE hProcess);
		~ProcessHandlesTracker();

		bool IsValid() const;

		uint32_t EnumHandles(bool clearHistory = false);
		const std::vector<HandleEntryInfo>& GetNewHandles() const;
		const std::vector<HandleEntryInfo>& GetClosedHandles() const;

	private:
		struct Impl;
		std::unique_ptr<Impl> _impl;
	};
}

