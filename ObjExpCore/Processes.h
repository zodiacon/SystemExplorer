#pragma once

#include <memory>
#include <string>
#include <optional>
#include <vector>
#include <memory>
#include "Thread.h"

namespace WinSys {
	enum class ProcessAccessMask : uint32_t {
		None = 0,
		AllAccess = PROCESS_ALL_ACCESS,
		CreateThread = PROCESS_CREATE_THREAD,
		Read = GENERIC_READ,
		Synchonize = SYNCHRONIZE,
		DuplicateHandle = PROCESS_DUP_HANDLE,
		VmRead = PROCESS_VM_READ,
		VmWrite = PROCESS_VM_WRITE,
		VmOperation = PROCESS_VM_OPERATION,
		Terminate = PROCESS_TERMINATE,
		QueryInformation = PROCESS_QUERY_INFORMATION,
		QueryLimitedInformation = PROCESS_QUERY_LIMITED_INFORMATION,
		SuspendResume = PROCESS_SUSPEND_RESUME
	};
	DEFINE_ENUM_FLAG_OPERATORS(ProcessAccessMask);

	enum class ProcessPriorityClass {
		Normal = NORMAL_PRIORITY_CLASS,
		BelowNormal = BELOW_NORMAL_PRIORITY_CLASS,
		AboveNormal = ABOVE_NORMAL_PRIORITY_CLASS,
		Idle = IDLE_PRIORITY_CLASS,
		High = HIGH_PRIORITY_CLASS,
		Realtime = REALTIME_PRIORITY_CLASS,
		Unknown = 0,
	};

	enum class IntegrityLevel : uint32_t {
		Untrusted = 0,
		Low = SECURITY_MANDATORY_LOW_RID,
		Medium = SECURITY_MANDATORY_MEDIUM_RID,
		MediumPlus = SECURITY_MANDATORY_MEDIUM_PLUS_RID,
		High = SECURITY_MANDATORY_HIGH_RID,
		System = SECURITY_MANDATORY_SYSTEM_RID,
		Protected = SECURITY_MANDATORY_PROTECTED_PROCESS_RID,
		Error = 0xffffffff,
	};

	enum class ProtectedProcessSigner : uint8_t;
	struct ProcessHandleInfo;

	enum class ProcessMitigationPolicy {
		DEPPolicy,
		ASLRPolicy,
		DynamicCodePolicy,
		StrictHandleCheckPolicy,
		SystemCallDisablePolicy,
		MitigationOptionsMask,
		ExtensionPointDisablePolicy,
		ControlFlowGuardPolicy,
		SignaturePolicy,
		FontDisablePolicy,
		ImageLoadPolicy,
		SystemCallFilterPolicy,
		PayloadRestrictionPolicy,
		ChildProcessPolicy,
		SideChannelIsolationPolicy,
	};

	struct ProcessProtection {
		union {
			uint8_t Level;
			struct {
				uint8_t Type : 3;
				uint8_t Audit : 1;
				ProtectedProcessSigner : 4;
			};
		};
	};

	struct ProcessWindowInfo {
		uint32_t Flags;
		std::wstring Title;
	};

	class Process {
	public:
		static std::unique_ptr<Process> OpenById(uint32_t pid, ProcessAccessMask access = ProcessAccessMask::QueryLimitedInformation);
		static std::unique_ptr<Process> GetCurrent();
		Process(HANDLE handle);
		~Process();

		bool IsValid() const;

		std::wstring GetFullImageName() const;
		std::wstring GetCommandLine() const;
		std::wstring GetUserName() const;
		std::wstring GetName() const;
		std::wstring GetWindowTitle() const;

		std::optional<ProcessProtection> GetProtection() const;
		bool Terminate(uint32_t exitCode = 0);
		bool Suspend();
		bool Resume();
		bool IsImmersive() const noexcept;
		bool IsProtected() const;
		bool IsSecure() const;
		bool IsInJob(HANDLE hJob = nullptr) const;
		bool IsWow64Process() const;
		bool IsManaged() const;
		bool IsElevated() const;
		IntegrityLevel GetIntegrityLevel() const;
		int GetMemoryPriority() const;
		IoPriority GetIoPriority() const;
		ProcessPriorityClass GetPriorityClass() const;
		bool SetPriorityClass(ProcessPriorityClass pc);
		uint32_t GetGdiObjectCount() const;
		uint32_t GetPeakGdiObjectCount() const;
		uint32_t GetUserObjectCount() const;
		uint32_t GetPeakUserObjectCount() const;

		uint32_t GetId() const;

		HANDLE GetHandle() const;
		std::optional<ProcessWindowInfo> GetWindowInformation() const;

		std::vector<std::shared_ptr<ProcessHandleInfo>> EnumHandles();

	private:
		wil::unique_handle _handle;
	};

}