#include "pch.h"
#include "ClipboardHelper.h"

bool ClipboardHelper::CopyText(HWND hWnd, PCWSTR text) {
	if (!::OpenClipboard(hWnd))
		return false;

	::EmptyClipboard();
	bool ok = false;
	auto size = (::wcslen(text) + 1) * sizeof(WCHAR);
	auto hData = ::GlobalAlloc(GMEM_MOVEABLE, size);
	if (hData) {
		if (auto p = ::GlobalLock(hData); p) {
			::memcpy(p, text, size);
			::GlobalUnlock(hData);		// the memory handle, not the pointer
			ok = ::SetClipboardData(CF_UNICODETEXT, hData) != nullptr;
		}
		// the clipboard owns the memory once it's set; otherwise it's still ours
		if (!ok)
			::GlobalFree(hData);
	}
	::CloseClipboard();
	return ok;
}
