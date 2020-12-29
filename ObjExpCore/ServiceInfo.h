#pragma once

#include <string>
#include <winsvc.h>

namespace WinSys {
	enum class ServiceEnumType {
		AllDrivers = SERVICE_DRIVER,
		KernelDrivers = SERVICE_KERNEL_DRIVER,
		FileSystemDrivers = SERVICE_FILE_SYSTEM_DRIVER,
		AllServices = SERVICE_WIN32,
		ServicesOwnProcess = SERVICE_WIN32_OWN_PROCESS,
		ServicesSharedProcess = SERVICE_WIN32_OWN_PROCESS,
	};
	DEFINE_ENUM_FLAG_OPERATORS(ServiceEnumType);

	enum class ServiceEnumState {
		Active = SERVICE_ACTIVE,
		Inactive = SERVICE_INACTIVE,
		All = Active | Inactive
	};
	DEFINE_ENUM_FLAG_OPERATORS(ServiceEnumState);

	enum class ServiceType {
		FileSystemDriver = SERVICE_FILE_SYSTEM_DRIVER,
		KernelDriver = SERVICE_KERNEL_DRIVER,
		Win32OwnProcess = SERVICE_WIN32_OWN_PROCESS,
		Win32SharedProcess = SERVICE_WIN32_SHARE_PROCESS,
		InteractiveProcess = SERVICE_INTERACTIVE_PROCESS,
		UserService = SERVICE_USER_SERVICE,
		UserServiceInstance = SERVICE_USERSERVICE_INSTANCE,
		PackageService = SERVICE_PKG_SERVICE,
	};
	DEFINE_ENUM_FLAG_OPERATORS(ServiceType);

	enum class ServiceState {
		Unknown = 0,
		ContinuePending = SERVICE_CONTINUE_PENDING,
		PausePending = SERVICE_PAUSE_PENDING,
		Paused = SERVICE_PAUSED,
		Running = SERVICE_RUNNING,
		StartPending = SERVICE_START_PENDING,
		StopPending = SERVICE_STOP_PENDING,
		Stopped = SERVICE_STOPPED
	};

	enum class ServiceControlsAccepted {
		None = 0,
		NetBindChange = SERVICE_ACCEPT_NETBINDCHANGE,
		ParamChange = SERVICE_ACCEPT_PARAMCHANGE,
		PauseContinue = SERVICE_ACCEPT_PAUSE_CONTINUE,
		Preshutdown = SERVICE_ACCEPT_PRESHUTDOWN,
		Shutdown = SERVICE_ACCEPT_SHUTDOWN,
		Stop = SERVICE_ACCEPT_STOP,
		HardwareProfileChange = SERVICE_ACCEPT_HARDWAREPROFILECHANGE,
		PowerEvent = SERVICE_ACCEPT_POWEREVENT,
		SessionChange = SERVICE_ACCEPT_SESSIONCHANGE,
		TimeChanged = SERVICE_ACCEPT_TIMECHANGE,
		TriggerEvent = SERVICE_ACCEPT_TRIGGEREVENT,
		UserLogOff = SERVICE_ACCEPT_USER_LOGOFF,
		LowResources = SERVICE_ACCEPT_LOWRESOURCES,
		SystemLowResources = SERVICE_ACCEPT_SYSTEMLOWRESOURCES,
		InternalReserved = 0x1000,
	};
	DEFINE_ENUM_FLAG_OPERATORS(ServiceControlsAccepted);

	enum class ServiceFlags {
		None = 0,
		RunsInSystemProcess = SERVICE_RUNS_IN_SYSTEM_PROCESS
	};

	struct ServiceStatusProcess {
		ServiceType Type;
		ServiceState CurrentState;
		ServiceControlsAccepted ControlsAccepted;
		uint32_t Win32ExitCode;
		uint32_t ServiceSpecificExitCode;
		uint32_t CheckPoint;
		uint32_t WaitHint;
		uint32_t ProcessId;
		ServiceFlags Flags;
	};

	static_assert(sizeof(ServiceStatusProcess) == sizeof(SERVICE_STATUS_PROCESS));

	struct ServiceInfo {
		friend class ServiceManager;
		friend class Service;

		const std::wstring& GetName() const {
			return _name;
		}

		const std::wstring& GetDisplayName() const {
			return _displayName;
		}

		const ServiceStatusProcess& GetStatusProcess() const {
			return _status;
		}

	private:
		std::wstring _name, _displayName;
		ServiceStatusProcess _status;
	};
}
