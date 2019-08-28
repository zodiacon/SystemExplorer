#pragma once

#define STATUS_UNSUCCESSFUL              ((NTSTATUS)0xC0000001L)

struct ObjectTypeInfo;

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

enum class PoolType {
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

struct ProcessInfo {
	ProcessInfo(DWORD id, PCWSTR name);

	DWORD Id;
	CString Name;
};

class ObjectManager {
public:
	NTSTATUS EnumObjects();

	const std::vector<std::shared_ptr<ObjectInfo>>& GetAllObjects() const;
	const std::vector<std::shared_ptr<ObjectTypeInfo>>& GetAllTypeObjects() const;

	bool EnumProcesses();
	const CString& GetProcessNameById(DWORD id) const;

private:
	std::vector<std::shared_ptr<ObjectInfo>> _allObjects;
	std::vector<std::shared_ptr<ObjectTypeInfo>> _allTypeObjects;
	std::unordered_map<DWORD, ProcessInfo> _processesById;
};

