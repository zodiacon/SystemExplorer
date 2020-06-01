#pragma once

#include <stdint.h>

namespace WinSys {
	int SetLastStatus(int status);
	int GetLastStatus();

	struct PerformanceInformation {
		int64_t IdleProcessTime;
		int64_t IoReadTransferCount;
		int64_t IoWriteTransferCount;
		int64_t IoOtherTransferCount;
		uint32_t IoReadOperationCount;
		uint32_t IoWriteOperationCount;
		uint32_t IoOtherOperationCount;
		uint32_t AvailablePages;
		uint32_t CommittedPages;
		uint32_t CommitLimit;
		uint32_t PeakCommitment;
		uint32_t PageFaultCount;
		uint32_t CopyOnWriteCount;
		uint32_t TransitionCount;
		uint32_t CacheTransitionCount;
		uint32_t DemandZeroCount;
		uint32_t PageReadCount;
		uint32_t PageReadIoCount;
		uint32_t CacheReadCount;
		uint32_t CacheIoCount;
		uint32_t DirtyPagesWriteCount;
		uint32_t DirtyWriteIoCount;
		uint32_t MappedPagesWriteCount;
		uint32_t MappedWriteIoCount;
		uint32_t PagedPoolPages;
		uint32_t NonPagedPoolPages;
		uint32_t PagedPoolAllocs;
		uint32_t PagedPoolFrees;
		uint32_t NonPagedPoolAllocs;
		uint32_t NonPagedPoolFrees;
		uint32_t FreeSystemPtes;
		uint32_t ResidentSystemCodePage;
		uint32_t TotalSystemDriverPages;
		uint32_t TotalSystemCodePages;
		uint32_t NonPagedPoolLookasideHits;
		uint32_t PagedPoolLookasideHits;
		uint32_t AvailablePagedPoolPages;
		uint32_t ResidentSystemCachePage;
		uint32_t ResidentPagedPoolPage;
		uint32_t ResidentSystemDriverPage;
		uint32_t CcFastReadNoWait;
		uint32_t CcFastReadWait;
		uint32_t CcFastReadResourceMiss;
		uint32_t CcFastReadNotPossible;
		uint32_t CcFastMdlReadNoWait;
		uint32_t CcFastMdlReadWait;
		uint32_t CcFastMdlReadResourceMiss;
		uint32_t CcFastMdlReadNotPossible;
		uint32_t CcMapDataNoWait;
		uint32_t CcMapDataWait;
		uint32_t CcMapDataNoWaitMiss;
		uint32_t CcMapDataWaitMiss;
		uint32_t CcPinMappedDataCount;
		uint32_t CcPinReadNoWait;
		uint32_t CcPinReadWait;
		uint32_t CcPinReadNoWaitMiss;
		uint32_t CcPinReadWaitMiss;
		uint32_t CcCopyReadNoWait;
		uint32_t CcCopyReadWait;
		uint32_t CcCopyReadNoWaitMiss;
		uint32_t CcCopyReadWaitMiss;
		uint32_t CcMdlReadNoWait;
		uint32_t CcMdlReadWait;
		uint32_t CcMdlReadNoWaitMiss;
		uint32_t CcMdlReadWaitMiss;
		uint32_t CcReadAheadIos;
		uint32_t CcLazyWriteIos;
		uint32_t CcLazyWritePages;
		uint32_t CcDataFlushes;
		uint32_t CcDataPages;
		uint32_t ContextSwitches;
		uint32_t FirstLevelTbFills;
		uint32_t SecondLevelTbFills;
		uint32_t SystemCalls;
		uint64_t CcTotalDirtyPages; // since THRESHOLD
		uint64_t CcDirtyPageThreshold; // since THRESHOLD
		int64_t ResidentAvailablePages; // since THRESHOLD
		uint64_t SharedCommittedPages; // since THRESHOLD
	};

	struct WindowsVersion {
		uint32_t Major, Minor;
		uint32_t Build;
	};

	enum class ProcessorArchitecture : uint16_t {
		x64 = 9,
		ARM = 5,
		x86 = 0,
		Itanium = 6,
		Unknown = 0xffff
	};

	struct BasicSystemInfo {
		ProcessorArchitecture ProcessorArchitecture;
		uint32_t PageSize;
		uint32_t NumberOfProcessors;
		uint16_t ProcessorLevel;
		uint16_t ProcessorRevision;
		size_t TotalPhysicalInPages;
		size_t CommitLimitInPages;
		void* MinimumAppAddress;
		void* MaximumAppAddress;
	};

	class SystemInformation {
	public:
		static PerformanceInformation GetPerformanceInformation();
		static const WindowsVersion& GetWindowsVersion();
		static const BasicSystemInfo& GetBasicSystemInfo();
	};

}
