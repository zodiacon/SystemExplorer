#include <ntifs.h>
#include <ntddk.h>
#include "ObjExp.h"
#include "KObjExp.h"

DRIVER_UNLOAD ObjExpUnload;

DRIVER_DISPATCH ObjExpCreateClose, ObjExpDeviceControl;

extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING) {
	PDEVICE_OBJECT DeviceObject;
	UNICODE_STRING devName = RTL_CONSTANT_STRING(L"\\Device\\KObjExp");
	auto status = IoCreateDevice(DriverObject, 0, &devName, FILE_DEVICE_UNKNOWN, 0, FALSE, &DeviceObject);
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
	NT_ASSERT_ASSUME(DriverObject);
	IoDeleteDevice(DriverObject->DeviceObject);
}

NTSTATUS ObjExpCreateClose(PDEVICE_OBJECT, PIRP Irp) {
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, 0);
	return STATUS_SUCCESS;
}

NTSTATUS ObjExpDeviceControl(PDEVICE_OBJECT, PIRP Irp) {
	auto& dic = IoGetCurrentIrpStackLocation(Irp)->Parameters.DeviceIoControl;
	auto status = STATUS_INVALID_DEVICE_REQUEST;
	ULONG len = 0;

	switch (dic.IoControlCode) {
		case IOCTL_KOBJEXP_OPEN_OBJECT:
			if (Irp->AssociatedIrp.SystemBuffer == nullptr) {
				status = STATUS_INVALID_PARAMETER;
				break;
			}
			if (dic.InputBufferLength < sizeof(OpenObjectData)) {
				status = STATUS_BUFFER_TOO_SMALL;
				break;
			}

			auto data = (OpenObjectData*)Irp->AssociatedIrp.SystemBuffer;
			HANDLE hObject;
			status = ObReferenceObjectByPointer(data->Address, data->Access, nullptr, KernelMode);
			if (!NT_SUCCESS(status))
				break;

			status = ObOpenObjectByPointer(data->Address, 0, nullptr, data->Access,
				nullptr, KernelMode, &hObject);
			if (NT_SUCCESS(status)) {
				*(HANDLE*)Irp->AssociatedIrp.SystemBuffer = hObject;
				len = sizeof(HANDLE);
			}
			ObDereferenceObject(data->Address);
			break;
	}

	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = len;
	IoCompleteRequest(Irp, 0);
	return status;
}
