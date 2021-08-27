#include "pch.h"
#include "Sid.h"
#include <sddl.h>

using namespace WinSys;

Sid::Sid() {
	::memset(_buffer, 0, sizeof(_buffer));
}

Sid::Sid(PSID sid) {
	::CopySid(sizeof(_buffer), (PSID)*this, sid);
}

Sid::Sid(const wchar_t* fromString) {
	PSID sid;
	if(::ConvertStringSidToSid(fromString, &sid))
		::CopySid(sizeof(_buffer), (PSID)*this, sid);
}

Sid::operator PSID() const {
	return (PSID)_buffer;
}

bool Sid::IsValid() const {
	return ::IsValidSid((PSID)*this);
}

std::wstring Sid::AsString() const {
	PWSTR str;
	std::wstring result;
	if (::ConvertSidToStringSid((PSID)*this, &str)) {
		result = str;
		::LocalFree(str);
	}
	return result;
}

std::wstring Sid::UserName(PSID_NAME_USE use) const {
	WCHAR name[64], domain[64];
	DWORD lname = _countof(name), ldomain = _countof(domain);
	std::wstring username;
	SID_NAME_USE dummy;
	if (use == nullptr)
		use = &dummy;
	if (::LookupAccountSid(nullptr, (PSID)_buffer, name, &lname, domain, &ldomain, use))
		return std::wstring(domain) + L"\\" + name;
	return L"";
}
