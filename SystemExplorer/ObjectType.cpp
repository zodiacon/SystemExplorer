#include "stdafx.h"
#include "ObjectType.h"

ObjectType::~ObjectType() = default;

int ObjectType::GetTypeIndex() const {
	return _index;
}

const CString & ObjectType::GetName() const {
	return _name;
}

ObjectType::ObjectType(int typeIndex, PCWSTR name) : _index(typeIndex), _name(name) {
}
