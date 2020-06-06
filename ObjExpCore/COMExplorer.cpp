#include <atlbase.h>
#include "ComExplorer.h"

using namespace WinSys;
using namespace ATL;

struct ComExplorer::Impl {
	CRegKey _root;

	bool Open(ComStore store, bool readOnly) {
		auto success = false;
		switch (store) {
			case ComStore::Default:
				_root.Attach(HKEY_CLASSES_ROOT);
				break;

			case ComStore::User:
				success = ERROR_SUCCESS == _root.Open(HKEY_CURRENT_USER, L"Software\\Classes", KEY_READ);
				break;

			case ComStore::Machine:
				success = ERROR_SUCCESS == _root.Open(HKEY_LOCAL_MACHINE, L"Software\\Classes", KEY_READ);
				break;
		}
		return success;
	}

	std::vector<ComClassInfo> EnumClasses(uint32_t start = 0, uint32_t maxCount = 0) {
		std::vector<ComClassInfo> classes;
		CRegKey hClasses;
		if (hClasses.Open(_root, L"CLSID", KEY_READ) != ERROR_SUCCESS)
			return classes;

		classes.reserve(1024);
		
		WCHAR name[MAX_PATH * 2];
		for (DWORD i = start; maxCount == 0 || classes.size() >= maxCount; i++) {
			DWORD len = _countof(name);
			if (ERROR_SUCCESS != hClasses.EnumKey(i, name, &len))
				break;

			ComClassInfo info;
			if (FAILED(::CLSIDFromString(name, &info.Clsid)))
				continue;

			CRegKey cls;
			if (ERROR_SUCCESS == cls.Open(hClasses, name)) {
				len = _countof(name);
				if (ERROR_SUCCESS == cls.QueryStringValue(L"", name, &len))
					info.FriendlyName = name;
			}

			CRegKey proc;
			if (ERROR_SUCCESS == proc.Open(cls, L"InProcServer32")) {
				info.InProcess = true;
				len = _countof(name);
				if (ERROR_SUCCESS == proc.QueryStringValue(L"ThreadingModel", name, &len))
					info.ThreadingModel = name;
			}
			else if (ERROR_SUCCESS == proc.Open(cls, L"LocalService32")) {
				info.InProcess = false;
			}
			else {
				continue;
			}
			len = _countof(name);
			if (ERROR_SUCCESS == proc.QueryStringValue(L"", name, &len))
				info.ModulePath = name;

			classes.push_back(std::move(info));
		}
		return classes;
	}
};

ComExplorer::ComExplorer() : _impl(new Impl) {
}

ComExplorer::~ComExplorer() = default;

bool ComExplorer::Open(ComStore store, bool readOnly) {
	return _impl->Open(store, readOnly);
}
