#include "pch.h"
#include "FileObjectType.h"

FileObjectType::FileObjectType(int index, PCWSTR name) : ObjectType(index, name) {
}

CString FileObjectType::GetDetails(HANDLE hObject) {
	//
	// GetFileInformationByHandle blocks for some files (e.g. a synchronous pipe that's being waited on), so it's called
	// on a thread that's abandoned if it takes too long. The thread writes only to its own heap block (never to this
	// stack frame), which is freed only once the thread is known to be gone, and it makes no allocations, so killing
	// it can't leave a lock (such as the heap's) held. The text is formatted here, after the thread is done.
	//
	struct Data {
		HANDLE hFile;
		BOOL ok;
		BY_HANDLE_FILE_INFORMATION info;
	};
	auto data = new Data{ hObject, FALSE };

	wil::unique_handle hThread(::CreateThread(nullptr, 1 << 13, [](auto p) -> DWORD {
		auto data = (Data*)p;
		data->ok = ::GetFileInformationByHandle(data->hFile, &data->info);
		return 0;
		}, data, STACK_SIZE_PARAM_IS_A_RESERVATION, nullptr));
	if (!hThread) {
		delete data;
		return L"";
	}

	if (::WaitForSingleObject(hThread.get(), 6) == WAIT_TIMEOUT) {
		// termination is asynchronous: free the block only if the thread is gone, or it may still write to it
		::TerminateThread(hThread.get(), 1);
		if (::WaitForSingleObject(hThread.get(), 200) == WAIT_OBJECT_0)
			delete data;
		return L"";
	}

	CString details;
	if (data->ok) {
		auto& info = data->info;
		CString size;
		if (info.nFileSizeHigh == 0 && info.nFileSizeLow < (10 << 10))
			size.Format(L"%u B", info.nFileSizeLow);
		else
			size.Format(L"%lld KB", (((ULONGLONG)info.nFileSizeHigh << 32) | info.nFileSizeLow) >> 10);
		details.Format(L"Size: %s, Created: %s, Last Write: %s", (PCWSTR)size,
			(PCWSTR)CTime(info.ftCreationTime).Format(L"%D %X"), (PCWSTR)CTime(info.ftLastWriteTime).Format(L"%D %X"));
	}
	delete data;
	return details;
}
