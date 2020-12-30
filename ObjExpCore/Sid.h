#pragma once

#include <string>

namespace WinSys {
	class Sid final {
	public:
		Sid();
		explicit Sid(PSID sid);
		explicit Sid(const wchar_t* fromString);

		operator PSID() const;

		bool IsValid() const;

		std::wstring AsString() const;
		std::wstring UserName(PSID_NAME_USE use = nullptr) const;

	private:
		BYTE _buffer[SECURITY_MAX_SID_SIZE]{ };
	};
}
