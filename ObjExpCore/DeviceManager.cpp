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

//
// The string properties are read into buffers sized from what the property takes, and the strings are cut to their
// real length: they were read into fixed 256 character strings, which kept that length (with embedded NULs) and
// came back empty when the value was longer.
//
static std::wstring StringFromBuffer(std::vector<WCHAR>& buffer) {
	buffer.back() = 0;
	return std::wstring(buffer.data());
}

static std::vector<std::wstring> MultiStringFromBuffer(std::vector<WCHAR>& buffer) {
	std::vector<std::wstring> result;
	// the buffer ends with (at least) two NULs, however the value itself ends
	buffer[buffer.size() - 1] = buffer[buffer.size() - 2] = 0;
	for (auto p = buffer.data(); p < buffer.data() + buffer.size() && *p; ) {
		result.push_back(p);
		p += ::wcslen(p) + 1;
	}
	return result;
}

static std::vector<WCHAR> ReadDeviceProperty(HDEVINFO hInfoSet, PSP_DEVINFO_DATA data, DWORD type) {
	DWORD size = 0;
	::SetupDiGetDeviceRegistryProperty(hInfoSet, data, type, nullptr, nullptr, 0, &size);
	if (size == 0)
		return {};
	std::vector<WCHAR> buffer(size / sizeof(WCHAR) + 2);
	if (!::SetupDiGetDeviceRegistryProperty(hInfoSet, data, type, nullptr, (BYTE*)buffer.data(), DWORD(buffer.size() * sizeof(WCHAR)), nullptr))
		return {};
	return buffer;
}

static std::vector<WCHAR> ReadClassProperty(const GUID* guid, DWORD type) {
	DWORD size = 0;
	::SetupDiGetClassRegistryProperty(guid, type, nullptr, nullptr, 0, &size, nullptr, nullptr);
	if (size == 0)
		return {};
	std::vector<WCHAR> buffer(size / sizeof(WCHAR) + 2);
	if (!::SetupDiGetClassRegistryProperty(guid, type, nullptr, (BYTE*)buffer.data(), DWORD(buffer.size() * sizeof(WCHAR)), nullptr, nullptr, nullptr))
		return {};
	return buffer;
}

std::wstring WinSys::DeviceManager::GetDeviceRegistryPropertyString(const DeviceInfo& di, DeviceRegistryPropertyType type) const {
	auto buffer = ReadDeviceProperty(_hInfoSet.get(), (PSP_DEVINFO_DATA)&di.Data, static_cast<DWORD>(type));
	return buffer.empty() ? L"" : StringFromBuffer(buffer);
}

std::vector<std::wstring> WinSys::DeviceManager::GetDeviceRegistryPropertyMultiString(const DeviceInfo& di, DeviceRegistryPropertyType type) const {
	auto buffer = ReadDeviceProperty(_hInfoSet.get(), (PSP_DEVINFO_DATA)&di.Data, static_cast<DWORD>(type));
	return buffer.empty() ? std::vector<std::wstring>() : MultiStringFromBuffer(buffer);
}

HICON WinSys::DeviceManager::GetDeviceIcon(const DeviceInfo& di, bool big) const {
	HICON hIcon = nullptr;
	auto size = big ? 32 : 16;
	::SetupDiLoadDeviceIcon(_hInfoSet.get(), (PSP_DEVINFO_DATA)&di.Data, size, size, 0, &hIcon);
	return hIcon;
}

std::wstring WinSys::DeviceManager::GetDeviceClassRegistryPropertyString(const GUID* guid, DeviceClassRegistryPropertyType type) {
	auto buffer = ReadClassProperty(guid, static_cast<DWORD>(type));
	return buffer.empty() ? L"" : StringFromBuffer(buffer);
}

std::vector<std::wstring> WinSys::DeviceManager::GetDeviceClassRegistryPropertyMultiString(const GUID* guid, DeviceClassRegistryPropertyType type) {
	auto buffer = ReadClassProperty(guid, static_cast<DWORD>(type));
	return buffer.empty() ? std::vector<std::wstring>() : MultiStringFromBuffer(buffer);
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
