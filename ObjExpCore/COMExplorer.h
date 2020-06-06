#pragma once

#include <vector>
#include <memory>
#include <string>

namespace WinSys {
	enum class ComStore {
		Default,
		User,
		Machine,

		Default32 = 0x10,
		User32,
		Machine32,
	};

	struct ComClassInfo {
		std::wstring FriendlyName;
		CLSID Clsid;
		std::wstring ModulePath;
		std::wstring ThreadingModel;
		bool InProcess;
	};

	class ComExplorer final {
	public:
		ComExplorer();
		~ComExplorer();
		bool Open(ComStore store, bool readOnly = true);

	private:
		struct Impl;
		std::unique_ptr<Impl> _impl;
	};
}

