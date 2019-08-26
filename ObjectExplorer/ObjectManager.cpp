#include "stdafx.h"
#include "ObjectManager.h"
#include "NtDll.h"

bool ObjectManager::EnumObjects() {
	ULONG len = 1 << 26;
	std::unique_ptr<BYTE[]> buffer;
	do {
		buffer = std::make_unique<BYTE[]>(len);
		auto status = ::NtQuerySystemInformation(SystemObjectInformation, buffer.get(), len, &len);
		if (status == 0xc0000004) {
			len <<= 1;
			continue;
		}
		if (status == 0)
			break;
		return false;
	} while (true);

	auto p = (SYSTEM_OBJECTTYPE_INFORMATION*)buffer.get();
	for (;;) {
		auto pObject = (SYSTEM_OBJECT_INFORMATION*)((BYTE*)p + sizeof(*p) + p->TypeName.MaximumLength);
		for (;;) {
			if (pObject->NextEntryOffset == 0)
				break;

			pObject = (SYSTEM_OBJECT_INFORMATION*)((BYTE*)p + pObject->NextEntryOffset);
		}

		if (p->NextEntryOffset == 0)
			break;

		p = (SYSTEM_OBJECTTYPE_INFORMATION*)(buffer.get() + p->NextEntryOffset);
	}

	return true;
}
