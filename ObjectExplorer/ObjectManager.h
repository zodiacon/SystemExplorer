#pragma once

//#define STATUS_UNSUCCESSFUL              ((NTSTATUS)0xC0000001L)

#include "ObjectType.h"

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

struct ObjectInfoEx {
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
	std::unique_ptr<ObjectType> TypeDetails;
};

struct ProcessInfo {
	ProcessInfo(DWORD id, PCWSTR name);

	DWORD Id;
	CString Name;
};

class ObjectManager {
public:
	bool EnumHandlesAndObjects(PCWSTR type = nullptr);
	int EnumTypes();

	const std::vector<std::shared_ptr<ObjectInfoEx>>& GetObjects() const;

	bool EnumProcesses();
	const CString& GetProcessNameById(DWORD id) const;

	static HANDLE DupHandle(ObjectInfoEx* pObject, ACCESS_MASK access = GENERIC_READ);

	enum class ChangeType {
		NoChange,
		TotalHandles,
		TotalObjects,
		PeakHandles,
		PeakObjects,
	};

	using Change = std::tuple<std::shared_ptr<ObjectTypeInfoEx>, ChangeType, int32_t>;

	bool GetObjectInfo(ObjectInfoEx* p, HANDLE hObject, ULONG pid, USHORT type) const;
	static std::shared_ptr<ObjectTypeInfoEx> GetType(USHORT index);
	static std::shared_ptr<ObjectTypeInfoEx> GetType(PCWSTR name);

private:
	std::unique_ptr<ObjectType> UpdateKnownTypes(const CString& name, int index);
	std::unique_ptr<ObjectType> CreateObjectType(int typeIndex, const CString& name) const;

private:
	std::unordered_map<DWORD, ProcessInfo> _processesById;
	static std::vector<std::shared_ptr<ObjectTypeInfoEx>> _types;
	static std::unordered_map<int16_t, std::shared_ptr<ObjectTypeInfoEx>> _typesMap;
	static std::unordered_map<std::wstring, std::shared_ptr<ObjectTypeInfoEx>> _typesNameMap;

	std::vector<std::shared_ptr<ObjectInfoEx>> _objects;
	std::unordered_map<PVOID, std::shared_ptr<ObjectInfoEx>> _objectsByAddress;
	std::vector<std::shared_ptr<HandleInfo>> _handles;
	static std::vector<Change> _changes;
	int64_t _totalHandles, _totalObjects;
};

