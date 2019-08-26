#pragma once

#define STATUS_UNSUCCESSFUL              ((NTSTATUS)0xC0000001L)

struct ObjectInfo {
	PVOID Object;
	DWORD CreatorUniqueProcess;
	USHORT CreatorBackTraceIndex;
	USHORT Flags;
	LONG PointerCount;
	LONG HandleCount;
	ULONG PagedPoolCharge;
	ULONG NonPagedPoolCharge;
	DWORD ExclusiveProcessId;
	PVOID SecurityDescriptor;
	CString Name;
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

class ObjectManager {
public:
	NTSTATUS EnumObjects();

	const std::vector<std::shared_ptr<ObjectInfo>>& GetAllObjects() const;
	const std::vector<std::shared_ptr<ObjectTypeInfo>>& GetAllTypeObjects() const;

private:
	std::vector<std::shared_ptr<ObjectInfo>> _allObjects;
	std::vector<std::shared_ptr<ObjectTypeInfo>> _allTypeObjects;

};

