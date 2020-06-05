#pragma once

class ObjectType;

class ObjectTypeFactory abstract final {
public:
	static std::unique_ptr<ObjectType> CreateObjectType(int typeIndex, const CString& name);
};

