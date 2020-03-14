#include "pch.h"
#include "ProcessInfo.h"

using namespace WinSys;

ProcessInfo::ProcessInfo() {
}

const std::vector<std::shared_ptr<ThreadInfo>>& ProcessInfo::GetThreads() const {
	return _threads;
}

void ProcessInfo::AddThread(std::shared_ptr<ThreadInfo> thread) {
	_threads.push_back(thread);
}

void ProcessInfo::ClearThreads() {
	_threads.clear();
}
