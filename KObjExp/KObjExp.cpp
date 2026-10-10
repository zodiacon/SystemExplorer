#include <ntifs.h>
#include <ntddk.h>
#include <wdmsec.h>
#include "KObjExp.h"

#define DRIVER_PREFIX "KObjExp: "
#define DRIVER_TAG 'pxOK'

DRIVER_UNLOAD ObjExpUnload;

DRIVER_DISPATCH ObjExpCreateClose, ObjExpDeviceControl;
//extern POBJECT_TYPE* ExWindowStationObjectType;

extern "C" POBJECT_TYPE ObGetObjectType(PVOID Object);

extern "C" NTSTATUS ZwOpenThread(
	_Out_ PHANDLE ThreadHandle,
	_In_ ACCESS_MASK DesiredAccess,
	_In_ POBJECT_ATTRIBUTES ObjectAttributes,
	_In_opt_ PCLIENT_ID ClientId);

extern "C" NTSTATUS NTAPI ZwQueryInformationProcess(
	_In_ HANDLE ProcessHandle,
	_In_ PROCESSINFOCLASS ProcessInformationClass,
	_Out_writes_bytes_(ProcessInformationLength) PVOID ProcessInformation,
	_In_ ULONG ProcessInformationLength,
	_Out_opt_ PULONG ReturnLength
);

extern "C" NTSTATUS NTAPI ZwQuerySystemInformation(
	_In_ ULONG SystemInformationClass,
	_Out_writes_bytes_opt_(SystemInformationLength) PVOID SystemInformation,
	_In_ ULONG SystemInformationLength,
	_Out_opt_ PULONG ReturnLength);

extern "C" NTSTATUS ObOpenObjectByName(
	_In_ POBJECT_ATTRIBUTES ObjectAttributes,
	_In_ POBJECT_TYPE ObjectType,
	_In_ KPROCESSOR_MODE AccessMode,
	_Inout_opt_ PACCESS_STATE AccessState,
	_In_opt_ ACCESS_MASK DesiredAccess,
	_Inout_opt_ PVOID ParseContext,
	_Out_ PHANDLE Handle);

extern "C" HANDLE NTAPI __win32kstub_NtUserOpenWindowStation(
	_In_ POBJECT_ATTRIBUTES attr,
	_In_ ACCESS_MASK access);

// SystemExtendedHandleInformation (undocumented)
const ULONG SystemExtendedHandleInformation = 64;

struct SYSTEM_HANDLE_TABLE_ENTRY_INFO_EX {
	PVOID Object;
	ULONG_PTR UniqueProcessId;
	ULONG_PTR HandleValue;
	ULONG GrantedAccess;
	USHORT CreatorBackTraceIndex;
	USHORT ObjectTypeIndex;
	ULONG HandleAttributes;
	ULONG Reserved;
};

struct SYSTEM_HANDLE_INFORMATION_EX {
	ULONG_PTR NumberOfHandles;
	ULONG_PTR Reserved;
	SYSTEM_HANDLE_TABLE_ENTRY_INFO_EX Handles[1];
};

static PVOID AllocatePaged(SIZE_T size) {
	// ExAllocatePool2 would limit the driver to Windows 10 2004 and later
#pragma warning(suppress: 4996)
	return ExAllocatePoolWithTag(PagedPool, size, DRIVER_TAG);
}

static void FreePool(PVOID p) {
	ExFreePoolWithTag(p, DRIVER_TAG);
}

extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING) {
	PDEVICE_OBJECT DeviceObject;
	UNICODE_STRING devName = RTL_CONSTANT_STRING(L"\\Device\\KObjExp");
	// FILE_DEVICE_SECURE_OPEN: the device's DACL (System and Administrators only) applies to every open,
	// including opens of a name under the device (\\.\KObjExp\anything)
	auto status = IoCreateDeviceSecure(DriverObject, 0, &devName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE,
		&SDDL_DEVOBJ_SYS_ALL_ADM_ALL, nullptr, &DeviceObject);
	if (!NT_SUCCESS(status)) {
		KdPrint((DRIVER_PREFIX "Failed to create device object (0x%X)\n", status));
		return status;
	}

	UNICODE_STRING symName = RTL_CONSTANT_STRING(L"\\??\\KObjExp");
	status = IoCreateSymbolicLink(&symName, &devName);
	if (!NT_SUCCESS(status)) {
		IoDeleteDevice(DeviceObject);
		KdPrint((DRIVER_PREFIX "Failed to create symbolic link (0x%X)\n", status));
		return status;
	}

	DriverObject->DriverUnload = ObjExpUnload;
	DriverObject->MajorFunction[IRP_MJ_CREATE] = DriverObject->MajorFunction[IRP_MJ_CLOSE] = ObjExpCreateClose;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = ObjExpDeviceControl;

	return status;
}

void ObjExpUnload(PDRIVER_OBJECT DriverObject) {
	UNICODE_STRING symName = RTL_CONSTANT_STRING(L"\\??\\KObjExp");
	IoDeleteSymbolicLink(&symName);
	IoDeleteDevice(DriverObject->DeviceObject);
}

//
// Checks that the calling process is one of the known clients, by its image file name. This is a speed bump, not a
// security boundary: the device's DACL (administrators only) is what keeps others out.
//
static NTSTATUS VerifyClient() {
	ULONG size = 0;
	auto status = ZwQueryInformationProcess(NtCurrentProcess(), ProcessImageFileName, nullptr, 0, &size);
	if (status != STATUS_INFO_LENGTH_MISMATCH)
		return NT_SUCCESS(status) ? STATUS_ACCESS_DENIED : status;
	if (size < sizeof(UNICODE_STRING) || size > (1 << 16))
		return STATUS_ACCESS_DENIED;

	auto path = static_cast<PUNICODE_STRING>(AllocatePaged(size));
	if (!path)
		return STATUS_INSUFFICIENT_RESOURCES;

	status = ZwQueryInformationProcess(NtCurrentProcess(), ProcessImageFileName, path, size, &size);
	if (NT_SUCCESS(status)) {
		status = STATUS_ACCESS_DENIED;
		if (path->Buffer && path->Length >= sizeof(WCHAR)) {
			// the file name is what follows the last backslash
			USHORT count = path->Length / sizeof(WCHAR);
			USHORT start = count;
			while (start > 0 && path->Buffer[start - 1] != L'\\')
				start--;
			UNICODE_STRING name;
			name.Buffer = path->Buffer + start;
			name.Length = name.MaximumLength = (USHORT)((count - start) * sizeof(WCHAR));

			static const UNICODE_STRING clients[] = {
				RTL_CONSTANT_STRING(L"SysExp.exe"),
				RTL_CONSTANT_STRING(L"ObjExp.exe"),
				RTL_CONSTANT_STRING(L"TotalSys.exe"),
			};
			for (auto& client : clients) {
				if (RtlEqualUnicodeString(&name, &client, TRUE)) {
					status = STATUS_SUCCESS;
					break;
				}
			}
		}
	}
	FreePool(path);
	return status;
}

NTSTATUS ObjExpCreateClose(PDEVICE_OBJECT, PIRP Irp) {
	auto status = STATUS_SUCCESS;
	auto stack = IoGetCurrentIrpStackLocation(Irp);
	if (stack->MajorFunction == IRP_MJ_CREATE) {
		// only the device itself can be opened, not a name under it
		if (stack->FileObject && stack->FileObject->FileName.Length > 0)
			status = STATUS_ACCESS_DENIED;
		else
			status = VerifyClient();
	}
	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, 0);
	return status;
}

//
// References the object a handle in another process refers to, if it's the expected object (the handle may have been
// closed and its value reused for another object since it was seen).
//
static NTSTATUS ReferenceObjectInProcess(ULONG_PTR pid, ULONG_PTR handle, PVOID expected, PVOID* object) {
	HANDLE hProcess;
	OBJECT_ATTRIBUTES procAttributes = RTL_CONSTANT_OBJECT_ATTRIBUTES(nullptr, OBJ_KERNEL_HANDLE);
	CLIENT_ID cid{};
	cid.UniqueProcess = (HANDLE)pid;
	auto status = ZwOpenProcess(&hProcess, PROCESS_DUP_HANDLE, &procAttributes, &cid);
	if (!NT_SUCCESS(status))
		return status;

	HANDLE hDup;
	status = ZwDuplicateObject(hProcess, (HANDLE)handle, NtCurrentProcess(), &hDup, 0, OBJ_KERNEL_HANDLE, DUPLICATE_SAME_ACCESS);
	ZwClose(hProcess);
	if (!NT_SUCCESS(status))
		return status;

	PVOID obj;
	status = ObReferenceObjectByHandle(hDup, 0, nullptr, KernelMode, &obj, nullptr);
	ZwClose(hDup);
	if (!NT_SUCCESS(status))
		return status;

	if (obj != expected) {
		ObDereferenceObject(obj);
		return STATUS_NOT_FOUND;
	}
	*object = obj;
	return STATUS_SUCCESS;
}

//
// References an object by its address. The address comes from the caller, so it is never dereferenced as is: it must
// be the address of an object that some process has a handle to, and the object is reached through that handle.
// A bogus address, or the address of an object that has since been freed, fails instead of crashing the system.
//
static NTSTATUS ReferenceObjectByAddress(PVOID address, PVOID* object) {
	if (address == nullptr)
		return STATUS_INVALID_PARAMETER;

	ULONG size = 1 << 22;
	SYSTEM_HANDLE_INFORMATION_EX* info = nullptr;
	NTSTATUS status;
	for (;;) {
		info = static_cast<SYSTEM_HANDLE_INFORMATION_EX*>(AllocatePaged(size));
		if (!info)
			return STATUS_INSUFFICIENT_RESOURCES;

		ULONG needed = 0;
		status = ZwQuerySystemInformation(SystemExtendedHandleInformation, info, size, &needed);
		if (status != STATUS_INFO_LENGTH_MISMATCH)
			break;

		FreePool(info);
		info = nullptr;
		if (needed > size)
			size = needed;
		// handles are added all the time, so leave some room
		if (size > (1UL << 30))
			return STATUS_INSUFFICIENT_RESOURCES;
		size += size / 4;
	}
	if (!NT_SUCCESS(status)) {
		FreePool(info);
		return status;
	}

	auto maxCount = (size - FIELD_OFFSET(SYSTEM_HANDLE_INFORMATION_EX, Handles)) / sizeof(SYSTEM_HANDLE_TABLE_ENTRY_INFO_EX);
	auto count = min(info->NumberOfHandles, (ULONG_PTR)maxCount);
	status = STATUS_NOT_FOUND;
	for (ULONG_PTR i = 0; i < count; i++) {
		auto& entry = info->Handles[i];
		if (entry.Object != address)
			continue;
		status = ReferenceObjectInProcess(entry.UniqueProcessId, entry.HandleValue, address, object);
		if (NT_SUCCESS(status))
			break;
	}
	FreePool(info);
	return status;
}

NTSTATUS ObjExpDeviceControl(PDEVICE_OBJECT, PIRP Irp) {
	const auto& dic = IoGetCurrentIrpStackLocation(Irp)->Parameters.DeviceIoControl;
	auto status = STATUS_INVALID_DEVICE_REQUEST;
	ULONG len = 0;

	switch (dic.IoControlCode) {
		case IOCTL_KOBJEXP_OPEN_OBJECT:
		{
			if (Irp->AssociatedIrp.SystemBuffer == nullptr) {
				status = STATUS_INVALID_PARAMETER;
				break;
			}
			if (dic.InputBufferLength < sizeof(OpenObjectData)) {
				status = STATUS_BUFFER_TOO_SMALL;
				break;
			}

			if (dic.OutputBufferLength < sizeof(HANDLE)) {
				status = STATUS_BUFFER_TOO_SMALL;
				break;
			}

			auto data = static_cast<OpenObjectData*>(Irp->AssociatedIrp.SystemBuffer);
			PVOID obj;
			status = ReferenceObjectByAddress(data->Address, &obj);
			if (!NT_SUCCESS(status))
				break;

			HANDLE hObject;
			status = ObOpenObjectByPointer(obj, 0, nullptr, data->Access, nullptr, KernelMode, &hObject);
			ObDereferenceObject(obj);
			if (NT_SUCCESS(status)) {
				*static_cast<HANDLE*>(Irp->AssociatedIrp.SystemBuffer) = hObject;
				len = sizeof(HANDLE);
			}
			break;
		}

		case IOCTL_KOBJEXP_DUP_HANDLE:
		{
			if (Irp->AssociatedIrp.SystemBuffer == nullptr) {
				status = STATUS_INVALID_PARAMETER;
				break;
			}
			if (dic.InputBufferLength < sizeof(DupHandleData)) {
				status = STATUS_BUFFER_TOO_SMALL;
				break;
			}

			if (dic.OutputBufferLength < sizeof(HANDLE)) {
				status = STATUS_BUFFER_TOO_SMALL;
				break;
			}
			const auto data = static_cast<DupHandleData*>(Irp->AssociatedIrp.SystemBuffer);
			const ULONG validFlags = DUPLICATE_CLOSE_SOURCE | DUPLICATE_SAME_ACCESS | DUPLICATE_SAME_ATTRIBUTES;
			if (data->Flags & ~validFlags) {
				status = STATUS_INVALID_PARAMETER;
				break;
			}
			// the System process' handles are kernel handles; closing one from under its owner can bring the system down
			if ((data->Flags & DUPLICATE_CLOSE_SOURCE) && data->SourcePid <= 4) {
				status = STATUS_ACCESS_DENIED;
				break;
			}

			HANDLE hProcess;
			OBJECT_ATTRIBUTES procAttributes = RTL_CONSTANT_OBJECT_ATTRIBUTES(nullptr, OBJ_KERNEL_HANDLE);
			CLIENT_ID pid{};
			pid.UniqueProcess = UlongToHandle(data->SourcePid);
			status = ZwOpenProcess(&hProcess, PROCESS_DUP_HANDLE, &procAttributes, &pid);
			if (!NT_SUCCESS(status)) {
				KdPrint((DRIVER_PREFIX "Failed to open process %u (0x%08X)\n", data->SourcePid, status));
				break;
			}

			HANDLE hTarget;
			status = ZwDuplicateObject(hProcess, ULongToHandle(data->Handle), NtCurrentProcess(),
				&hTarget, data->AccessMask, 0, data->Flags);
			ZwClose(hProcess);
			if (!NT_SUCCESS(status)) {
				KdPrint((DRIVER_PREFIX "Failed to duplicate handle (0x%08X)\n", status));
				break;
			}

			*(HANDLE*)Irp->AssociatedIrp.SystemBuffer = hTarget;
			len = sizeof(HANDLE);
			break;
		}

		case IOCTL_KOBJEXP_OPEN_EVENT_BY_NAME:
		case IOCTL_KOBJEXP_OPEN_DESKTOP_BY_NAME:
		case IOCTL_KOBJEXP_OPEN_SEMAPHORE_BY_NAME:
		case IOCTL_KOBJEXP_OPEN_JOB_BY_NAME:
		{
			POBJECT_TYPE ObjectType;
			switch (dic.IoControlCode) {
				case IOCTL_KOBJEXP_OPEN_EVENT_BY_NAME: ObjectType = *ExEventObjectType; break;
				case IOCTL_KOBJEXP_OPEN_DESKTOP_BY_NAME: ObjectType = *ExDesktopObjectType; break;
				case IOCTL_KOBJEXP_OPEN_SEMAPHORE_BY_NAME: ObjectType = *ExSemaphoreObjectType; break;
				default: ObjectType = *PsJobType; break;
			}

			if (Irp->AssociatedIrp.SystemBuffer == nullptr) {
				status = STATUS_INVALID_PARAMETER;
				break;
			}
			if (dic.OutputBufferLength < sizeof(HANDLE)) {
				status = STATUS_BUFFER_TOO_SMALL;
				break;
			}
			// the name: at least one character, NUL terminated, whole WCHARs
			if (dic.InputBufferLength < 2 * sizeof(WCHAR) || dic.InputBufferLength % sizeof(WCHAR) != 0) {
				status = STATUS_INVALID_PARAMETER;
				break;
			}
			auto name = (WCHAR*)Irp->AssociatedIrp.SystemBuffer;
			if (name[dic.InputBufferLength / sizeof(WCHAR) - 1] != L'\0') {
				status = STATUS_INVALID_PARAMETER;
				break;
			}
			UNICODE_STRING uname;
			status = RtlInitUnicodeStringEx(&uname, name);
			if (!NT_SUCCESS(status))
				break;

			OBJECT_ATTRIBUTES attr;
			InitializeObjectAttributes(&attr, &uname, OBJ_CASE_INSENSITIVE, nullptr, nullptr);
			HANDLE hObject;
			status = ObOpenObjectByName(&attr, ObjectType, KernelMode, nullptr, GENERIC_READ, nullptr, &hObject);
			if (NT_SUCCESS(status)) {
				*(HANDLE*)Irp->AssociatedIrp.SystemBuffer = hObject;
				len = sizeof(HANDLE);
			}
			break;
		}

		case IOCTL_KOBJEXP_OPEN_PROCESS:
		case IOCTL_KOBJEXP_OPEN_THREAD:
		case IOCTL_KOBJEXP_OPEN_PROCESS_TOKEN:
		{
			if (Irp->AssociatedIrp.SystemBuffer == nullptr) {
				status = STATUS_INVALID_PARAMETER;
				break;
			}
			if (dic.InputBufferLength < sizeof(OpenProcessThreadData) || dic.OutputBufferLength < sizeof(HANDLE)) {
				status = STATUS_BUFFER_TOO_SMALL;
				break;
			}
			auto data = (OpenProcessThreadData*)Irp->AssociatedIrp.SystemBuffer;
			CLIENT_ID id{};
			HANDLE hObject{ nullptr };
			if (dic.IoControlCode == IOCTL_KOBJEXP_OPEN_PROCESS) {
				OBJECT_ATTRIBUTES attr = RTL_CONSTANT_OBJECT_ATTRIBUTES(nullptr, 0);
				id.UniqueProcess = UlongToHandle(data->Id);
				status = ZwOpenProcess(&hObject, data->AccessMask, &attr, &id);
			}
			else if (dic.IoControlCode == IOCTL_KOBJEXP_OPEN_THREAD) {
				OBJECT_ATTRIBUTES attr = RTL_CONSTANT_OBJECT_ATTRIBUTES(nullptr, 0);
				id.UniqueThread = UlongToHandle(data->Id);
				status = ZwOpenThread(&hObject, data->AccessMask, &attr, &id);
			}
			else {
				// the process handle is only needed here, so it's a kernel handle: the caller can't close or replace it
				OBJECT_ATTRIBUTES procAttributes = RTL_CONSTANT_OBJECT_ATTRIBUTES(nullptr, OBJ_KERNEL_HANDLE);
				id.UniqueProcess = UlongToHandle(data->Id);
				const ACCESS_MASK PROCESS_QUERY_INFORMATION = 0x400;
				HANDLE hProcess;
				status = ZwOpenProcess(&hProcess, PROCESS_QUERY_INFORMATION, &procAttributes, &id);
				if (NT_SUCCESS(status)) {
					status = ZwOpenProcessTokenEx(hProcess, data->AccessMask, 0, &hObject);
					ZwClose(hProcess);
				}
			}
			if (NT_SUCCESS(status)) {
				*(HANDLE*)Irp->AssociatedIrp.SystemBuffer = hObject;
				len = sizeof(HANDLE);
			}
			break;
		}

		case IOCTL_KOBJEXP_GET_VERSION:
			if (Irp->AssociatedIrp.SystemBuffer == nullptr) {
				status = STATUS_INVALID_PARAMETER;
				break;
			}
			if (dic.OutputBufferLength < sizeof(USHORT)) {
				status = STATUS_BUFFER_TOO_SMALL;
				break;
			}
			*(USHORT*)Irp->AssociatedIrp.SystemBuffer = DRIVER_CURRENT_VERSION;
			len = sizeof(USHORT);
			status = STATUS_SUCCESS;
			break;

		case IOCTL_KOBJEXP_GET_OBJECT_ADDRESS:
		{
			if (Irp->AssociatedIrp.SystemBuffer == nullptr) {
				status = STATUS_INVALID_PARAMETER;
				break;
			}
			if (dic.InputBufferLength < sizeof(HANDLE) || dic.OutputBufferLength < sizeof(void*)) {
				status = STATUS_BUFFER_TOO_SMALL;
				break;
			}
			// UserMode: the handle must be one of the caller's own, never a kernel handle
			PVOID Object;
			status = ObReferenceObjectByHandle(*(HANDLE*)Irp->AssociatedIrp.SystemBuffer, 0, nullptr, UserMode, &Object, nullptr);
			if (NT_SUCCESS(status)) {
				*(PVOID*)Irp->AssociatedIrp.SystemBuffer = Object;
				ObDereferenceObject(Object);
				len = sizeof(Object);
			}
			break;
		}
	}

	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = len;
	IoCompleteRequest(Irp, 0);
	return status;
}
