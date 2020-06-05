#pragma once

struct FormatHelper {
	static CString TimeSpanToString(int64_t ts);
	static CString FormatWithCommas(long long size);
};

