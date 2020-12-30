#pragma once

#include <ServiceManager.h>
#include <Service.h>
#include <Sid.h>

struct ServiceInfoEx {
	ServiceInfoEx(PCWSTR name);
	const WinSys::ServiceConfiguration* GetConfiguration() const;
	const CString& GetDescription() const;
	const CString& GetPrivileges() const;
	const CString& GetTriggers() const;
	const CString& GetDependencies() const;
	const CString& GetSID() const;
	WinSys::ServiceSidType GetSidType() const;

private:
	mutable std::unique_ptr<WinSys::ServiceConfiguration> _config;
	std::wstring _name;
	mutable CString _desc, _privileges, _triggers, _dependencies;
	mutable bool _flagPriveleges{ false };
	mutable bool _flagTriggers{ false };
	mutable bool _flagDependencies{ false };
	mutable CString _sid;
};
