#pragma once

#include "Sid.h"

namespace WinSys {
	enum class TokenAccessMask : uint32_t {
		Query = TOKEN_QUERY,
		QuerySource = TOKEN_QUERY_SOURCE,
		AssignPrimary = TOKEN_ASSIGN_PRIMARY,
		AdjustPrivileges = TOKEN_ADJUST_PRIVILEGES,
		AdjustDefault = TOKEN_ADJUST_DEFAULT,
		AdjustGroups = TOKEN_ADJUST_GROUPS,
		AdjustSessionId = TOKEN_ADJUST_SESSIONID,
	};

	class Token final {
	public:
		Token(HANDLE hToken);
		Token(DWORD pid, TokenAccessMask access = TokenAccessMask::Query);
		static std::unique_ptr<Token> Open(DWORD pid, TokenAccessMask access = TokenAccessMask::Query);

		std::pair<std::wstring, Sid> GetUserNameAndSid() const;

		bool IsValid() const;

	private:
		wil::unique_handle _handle;
	};
}

