#define _HAS_EXCEPTIONS 0
#include <atlbase.h>
#include "ComExplorer.h"

using namespace ATL;
using namespace WinSys;

struct ComExplorer::Impl {
	CRegKey _root;

	bool Open(ComStore store, bool readOnly) {
		_root.Close();
		auto success = false;
		switch (store) {
			case ComStore::Default:
				_root.Attach(HKEY_CLASSES_ROOT);
				success = true;
				break;

			case ComStore::User:
				success = ERROR_SUCCESS == _root.Open(HKEY_CURRENT_USER, L"Software\\Classes", KEY_READ);
				break;

			case ComStore::Machine:
				success = ERROR_SUCCESS == _root.Open(HKEY_LOCAL_MACHINE, L"Software\\Classes", KEY_READ);
				break;

			case ComStore::Default32:
				success = ERROR_SUCCESS == _root.Open(HKEY_CLASSES_ROOT, L"Wow6432Node", KEY_READ);
				break;

			case ComStore::User32:
				success = ERROR_SUCCESS == _root.Open(HKEY_CURRENT_USER, L"Software\\Classes\\Wow6432Node", KEY_READ);
				break;

			case ComStore::Machine32:
				success = ERROR_SUCCESS == _root.Open(HKEY_LOCAL_MACHINE, L"Software\\Classes\\Wow6432Node", KEY_READ);
				break;
		}
		return success;
	}

	std::vector<ComClassInfo> EnumClasses(uint32_t start = 0, uint32_t maxCount = 0) {
		std::vector<ComClassInfo> classes;
		CRegKey hClasses;
		if (hClasses.Open(_root, L"CLSID", KEY_READ) != ERROR_SUCCESS)
			return classes;

		DWORD subkeys = 0;
		::RegQueryInfoKey(hClasses.m_hKey, nullptr, nullptr, nullptr, &subkeys, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);

		classes.reserve(subkeys);

		WCHAR name[MAX_PATH * 2];
		for (DWORD i = start; (maxCount == 0 && i < subkeys) || classes.size() >= maxCount; i++) {
			DWORD len = _countof(name);
			if (ERROR_SUCCESS != hClasses.EnumKey(i, name, &len))
				break;

			ComClassInfo info{};
			if (FAILED(::CLSIDFromString(name, &info.Clsid)))
				continue;

			CRegKey cls;
			if (ERROR_SUCCESS == cls.Open(hClasses, name, KEY_READ)) {
				len = _countof(name);
				if (ERROR_SUCCESS == cls.QueryStringValue(L"", name, &len))
					info.FriendlyName = name;
				len = _countof(name);
				if (ERROR_SUCCESS == cls.QueryStringValue(L"AppID", name, &len))
					::CLSIDFromString(name, &info.AppId);
			}

			if (cls) {
				CRegKey proc;
				len = _countof(name);
				if (ERROR_SUCCESS == proc.Open(cls, L"LocalServer32")) {
					info.ServerType = ComServerType::OutOfProc;
				}
				else if (ERROR_SUCCESS == proc.Open(cls, L"InProcServer32")) {
					info.ServerType = ComServerType::InProc;
					len = _countof(name);
					if (ERROR_SUCCESS == proc.QueryStringValue(L"ThreadingModel", name, &len))
						info.ThreadingModel = name;
				}
				else if (ERROR_SUCCESS == cls.QueryStringValue(L"LocalService", name, &len)) {
					info.ServerType = ComServerType::Service;
					info.ModulePath = name;
				}
				if (proc) {
					len = _countof(name);
					if (ERROR_SUCCESS == proc.QueryStringValue(L"", name, &len))
						info.ModulePath = name;
				}
			}
			classes.push_back(std::move(info));
		}
		return classes;
	}

	std::vector<ComInterfaceInfo> EnumInterfaces(uint32_t start, uint32_t maxCount) {
		std::vector<ComInterfaceInfo> interfaces;
		CRegKey hInterfaces;
		if (hInterfaces.Open(_root, L"Interface", KEY_READ) != ERROR_SUCCESS)
			return interfaces;

		DWORD subkeys = 0;
		::RegQueryInfoKey(hInterfaces.m_hKey, nullptr, nullptr, nullptr, &subkeys, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);

		interfaces.reserve(subkeys);

		WCHAR name[MAX_PATH * 2];
		for (DWORD i = start; (maxCount == 0 && i < subkeys) || interfaces.size() >= maxCount; i++) {
			DWORD len = _countof(name);
			if (ERROR_SUCCESS != hInterfaces.EnumKey(i, name, &len))
				break;

			ComInterfaceInfo info{};
			if (FAILED(::CLSIDFromString(name, &info.Iid)))
				continue;

			CRegKey iface;
			if (ERROR_SUCCESS == iface.Open(hInterfaces, name, KEY_READ)) {
				len = _countof(name);
				if (ERROR_SUCCESS == iface.QueryStringValue(L"", name, &len))
					info.FriendlyName = name;
			}

			CRegKey ps;
			if (ERROR_SUCCESS == ps.Open(iface, L"ProxyStubClsid32", KEY_READ)) {
				len = _countof(name);
				if (ERROR_SUCCESS == ps.QueryStringValue(L"", name, &len))
					::CLSIDFromString(name, &info.ProxyStub);
				ps.Close();
				if (ERROR_SUCCESS == ps.Open(iface, L"TypeLib", KEY_READ)) {
					len = _countof(name);
					if (ERROR_SUCCESS == ps.QueryStringValue(L"", name, &len))
						::CLSIDFromString(name, &info.TypeLib);
				}
			}
			interfaces.push_back(std::move(info));
		}
		return interfaces;
	}

	std::vector<ComTypeLibInfo> EnumTypeLibraries() {
		std::vector<ComTypeLibInfo> libs;
		CRegKey hTypeLibs;
		if (ERROR_SUCCESS != hTypeLibs.Open(_root, L"Typelib", KEY_READ))
			return libs;

		DWORD subkeys = 0;
		::RegQueryInfoKey(hTypeLibs.m_hKey, nullptr, nullptr, nullptr, &subkeys, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);

		libs.reserve(subkeys);
		WCHAR name[MAX_PATH * 2];

		for (DWORD i = 0; i < subkeys; i++) {
			DWORD len = _countof(name);
			if (ERROR_SUCCESS != hTypeLibs.EnumKey(i, name, &len))
				break;

			ComTypeLibInfo info{};
			::CLSIDFromString(name, &info.TypeLibId);
			CRegKey hLib;
			if (ERROR_SUCCESS == hLib.Open(hTypeLibs, name, KEY_READ)) {
				// assume just one version
				len = _countof(name);
				if (hLib.EnumKey(0, name, &len) == ERROR_SUCCESS) {
					CRegKey key;
					if (key.Open(hLib, (std::wstring(name) + L"\\0\\win32").c_str(), KEY_READ) == ERROR_SUCCESS) {
						len = _countof(name);
						if (ERROR_SUCCESS == key.QueryStringValue(nullptr, name, &len))
							info.Win32Path = name;
						key.Close();
					}
					if (key.Open(hLib, (std::wstring(name) + L"\\0\\win64").c_str(), KEY_READ) == ERROR_SUCCESS) {
						len = _countof(name);
						if (ERROR_SUCCESS == key.QueryStringValue(nullptr, name, &len))
							info.Win64Path = name;
					}
				}
			}
			libs.push_back(info);
		}
		return libs;
	}
};

ComExplorer::ComExplorer() : _impl(new Impl) {
}

ComExplorer::~ComExplorer() = default;

bool ComExplorer::Open(ComStore store, bool readOnly) {
	return _impl->Open(store, readOnly);
}

std::vector<ComClassInfo> ComExplorer::EnumClasses(uint32_t start, uint32_t maxCount) {
	return _impl->EnumClasses(start, maxCount);
}

std::vector<ComInterfaceInfo> ComExplorer::EnumInterfaces(uint32_t start, uint32_t maxCount) {
	return _impl->EnumInterfaces(start, maxCount);
}

std::vector<ComTypeLibInfo> ComExplorer::EnumTypeLibraries() {
	return _impl->EnumTypeLibraries();
}

