#include "pch.h"
#include "Processes.h"
#include <Psapi.h>
#include <VersionHelpers.h>
#include <array>
#include "Thread.h"

using namespace WinSys;

namespace WinSys {
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
}

bool GetExtendedInfo(HANDLE hProcess, PROCESS_EXTENDED_BASIC_INFORMATION* info) {
	ULONG len;
	info->Size = sizeof(*info);
	auto status = ::NtQueryInformationProcess(hProcess, ProcessBasicInformation, info, sizeof(*info), &len);
	return NT_SUCCESS(status);
}

uint32_t Process::GetId() const {
	return ::GetProcessId(_handle.get());
}

std::wstring Process::GetName() const {
	auto name = GetFullImageName();
	auto slash = name.rfind(L'\\');
	return slash == std::wstring::npos ? name : name.substr(slash + 1);
}

std::optional<ProcessWindowInfo> Process::GetWindowInformation() const {
	auto buffer = std::make_unique<BYTE[]>(1024);
	std::optional<ProcessWindowInfo> info;
	ULONG len;
	if (!NT_SUCCESS(::NtQueryInformationProcess(_handle.get(), ProcessWindowInformation, buffer.get(), 1024, &len)))
		return info;

	auto p = reinterpret_cast<PROCESS_WINDOW_INFORMATION*>(buffer.get());
	info->Flags = p->WindowFlags;
	info->Title = std::wstring(p->WindowTitle, p->WindowTitleLength);
	return info;
}

int Process::GetMemoryPriority() const {
	int priority = -1;
	ULONG len;
	::NtQueryInformationProcess(_handle.get(), ProcessPagePriority, &priority, sizeof(priority), &len);
	return priority;
}

IoPriority Process::GetIoPriority() const {
	IoPriority priority = IoPriority::Unknown;
	ULONG len;
	::NtQueryInformationProcess(_handle.get(), ProcessIoPriority, &priority, sizeof(priority), &len);
	return priority;
}

HANDLE Process::GetHandle() const {
	return _handle.get();
}

bool Process::IsElevated() const {
	wil::unique_handle hToken;
	if (!::OpenProcessToken(_handle.get(), TOKEN_QUERY, hToken.addressof()))
		return false;

	TOKEN_ELEVATION elevation;
	DWORD size;
	if (!::GetTokenInformation(hToken.get(), TokenElevation, &elevation, sizeof(elevation), &size))
		return false;
	return elevation.TokenIsElevated ? true : false;
}

std::wstring Process::GetWindowTitle() const {
	BYTE buffer[1024];
	ULONG len;
	auto status = ::NtQueryInformationProcess(_handle.get(), ProcessWindowInformation, buffer, 1024, &len);
	if (!NT_SUCCESS(status))
		return L"";

	auto name = reinterpret_cast<PROCESS_WINDOW_INFORMATION*>(buffer);
	return std::wstring(name->WindowTitle, name->WindowTitleLength / sizeof(WCHAR));
}

IntegrityLevel Process::GetIntegrityLevel() const {
	wil::unique_handle hToken;
	if (!::OpenProcessToken(_handle.get(), TOKEN_QUERY, hToken.addressof()))
		return IntegrityLevel::Error;

	BYTE buffer[256];
	DWORD len;
	if (!::GetTokenInformation(hToken.get(), TokenIntegrityLevel, buffer, 256, &len))
		return IntegrityLevel::Error;

	auto integrity = reinterpret_cast<TOKEN_MANDATORY_LABEL*>(buffer);

	auto sid = integrity->Label.Sid;
	return (IntegrityLevel)(*::GetSidSubAuthority(sid, *::GetSidSubAuthorityCount(sid) - 1));
}


std::unique_ptr<Process> WinSys::Process::GetCurrent() {
	return std::make_unique<Process>(NtCurrentProcess());
}

Process::Process(HANDLE handle) : _handle(handle) {
}

std::unique_ptr<Process> Process::OpenById(uint32_t pid, ProcessAccessMask access) {
	auto handle = ::OpenProcess(static_cast<ACCESS_MASK>(access), FALSE, pid);
	return handle ? std::make_unique<Process>(handle) : nullptr;
}

Process::~Process() = default;

bool WinSys::Process::IsValid() const {
	return _handle != nullptr;
}

std::wstring Process::GetFullImageName() const {
	DWORD size = MAX_PATH;
	WCHAR name[MAX_PATH];
	auto success = ::QueryFullProcessImageName(_handle.get(), 0, name, &size);
	return success ? std::wstring(name) : L"";
}

std::wstring Process::GetCommandLine() const {
	ULONG size = 8192;
	auto buffer = std::make_unique<BYTE[]>(size);
	auto status = ::NtQueryInformationProcess(_handle.get(), ProcessCommandLineInformation, buffer.get(), size, &size);
	if (NT_SUCCESS(status)) {
		auto str = (UNICODE_STRING*)buffer.get();
		return std::wstring(str->Buffer, str->Length / sizeof(WCHAR));
	}
	return L"";
}

std::wstring Process::GetUserName() const {
	wil::unique_handle hToken;
	if (!::OpenProcessToken(_handle.get(), TOKEN_QUERY, hToken.addressof()))
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

std::optional<ProcessProtection> Process::GetProtection() const {
	ProcessProtection protection;
	ULONG len;
	auto status = ::NtQueryInformationProcess(_handle.get(), ProcessProtectionInformation, &protection, sizeof(protection), &len);
	if (!NT_SUCCESS(status))
		return std::optional<ProcessProtection>();
	return protection;
}

bool Process::Terminate(uint32_t exitCode) {
	return NT_SUCCESS(NtTerminateProcess(_handle.get(), exitCode));
}

bool Process::Suspend() {
	return NT_SUCCESS(::NtSuspendProcess(_handle.get()));
}

bool Process::Resume() {
	return NT_SUCCESS(::NtResumeProcess(_handle.get()));
}

bool Process::IsImmersive() const noexcept {
	return IsWindows8OrGreater() && ::IsImmersiveProcess(_handle.get()) ? true : false;
}

bool Process::IsProtected() const {
	PROCESS_EXTENDED_BASIC_INFORMATION info;
	if (!GetExtendedInfo(GetHandle(), &info))
		return false;

	return info.IsProtectedProcess ? true : false;
}

bool Process::IsSecure() const {
	PROCESS_EXTENDED_BASIC_INFORMATION info;
	if (!GetExtendedInfo(GetHandle(), &info))
		return false;

	return info.IsSecureProcess ? true : false;
}

bool Process::IsInJob(HANDLE hJob) const {
	BOOL injob = FALSE;
	::IsProcessInJob(_handle.get(), hJob, &injob);
	return injob ? true : false;
}

bool Process::IsWow64Process() const {
	PROCESS_EXTENDED_BASIC_INFORMATION info;
	if (!GetExtendedInfo(GetHandle(), &info))
		return false;

	return info.IsWow64Process ? true : false;
}

bool Process::IsManaged() const {
	wil::unique_handle hProcess;
	if (!::DuplicateHandle(::GetCurrentProcess(), _handle.get(), ::GetCurrentProcess(), hProcess.addressof(),
		PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, 0))
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

WinSys::ProcessPriorityClass Process::GetPriorityClass() const {
	return static_cast<ProcessPriorityClass>(::GetPriorityClass(_handle.get()));
}

bool WinSys::Process::SetPriorityClass(ProcessPriorityClass pc) {
	return ::SetPriorityClass(_handle.get(), static_cast<DWORD>(pc));
}

uint32_t WinSys::Process::GetGdiObjectCount() const {
	return ::GetGuiResources(_handle.get(), GR_GDIOBJECTS);
}

uint32_t WinSys::Process::GetPeakGdiObjectCount() const {
	return ::GetGuiResources(_handle.get(), GR_GDIOBJECTS_PEAK);
}

uint32_t WinSys::Process::GetUserObjectCount() const {
	return ::GetGuiResources(_handle.get(), GR_USEROBJECTS);
}

uint32_t WinSys::Process::GetPeakUserObjectCount() const {
	return ::GetGuiResources(_handle.get(), GR_USEROBJECTS_PEAK);
}
