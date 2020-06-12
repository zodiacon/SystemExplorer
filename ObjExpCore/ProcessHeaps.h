#pragma once

namespace WinSys {
	struct ProcessHeap {
		void* Address;
		size_t Size;
	};

	enum class ProcessHeapEntryFlags {
		Fixed = 1,
		Free = 2,
		Moveable = 4,
	};

	struct ProcessHeapEntry {
		void* Address;
		size_t BlockSize;
		ProcessHeapEntryFlags Flag;
	};

	class ProcessHeaps {
	public:
		ProcessHeaps(uint32_t pid);

		std::vector<ProcessHeap> EnumHeaps();
		std::vector<ProcessHeapEntry> EnumHeapEntries(const ProcessHeap& heap);

	private:
		uint32_t _pid;
		wil::unique_handle _hSnapshot;
	};
}

