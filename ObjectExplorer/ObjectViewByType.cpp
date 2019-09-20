#include "stdafx.h"
#include "ObjectViewByType.h"
#include "ObjectManager.h"

ObjectViewByType::ObjectViewByType(PCWSTR name, PCWSTR title) : _type(name), _title(title) {
}

bool ObjectViewByType::ShowObject(ObjectInfo * info) {
	return _type == info->TypeName;
}

CString ObjectViewByType::GetTitle() {
	return _title;
}


