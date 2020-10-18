#pragma once

struct ThreadInfoEx final {
	explicit ThreadInfoEx(WinSys::ThreadInfo* ti) : Info(ti) {}
	DWORD64 TargetTime{ 0 };
	bool IsNew{ false };
	bool IsTerminating{ false };
	bool IsTerminated{ false };
	WinSys::ThreadInfo* Info;
	WinSys::ComFlags GetComFlags() const;
	int GetMemoryPriority() const;
	WinSys::IoPriority GetIoPriority() const;

private:
	WinSys::Thread* OpenThread() const;
	mutable std::unique_ptr<WinSys::Thread> _thread;
};
