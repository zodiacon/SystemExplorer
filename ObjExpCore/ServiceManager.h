#pragma once

#include <vector>
#include "ServiceInfo.h"
#include <memory>
#include <string>

namespace WinSys {
	struct ServiceInfo;

	enum class ServiceAccessMask : uint32_t {
		None = 0,
		AllAccess = SERVICE_ALL_ACCESS,
		QueryConfig = SERVICE_QUERY_CONFIG,
		ChangeConfig = SERVICE_CHANGE_CONFIG,
		QueryStatus = SERVICE_QUERY_STATUS,
		EnumerateDependents = SERVICE_ENUMERATE_DEPENDENTS,
		Start = SERVICE_START,
		Stop = SERVICE_STOP,
		PauseContinue = SERVICE_PAUSE_CONTINUE,
		Interrogate = SERVICE_INTERROGATE,
		UserDefinedControl = SERVICE_USER_DEFINED_CONTROL,
		Delete = DELETE
	};
	DEFINE_ENUM_FLAG_OPERATORS(ServiceAccessMask);

	enum class ServiceStartType {
		Boot = 0,
		System = 1,
		Auto = 2,
		Demand = 3,
		Disabled = 4,
	};

	enum class ServiceErrorControl {
		Ignore,
		Normal,
		Severe,
		Critical
	};

	struct ServiceConfiguration {
		ServiceType Type;
		ServiceStartType  StartType;
		ServiceErrorControl ErrorControl;
		std::wstring BinaryPathName;
		std::wstring LoadOrderGroup;
		uint32_t Tag;
		std::vector<std::wstring> Dependencies;
		std::wstring AccountName;
		std::wstring DisplayName;
		bool DelayedAutoStart;
		bool TriggerStart;
	};

	class ServiceManager final abstract {
		friend class Service;
	public:
		static std::vector<ServiceInfo> EnumServices(ServiceEnumType enumType = ServiceEnumType::All);
		static std::unique_ptr<ServiceConfiguration> GetServiceConfiguration(const std::wstring& serviceName);
		static std::wstring GetServiceDescription(const std::wstring& name);
		static ServiceState GetServiceState(const std::wstring& name);
		static ServiceStatusProcess GetServiceStatus(const std::wstring& name);
		static std::unique_ptr<Service> Install(const std::wstring& name, const std::wstring& displayName, ServiceAccessMask desiredAccess, ServiceType type, 
			ServiceStartType startType, ServiceErrorControl errorControl, const std::wstring& imagePath);

		static std::unique_ptr<Service> Install(const std::wstring& name, ServiceType type,	ServiceStartType startType, const std::wstring& imagePath);
		static bool Uninstall(const std::wstring& name);

	private:
		static wil::unique_schandle OpenServiceHandle(const std::wstring& name, ServiceAccessMask accessMask = ServiceAccessMask::QueryConfig | ServiceAccessMask::QueryStatus);
	};
}
