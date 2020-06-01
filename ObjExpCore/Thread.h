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

	class Thread {
	public:
		static std::unique_ptr<Thread> OpenById(uint32_t tid, ThreadAccessMask accessMask = ThreadAccessMask::QueryInformation);
		Thread(HANDLE handle, ThreadAccessMask);

		HANDLE GetHandle() const {
			return _handle.get();
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

	private:
		wil::unique_handle _handle;
		ThreadAccessMask _access;
	};

}
