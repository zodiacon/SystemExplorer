#include "stdafx.h"
#include "FileObjectType.h"

FileObjectType::FileObjectType(int index, PCWSTR name) : ObjectType(index, name) {
}

CString FileObjectType::GetDetails(HANDLE hObject) {
	struct Data {
		CString details;
		HANDLE hFile;
	} data;

	data.hFile = hObject;

	wil::unique_handle hThread(::CreateThread(nullptr, 1 << 13, [](auto p) -> DWORD {
		BY_HANDLE_FILE_INFORMATION info;
		auto data = (Data*)p;
		if (::GetFileInformationByHandle(data->hFile, &info)) {
			CString size;
			if (info.nFileSizeHigh == 0 && info.nFileSizeLow < (10 << 10))
				size.Format(L"%u B", info.nFileSizeLow);
			else
				size.Format(L"%lld KB", (((ULONGLONG)info.nFileSizeHigh << 32) | info.nFileSizeLow) >> 10);
			data->details.Format(L"Size: %s, Created: %s, Last Write: %s", (PCWSTR)size,
				(PCWSTR)CTime(info.ftCreationTime).Format(L"%D %X"), (PCWSTR)CTime(info.ftLastWriteTime).Format(L"%D %X"));
		}
		return 0;
		}, &data, STACK_SIZE_PARAM_IS_A_RESERVATION, nullptr));

	if (::WaitForSingleObject(hThread.get(), 6) == WAIT_TIMEOUT) {
		::TerminateThread(hThread.get(), 1);
		return L"";
	}
	return data.details;
}
