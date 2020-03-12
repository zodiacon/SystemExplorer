#include "pch.h"
#include "Service.h"

using namespace WinSys;

Service::Service(wil::unique_schandle handle) noexcept : _handle(std::move(handle)) {}

bool Service::Start() {
	return Start(std::vector<const wchar_t*>());
}

bool Service::Start(const std::vector<const wchar_t*>& args) {
	return ::StartService(_handle.get(), static_cast<DWORD>(args.size()), const_cast<PCWSTR*>(args.data())) ? true : false;
}

bool Service::Stop() {
	SERVICE_STATUS status;
	return ::ControlService(_handle.get(), SERVICE_CONTROL_STOP, &status) ? true : false;
}

ServiceStatusProcess Service::GetStatus() const {
	ServiceStatusProcess status{};
	DWORD len;
	::QueryServiceStatusEx(_handle.get(), SC_STATUS_PROCESS_INFO, (BYTE*)&status, sizeof(status), &len);
	return status;
}

std::vector<ServiceTrigger> Service::GetTriggers() const {
	std::vector<ServiceTrigger> triggers;
	DWORD needed;
	::QueryServiceConfig2(_handle.get(), SERVICE_CONFIG_TRIGGER_INFO, nullptr, 0, &needed);
	if (::GetLastError() != ERROR_INSUFFICIENT_BUFFER)
		return triggers;

	auto buffer = std::make_unique<BYTE[]>(needed);
	if (!::QueryServiceConfig2(_handle.get(), SERVICE_CONFIG_TRIGGER_INFO, buffer.get(), needed, &needed))
		return triggers;

	auto info = reinterpret_cast<SERVICE_TRIGGER_INFO*>(buffer.get());
	triggers.reserve(info->cTriggers);
	for (DWORD i = 0; i < info->cTriggers; i++) {
		auto& tinfo = info->pTriggers[i];
		ServiceTrigger trigger;
		trigger.Type = static_cast<ServiceTriggerType>(tinfo.dwTriggerType);
		trigger.TriggerSubtype = tinfo.pTriggerSubtype;
		trigger.Action = static_cast<ServiceTriggerAction>(tinfo.dwAction);

		triggers.push_back(trigger);
	}

	return triggers;
}

std::vector<std::wstring> Service::GetRequiredPrivileges() const {
	std::vector<std::wstring> privileges;
	DWORD needed;
	::QueryServiceConfig2(_handle.get(), SERVICE_CONFIG_REQUIRED_PRIVILEGES_INFO, nullptr, 0, &needed);
	if (::GetLastError() != ERROR_INSUFFICIENT_BUFFER)
		return privileges;

	auto buffer = std::make_unique<BYTE[]>(needed);
	if (!::QueryServiceConfig2(_handle.get(), SERVICE_CONFIG_REQUIRED_PRIVILEGES_INFO, buffer.get(), needed, &needed))
		return privileges;

	auto info = reinterpret_cast<SERVICE_REQUIRED_PRIVILEGES_INFO*>(buffer.get());
	auto p = info->pmszRequiredPrivileges;
	while (p && *p) {
		privileges.emplace_back(p);
		p += ::wcslen(p) + 1;
	}
	return privileges;
}

std::unique_ptr<WinSys::Service> Service::Open(const std::wstring & name, ServiceAccessMask access) noexcept {
	auto hService = ServiceManager::OpenServiceHandle(name, access);
	if (!hService)
		return nullptr;

	return std::make_unique<WinSys::Service>(std::move(hService));
}

bool Service::Refresh(ServiceInfo& info) {
	DWORD bytes;
	return ::QueryServiceStatusEx(_handle.get(), SC_STATUS_PROCESS_INFO, (BYTE*)&info._status, sizeof(SERVICE_STATUS_PROCESS), &bytes);
}

bool Service::Pause() {
	SERVICE_STATUS status;
	return ::ControlService(_handle.get(), SERVICE_CONTROL_PAUSE, &status) ? true : false;
}

bool Service::Continue() {
	SERVICE_STATUS status;
	return ::ControlService(_handle.get(), SERVICE_CONTROL_CONTINUE, &status) ? true : false;
}
