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

	typedef enum _EVENT_INFORMATION_CLASS {
		EventBasicInformation
	} EVENT_INFORMATION_CLASS;

	typedef enum _EVENT_TYPE {
		NotificationEvent,
		SynchronizationEvent
	} EVENT_TYPE;

	typedef struct _EVENT_BASIC_INFORMATION {
		EVENT_TYPE EventType;
		LONG EventState;
	} EVENT_BASIC_INFORMATION, *PEVENT_BASIC_INFORMATION;

	typedef enum _SEMAPHORE_INFORMATION_CLASS {
		SemaphoreBasicInformation
	} SEMAPHORE_INFORMATION_CLASS;

	typedef struct _SEMAPHORE_BASIC_INFORMATION {
		LONG CurrentCount;
		LONG MaximumCount;
	} SEMAPHORE_BASIC_INFORMATION, *PSEMAPHORE_BASIC_INFORMATION;

	typedef enum _MUTANT_INFORMATION_CLASS {
		MutantBasicInformation,
		MutantOwnerInformation
	} MUTANT_INFORMATION_CLASS;

	typedef struct _MUTANT_BASIC_INFORMATION {
		LONG CurrentCount;
		BOOLEAN OwnedByCaller;
		BOOLEAN AbandonedState;
	} MUTANT_BASIC_INFORMATION, *PMUTANT_BASIC_INFORMATION;

	typedef struct _MUTANT_OWNER_INFORMATION {
		CLIENT_ID ClientId;
	} MUTANT_OWNER_INFORMATION, *PMUTANT_OWNER_INFORMATION;

	typedef enum _TIMER_INFORMATION_CLASS {
		TimerBasicInformation
	} TIMER_INFORMATION_CLASS;

	typedef struct _TIMER_BASIC_INFORMATION {
		LARGE_INTEGER RemainingTime;
		BOOLEAN TimerState;
	} TIMER_BASIC_INFORMATION, *PTIMER_BASIC_INFORMATION;

	typedef enum _IO_COMPLETION_INFORMATION_CLASS {
		IoCompletionBasicInformation
	} IO_COMPLETION_INFORMATION_CLASS;

	typedef struct _IO_COMPLETION_BASIC_INFORMATION {
		LONG Depth;
	} IO_COMPLETION_BASIC_INFORMATION, *PIO_COMPLETION_BASIC_INFORMATION;

	typedef enum _PORT_INFORMATION_CLASS {
		PortBasicInformation,
		PortDumpInformation
	} PORT_INFORMATION_CLASS;

	typedef enum _SECTION_INFORMATION_CLASS {
		SectionBasicInformation, // q; SECTION_BASIC_INFORMATION
		SectionImageInformation, // q; SECTION_IMAGE_INFORMATION
		SectionRelocationInformation, // name:wow64:whNtQuerySection_SectionRelocationInformation
		SectionOriginalBaseInformation, // PVOID BaseAddress
		SectionInternalImageInformation, // SECTION_INTERNAL_IMAGE_INFORMATION // since REDSTONE2
		MaxSectionInfoClass
	} SECTION_INFORMATION_CLASS;

	typedef struct _SECTION_BASIC_INFORMATION {
		PVOID BaseAddress;
		ULONG AllocationAttributes;
		LARGE_INTEGER MaximumSize;
	} SECTION_BASIC_INFORMATION, *PSECTION_BASIC_INFORMATION;

	typedef enum _KEY_INFORMATION_CLASS {
		KeyBasicInformation, // KEY_BASIC_INFORMATION
		KeyNodeInformation, // KEY_NODE_INFORMATION
		KeyFullInformation, // KEY_FULL_INFORMATION
		KeyNameInformation, // KEY_NAME_INFORMATION
		KeyCachedInformation, // KEY_CACHED_INFORMATION
		KeyFlagsInformation, // KEY_FLAGS_INFORMATION
		KeyVirtualizationInformation, // KEY_VIRTUALIZATION_INFORMATION
		KeyHandleTagsInformation, // KEY_HANDLE_TAGS_INFORMATION
		KeyTrustInformation, // KEY_TRUST_INFORMATION
		KeyLayerInformation, // KEY_LAYER_INFORMATION
		MaxKeyInfoClass
	} KEY_INFORMATION_CLASS;

	typedef struct _KEY_BASIC_INFORMATION {
		LARGE_INTEGER LastWriteTime;
		ULONG TitleIndex;
		ULONG NameLength;
		WCHAR Name[1];
	} KEY_BASIC_INFORMATION, *PKEY_BASIC_INFORMATION;

	typedef struct _SECTION_IMAGE_INFORMATION {
		PVOID TransferAddress;
		ULONG ZeroBits;
		SIZE_T MaximumStackSize;
		SIZE_T CommittedStackSize;
		ULONG SubSystemType;
		union {
			struct {
				USHORT SubSystemMinorVersion;
				USHORT SubSystemMajorVersion;
			};
			ULONG SubSystemVersion;
		};
		union {
			struct {
				USHORT MajorOperatingSystemVersion;
				USHORT MinorOperatingSystemVersion;
			};
			ULONG OperatingSystemVersion;
		};
		USHORT ImageCharacteristics;
		USHORT DllCharacteristics;
		USHORT Machine;
		BOOLEAN ImageContainsCode;
		union {
			UCHAR ImageFlags;
			struct {
				UCHAR ComPlusNativeReady : 1;
				UCHAR ComPlusILOnly : 1;
				UCHAR ImageDynamicallyRelocated : 1;
				UCHAR ImageMappedFlat : 1;
				UCHAR BaseBelow4gb : 1;
				UCHAR ComPlusPrefer32bit : 1;
				UCHAR Reserved : 2;
			};
		};
		ULONG LoaderFlags;
		ULONG ImageFileSize;
		ULONG CheckSum;
	} SECTION_IMAGE_INFORMATION, *PSECTION_IMAGE_INFORMATION;

	extern"C" NTSTATUS NTAPI NtQuerySection(
			_In_ HANDLE SectionHandle,
			_In_ SECTION_INFORMATION_CLASS SectionInformationClass,
			_Out_writes_bytes_(SectionInformationLength) PVOID SectionInformation,
			_In_ SIZE_T SectionInformationLength,
			_Out_opt_ PSIZE_T ReturnLength);

	extern "C" NTSTATUS NTAPI NtQueryInformationPort(
			_In_ HANDLE PortHandle,
			_In_ PORT_INFORMATION_CLASS PortInformationClass,
			_Out_writes_bytes_to_(Length, *ReturnLength) PVOID PortInformation,
			_In_ ULONG Length,
			_Out_opt_ PULONG ReturnLength);

	extern "C" NTSTATUS	NTAPI NtQueryIoCompletion(
			_In_ HANDLE IoCompletionHandle,
			_In_ IO_COMPLETION_INFORMATION_CLASS IoCompletionInformationClass,
			_Out_writes_bytes_(IoCompletionInformationLength) PVOID IoCompletionInformation,
			_In_ ULONG IoCompletionInformationLength,
			_Out_opt_ PULONG ReturnLength);

	extern "C" NTSTATUS NTAPI NtQueryTimer(
			_In_ HANDLE TimerHandle,
			_In_ TIMER_INFORMATION_CLASS TimerInformationClass,
			_Out_writes_bytes_(TimerInformationLength) PVOID TimerInformation,
			_In_ ULONG TimerInformationLength,
			_Out_opt_ PULONG ReturnLength);

	extern "C" NTSTATUS NTAPI NtQuerySemaphore(
			_In_ HANDLE SemaphoreHandle,
			_In_ SEMAPHORE_INFORMATION_CLASS SemaphoreInformationClass,
			_Out_writes_bytes_(SemaphoreInformationLength) PVOID SemaphoreInformation,
			_In_ ULONG SemaphoreInformationLength,
			_Out_opt_ PULONG ReturnLength);

	extern "C" NTSTATUS	NTAPI NtQueryMutant(
			_In_ HANDLE MutantHandle,
			_In_ MUTANT_INFORMATION_CLASS MutantInformationClass,
			_Out_writes_bytes_(MutantInformationLength) PVOID MutantInformation,
			_In_ ULONG MutantInformationLength,
			_Out_opt_ PULONG ReturnLength);

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

	extern "C" NTSTATUS NTAPI NtQueryEvent(
			_In_ HANDLE EventHandle,
			_In_ EVENT_INFORMATION_CLASS EventInformationClass,
			_Out_writes_bytes_(EventInformationLength) PVOID EventInformation,
			_In_ ULONG EventInformationLength,
			_Out_opt_ PULONG ReturnLength);

	extern "C" NTSTATUS	NTAPI NtQueryKey(
			_In_ HANDLE KeyHandle,
			_In_ KEY_INFORMATION_CLASS KeyInformationClass,
			_Out_writes_bytes_opt_(Length) PVOID KeyInformation,
			_In_ ULONG Length,
			_Out_ PULONG ResultLength);

	extern "C" NTSTATUS	NTAPI NtQuerySymbolicLinkObject(
			_In_ HANDLE LinkHandle,
			_Inout_ PUNICODE_STRING LinkTarget,
			_Out_opt_ PULONG ReturnedLength);

	extern "C" NTSTATUS	NTAPI NtDuplicateObject(
			_In_ HANDLE SourceProcessHandle,
			_In_ HANDLE SourceHandle,
			_In_opt_ HANDLE TargetProcessHandle,
			_Out_opt_ PHANDLE TargetHandle,
			_In_ ACCESS_MASK DesiredAccess,
			_In_ ULONG HandleAttributes,
			_In_ ULONG Options);

}
