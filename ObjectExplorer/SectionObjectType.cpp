#include "stdafx.h"
#include "SectionObjectType.h"
#include "NtDll.h"

SectionObjectType::SectionObjectType(int index, PCWSTR name) : ObjectType(index, name) {
}

CString SectionObjectType::GetDetails(HANDLE hSection) {
	CString details;
	NT::SECTION_BASIC_INFORMATION bi;
	if (NT_SUCCESS(NT::NtQuerySection(hSection, NT::SectionBasicInformation, &bi, sizeof(bi), nullptr))) {
		details.Format(L"Size: %d KiB, Attributes: 0x%X (%s)",
			bi.MaximumSize.QuadPart >> 10, bi.AllocationAttributes, SectionAttributesToString(bi.AllocationAttributes));
	}
	return details;
}

CString SectionObjectType::SectionAttributesToString(DWORD value) {
	CString text;
	struct {
		DWORD attribute;
		PCWSTR text;
	} attributes[] = {
		{ SEC_COMMIT, L"Commit" },
		{ SEC_RESERVE, L"Reserve" },
		{ SEC_IMAGE, L"Image" },
		{ SEC_NOCACHE, L"No Cache" },
		{ SEC_FILE, L"File" },
		{ SEC_WRITECOMBINE, L"Write Combine" },
		{ SEC_PROTECTED_IMAGE, L"Protected Image" },
		{ SEC_LARGE_PAGES, L"Large Pages" },
		{ SEC_IMAGE_NO_EXECUTE, L"No Execute" },
	};

	for (auto& item : attributes)
		if (value & item.attribute)
			(text += item.text) += L", ";
	if (text.GetLength() == 0)
		text = L"None";
	else
		text = text.Left(text.GetLength() - 2);
	return text;
}
