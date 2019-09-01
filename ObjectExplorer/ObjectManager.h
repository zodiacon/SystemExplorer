#pragma once

#define STATUS_UNSUCCESSFUL              ((NTSTATUS)0xC0000001L)

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
};

struct ObjectInfo {
	PVOID Object;
	DWORD CreatorProcess;
	USHORT CreatorBackTraceIndex;
	USHORT Flags;
	LONG PointerCount;
	LONG HandleCount;
	ULONG PagedPoolCharge;
	ULONG NonPagedPoolCharge;
	DWORD ExclusiveProcessId;
	PVOID SecurityDescriptor;
	CString Name;
	CString CreatorName;
	ObjectTypeInfo* Type;
};

struct ObjectInfoEx {
	PVOID Object;
	LONG HandleCount;
	CString Name;
	USHORT TypeIndex;
	std::vector<std::shared_ptr<HandleInfo>> Handles;
};

struct ObjectTypeInfo {
	ULONG NumberOfObjects;
	ULONG NumberOfHandles;
	ULONG TypeIndex;
	ULONG InvalidAttributes;
	GENERIC_MAPPING GenericMapping;
	ULONG ValidAccessMask;
	PoolType PoolType;
	CString Name;
	std::vector<std::shared_ptr<ObjectInfo>> Objects;
	bool SecurityRequired;
	bool WaitableObject;
};

struct ObjectTypeInfoEx {
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
	std::vector<std::shared_ptr<ObjectInfoEx>> Objects;
};

struct ProcessInfo {
	ProcessInfo(DWORD id, PCWSTR name);

	DWORD Id;
	CString Name;
};

class ObjectManager {
public:
	NTSTATUS EnumObjects();
	bool EnumHandlesAndObjects();
	int EnumTypes();

	const std::vector<std::shared_ptr<ObjectInfo>>& GetAllObjects() const;
	const std::vector<std::shared_ptr<ObjectTypeInfo>>& GetAllTypeObjects() const;

	const std::vector<std::shared_ptr<ObjectInfoEx>>& GetObjects() const;

	bool EnumProcesses();
	const CString& GetProcessNameById(DWORD id) const;

	enum class ChangeType {
		NoChange,
		TotalHandles,
		TotalObjects,
		PeakHandles,
		PeakObjects,
	};

	using Change = std::tuple<std::shared_ptr<ObjectTypeInfoEx>, ChangeType, int32_t>;

	CString GetObjectName(HANDLE hObject, ULONG pid, USHORT type) const;
	std::shared_ptr<ObjectTypeInfoEx> GetType(USHORT index) const;

private:
	std::vector<std::shared_ptr<ObjectInfo>> _allObjects;
	std::vector<std::shared_ptr<ObjectTypeInfo>> _allTypeObjects;
	std::unordered_map<DWORD, ProcessInfo> _processesById;
	std::vector<std::shared_ptr<ObjectTypeInfoEx>> _types;
	std::unordered_map<int16_t, std::shared_ptr<ObjectTypeInfoEx>> _typesMap;
	std::vector<std::shared_ptr<ObjectInfoEx>> _objects;
	std::unordered_map<PVOID, std::shared_ptr<ObjectInfoEx>> _objectsByAddress;
	std::vector<std::shared_ptr<HandleInfo>> _handles;
	std::vector<Change> _changes;
	int64_t _totalHandles, _totalObjects;
};

