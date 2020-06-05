#pragma once

enum class ProcessAttributes {
	NotComputed = -1,
	None = 0,
	Protected = 1,
	InJob = 2,
	Service = 4,
	Managed = 8,
	Secure = 0x10,
	Immersive = 0x20,
};
DEFINE_ENUM_FLAG_OPERATORS(ProcessAttributes);

class ProcessInfoEx {
public:
	ProcessInfoEx(WinSys::ProcessInfo* pi);
	ProcessAttributes GetAttributes(const WinSys::ProcessManager& pm) const;
	const std::wstring& GetExecutablePath() const;
	const std::wstring& UserName() const;
	int GetImageIndex(CImageList images) const;
	WinSys::IoPriority GetIoPriority() const;
	int GetMemoryPriority() const;
	WinSys::ProcessPriorityClass GetPriorityClass();
	const std::wstring& GetCommandLine() const;
	bool IsElevated() const;
	uint32_t GetGdiObjects() const;
	uint32_t GetUserObjects() const;
	uint32_t GetPeakGdiObjects() const;
	uint32_t GetPeakUserObjects() const;
	WinSys::IntegrityLevel GetIntegrityLevel() const;
	WinSys::VirtualizationState GetVirtualizationState() const;

	DWORD64 TargetTime;
	bool IsNew{ false };
	bool IsTerminated{ false };

private:
	std::unique_ptr<WinSys::Process> _process;
	WinSys::ProcessInfo* _pi;
	mutable int _image = -1;
	mutable ProcessAttributes _attributes = ProcessAttributes::NotComputed;
	mutable std::wstring _executablePath;
	mutable std::wstring _username;
	mutable std::wstring _commandLine;
	mutable bool _elevated : 1, _elevatedChecked : 1 {false};
};

