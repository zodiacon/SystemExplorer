#pragma once

//#include <winternl.h>

#pragma comment(lib, "ntdll")

#define DIRECTORY_QUERY                 (0x0001)
#define DIRECTORY_TRAVERSE              (0x0002)
#define DIRECTORY_CREATE_OBJECT         (0x0004)
#define DIRECTORY_CREATE_SUBDIRECTORY   (0x0008)

#define DIRECTORY_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED | 0xF)

typedef _Return_type_success_(return >= 0) LONG NTSTATUS;

#ifndef NT_SUCCESS
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
#endif

#define STATUS_ACCESS_DENIED             ((NTSTATUS)0xC0000022L)

typedef LONG KPRIORITY;

typedef struct _UNICODE_STRING {
	USHORT Length;
	USHORT MaximumLength;
	PWSTR  Buffer;
} UNICODE_STRING;
typedef UNICODE_STRING* PUNICODE_STRING;
typedef const UNICODE_STRING* PCUNICODE_STRING;

typedef enum _PROCESSINFOCLASS {
	ProcessBasicInformation = 0,
	ProcessDebugPort = 7,
	ProcessWow64Information = 26,
	ProcessImageFileName = 27,
	ProcessBreakOnTermination = 29
} PROCESSINFOCLASS;

extern "C" VOID NTAPI RtlInitUnicodeString(
	PUNICODE_STRING DestinationString,
	PCWSTR SourceString);

#ifndef InitializeObjectAttributes
#define InitializeObjectAttributes( p, n, a, r, s ) { \
    (p)->Length = sizeof( OBJECT_ATTRIBUTES );          \
    (p)->RootDirectory = r;                             \
    (p)->Attributes = a;                                \
    (p)->ObjectName = n;                                \
    (p)->SecurityDescriptor = s;                        \
    (p)->SecurityQualityOfService = NULL;               \
    }
#endif

typedef struct _IO_STATUS_BLOCK {
#pragma warning(push)
#pragma warning(disable: 4201) // we'll always use the Microsoft compiler
	union {
		NTSTATUS Status;
		PVOID Pointer;
	} DUMMYUNIONNAME;
#pragma warning(pop)

	ULONG_PTR Information;
} IO_STATUS_BLOCK, * PIO_STATUS_BLOCK;

typedef struct _OBJECT_ATTRIBUTES {
	ULONG Length;
	HANDLE RootDirectory;
	PUNICODE_STRING ObjectName;
	ULONG Attributes;
	PVOID SecurityDescriptor;
	PVOID SecurityQualityOfService;
} OBJECT_ATTRIBUTES;
typedef OBJECT_ATTRIBUTES* POBJECT_ATTRIBUTES;

typedef struct _CLIENT_ID {
	HANDLE UniqueProcess;
	HANDLE UniqueThread;
} CLIENT_ID;

typedef struct _PEB_LDR_DATA {
	BYTE Reserved1[8];
	PVOID Reserved2[3];
	LIST_ENTRY InMemoryOrderModuleList;
} PEB_LDR_DATA, * PPEB_LDR_DATA;

typedef struct _LDR_DATA_TABLE_ENTRY {
	PVOID Reserved1[2];
	LIST_ENTRY InMemoryOrderLinks;
	PVOID Reserved2[2];
	PVOID DllBase;
	PVOID Reserved3[2];
	UNICODE_STRING FullDllName;
	BYTE Reserved4[8];
	PVOID Reserved5[3];
#pragma warning(push)
#pragma warning(disable: 4201) // we'll always use the Microsoft compiler
	union {
		ULONG CheckSum;
		PVOID Reserved6;
	} DUMMYUNIONNAME;
#pragma warning(pop)
	ULONG TimeDateStamp;
} LDR_DATA_TABLE_ENTRY, * PLDR_DATA_TABLE_ENTRY;

typedef struct _RTL_USER_PROCESS_PARAMETERS {
	BYTE Reserved1[16];
	PVOID Reserved2[10];
	UNICODE_STRING ImagePathName;
	UNICODE_STRING CommandLine;
} RTL_USER_PROCESS_PARAMETERS, * PRTL_USER_PROCESS_PARAMETERS;

typedef
VOID
(NTAPI* PPS_POST_PROCESS_INIT_ROUTINE) (
	VOID
	);

typedef struct _PEB {
	BYTE Reserved1[2];
	BYTE BeingDebugged;
	BYTE Reserved2[1];
	PVOID Reserved3[2];
	PPEB_LDR_DATA Ldr;
	PRTL_USER_PROCESS_PARAMETERS ProcessParameters;
	PVOID Reserved4[3];
	PVOID AtlThunkSListPtr;
	PVOID Reserved5;
	ULONG Reserved6;
	PVOID Reserved7;
	ULONG Reserved8;
	ULONG AtlThunkSListPtr32;
	PVOID Reserved9[45];
	BYTE Reserved10[96];
	PPS_POST_PROCESS_INIT_ROUTINE PostProcessInitRoutine;
	BYTE Reserved11[128];
	PVOID Reserved12[1];
	ULONG SessionId;
} PEB, * PPEB;

typedef struct _TEB {
	PVOID Reserved1[12];
	PPEB ProcessEnvironmentBlock;
	PVOID Reserved2[399];
	BYTE Reserved3[1952];
	PVOID TlsSlots[64];
	BYTE Reserved4[8];
	PVOID Reserved5[26];
	PVOID ReservedForOle;  // Windows 2000 only
	PVOID Reserved6[4];
	PVOID TlsExpansionSlots;
} TEB, * PTEB;

typedef struct _PROCESS_BASIC_INFORMATION {
	PVOID Reserved1;
	PPEB PebBaseAddress;
	PVOID Reserved2[2];
	ULONG_PTR UniqueProcessId;
	PVOID Reserved3;
} PROCESS_BASIC_INFORMATION;
typedef PROCESS_BASIC_INFORMATION* PPROCESS_BASIC_INFORMATION;

extern "C" {
	namespace NT {
		typedef struct _OBJECT_DIRECTORY_INFORMATION {
			UNICODE_STRING Name;
			UNICODE_STRING TypeName;
		} OBJECT_DIRECTORY_INFORMATION, * POBJECT_DIRECTORY_INFORMATION;

		typedef enum _THREADINFOCLASS {
			ThreadBasicInformation, // q: THREAD_BASIC_INFORMATION
		} THREADINFOCLASS;

		typedef struct _THREAD_BASIC_INFORMATION {
			NTSTATUS ExitStatus;
			PTEB TebBaseAddress;
			CLIENT_ID ClientId;
			ULONG_PTR AffinityMask;
			KPRIORITY Priority;
			LONG BasePriority;
		} THREAD_BASIC_INFORMATION, * PTHREAD_BASIC_INFORMATION;

		typedef enum _WORKERFACTORYINFOCLASS {
			WorkerFactoryTimeout, // q; s: LARGE_INTEGER
			WorkerFactoryRetryTimeout, // q; s: LARGE_INTEGER
			WorkerFactoryIdleTimeout, // q; s: LARGE_INTEGER
			WorkerFactoryBindingCount,
			WorkerFactoryThreadMinimum, // q; s: ULONG
			WorkerFactoryThreadMaximum, // q; s: ULONG
			WorkerFactoryPaused, // ULONG or BOOLEAN
			WorkerFactoryBasicInformation, // WORKER_FACTORY_BASIC_INFORMATION
			WorkerFactoryAdjustThreadGoal,
			WorkerFactoryCallbackType,
			WorkerFactoryStackInformation, // 10
			WorkerFactoryThreadBasePriority,
			WorkerFactoryTimeoutWaiters, // since THRESHOLD
			WorkerFactoryFlags,
			WorkerFactoryThreadSoftMaximum,
			WorkerFactoryThreadCpuSets, // since REDSTONE5
			MaxWorkerFactoryInfoClass
		} WORKERFACTORYINFOCLASS;

		typedef struct _WORKER_FACTORY_BASIC_INFORMATION {
			LARGE_INTEGER Timeout;
			LARGE_INTEGER RetryTimeout;
			LARGE_INTEGER IdleTimeout;
			BOOLEAN Paused;
			BOOLEAN TimerSet;
			BOOLEAN QueuedToExWorker;
			BOOLEAN MayCreate;
			BOOLEAN CreateInProgress;
			BOOLEAN InsertedIntoQueue;
			BOOLEAN Shutdown;
			ULONG BindingCount;
			ULONG ThreadMinimum;
			ULONG ThreadMaximum;
			ULONG PendingWorkerCount;
			ULONG WaitingWorkerCount;
			ULONG TotalWorkerCount;
			ULONG ReleaseCount;
			LONGLONG InfiniteWaitGoal;
			PVOID StartRoutine;
			PVOID StartParameter;
			HANDLE ProcessId;
			SIZE_T StackReserve;
			SIZE_T StackCommit;
			NTSTATUS LastThreadCreationStatus;
		} WORKER_FACTORY_BASIC_INFORMATION, * PWORKER_FACTORY_BASIC_INFORMATION;

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
		} EVENT_BASIC_INFORMATION, * PEVENT_BASIC_INFORMATION;

		typedef enum _SEMAPHORE_INFORMATION_CLASS {
			SemaphoreBasicInformation
		} SEMAPHORE_INFORMATION_CLASS;

		typedef struct _SEMAPHORE_BASIC_INFORMATION {
			LONG CurrentCount;
			LONG MaximumCount;
		} SEMAPHORE_BASIC_INFORMATION, * PSEMAPHORE_BASIC_INFORMATION;

		typedef enum _MUTANT_INFORMATION_CLASS {
			MutantBasicInformation,
			MutantOwnerInformation
		} MUTANT_INFORMATION_CLASS;

		typedef struct _MUTANT_BASIC_INFORMATION {
			LONG CurrentCount;
			BOOLEAN OwnedByCaller;
			BOOLEAN AbandonedState;
		} MUTANT_BASIC_INFORMATION, * PMUTANT_BASIC_INFORMATION;

		typedef struct _MUTANT_OWNER_INFORMATION {
			CLIENT_ID ClientId;
		} MUTANT_OWNER_INFORMATION, * PMUTANT_OWNER_INFORMATION;

		typedef enum _TIMER_INFORMATION_CLASS {
			TimerBasicInformation
		} TIMER_INFORMATION_CLASS;

		typedef struct _TIMER_BASIC_INFORMATION {
			LARGE_INTEGER RemainingTime;
			BOOLEAN TimerState;
		} TIMER_BASIC_INFORMATION, * PTIMER_BASIC_INFORMATION;

		typedef enum _IO_COMPLETION_INFORMATION_CLASS {
			IoCompletionBasicInformation
		} IO_COMPLETION_INFORMATION_CLASS;

		typedef struct _IO_COMPLETION_BASIC_INFORMATION {
			LONG Depth;
		} IO_COMPLETION_BASIC_INFORMATION, * PIO_COMPLETION_BASIC_INFORMATION;

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
		} SECTION_BASIC_INFORMATION, * PSECTION_BASIC_INFORMATION;

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
		} KEY_BASIC_INFORMATION, * PKEY_BASIC_INFORMATION;

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
		} SECTION_IMAGE_INFORMATION, * PSECTION_IMAGE_INFORMATION;

		NTSTATUS NTAPI NtQuerySection(
			_In_ HANDLE SectionHandle,
			_In_ SECTION_INFORMATION_CLASS SectionInformationClass,
			_Out_writes_bytes_(SectionInformationLength) PVOID SectionInformation,
			_In_ SIZE_T SectionInformationLength,
			_Out_opt_ PSIZE_T ReturnLength);

		NTSTATUS NTAPI NtQueryInformationPort(
			_In_ HANDLE PortHandle,
			_In_ PORT_INFORMATION_CLASS PortInformationClass,
			_Out_writes_bytes_to_(Length, *ReturnLength) PVOID PortInformation,
			_In_ ULONG Length,
			_Out_opt_ PULONG ReturnLength);

		NTSTATUS NTAPI NtQueryIoCompletion(
			_In_ HANDLE IoCompletionHandle,
			_In_ IO_COMPLETION_INFORMATION_CLASS IoCompletionInformationClass,
			_Out_writes_bytes_(IoCompletionInformationLength) PVOID IoCompletionInformation,
			_In_ ULONG IoCompletionInformationLength,
			_Out_opt_ PULONG ReturnLength);

		NTSTATUS NTAPI NtQueryTimer(
			_In_ HANDLE TimerHandle,
			_In_ TIMER_INFORMATION_CLASS TimerInformationClass,
			_Out_writes_bytes_(TimerInformationLength) PVOID TimerInformation,
			_In_ ULONG TimerInformationLength,
			_Out_opt_ PULONG ReturnLength);

		NTSTATUS NTAPI NtQuerySemaphore(
			_In_ HANDLE SemaphoreHandle,
			_In_ SEMAPHORE_INFORMATION_CLASS SemaphoreInformationClass,
			_Out_writes_bytes_(SemaphoreInformationLength) PVOID SemaphoreInformation,
			_In_ ULONG SemaphoreInformationLength,
			_Out_opt_ PULONG ReturnLength);

		NTSTATUS NTAPI NtQueryMutant(
			_In_ HANDLE MutantHandle,
			_In_ MUTANT_INFORMATION_CLASS MutantInformationClass,
			_Out_writes_bytes_(MutantInformationLength) PVOID MutantInformation,
			_In_ ULONG MutantInformationLength,
			_Out_opt_ PULONG ReturnLength);

		NTSTATUS NTAPI NtQueryInformationProcess(
				IN HANDLE ProcessHandle,
				IN PROCESSINFOCLASS ProcessInformationClass,
				OUT PVOID ProcessInformation,
				IN ULONG ProcessInformationLength,
				OUT PULONG ReturnLength OPTIONAL
			);

		NTSTATUS NTAPI NtQueryObject(
			_In_opt_ HANDLE Handle,
			_In_ OBJECT_INFORMATION_CLASS ObjectInformationClass,
			_Out_writes_bytes_opt_(ObjectInformationLength) PVOID ObjectInformation,
			_In_ ULONG ObjectInformationLength,
			_Out_opt_ PULONG ReturnLength);

		NTSTATUS NTAPI NtQuerySystemInformation(
			_In_ SYSTEM_INFORMATION_CLASS SystemInformationClass,
			_Out_writes_bytes_opt_(SystemInformationLength) PVOID SystemInformation,
			_In_ ULONG SystemInformationLength,
			_Out_opt_ PULONG ReturnLength);

		NTSTATUS NTAPI NtQueryEvent(
			_In_ HANDLE EventHandle,
			_In_ EVENT_INFORMATION_CLASS EventInformationClass,
			_Out_writes_bytes_(EventInformationLength) PVOID EventInformation,
			_In_ ULONG EventInformationLength,
			_Out_opt_ PULONG ReturnLength);

		NTSTATUS NTAPI NtQueryKey(
			_In_ HANDLE KeyHandle,
			_In_ KEY_INFORMATION_CLASS KeyInformationClass,
			_Out_writes_bytes_opt_(Length) PVOID KeyInformation,
			_In_ ULONG Length,
			_Out_ PULONG ResultLength);

		NTSTATUS NTAPI NtQuerySymbolicLinkObject(
			_In_ HANDLE LinkHandle,
			_Inout_ PUNICODE_STRING LinkTarget,
			_Out_opt_ PULONG ReturnedLength);

		NTSTATUS NTAPI NtDuplicateObject(
			_In_ HANDLE SourceProcessHandle,
			_In_ HANDLE SourceHandle,
			_In_opt_ HANDLE TargetProcessHandle,
			_Out_opt_ PHANDLE TargetHandle,
			_In_ ACCESS_MASK DesiredAccess,
			_In_ ULONG HandleAttributes,
			_In_ ULONG Options);

		NTSTATUS NTAPI NtOpenDirectoryObject(
			_Out_ PHANDLE hDirectory,
			_In_ ACCESS_MASK AccessMask,
			_In_ POBJECT_ATTRIBUTES ObjectAttributes);

		NTSTATUS NTAPI NtOpenSymbolicLinkObject(
			_Out_  PHANDLE LinkHandle,
			_In_   ACCESS_MASK DesiredAccess,
			_In_   POBJECT_ATTRIBUTES ObjectAttributes);

		NTSTATUS NTAPI NtQueryDirectoryObject(
			_In_  HANDLE hDirectory,
			_Out_ POBJECT_DIRECTORY_INFORMATION DirectoryEntryBuffer,
			_In_  ULONG DirectoryEntryBufferSize,
			_In_  BOOLEAN  bOnlyFirstEntry,
			_In_  BOOLEAN bFirstEntry,
			_In_  PULONG  EntryIndex,
			_Out_ PULONG  BytesReturned);

		NTSTATUS NTAPI NtOpenEvent(
			_Out_ PHANDLE EventHandle,
			_In_  ACCESS_MASK DesiredAccess,
			_In_  POBJECT_ATTRIBUTES ObjectAttributes);

		NTSTATUS NTAPI NtQuerySecurityObject(
			_In_ HANDLE hObject,
			_In_ SECURITY_INFORMATION SecurityInfoRequested,
			_In_ PSECURITY_DESCRIPTOR pSecurityDescriptor,
			_In_ ULONG pSecurityDescriptorLength,
			_Out_ PULONG BytesRequired);

		NTSTATUS NTAPI NtOpenMutant(
			_Out_ PHANDLE hMutex,
			_In_ ACCESS_MASK DesiredAccess,
			_In_ POBJECT_ATTRIBUTES ObjectAttributes);

		NTSTATUS NTAPI NtOpenSemaphore(
			_Out_ PHANDLE hSemaphore,
			_In_ ACCESS_MASK DesiredAccess,
			_In_ POBJECT_ATTRIBUTES ObjectAttributes);

		NTSTATUS NTAPI NtOpenSection(
			_Out_ PHANDLE phSection,
			_In_  ACCESS_MASK DesiredAccess,
			_In_  POBJECT_ATTRIBUTES ObjectAttributes);

		NTSTATUS NTAPI NtOpenIoCompletion(
			_Out_ PHANDLE phIoCompletionPort,
			_In_ ACCESS_MASK DesiredAccess,
			_In_ POBJECT_ATTRIBUTES ObjectAttributes);

		NTSTATUS NTAPI NtQueryIoCompletion(
			_In_ HANDLE IoCompletionHandle,
			_In_ IO_COMPLETION_INFORMATION_CLASS IoCompletionInformationClass,
			_Out_writes_bytes_(IoCompletionInformationLength) PVOID IoCompletionInformation,
			_In_ ULONG IoCompletionInformationLength,
			_Out_opt_ PULONG ReturnLength);

		NTSTATUS NTAPI NtOpenFile(
			_Out_ PHANDLE phFile,
			_In_ ACCESS_MASK DesiredAccess,
			_In_ POBJECT_ATTRIBUTES ObjectAttributes,
			_Out_ PIO_STATUS_BLOCK pIoStatusBlock,
			_In_ ULONG ShareMode,
			_In_ ULONG OpenMode);

		NTSTATUS NTAPI NtOpenEventPair(
			_Out_ PHANDLE hEventPair,
			_In_ ACCESS_MASK AccessMask,
			_In_ POBJECT_ATTRIBUTES ObjectAttributes);

		NTSTATUS NTAPI NtOpenTimer(
			_Out_ PHANDLE phTimer,
			_In_ ACCESS_MASK AccessMask,
			_In_ POBJECT_ATTRIBUTES ObjectAttributes);

		NTSTATUS NTAPI NtOpenKey(
			_Out_ PHANDLE phKey,
			_In_ ACCESS_MASK DesiredAccess,
			_In_ POBJECT_ATTRIBUTES ObjectAttributes);

		NTSTATUS NTAPI NtOpenJobObject(
			_Out_ PHANDLE JobHandle,
			_In_ ACCESS_MASK DesiredAccess,
			_In_ POBJECT_ATTRIBUTES ObjectAttributes);

		NTSTATUS NTAPI NtOpenSession(
			_Out_ PHANDLE SessionHandle,
			_In_ ACCESS_MASK DesiredAccess,
			_In_ POBJECT_ATTRIBUTES ObjectAttributes);

		NTSTATUS NTAPI NtQueryInformationWorkerFactory(
			_In_ HANDLE WorkerFactoryHandle,
			_In_ WORKERFACTORYINFOCLASS WorkerFactoryInformationClass,
			_Out_writes_bytes_(WorkerFactoryInformationLength) PVOID WorkerFactoryInformation,
			_In_ ULONG WorkerFactoryInformationLength,
			_Out_opt_ PULONG ReturnLength);

		NTSTATUS NTAPI NtGetNextThread(
			_In_ HANDLE ProcessHandle,
			_In_ HANDLE ThreadHandle,
			_In_ ACCESS_MASK DesiredAccess,
			_In_ ULONG HandleAttributes,
			_In_ ULONG Flags,
			_Out_ PHANDLE NewThreadHandle);

		NTSTATUS NTAPI NtQueryInformationThread(
			_In_ HANDLE ThreadHandle,
			_In_ THREADINFOCLASS ThreadInformationClass,
			_Out_writes_bytes_(ThreadInformationLength) PVOID ThreadInformation,
			_In_ ULONG ThreadInformationLength,
			_Out_opt_ PULONG ReturnLength);

	}
}
