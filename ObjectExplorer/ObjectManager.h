#pragma once

struct ObjectTypeInfo;

enum class PoolType {
	PagedPool = 1,
	NonPagedPool = 0,
	NonPagedPoolNx = 0x200,
	NonPagedPoolSessionNx = NonPagedPoolNx + 32,
	PagedPoolSessionNx = NonPagedPoolNx + 33
};

struct HandleInfo {
	PVOID Object;
	ULONG ProcessId;
	ULONG HandleValue;
	ULONG GrantedAccess;
	ULONG HandleAttributes;
	USHORT ObjectTypeIndex;
	CString Name;
};

struct ObjectInfo {
	PVOID Object;
	int HandleCount;
	int PointerCount;
	int64_t CreateTime;
	CString Name;
	USHORT TypeIndex;
	std::vector<std::shared_ptr<HandleInfo>> Handles;
	wil::unique_handle LocalHandle;
	PCWSTR TypeName;
};

struct ObjectTypeInfo {
	uint32_t TotalNumberOfObjects;
	uint32_t TotalNumberOfHandles;
	uint32_t TotalPagedPoolUsage;
	uint32_t TotalNonPagedPoolUsage;
	uint32_t TotalNamePoolUsage;
	uint32_t TotalHandleTableUsage;
	uint32_t HighWaterNumberOfObjects;
	uint32_t HighWaterNumberOfHandles;
	uint32_t HighWaterPagedPoolUsage;
	uint32_t HighWaterNonPagedPoolUsage;
	uint32_t HighWaterNamePoolUsage;
	uint32_t HighWaterHandleTableUsage;
	uint32_t InvalidAttributes;
	GENERIC_MAPPING GenericMapping;
	uint32_t ValidAccessMask;
	PoolType PoolType;
	uint32_t DefaultPagedPoolCharge;
	uint32_t DefaultNonPagedPoolCharge;
	CString TypeName;
	uint8_t TypeIndex;
	bool SecurityRequired;
	bool MaintainHandleCount;
	std::vector<std::shared_ptr<ObjectInfo>> Objects;
};

struct ObjectNameAndType {
	std::wstring Name;
	std::wstring TypeName;
};

enum class GdiObjectType {
	DeviceContext = 1,
	Region = 4,
	Bitmap = 5,
	Palette = 8,
	Font = 10,
	Brush = 0x10,
	Metafile = 0x15,
};

struct GdiObject {
	HGDIOBJ Handle;
	GdiObjectType Type;
	USHORT Count;
	USHORT Index;
	void* KernelAddress;
};

class ObjectManager {
public:
	bool EnumHandlesAndObjects(PCWSTR type = nullptr, DWORD pid = 0);
	bool EnumHandles(PCWSTR type = nullptr, DWORD pid = 0);
	int EnumTypes();

	const std::vector<std::shared_ptr<ObjectInfo>>& GetObjects() const;

	static HANDLE DupHandle(ObjectInfo* pObject, ACCESS_MASK access = GENERIC_READ);
	static HANDLE DupHandle(HANDLE h, DWORD pid, USHORT type, ACCESS_MASK access = 0, DWORD flags = DUPLICATE_SAME_ACCESS);
	static NTSTATUS OpenObject(PCWSTR path, PCWSTR type, HANDLE* pHandle, DWORD access = GENERIC_READ);

	static int64_t GetTotalHandles();
	static int64_t GetTotalObjects();
	static std::vector<HWND> EnumDsktopWindows(HANDLE hDesktop);
	static std::vector<HWND> EnumChildWindows(HWND hWnd);
	static std::vector<GdiObject> EnumGdiObjects(DWORD pid);

	enum class ChangeType {
		NoChange,
		TotalHandles,
		TotalObjects,
		PeakHandles,
		PeakObjects,
	};

	using Change = std::tuple<std::shared_ptr<ObjectTypeInfo>, ChangeType, int32_t>;
	const std::vector<Change>& GetChanges() const;

	bool GetObjectInfo(ObjectInfo* p, HANDLE hObject, ULONG pid, USHORT type) const;
	CString GetObjectName(HANDLE hObject, ULONG pid, USHORT type) const;
	CString GetObjectName(HANDLE hDup, USHORT type) const;

	static std::shared_ptr<ObjectTypeInfo> GetType(USHORT index);
	static std::shared_ptr<ObjectTypeInfo> GetType(PCWSTR name);
	static const std::vector<std::shared_ptr<ObjectTypeInfo>>& GetObjectTypes();
	const std::vector<std::shared_ptr<HandleInfo>>& GetHandles() const;

	static std::vector<ObjectNameAndType> EnumDirectoryObjects(PCWSTR path);
	static CString GetSymbolicLinkTarget(PCWSTR path);

private:
	static std::vector<std::shared_ptr<ObjectTypeInfo>> _types;
	static std::unordered_map<int16_t, std::shared_ptr<ObjectTypeInfo>> _typesMap;
	static std::unordered_map<std::wstring, std::shared_ptr<ObjectTypeInfo>> _typesNameMap;

	std::vector<std::shared_ptr<ObjectInfo>> _objects;
	std::unordered_map<PVOID, std::shared_ptr<ObjectInfo>> _objectsByAddress;
	std::vector<std::shared_ptr<HandleInfo>> _handles;
	static std::vector<Change> _changes;
	static int64_t _totalHandles, _totalObjects;
	bool _skipThisProcess = false;
};

