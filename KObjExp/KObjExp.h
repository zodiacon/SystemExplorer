#pragma once

#define IOCTL_KOBJEXP_OPEN_OBJECT	CTL_CODE(0x8000, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_KOBJEXP_DUP_HANDLE	CTL_CODE(0x8000, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)

struct OpenObjectData {
	void* Address;
	ACCESS_MASK Access;
};

struct DupHandleData {
	ULONG Handle;
	ULONG SourcePid;
	ACCESS_MASK AccessMask;
};
