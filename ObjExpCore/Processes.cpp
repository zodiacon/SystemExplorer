#include "pch.h"
#include "Processes.h"
#include <Psapi.h>
#include <VersionHelpers.h>
#include <array>
#include "Thread.h"

using namespace WinSys;

enum class ProcessProtectionSigner : uint8_t {
	None = PsProtectedSignerNone,
	Authenticode = PsProtectedSignerAuthenticode,
	CodeGen = PsProtectedSignerCodeGen,
	Antimalware = PsProtectedSignerAntimalware,
	Lsa = PsProtectedSignerLsa,
	Windows = PsProtectedSignerWindows,
	WinTcb = PsProtectedSignerWinTcb,
	WinSystem = PsProtectedSignerWinSystem,
	App = PsProtectedSignerApp,
	Max = PsProtectedSignerMax
};

struct Process::Impl {
	HANDLE _handle;
	ProcessAccessMask _access;

	Impl(HANDLE handle, ProcessAccessMask access) : _handle(handle), _access(access) {}
	~Impl();

	bool GetExtendedInfo(PROCESS_EXTENDED_BASIC_INFORMATION* info) const;
	static std::unique_ptr<Process> OpenById(uint32_t pid, ProcessAccessMask access);
	std::wstring GetFullImageName() const;
	std::wstring GetCommandLine() const;
	std::wstring GetUserName() const;
	std::optional<ProcessProtection> GetProtection() const;
	bool Terminate(uint32_t exitCode = 0);
	bool Suspend();
	bool Resume();
	bool IsImmersive() const noexcept;
	bool IsProtected() const;
	bool IsSecure() const;
	bool IsInJob(HANDLE hJob = nullptr) const;
	bool IsWow64Process() const;
	bool IsManaged() const;

	uint32_t GetId() const {
		return ::GetProcessId(_handle);
	}

	std::wstring GetName() const {
		auto name = GetFullImageName();
		auto slash = name.rfind(L'\\');
		return slash == std::wstring::npos ? name : name.substr(slash + 1);
	}

	std::optional<ProcessWindowInfo> GetWindowInformation() const {
		auto buffer = std::make_unique<BYTE[]>(1024);
		std::optional<ProcessWindowInfo> info;
		ULONG len;
		if (!NT_SUCCESS(::NtQueryInformationProcess(_handle, ProcessWindowInformation, buffer.get(), 1024, &len)))
			return info;

		auto p = reinterpret_cast<PROCESS_WINDOW_INFORMATION*>(buffer.get());
		info->Flags = p->WindowFlags;
		info->Title = std::wstring(p->WindowTitle, p->WindowTitleLength);
		return info;
	}

	int GetMemoryPriority() const {
		int priority = -1;
		ULONG len;
		::NtQueryInformationProcess(_handle, ProcessPagePriority, &priority, sizeof(priority), &len);
		return priority;
	}

	IoPriority GetIoPriority() const {
		IoPriority priority = IoPriority::Unknown;
		ULONG len;
		::NtQueryInformationProcess(_handle, ProcessIoPriority, &priority, sizeof(priority), &len);
		return priority;
	}

	HANDLE GetHandle() const {
		return _handle;
	}

	bool IsElevated() const {
		wil::unique_handle hToken;
		if (!::OpenProcessToken(_handle, TOKEN_QUERY, hToken.addressof()))
			return false;

		TOKEN_ELEVATION elevation;
		DWORD size;
		if (!::GetTokenInformation(hToken.get(), TokenElevation, &elevation, sizeof(elevation), &size))
			return false;
		return elevation.TokenIsElevated ? true : false;
	}

	std::wstring GetWindowTitle() const {
		BYTE buffer[1024];
		ULONG len;
		auto status = ::NtQueryInformationProcess(_handle, ProcessWindowInformation, buffer, 1024, &len);
		if (!NT_SUCCESS(status))
			return L"";

		auto name = reinterpret_cast<PROCESS_WINDOW_INFORMATION*>(buffer);
		return std::wstring(name->WindowTitle, name->WindowTitleLength / sizeof(WCHAR));
	}

	IntegrityLevel GetIntegrityLevel() const {
		wil::unique_handle hProcessManaged;
		HANDLE hProcess = _handle;
		if ((_access & ProcessAccessMask::QueryInformation) == ProcessAccessMask::None) {
			if (!::DuplicateHandle(::GetCurrentProcess(), _handle, ::GetCurrentProcess(), hProcessManaged.addressof(), PROCESS_QUERY_INFORMATION, FALSE, 0))
				return IntegrityLevel::Unknown;
			hProcess = hProcessManaged.get();
		}

		wil::unique_handle hToken;
		if (!::OpenProcessToken(hProcess, TOKEN_QUERY, hToken.addressof()))
			return IntegrityLevel::Unknown;

		BYTE buffer[256];
		DWORD len;
		if (!::GetTokenInformation(hToken.get(), TokenIntegrityLevel, buffer, 256, &len))
			return IntegrityLevel::Unknown;

		auto integrity = reinterpret_cast<TOKEN_MANDATORY_LABEL*>(buffer);

		auto sid = integrity->Label.Sid;
		return (IntegrityLevel)(*::GetSidSubAuthority(sid, *::GetSidSubAuthorityCount(sid) - 1));
	}
};

std::unique_ptr<Process> Process::OpenById(uint32_t pid, ProcessAccessMask access) {
	return Impl::OpenById(pid, access);
}

std::unique_ptr<Process> WinSys::Process::GetCurrent() {
	return std::make_unique<Process>(NtCurrentProcess(), ProcessAccessMask::AllAccess);
}

std::wstring Process::GetFullImageName() const {
	return _impl->GetFullImageName();
}

std::wstring Process::GetCommandLine() const {
	return _impl->GetCommandLine();
}

std::wstring Process::GetUserName() const {
	return _impl->GetUserName();
}

std::wstring WinSys::Process::GetName() const {
	return _impl->GetName();
}

std::wstring WinSys::Process::GetWindowTitle() const {
	return _impl->GetWindowTitle();
}

std::optional<ProcessProtection> Process::GetProtection() const {
	return _impl->GetProtection();
}

bool Process::Terminate(uint32_t exitCode) {
	return _impl->Terminate(exitCode);
}

bool Process::Suspend() {
	return _impl->Suspend();
}

bool Process::Resume() {
	return _impl->Resume();
}

bool Process::IsImmersive() const noexcept {
	return _impl->IsImmersive();
}

bool Process::IsProtected() const {
	return _impl->IsProtected();
}

bool Process::IsSecure() const {
	return _impl->IsSecure();
}

bool Process::IsInJob(HANDLE hJob) const {
	return _impl->IsInJob(hJob);
}

bool Process::IsWow64Process() const {
	return _impl->IsWow64Process();
}

bool Process::IsManaged() const {
	return _impl->IsManaged();
}

bool WinSys::Process::IsElevated() const {
	return _impl->IsElevated();
}

IntegrityLevel Process::GetIntegrityLevel() const {
	return _impl->GetIntegrityLevel();
}

int Process::GetMemoryPriority() const {
	return _impl->GetMemoryPriority();
}

IoPriority Process::GetIoPriority() const {
	return _impl->GetIoPriority();
}

uint32_t WinSys::Process::GetId() const {
	return _impl->GetId();
}

HANDLE Process::GetHandle() const {
	return _impl->GetHandle();
}

std::optional<ProcessWindowInfo> WinSys::Process::GetWindowInformation() const {
	return _impl->GetWindowInformation();
}

Process::Impl::~Impl() {
	if (_handle != NtCurrentProcess())
		::CloseHandle(_handle);
}

Process::Process(HANDLE handle, ProcessAccessMask access) : _impl(std::make_unique<Impl>(handle, access)) {
}

Process::Process(Process && other) noexcept {
	_impl = std::move(other._impl);
}

Process & Process::operator=(Process && other) noexcept {
	if (this != &other) {
		_impl.reset();
		_impl = std::move(other._impl);
	}
	return *this;
}

bool Process::Impl::GetExtendedInfo(PROCESS_EXTENDED_BASIC_INFORMATION* info) const {
	ULONG len;
	info->Size = sizeof(*info);
	auto status = ::NtQueryInformationProcess(_handle, ProcessBasicInformation, info, sizeof(*info), &len);
	return NT_SUCCESS(status);
}

std::unique_ptr<Process> Process::Impl::OpenById(uint32_t pid, ProcessAccessMask access) {
	auto handle = ::OpenProcess(static_cast<ACCESS_MASK>(access), FALSE, pid);
	return handle ? std::make_unique<Process>(handle, access) : nullptr;
}

Process::~Process() = default;

std::wstring Process::Impl::GetFullImageName() const {
	DWORD size = MAX_PATH;
	WCHAR name[MAX_PATH];
	auto success = ::QueryFullProcessImageName(_handle, 0, name, &size);
	return success ? std::wstring(name) : L"";
}

std::wstring Process::Impl::GetCommandLine() const {
	ULONG size = 8192;
	auto buffer = std::make_unique<BYTE[]>(size);
	auto status = ::NtQueryInformationProcess(_handle, ProcessCommandLineInformation, buffer.get(), size, &size);
	if (NT_SUCCESS(status)) {
		auto str = (UNICODE_STRING*)buffer.get();
		return std::wstring(str->Buffer, str->Length / sizeof(WCHAR));
	}
	return L"";
}

std::wstring Process::Impl::GetUserName() const {
	wil::unique_handle hToken;
	if (!NT_SUCCESS(::NtOpenProcessToken(_handle, TOKEN_QUERY, hToken.addressof())))
		return L"";

	BYTE buffer[128];
	DWORD len;
	if (!::GetTokenInformation(hToken.get(), TokenUser, buffer, sizeof(buffer), &len))
		return L"";

	auto user = reinterpret_cast<TOKEN_USER*>(buffer);
	DWORD userMax = TOKEN_USER_MAX_SIZE;
	wchar_t name[TOKEN_USER_MAX_SIZE];
	DWORD domainMax = 64;
	wchar_t domain[64];
	SID_NAME_USE use;
	if (!::LookupAccountSid(nullptr, user->User.Sid, name, &userMax, domain, &domainMax, &use))
		return L"";

	return std::wstring(domain) + L"\\" + name;
}

std::optional<ProcessProtection> Process::Impl::GetProtection() const {
	ProcessProtection protection;
	ULONG len;
	auto status = ::NtQueryInformationProcess(_handle, ProcessProtectionInformation, &protection, sizeof(protection), &len);
	if (!NT_SUCCESS(status))
		return std::optional<ProcessProtection>();
	return protection;
}

bool Process::Impl::Terminate(uint32_t exitCode) {
	return NT_SUCCESS(NtTerminateProcess(_handle, exitCode));
}

bool Process::Impl::Suspend() {
	return NT_SUCCESS(::NtSuspendProcess(_handle));
}

bool Process::Impl::Resume() {
	return NT_SUCCESS(::NtResumeProcess(_handle));
}

bool Process::Impl::IsImmersive() const noexcept {
	return IsWindows8OrGreater() && ::IsImmersiveProcess(_handle) ? true : false;
}

bool Process::Impl::IsProtected() const {
	PROCESS_EXTENDED_BASIC_INFORMATION info;
	if (!GetExtendedInfo(&info))
		return false;

	return info.IsProtectedProcess ? true : false;
}

bool Process::Impl::IsSecure() const {
	PROCESS_EXTENDED_BASIC_INFORMATION info;
	if (!GetExtendedInfo(&info))
		return false;

	return info.IsSecureProcess ? true : false;
}

bool Process::Impl::IsInJob(HANDLE hJob) const {
	BOOL injob = FALSE;
	::IsProcessInJob(_handle, hJob, &injob);
	return injob ? true : false;
}

bool Process::Impl::IsWow64Process() const {
	PROCESS_EXTENDED_BASIC_INFORMATION info;
	if (!GetExtendedInfo(&info))
		return false;

	return info.IsWow64Process ? true : false;
}

bool Process::Impl::IsManaged() const {
	wil::unique_handle hProcess;
	if (!::DuplicateHandle(::GetCurrentProcess(), _handle, ::GetCurrentProcess(), hProcess.addressof(), PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, 0))
		return false;

	WCHAR filename[MAX_PATH], sysPath[MAX_PATH];
	BOOL wow64 = FALSE;
	::IsWow64Process(hProcess.get(), &wow64);
	::GetSystemDirectory(sysPath, MAX_PATH);
	::wcscat_s(sysPath, L"\\mscoree.dll");

	HMODULE hModule[64];
	DWORD needed;
	if (!::EnumProcessModulesEx(hProcess.get(), hModule, sizeof(hModule), &needed, wow64 ? LIST_MODULES_32BIT : LIST_MODULES_ALL))
		return false;

	int count = min(_countof(hModule), needed / sizeof(HMODULE));

	for (int i = 0; i < count; i++) {
		if (::GetModuleFileNameEx(hProcess.get(), hModule[i], filename, MAX_PATH) == 0)
			continue;
		if (::_wcsicmp(filename, sysPath) == 0)
			return true;
	}
	return false;
}

