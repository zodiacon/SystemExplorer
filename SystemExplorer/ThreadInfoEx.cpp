#include "pch.h"
#include "ThreadInfoEx.h"
#include "DriverHelper.h"

WinSys::ComFlags ThreadInfoEx::GetComFlags() const {
	if (IsTerminating || IsTerminated)
		return WinSys::ComFlags::None;
	auto t = OpenThread();
	return t ? t->GetComFlags() : WinSys::ComFlags::Error;
}

int ThreadInfoEx::GetMemoryPriority() const {
	auto t = OpenThread();
	return t ? t->GetMemoryPriority() : -1;
}

WinSys::IoPriority ThreadInfoEx::GetIoPriority() const {
	auto t = OpenThread();
	return t ? t->GetIoPriority() : WinSys::IoPriority::Unknown;
}

WinSys::Thread* ThreadInfoEx::OpenThread() const {
	if (_thread == nullptr) {
		auto hThread = DriverHelper::OpenThread(Info->Id, THREAD_QUERY_INFORMATION);
		if (hThread)
			_thread = std::make_unique<WinSys::Thread>(hThread, true);
	}
	return _thread.get();
}
