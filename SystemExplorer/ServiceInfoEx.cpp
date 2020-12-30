#include "pch.h"
#include "ServiceInfoEx.h"
#include "ServicesView.h"

using namespace WinSys;

ServiceInfoEx::ServiceInfoEx(PCWSTR name) : _name(name) {
}

const WinSys::ServiceConfiguration* ServiceInfoEx::GetConfiguration() const {
	if (_config == nullptr)
		_config = ServiceManager::GetServiceConfiguration(_name);
	return _config.get();
}

const CString& ServiceInfoEx::GetDescription() const {
	if (_desc.IsEmpty()) {
		_desc = WinSys::ServiceManager::GetServiceDescription(_name).c_str();
	}
	return _desc;
}

const CString& ServiceInfoEx::GetPrivileges() const {
	if (!_flagPriveleges) {
		auto svc = Service::Open(_name, ServiceAccessMask::QueryConfig);
		if (svc) {
			auto privs = svc->GetRequiredPrivileges();
			CString text;
			for (auto& priv : privs) {
				if (!text.IsEmpty())
					text += L", ";
				text += priv.substr(2, priv.size() - 11).c_str();
			}
			_privileges = std::move(text);
		}
		_flagPriveleges = true;
	}
	return _privileges;
}

const CString& ServiceInfoEx::GetTriggers() const {
	if (!_flagTriggers) {
		_flagTriggers = true;
		auto svc = Service::Open(_name, ServiceAccessMask::QueryConfig);
		if (svc) {
			auto triggers = svc->GetTriggers();
			CString text;
			for (auto& trigger : triggers) {
				auto desc = CServicesView::TriggerToText(trigger);
				if (text.Find(desc) < 0) {
					if (!text.IsEmpty())
						text += L", ";
					text += desc;
				}
			}
			_triggers = std::move(text);
		}
	}
	return _triggers;
}

const CString& ServiceInfoEx::GetDependencies() const {
	if (!_flagDependencies) {
		_flagDependencies = true;
		if (GetConfiguration())
			_dependencies = CServicesView::DependenciesToString(GetConfiguration()->Dependencies);
	}
	return _dependencies;
}

const CString& ServiceInfoEx::GetSID() const {
	if (_sid.IsEmpty()) {
		_sid = ServiceManager::GetServiceSid(_name.c_str()).AsString().c_str();
	}
	return _sid;
}

WinSys::ServiceSidType ServiceInfoEx::GetSidType() const {
	auto svc = Service::Open(_name, ServiceAccessMask::QueryConfig);
	ServiceSidType type = ServiceSidType::Unknown;
	if (svc)
		type = svc->GetSidType();
	return type;
}

