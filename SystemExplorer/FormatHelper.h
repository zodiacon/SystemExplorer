#pragma once

struct FormatHelper {
	static CString TimeSpanToString(int64_t ts);
	static CString FormatWithCommas(long long size);
	static CString TimeToString(int64_t time, bool includeMS = true);
};

