#include "pch.h"
#include "ObjectSearcher.h"
#include "ObjectManager.h"

ObjectSearcher::ObjectSearcher(HWND hWnd, ObjectSearchType searchType, DWORD pid, const CString& filter)
	: _hWnd(hWnd), _type(searchType), _pid(pid), _filter(filter) {
}

void ObjectSearcher::SearchAsync(const CString& text) {
	_text = text;
	::QueueUserWorkItem([](PVOID p) -> DWORD {
		((ObjectSearcher*)p)->DoSearch();
		return 0;
		}, this, WT_EXECUTEDEFAULT);
}

void ObjectSearcher::DoSearch() {
	CString stext(_text);
	stext.MakeLower();
	_items.clear();
	_items.reserve(64);

	auto type = _type;
	if (type == ObjectSearchType::Default)
		type = ObjectSearchType::Handles;

	if ((type & ObjectSearchType::Handles) == ObjectSearchType::Handles)
		SearchHandles(stext);

	// when done, post message
	::PostMessage(_hWnd, SearchDoneMessage, 0, 0);
}

void ObjectSearcher::SearchHandles(const CString& stext) {
	ObjectManager om;
	om.EnumHandles(_filter, _pid, true);
	for (auto& h : om.GetHandles()) {
		CString name(h->Name);
		name.MakeLower();
		if (name.Find(stext) >= 0) {
			SearchResultItem item;
			item.Id = h->HandleValue;
			item.Name = h->Name;
			item.Type = om.GetType(h->ObjectTypeIndex)->TypeName;

			_items.emplace_back(std::move(item));
		}
	}
}
