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
	std::unique_ptr<ObjectType> TypeDetails;
};

struct ProcessInfo {
	ProcessInfo(DWORD id, PCWSTR name);

	DWORD Id;
	CString Name;
};

class ObjectManager {
public:
	bool EnumHandlesAndObjects(PCWSTR type = nullptr, DWORD pid = 0);
	bool EnumHandles(PCWSTR type = nullptr, DWORD pid = 0);
	int EnumTypes();

	const std::vector<std::shared_ptr<ObjectInfo>>& GetObjects() const;

	bool EnumProcesses();
	const CString& GetProcessNameById(DWORD id) const;

	static HANDLE DupHandle(ObjectInfo* pObject, ACCESS_MASK access = GENERIC_READ);
	static HANDLE ObjectManager::DupHandle(HANDLE h, DWORD pid, USHORT type, ACCESS_MASK access = 0, DWORD flags = DUPLICATE_SAME_ACCESS);

	static int64_t GetTotalHandles();
	static int64_t GetTotalObjects();

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
	const std::vector<ProcessInfo>& GetProcesses() const {
		return _processes;
	}

private:
	std::unique_ptr<ObjectType> CreateObjectType(int typeIndex, const CString& name) const;

private:
	std::unordered_map<DWORD, ProcessInfo> _processesById;
	std::vector<ProcessInfo> _processes;
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

