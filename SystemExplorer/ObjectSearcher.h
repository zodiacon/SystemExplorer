#pragma once

#include <mutex>

enum class ObjectSearchType {
	Default = 0,
	Handles = 1,
	Devices = 2,
	WindowTitles = 4,
	DLLs = 8,
};
DEFINE_ENUM_FLAG_OPERATORS(ObjectSearchType);

struct SearchResultItem {
	CString Name;
	CString Type;
	CString Details;
	DWORD64 Id;
	DWORD ProcessId;
};

class ObjectSearcher final {
public:
	ObjectSearcher(HWND hWnd, ObjectSearchType searchType = ObjectSearchType::Default, DWORD pid = 0, const CString& filter = L"");

	void SearchAsync(const CString& text);
	bool IsRunning() const {
		return _running;
	}

	std::vector<SearchResultItem> GetResults();

	inline static UINT SearchDoneMessage = ::RegisterWindowMessage(L"SearchDone");

private:
	void SearchHandles(const CString& text);
	void SearchDLLs(const CString& text);
	void DoSearch();
	void CancelSearch();

private:
	ObjectSearchType _type;
	HWND _hWnd;
	CString _filter, _text;
	DWORD _pid;
	std::vector<SearchResultItem> _items;
	std::mutex _lock;
	std::atomic<bool> _cancelRequested{ false };
	std::atomic<bool> _running{ false };
};

