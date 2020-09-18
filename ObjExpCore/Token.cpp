#include "pch.h"
#include "Token.h"
#include <assert.h>

using namespace WinSys;

Token::Token(HANDLE hToken) : _handle(hToken) {
}

Token::Token(DWORD pid, TokenAccessMask access) {
	wil::unique_handle hProcess(::OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid));
	if (hProcess) {
		::OpenProcessToken(hProcess.get(), static_cast<DWORD>(access), _handle.addressof());
	}
}

Token::Token(HANDLE hProcess, TokenAccessMask access) {
	::OpenProcessToken(hProcess, static_cast<DWORD>(access), _handle.addressof());
}

std::unique_ptr<Token> Token::Open(DWORD pid, TokenAccessMask access) {
	HANDLE hToken;
	wil::unique_handle hProcess(::OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid));
	if (hProcess) {
		if (::OpenProcessToken(hProcess.get(), static_cast<DWORD>(access), &hToken))
			return std::make_unique<Token>(hToken);
	}
	return nullptr;
}

std::pair<std::wstring, Sid> WinSys::Token::GetUserNameAndSid() const {
	assert(_handle);
	BYTE buffer[256];
	DWORD len;
	if (::GetTokenInformation(_handle.get(), TokenUser, buffer, sizeof(buffer), &len)) {
		auto data = (TOKEN_USER*)buffer;
		Sid sid(data->User.Sid);
		WCHAR name[64], domain[64];
		DWORD lname = _countof(name), ldomain = _countof(domain);
		SID_NAME_USE use;
		std::wstring username;
		if (::LookupAccountSid(nullptr, sid, name, &lname, domain, &ldomain, &use))
			username = std::wstring(domain) + L"\\" + name;
		return { username, sid };
	}
	return { L"", Sid() };
}

bool Token::IsValid() const {
	return _handle != nullptr;
}

bool Token::IsElevated() const {
	ULONG elevated = 0;
	DWORD len;
	::GetTokenInformation(_handle.get(), TokenElevation, &elevated, sizeof(elevated), &len);
	return elevated ? true : false;
}

VirtualizationState Token::GetVirtualizationState() const {
	ULONG virt = 0;
	DWORD len;
	if (!::GetTokenInformation(_handle.get(), TokenVirtualizationAllowed, &virt, sizeof(virt), &len))
		return VirtualizationState::Unknown;

	if (!virt)
		return VirtualizationState::NotAllowed;

	if (::GetTokenInformation(_handle.get(), TokenVirtualizationEnabled, &virt, sizeof(virt), &len))
		return virt ? VirtualizationState::Enabled : VirtualizationState::Disabled;

	return VirtualizationState::Unknown;
}

IntegrityLevel Token::GetIntegrityLevel() const {
	BYTE buffer[TOKEN_INTEGRITY_LEVEL_MAX_SIZE];
	DWORD len;
	if (!::GetTokenInformation(_handle.get(), TokenIntegrityLevel, buffer, sizeof(buffer), &len))
		return IntegrityLevel::Error;

	auto p = (TOKEN_MANDATORY_LABEL*)buffer;
	return (IntegrityLevel)*GetSidSubAuthority(p->Label.Sid, *GetSidSubAuthorityCount(p->Label.Sid) - 1);
}

DWORD Token::GetSessionId() const {
	DWORD id = 0, len;
	::GetTokenInformation(_handle.get(), TokenSessionId, &id, sizeof(id), &len);
	return id;
}

TOKEN_STATISTICS Token::GetStats() const {
	TOKEN_STATISTICS stats{};
	DWORD len;
	::GetTokenInformation(_handle.get(), TokenStatistics, &stats, sizeof(stats), &len);
	return stats;
}

std::vector<TokenGroup> WinSys::Token::EnumGroups(bool caps) const {
	std::vector<TokenGroup> groups;
	BYTE buffer[1 << 13];
	DWORD len;
	if (!::GetTokenInformation(_handle.get(), caps ? TokenCapabilities : TokenGroups, buffer, sizeof(buffer), &len))
		return groups;

	auto data = (TOKEN_GROUPS*)buffer;
	groups.reserve(data->GroupCount);
	for (ULONG i = 0; i < data->GroupCount; i++) {
		auto const& g = data->Groups[i];
		TokenGroup group;
		Sid sid(g.Sid);
		group.Sid = sid.AsString();
		group.Name = sid.UserName(&group.Use);
		if (group.Name.empty()) {
			group.Name = group.Sid;
			group.Use = SidTypeUnknown;
		}
		group.Attributes = (SidGroupAttributes)g.Attributes;
		groups.push_back(std::move(group));
	}
	return groups;
}

std::vector<TokenPrivilege> Token::EnumPrivileges() const {
	std::vector<TokenPrivilege> privs;
	BYTE buffer[1 << 13];
	DWORD len;
	if (!::GetTokenInformation(_handle.get(), TokenPrivileges, buffer, sizeof(buffer), &len))
		return privs;

	auto data = (TOKEN_PRIVILEGES*)buffer;
	auto count = data->PrivilegeCount;
	privs.reserve(count);

	WCHAR name[64];
	for (ULONG i = 0; i < count; i++) {
		TokenPrivilege priv;
		auto& p = data->Privileges[i];
		priv.Privilege = p.Luid;
		len = _countof(name);
		if (::LookupPrivilegeName(nullptr, &p.Luid, name, &len))
			priv.Name = name;
		priv.Attributes = p.Attributes;
		privs.push_back(std::move(priv));
	}
	return privs;
}

bool Token::EnablePrivilege(PCWSTR privName, bool enable) {
	wil::unique_handle hToken;
	if (!::DuplicateHandle(::GetCurrentProcess(), _handle.get(), ::GetCurrentProcess(), hToken.addressof(), 
		TOKEN_ADJUST_PRIVILEGES, FALSE, 0))
		return false;

	bool result = false;
	TOKEN_PRIVILEGES tp;
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Attributes = enable ? SE_PRIVILEGE_ENABLED : 0;
	if (::LookupPrivilegeValue(nullptr, privName,
		&tp.Privileges[0].Luid)) {
		if (::AdjustTokenPrivileges(hToken.get(), FALSE, &tp, sizeof(tp),
			nullptr, nullptr))
			result = ::GetLastError() == ERROR_SUCCESS;
	}
	return result;
}

