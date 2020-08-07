#include "pch.h"
#include "FormatHelper.h"

CString FormatHelper::TimeSpanToString(int64_t ts) {
	auto str = CTimeSpan(ts / 10000000).Format(L"%D.%H:%M:%S");

	str.Format(L"%s.%03d", str, (ts / 10000) % 1000);
	return str;
}

CString FormatHelper::FormatWithCommas(long long size) {
	CString result;
	result.Format(L"%lld", size);
	int i = 3;
	while (result.GetLength() - i > 0) {
		result = result.Left(result.GetLength() - i) + L"," + result.Right(i);
		i += 4;
	}
	return result;
}

CString FormatHelper::TimeToString(int64_t time, bool includeMS) {
	auto str = CTime(*(FILETIME*)&time).Format(L"%x %X");
	if (includeMS) {
		str.Format(L"%s.%03d", str, (time / 10000) % 1000);
	}
	return str;
}
