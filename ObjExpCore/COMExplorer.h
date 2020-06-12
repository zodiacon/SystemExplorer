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

	enum class ComServerType {
		InProc,
		OutOfProc,
		Service
	};

	struct ComClassInfo {
		std::wstring FriendlyName;
		CLSID Clsid;
		std::wstring ModulePath;
		std::wstring ThreadingModel;
		ComServerType ServerType;
	};

	struct ComInterfaceInfo {
		std::wstring FriendlyName;
		IID Iid;
		CLSID ProxyStub;
		GUID TypeLib;
		int NumMethods;
	};

	class ComExplorer final {
	public:
		ComExplorer();
		~ComExplorer();
		bool Open(ComStore store, bool readOnly = true);
		std::vector<ComClassInfo> EnumClasses(uint32_t start = 0, uint32_t maxCount = 0);
		std::vector<ComInterfaceInfo> EnumInterfaces(uint32_t start = 0, uint32_t maxCount = 0);

	private:
		struct Impl;
		std::unique_ptr<Impl> _impl;
	};
}

