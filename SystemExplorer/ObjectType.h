#pragma once

class ObjectType abstract {
public:
	virtual ~ObjectType();

	int GetTypeIndex() const;
	const CString& GetName() const;

	virtual CString GetDetails(HANDLE hObject) = 0;

protected:
	ObjectType(int typeIndex, PCWSTR name);

private:
	CString _name;
	int _index;
};

