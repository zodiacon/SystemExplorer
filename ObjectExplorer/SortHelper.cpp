#include "stdafx.h"
#include "SortHelper.h"

bool SortHelper::SortStrings(const CString& s1, const CString& s2, bool ascending) {
	return ascending ? s2.CompareNoCase(s1) > 0 : s2.CompareNoCase(s1) < 0;
}

bool SortHelper::SortStrings(const std::string& s1, const std::string& s2, bool ascending) {
	auto compare = ::_stricmp(s2.c_str(), s1.c_str());
	return ascending ? compare > 0 : compare < 0;
}

bool SortHelper::SortBoolean(bool a, bool b, bool asc) {
	return asc ? b > a : a > b;
}
