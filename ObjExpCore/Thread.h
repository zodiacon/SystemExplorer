#pragma once

#include <memory>
#include <string>

namespace WinSys {
	enum class ThreadPriority {
		Idle = THREAD_PRIORITY_IDLE,
		Lowest = THREAD_PRIORITY_LOWEST,
		BelowNormal = THREAD_PRIORITY_BELOW_NORMAL,
		Normal = THREAD_PRIORITY_NORMAL,
		AboveNormal = THREAD_PRIORITY_ABOVE_NORMAL,
		Highest = THREAD_PRIORITY_HIGHEST,
		TimeCritical = THREAD_PRIORITY_TIME_CRITICAL
	};

	enum class ThreadAccessMask : uint32_t {
		Synchronize = SYNCHRONIZE,
		DirectImpersonation = THREAD_DIRECT_IMPERSONATION,
		GetContext = THREAD_GET_CONTEXT,
		SetContext = THREAD_SET_CONTEXT,
		SetInformation = THREAD_SET_INFORMATION,
		Impersonate = THREAD_IMPERSONATE,
		QueryInformation = THREAD_QUERY_INFORMATION,
		QueryLimitedInformation = THREAD_QUERY_LIMITED_INFORMATION,
		SetLimitedInformation = THREAD_SET_LIMITED_INFORMATION,
		SetThreadToken = THREAD_SET_THREAD_TOKEN,
		SuspendResume = THREAD_SUSPEND_RESUME,
		Terminate = THREAD_TERMINATE,
		SystemSecurity = ACCESS_SYSTEM_SECURITY,
		AllAccess = THREAD_ALL_ACCESS
	};

	enum class IoPriority {
		Unknown = -1,
		VeryLow = 0,
		Low,
		Normal,
		High,
		Critical
	};

	struct CpuNumber {
		uint16_t Group;
		uint8_t Number;
	};

	enum class ComFlags : uint32_t {
		None = 0,
		LocalTid =					0x01,		
		UuidInitialized =			0x02, 
		InThreadDetach =			0x04,
		ChannelInitialized =		0x08,
		WowThread =					0x10,   
		ThreadUninitializing =		0x20,
		DisableOle1DDE =			0x40,
		STA =						0x80,
		MTA =						0x100,
		Impersonating =				0x200,
		DisableEventLogger =		0x400,  
		InNeutralApartment =		0x800,
		DispatchThread =			0x1000, 
		HostThread =				0x2000, 
		AllowCoInit =				0x4000,
		PendingUninit =				0x8000,
		FirstMTAInit =				0x10000,
		FirstNTAInit =				0x20000,
		ApartmentInitializing =		0x40000,
		UIMessageInModalLoop =		0x80000,
		MarshallingErrorObject =	0x100000,
		WinRTInitialize =			0x200000,
		ASTA =						0x400000,
		InShutdownCallbacks =		0x800000,
		PointerInputBlocked =		0x1000000,
		InActivationFilter =		0x2000000,
		ASTAtoASTAExempQuirk =		0x4000000,
		ASTAtoASTAExempProxy =		0x8000000,
		ASTAtoASTAExempIndoubt =	0x10000000,
		DetectedUserInit =			0x20000000,
		BridgeSTA =					0x40000000,
		MainInitializing =			0x80000000,

		Error = 0xffffffff
	};
	DEFINE_ENUM_FLAG_OPERATORS(WinSys::ComFlags);

	class Thread final {
	public:
		static std::unique_ptr<Thread> OpenById(uint32_t tid, ThreadAccessMask accessMask = ThreadAccessMask::QueryInformation);
		explicit Thread(HANDLE handle, bool own = false);
		explicit Thread(uint32_t tid, ThreadAccessMask accessMask = ThreadAccessMask::QueryInformation);
		~Thread();

		HANDLE GetHandle() const {
			return _handle;
		}

		bool IsValid() const;

		ThreadPriority GetPriority() const;
		bool SetPriority(ThreadPriority priority);
		CpuNumber GetIdealProcessor() const;
		bool Terminate(uint32_t exitCode = 0);
		int GetMemoryPriority() const;
		IoPriority GetIoPriority() const;
		size_t GetSubProcessTag() const;
		std::wstring GetServiceNameByTag(uint32_t pid);
		ComFlags GetComFlags() const;

	private:
		HANDLE _handle;
		bool _own;
	};

}
