#pragma once

#include <memory>
#include <vector>
#include "Keys.h"

namespace WinSys {
	struct ProcessInfo;
	struct ThreadInfo;

	class ProcessManager {
	public:
		ProcessManager();
		~ProcessManager();
		ProcessManager(const ProcessManager&) = delete;
		ProcessManager& operator=(const ProcessManager&) = delete;

		size_t EnumProcesses();
		size_t EnumProcessesAndThreads(uint32_t pid = 0);

		[[nodiscard]] std::vector<std::shared_ptr<ProcessInfo>>& GetProcesses();
		[[nodiscard]] const std::vector<std::shared_ptr<ProcessInfo>>& GetProcesses() const;

		[[nodiscard]] std::vector<std::shared_ptr<ThreadInfo>>& GetThreads();
		[[nodiscard]] const std::vector<std::shared_ptr<ThreadInfo>>& GetThreads() const;

		[[nodiscard]] std::shared_ptr<ProcessInfo> GetProcessInfo(int index) const;
		[[nodiscard]] std::shared_ptr<ProcessInfo> GetProcessById(uint32_t pid) const;
		[[nodiscard]] std::shared_ptr<ProcessInfo> GetProcessByKey(const ProcessOrThreadKey& key) const;
		[[nodiscard]] const std::vector<std::shared_ptr<ProcessInfo>>& GetTerminatedProcesses() const;
		[[nodiscard]] const std::vector<std::shared_ptr<ProcessInfo>>& GetNewProcesses() const;

		[[nodiscard]] std::shared_ptr<ThreadInfo> GetThreadInfo(int index) const;
		[[nodiscard]] std::shared_ptr<ThreadInfo> GetThreadByKey(const ProcessOrThreadKey& key) const;
		[[nodiscard]] const std::vector<std::shared_ptr<ThreadInfo>>& GetTerminatedThreads() const;
		[[nodiscard]] const std::vector<std::shared_ptr<ThreadInfo>>& GetNewThreads() const;

		[[nodiscard]] size_t GetThreadCount() const;
		[[nodiscard]] size_t GetProcessCount() const;
		[[nodiscard]] std::wstring GetProcessNameById(uint32_t pid) const;

	private:
		struct Impl;
		std::unique_ptr<Impl> _impl;
	};
}
