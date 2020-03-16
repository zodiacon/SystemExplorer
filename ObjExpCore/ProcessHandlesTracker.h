#pragma once

namespace WinSys {
	struct HandleEntryInfo {
		HANDLE HandleValue;
		uint32_t ObjectTypeIndex;

		bool operator==(const HandleEntryInfo& other) const;
	};

	class ProcessHandlesTracker {
	public:
		ProcessHandlesTracker(uint32_t pid);
		ProcessHandlesTracker(HANDLE hProcess);
		~ProcessHandlesTracker();

		uint32_t EnumHandles();
		const std::vector<HandleEntryInfo>& GetNewHandles() const;
		const std::vector<HandleEntryInfo>& GetClosedHandles() const;

	private:
		struct Impl;
		std::unique_ptr<Impl> _impl;
	};
}

