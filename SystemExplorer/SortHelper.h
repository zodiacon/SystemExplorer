#pragma once

struct SortHelper final abstract {
	static bool SortStrings(const ATL::CString& s1, const ATL::CString& s2, bool ascending);
	static bool SortStrings(const std::string& s1, const std::string& s2, bool ascending);
	static bool SortStrings(const std::wstring& s1, const std::wstring& s2, bool ascending);
	static bool SortStrings(PCWSTR s1, PCWSTR s2, bool ascending);
	static bool SortBoolean(bool a, bool b, bool asc);

	template<typename Number>
	static bool SortNumbers(const Number n1, const Number n2, bool ascending) {
		return ascending ? n2 > n1 : n2 < n1;
	}

	//
	// Sorts items by a key read once per item. For values queried from the system (not part of a snapshot): read
	// in the comparison, such a value can change in the middle of the sort, and std::sort's behavior is undefined if
	// the comparison isn't consistent (it can even read out of range); it's also one query per comparison.
	//
	template<typename Item, typename GetKey, typename Less>
	static void SortByKey(std::vector<Item>& items, GetKey getKey, Less less) {
		using Key = std::decay_t<decltype(getKey(items.front()))>;
		std::vector<std::pair<Key, Item>> keyed;
		keyed.reserve(items.size());
		for (auto& item : items) {
			Key key = getKey(item);
			keyed.emplace_back(std::move(key), std::move(item));
		}
		std::sort(keyed.begin(), keyed.end(), [&](const auto& a, const auto& b) { return less(a.first, b.first); });
		for (size_t i = 0; i < items.size(); i++)
			items[i] = std::move(keyed[i].second);
	}
};
