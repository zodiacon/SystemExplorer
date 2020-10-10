#pragma once

namespace WinSys {
	enum class SecurityLogonType {
		UndefinedLogonType,
		Interactive = 2,
		Network,
		Batch,
		Service,
		Proxy,
		Unlock,
		NetworkCleartext,
		NewCredentials,
		RemoteInteractive,
		CachedInteractive,
		CachedRemoteInteractive,
		CachedUnlock
	};

	struct LogonSessionData {
		ULONG         Index;
		LUID          LogonId;
		std::wstring  UserName;
		std::wstring  LogonDomain;
		std::wstring  AuthenticationPackage;
		SecurityLogonType LogonType;
		ULONG         Session;
		BYTE          Sid[SECURITY_MAX_SID_SIZE];
		LARGE_INTEGER LogonTime;

		std::wstring  LogonServer;
		std::wstring  DnsDomainName;
		std::wstring  Upn;
		ULONG         UserFlags;
		LARGE_INTEGER LastSuccessfulLogon;
		LARGE_INTEGER LastFailedLogon;
		ULONG FailedAttemptCountSinceLastSuccessfulLogon;

		std::wstring  LogonScript;
		std::wstring  ProfilePath;
		std::wstring  HomeDirectory;
		std::wstring  HomeDirectoryDrive;

		LARGE_INTEGER LogoffTime;
		LARGE_INTEGER KickOffTime;
		LARGE_INTEGER PasswordLastSet;
		LARGE_INTEGER PasswordCanChange;
		LARGE_INTEGER PasswordMustChange;
	};

	class LsaSecurity abstract final {
	public:
		static std::vector<LogonSessionData> EnumLogonSessions();

	};
}

