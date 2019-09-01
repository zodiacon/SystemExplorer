#pragma once

#include <winternl.h>

#pragma comment(lib, "ntdll")

namespace NT {
	typedef struct _SYSTEM_OBJECTTYPE_INFORMATION {
		ULONG NextEntryOffset;
		ULONG NumberOfObjects;
		ULONG NumberOfHandles;
		ULONG TypeIndex;
		ULONG InvalidAttributes;
		GENERIC_MAPPING GenericMapping;
		ULONG ValidAccessMask;
		ULONG PoolType;
		BOOLEAN SecurityRequired;
		BOOLEAN WaitableObject;
		UNICODE_STRING TypeName;
	} SYSTEM_OBJECTTYPE_INFORMATION, * PSYSTEM_OBJECTTYPE_INFORMATION;

	typedef struct _SYSTEM_OBJECT_INFORMATION {
		ULONG NextEntryOffset;
		PVOID Object;
		HANDLE CreatorUniqueProcess;
		USHORT CreatorBackTraceIndex;
		USHORT Flags;
		LONG PointerCount;
		LONG HandleCount;
		ULONG PagedPoolCharge;
		ULONG NonPagedPoolCharge;
		HANDLE ExclusiveProcessId;
		PVOID SecurityDescriptor;
		UNICODE_STRING NameInfo;
	} SYSTEM_OBJECT_INFORMATION, * PSYSTEM_OBJECT_INFORMATION;

	typedef struct _OBJECT_NAME_INFORMATION {
		UNICODE_STRING Name;
	} OBJECT_NAME_INFORMATION, * POBJECT_NAME_INFORMATION;

	typedef enum _SYSTEM_INFORMATION_CLASS {
		SystemObjectInformation = 17,
		SystemExtendedHandleInformation = 64, // q: SYSTEM_HANDLE_INFORMATION_EX
	} SYSTEM_INFORMATION_CLASS;

	typedef struct _OBJECT_BASIC_INFORMATION {
		ULONG Attributes;
		ACCESS_MASK GrantedAccess;
		ULONG HandleCount;
		ULONG PointerCount;
		ULONG PagedPoolCharge;
		ULONG NonPagedPoolCharge;
		ULONG Reserved[3];
		ULONG NameInfoSize;
		ULONG TypeInfoSize;
		ULONG SecurityDescriptorSize;
		LARGE_INTEGER CreationTime;
	} OBJECT_BASIC_INFORMATION, * POBJECT_BASIC_INFORMATION;

	typedef struct _OBJECT_TYPE_INFORMATION {
		UNICODE_STRING TypeName;
		ULONG TotalNumberOfObjects;
		ULONG TotalNumberOfHandles;
		ULONG TotalPagedPoolUsage;
		ULONG TotalNonPagedPoolUsage;
		ULONG TotalNamePoolUsage;
		ULONG TotalHandleTableUsage;
		ULONG HighWaterNumberOfObjects;
		ULONG HighWaterNumberOfHandles;
		ULONG HighWaterPagedPoolUsage;
		ULONG HighWaterNonPagedPoolUsage;
		ULONG HighWaterNamePoolUsage;
		ULONG HighWaterHandleTableUsage;
		ULONG InvalidAttributes;
		GENERIC_MAPPING GenericMapping;
		ULONG ValidAccessMask;
		BOOLEAN SecurityRequired;
		BOOLEAN MaintainHandleCount;
		UCHAR TypeIndex; // since WINBLUE
		CHAR ReservedByte;
		ULONG PoolType;
		ULONG DefaultPagedPoolCharge;
		ULONG DefaultNonPagedPoolCharge;
	} OBJECT_TYPE_INFORMATION, * POBJECT_TYPE_INFORMATION;

	typedef enum _OBJECT_INFORMATION_CLASS {
		ObjectBasicInformation, // OBJECT_BASIC_INFORMATION
		ObjectNameInformation, // OBJECT_NAME_INFORMATION
		ObjectTypeInformation, // OBJECT_TYPE_INFORMATION
		ObjectTypesInformation, // OBJECT_TYPES_INFORMATION
		ObjectHandleFlagInformation, // OBJECT_HANDLE_FLAG_INFORMATION
		ObjectSessionInformation,
		ObjectSessionObjectInformation,
		MaxObjectInfoClass
	} OBJECT_INFORMATION_CLASS;

	typedef struct _SYSTEM_HANDLE_TABLE_ENTRY_INFO_EX {
		PVOID Object;
		ULONG_PTR UniqueProcessId;
		ULONG_PTR HandleValue;
		ULONG GrantedAccess;
		USHORT CreatorBackTraceIndex;
		USHORT ObjectTypeIndex;
		ULONG HandleAttributes;
		ULONG Reserved;
	} SYSTEM_HANDLE_TABLE_ENTRY_INFO_EX, * PSYSTEM_HANDLE_TABLE_ENTRY_INFO_EX;

	typedef struct _SYSTEM_HANDLE_INFORMATION_EX {
		ULONG_PTR NumberOfHandles;
		ULONG_PTR Reserved;
		SYSTEM_HANDLE_TABLE_ENTRY_INFO_EX Handles[1];
	} SYSTEM_HANDLE_INFORMATION_EX, * PSYSTEM_HANDLE_INFORMATION_EX;

	typedef struct _OBJECT_TYPES_INFORMATION {
		ULONG NumberOfTypes;
		OBJECT_TYPE_INFORMATION TypeInformation[1];
	} OBJECT_TYPES_INFORMATION, * POBJECT_TYPES_INFORMATION;

	extern "C" NTSTATUS NTAPI NtQueryObject(
		_In_opt_ HANDLE Handle,
		_In_ OBJECT_INFORMATION_CLASS ObjectInformationClass,
		_Out_writes_bytes_opt_(ObjectInformationLength) PVOID ObjectInformation,
		_In_ ULONG ObjectInformationLength,
		_Out_opt_ PULONG ReturnLength);

	extern "C" NTSTATUS NTAPI NtQuerySystemInformation(
		_In_ SYSTEM_INFORMATION_CLASS SystemInformationClass,
		_Out_writes_bytes_opt_(SystemInformationLength) PVOID SystemInformation,
		_In_ ULONG SystemInformationLength,
		_Out_opt_ PULONG ReturnLength);
}
