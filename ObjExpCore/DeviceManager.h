#pragma once

#include <cfgmgr32.h>

namespace wil {
	using unique_hinfoset = unique_any_handle_invalid<decltype(&::SetupDiDestroyDeviceInfoList), ::SetupDiDestroyDeviceInfoList>;
}

template<>
struct std::hash<GUID> {
	size_t operator()(const GUID& key) const {
		return key.Data1 ^ ((key.Data3 << 16) | key.Data2);
	}
};

namespace WinSys {
	enum class DeviceClassRegistryPropertyType {
		Characteristics = SPCRP_CHARACTERISTICS,
		DeviceType = SPCRP_DEVTYPE,
		UpperFilters = SPCRP_UPPERFILTERS,
		LowerFilters = SPCRP_LOWERFILTERS,
		Exclusive = SPCRP_EXCLUSIVE,
		SecurityDescriptor = SPCRP_SECURITY,
		SecurityDescriptionString = SPCRP_SECURITY_SDS
	};

	enum class DeviceRegistryPropertyType {
		Description = SPDRP_DEVICEDESC,
		HardwareId = SPDRP_HARDWAREID,
		CompatibleIds = SPDRP_COMPATIBLEIDS,
		Service = SPDRP_SERVICE,
		Class = SPDRP_CLASS,
		ClassGuid = SPDRP_CLASSGUID,
		Driver = SPDRP_DRIVER,
		ConfigFlags = SPDRP_CONFIGFLAGS,
		Mfg = SPDRP_MFG,
		FriendlyName = SPDRP_FRIENDLYNAME,
		UpperFilters = SPDRP_UPPERFILTERS,
		LowerFilters = SPDRP_LOWERFILTERS,
		Capabilities = SPDRP_CAPABILITIES,
		PdoName = SPDRP_PHYSICAL_DEVICE_OBJECT_NAME,
		Enumerator = SPDRP_ENUMERATOR_NAME,
		Location = SPDRP_LOCATION_INFORMATION,
		UINumber = SPDRP_UI_NUMBER,
		BusTypeGuid = SPDRP_BUSTYPEGUID,
		Exclusive = SPDRP_EXCLUSIVE,
		DeviceType = SPDRP_DEVTYPE,
		BusNumber = SPDRP_BUSNUMBER,
		Address = SPDRP_ADDRESS,
		Characteristics = SPDRP_CHARACTERISTICS,
		LegacyBusType = SPDRP_LEGACYBUSTYPE,
		InstallState = SPDRP_INSTALL_STATE,
		PowerData = SPDRP_DEVICE_POWER_DATA,
		SecurityDescriptor = SPDRP_SECURITY,
		SecurityDescriptorString = SPDRP_SECURITY_SDS,
		LocationPaths = SPDRP_LOCATION_PATHS,
		UINumberDescFormat = SPDRP_UI_NUMBER_DESC_FORMAT,
		RemovalPolicy = SPDRP_REMOVAL_POLICY,
		RemovalPolicyHwDefault = SPDRP_REMOVAL_POLICY_HW_DEFAULT,
		RemovalPolicyOverride = SPDRP_REMOVAL_POLICY_OVERRIDE,
		BaseContainerId = SPDRP_BASE_CONTAINERID
	};

	enum class InfoSetOptions {
		AllClasses = DIGCF_ALLCLASSES,
		Profile = DIGCF_PROFILE,
		Present = DIGCF_PRESENT,
		Default = DIGCF_DEFAULT,
		DeviceInterface = DIGCF_DEVICEINTERFACE
	};
	DEFINE_ENUM_FLAG_OPERATORS(WinSys::InfoSetOptions);

	struct DeviceInfo {
		std::wstring Description;
		SP_DEVINFO_DATA Data;
	};


	class DeviceManager final {
	public:
		static std::unique_ptr<DeviceManager> Create(const wchar_t* computerName = nullptr, const GUID* classGuid = nullptr, const wchar_t* enumerator = nullptr,
			InfoSetOptions options = InfoSetOptions::Present | InfoSetOptions::AllClasses);

		std::vector<DeviceInfo> EnumDevices();
		static std::wstring GetDeviceClassDescription(const GUID* guid, const wchar_t* computerName = nullptr);
		static HIMAGELIST GetClassImageList();
		static int GetClassImageIndex(const GUID* guid);
		static std::vector<DEVPROPKEY> GetClassPropertyKeys(const GUID* guid);

		// device
		std::wstring GetDeviceRegistryPropertyString(const DeviceInfo& di, DeviceRegistryPropertyType type) const;
		std::vector<std::wstring> GetDeviceRegistryPropertyMultiString(const DeviceInfo& di, DeviceRegistryPropertyType type) const;
		template<typename T>
		T GetDeviceRegistryProperty(const DeviceInfo& di, DeviceRegistryPropertyType type) const;
		HICON GetDeviceIcon(const DeviceInfo& di, bool big = false) const;

		// device class
		static std::wstring GetDeviceClassRegistryPropertyString(const GUID* guid, DeviceClassRegistryPropertyType type);
		static std::vector<std::wstring> GetDeviceClassRegistryPropertyMultiString(const GUID* guid, DeviceClassRegistryPropertyType type);
		
		template<typename T>
		static T GetDeviceClassRegistryProperty(const GUID* guid, DeviceClassRegistryPropertyType type);

	private:
		DeviceManager(const wchar_t* computerName = nullptr, const GUID* classGuid = nullptr, const wchar_t* enumerator = nullptr,
			InfoSetOptions options = InfoSetOptions::Present | InfoSetOptions::AllClasses);

	private:
		wil::unique_hinfoset _hInfoSet;
	};

	template<typename T>
	inline T DeviceManager::GetDeviceRegistryProperty(const DeviceInfo& di, DeviceRegistryPropertyType type) const {
		static_assert(std::is_trivially_copyable<T>());
		T result{};

		::SetupDiGetDeviceRegistryProperty(_hInfoSet.get(), (PSP_DEVINFO_DATA)&di.Data, static_cast<DWORD>(type), nullptr,
			(BYTE*)&result, sizeof(T), nullptr);
		return result;
	}

	template<typename T>
	inline T DeviceManager::GetDeviceClassRegistryProperty(const GUID* guid, DeviceClassRegistryPropertyType type) {
		static_assert(std::is_trivially_copyable<T>());
		T result{};
		DWORD regType;
		if (::SetupDiGetClassRegistryProperty(guid, static_cast<DWORD>(type), &regType,
			(BYTE*)&result, sizeof(T), nullptr, nullptr, nullptr)) {
		}
		return result;
	}
}

