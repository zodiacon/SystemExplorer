#include "pch.h"
#include "ServiceManager.h"
#include "Service.h"
//#include "subprocesstag.h"

using namespace WinSys;

std::vector<ServiceInfo> ServiceManager::EnumServices(ServiceEnumType enumType) {
	std::vector<ServiceInfo> services;
	wil::unique_schandle hScm(::OpenSCManager(nullptr, nullptr, SC_MANAGER_ENUMERATE_SERVICE));
	if (!hScm)
		return services;

	auto buffer = std::make_unique<BYTE[]>(1 << 18);
	DWORD needed;
	DWORD count;
	auto ok = ::EnumServicesStatusEx(hScm.get(), SC_ENUM_PROCESS_INFO, SERVICE_WIN32,
		static_cast<DWORD>(enumType), buffer.get(), 1 << 18, &needed, &count, nullptr, nullptr);
	if (!ok)
		return services;

//	auto QueryTagInformation = (PQUERY_TAG_INFORMATION)::GetProcAddress(::GetModuleHandle(L"advapi32"), "I_QueryTagInformation");
	services.reserve(count);
	for (size_t i = 0; i < count; i++) {
		auto data = (ENUM_SERVICE_STATUS_PROCESS*)buffer.get() + i;
		ServiceInfo svc;
		svc._name = data->lpServiceName;
		svc._displayName = data->lpDisplayName;
		::memcpy(&svc._status, &data->ServiceStatusProcess, sizeof(SERVICE_STATUS_PROCESS));

		services.push_back(std::move(svc));
	}
	return services;
}

std::unique_ptr<ServiceConfiguration> ServiceManager::GetServiceConfiguration(const std::wstring & serviceName) {
	auto hService(OpenServiceHandle(serviceName));
	if (!hService)
		return nullptr;

	DWORD needed = 0;
	::QueryServiceConfig(hService.get(), nullptr, 0, &needed);
	if (needed == 0)
		return nullptr;

	auto buffer = std::make_unique<BYTE[]>(needed);
	auto config = reinterpret_cast<QUERY_SERVICE_CONFIG*>(buffer.get());
	if (!::QueryServiceConfig(hService.get(), config, needed, &needed))
		return nullptr;

	auto result = std::make_unique<ServiceConfiguration>();
	result->AccountName = config->lpServiceStartName;
	result->DisplayName = config->lpDisplayName;
	result->Tag = config->dwTagId;
	result->StartType = static_cast<ServiceStartType>(config->dwStartType);
	if (result->StartType == ServiceStartType::Auto) {
		// check if delayed auto start
		SERVICE_DELAYED_AUTO_START_INFO info;
		DWORD len;
		auto ok = ::QueryServiceConfig2(hService.get(), SERVICE_CONFIG_DELAYED_AUTO_START_INFO, (BYTE*)&info, sizeof(info), &len);
		if (ok)
			result->DelayedAutoStart = info.fDelayedAutostart ? true : false;
		else
			result->DelayedAutoStart = false;
	}

	// check if trigger start
	{
		BYTE buffer[1 << 10];
		DWORD len;
		auto ok = ::QueryServiceConfig2(hService.get(), SERVICE_CONFIG_TRIGGER_INFO, buffer, 1024, &len);
		if (ok) {
			auto& info = (SERVICE_TRIGGER_INFO&)*buffer;
			result->TriggerStart = info.cTriggers > 0;
		}
	}

	result->Type = static_cast<ServiceType>(config->dwServiceType);
	result->BinaryPathName = config->lpBinaryPathName;
	auto s = config->lpDependencies;
	while (s && *s) {
		result->Dependencies.emplace_back(s);
		s = s + ::wcslen(s) + 1;
	}
	result->LoadOrderGroup = config->lpLoadOrderGroup;

	return result;
}

std::wstring WinSys::ServiceManager::GetServiceDescription(const std::wstring & name) {
	wil::unique_schandle hService(OpenServiceHandle(name));
	if (!hService)
		return L"";
	BYTE buffer[1024];
	DWORD len;
	if (!::QueryServiceConfig2(hService.get(), SERVICE_CONFIG_DESCRIPTION, buffer, sizeof(buffer), &len))
		return L"";
	auto desc = (SERVICE_DESCRIPTION*)buffer;
	if (desc->lpDescription == nullptr)
		return L"";
	return desc->lpDescription;
}

wil::unique_schandle ServiceManager::OpenServiceHandle(const std::wstring & name, ServiceAccessMask access) {
	wil::unique_schandle hScm(::OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS));
	if (!hScm)
		return nullptr;

	wil::unique_schandle hService(::OpenService(hScm.get(), name.c_str(), static_cast<ACCESS_MASK>(access)));
	return hService;
}

ServiceState ServiceManager::GetServiceState(const std::wstring& name) {
	wil::unique_schandle hService(OpenServiceHandle(name, ServiceAccessMask::QueryStatus));
	if (!hService)
		return ServiceState::Unknown;

	SERVICE_STATUS_PROCESS status;
	DWORD len;
	if (!::QueryServiceStatusEx(hService.get(), SC_STATUS_PROCESS_INFO, (BYTE*)&status, sizeof(status), &len))
		return ServiceState::Unknown;

	return static_cast<ServiceState>(status.dwCurrentState);
}

ServiceStatusProcess ServiceManager::GetServiceStatus(const std::wstring& name) {
	ServiceStatusProcess status{};
	wil::unique_schandle hService(OpenServiceHandle(name, ServiceAccessMask::QueryStatus));
	if (!hService)
		return status;

	DWORD len;
	::QueryServiceStatusEx(hService.get(), SC_STATUS_PROCESS_INFO, (BYTE*)&status, sizeof(status), &len);

	return status;
}

std::unique_ptr<WinSys::Service> ServiceManager::Install(const std::wstring& name, const std::wstring& displayName, ServiceAccessMask desiredAccess, ServiceType type,
	ServiceStartType startType, ServiceErrorControl errorControl, const std::wstring& imagePath) {

	wil::unique_schandle hScm(::OpenSCManager(nullptr, nullptr, SC_MANAGER_CREATE_SERVICE));
	if (!hScm)
		return nullptr;

	wil::unique_schandle hService(::CreateService(hScm.get(), name.c_str(), displayName.c_str(), static_cast<DWORD>(desiredAccess), static_cast<DWORD>(type), static_cast<DWORD>(startType),
		static_cast<DWORD>(errorControl), imagePath.c_str(), nullptr, nullptr, nullptr, nullptr, nullptr));
	if (!hService)
		return nullptr;

	return std::make_unique<WinSys::Service>(std::move(hService));
}

std::unique_ptr<WinSys::Service> ServiceManager::Install(const std::wstring& name, ServiceType type, ServiceStartType startType, const std::wstring& imagePath) {
	return Install(name, L"", ServiceAccessMask::AllAccess, type, startType, ServiceErrorControl::Normal, imagePath);
}

bool WinSys::ServiceManager::Uninstall(const std::wstring& name) {
	auto svc = OpenServiceHandle(name, ServiceAccessMask::Delete);
	if (!svc)
		return false;
	return ::DeleteService(svc.get());
}

