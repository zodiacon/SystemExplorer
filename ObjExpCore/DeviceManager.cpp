#include "pch.h"
#include "DeviceManager.h"
#include <assert.h>
#include <newdev.h>

#pragma comment(lib, "setupapi")

using namespace WinSys;

SP_CLASSIMAGELIST_DATA g_ClassImageList;

std::unique_ptr<DeviceManager> DeviceManager::Create(const wchar_t* computerName, const GUID* classGuid, const wchar_t* enumerator, InfoSetOptions options) {
	auto dm = new DeviceManager(computerName, classGuid, enumerator, options);
	if (dm->_hInfoSet)
		return std::unique_ptr<DeviceManager>(dm);
	delete dm;
	return nullptr;
}

std::wstring WinSys::DeviceManager::GetDeviceClassDescription(const GUID* guid, const wchar_t* computerName) {
	wchar_t desc[256];
	if (::SetupDiGetClassDescriptionEx(guid, desc, _countof(desc), nullptr, computerName, nullptr)) {
		return desc;
	}
	return L"";
}

HIMAGELIST WinSys::DeviceManager::GetClassImageList() {
	if (g_ClassImageList.ImageList == nullptr) {
		g_ClassImageList.cbSize = sizeof(g_ClassImageList);
		::SetupDiGetClassImageList(&g_ClassImageList);
	}
	return g_ClassImageList.ImageList;
}

int WinSys::DeviceManager::GetClassImageIndex(const GUID* guid) {
	int index = -1;
	::SetupDiGetClassImageIndex(&g_ClassImageList, guid, &index);
	return index;
}

std::vector<DEVPROPKEY> DeviceManager::GetClassPropertyKeys(const GUID* guid) {
	std::vector<DEVPROPKEY> keys;
	DWORD count = 0;
	::SetupDiGetClassPropertyKeys(guid, nullptr, 0, &count, DICLASSPROP_INSTALLER);
	if (count) {
		keys.resize(count);
		::SetupDiGetClassPropertyKeys(guid, keys.data(), count, nullptr, DICLASSPROP_INSTALLER);
	}
	return keys;
}

std::wstring WinSys::DeviceManager::GetDeviceRegistryPropertyString(const DeviceInfo& di, DeviceRegistryPropertyType type) const {
	std::wstring result;
	result.resize(256);

	DWORD regType;
	if (::SetupDiGetDeviceRegistryProperty(_hInfoSet.get(), (PSP_DEVINFO_DATA)&di.Data, static_cast<DWORD>(type), &regType,
		(BYTE*)result.data(), DWORD(result.size() * sizeof(wchar_t)), nullptr)) {
		assert(regType == REG_SZ);
		return result;
	}
	return L"";
}

std::vector<std::wstring> WinSys::DeviceManager::GetDeviceRegistryPropertyMultiString(const DeviceInfo& di, DeviceRegistryPropertyType type) const {
	std::vector<std::wstring> result;
	WCHAR buffer[1 << 11];
	DWORD regType;
	auto ok = ::SetupDiGetDeviceRegistryProperty(_hInfoSet.get(), (PSP_DEVINFO_DATA)&di.Data, static_cast<DWORD>(type), &regType, (BYTE*)buffer, sizeof(buffer), nullptr);
	if (!ok)
		return result;

	assert(regType == REG_MULTI_SZ);
	for (auto p = buffer; *p;) {
		result.push_back(p);
		p += ::wcslen(p) + 1;
	}
	return result;
}

HICON WinSys::DeviceManager::GetDeviceIcon(const DeviceInfo& di, bool big) const {
	HICON hIcon = nullptr;
	auto size = big ? 32 : 16;
	::SetupDiLoadDeviceIcon(_hInfoSet.get(), (PSP_DEVINFO_DATA)&di.Data, size, size, 0, &hIcon);
	return hIcon;
}

std::wstring WinSys::DeviceManager::GetDeviceClassRegistryPropertyString(const GUID* guid, DeviceClassRegistryPropertyType type) {
	DWORD regType;
	std::wstring result;
	result.resize(256);
	if (::SetupDiGetClassRegistryProperty(guid, static_cast<DWORD>(type), &regType, 
		(BYTE*)result.data(), DWORD(result.size() * sizeof(WCHAR)), nullptr, nullptr, nullptr)) {
		assert(regType == REG_SZ);
		return result;
	}

	return L"";
}

std::vector<std::wstring> WinSys::DeviceManager::GetDeviceClassRegistryPropertyMultiString(const GUID* guid, DeviceClassRegistryPropertyType type) {
	std::vector<std::wstring> result;
	WCHAR buffer[1 << 12];
	DWORD regType;
	if (::SetupDiGetClassRegistryProperty(guid, static_cast<DWORD>(type), &regType, (BYTE*)buffer, sizeof(buffer), nullptr, nullptr, nullptr)) {
		assert(regType == REG_MULTI_SZ);
		for (auto p = buffer; *p;) {
			result.push_back(p);
			p += ::wcslen(p) + 1;
		}
	}
	return result;
}

DeviceManager::DeviceManager(const wchar_t* computerName, const GUID* classGuid, const wchar_t* enumerator, InfoSetOptions options) {
	_hInfoSet.reset(::SetupDiGetClassDevsEx(classGuid, enumerator, nullptr, static_cast<DWORD>(options), nullptr, computerName, nullptr));
}

std::vector<DeviceInfo> DeviceManager::EnumDevices() {
	std::vector<DeviceInfo> devices;
	SP_DEVINFO_DATA data = { sizeof(data) };
	wchar_t name[512];

	for (DWORD i = 0; ; i++) {
		if (!::SetupDiEnumDeviceInfo(_hInfoSet.get(), i, &data))
			break;

		DeviceInfo di;
		di.Data = data;
		if (::SetupDiGetDeviceRegistryProperty(_hInfoSet.get(), &data, SPDRP_FRIENDLYNAME, nullptr, (BYTE*)name, sizeof(name), nullptr)) {
			di.Description = name;
		}
		if (di.Description.empty()) {
			if (::SetupDiGetDeviceRegistryProperty(_hInfoSet.get(), &data, SPDRP_DEVICEDESC, nullptr, (BYTE*)name, sizeof(name), nullptr)) {
				di.Description = name;
			}
		}
		devices.push_back(std::move(di));
	}
	return devices;
}
