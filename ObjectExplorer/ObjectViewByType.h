#pragma once

#include "Interfaces.h"

class ObjectViewByType : public IObjectsView {
public:
	ObjectViewByType(PCWSTR type, PCWSTR title);

	// Inherited via IObjectsView
	CString GetTitle() override;
	bool ShowObject(ObjectInfo* info) override;

private:
	CString _type, _title;

};

