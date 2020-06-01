#include "pch.h"
#include "Sid.h"
#include <sddl.h>

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
	if (::ConvertSidToStringSid((PSID)*this, &str))
		return str;
	return L"";
}
