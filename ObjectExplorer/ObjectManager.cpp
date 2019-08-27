#include "stdafx.h"
#include "ObjectManager.h"
#include "NtDll.h"

#define STATUS_INFO_LENGTH_MISMATCH      ((NTSTATUS)0xC0000004L)

NTSTATUS ObjectManager::EnumObjects() {
	ULONG len = 1 << 26;
	std::unique_ptr<BYTE[]> buffer;
	do {
		buffer = std::make_unique<BYTE[]>(len);
		auto status = ::NtQuerySystemInformation(SystemObjectInformation, buffer.get(), len, &len);
		if (status == STATUS_INFO_LENGTH_MISMATCH) {
			len <<= 1;
			continue;
		}
		if (status == 0)
			break;
		return status;
	} while (true);

	bool first = _allTypeObjects.empty();
	if (first) {
		_allTypeObjects.reserve(64);
		_allObjects.reserve(1 << 17);
	}
	else {
		_allObjects.clear();
	}

	auto p = (SYSTEM_OBJECTTYPE_INFORMATION*)buffer.get();
	int itype = 0;
	for (;;) {
		ATLASSERT(((ULONG_PTR)p & 7) == 0);
		std::shared_ptr< ObjectTypeInfo> type;
		if (first) {
			type = std::make_shared<ObjectTypeInfo>();
			type->TypeIndex = p->TypeIndex;
			type->GenericMapping = p->GenericMapping;
			type->InvalidAttributes = p->InvalidAttributes;
			type->Name = CString(p->TypeName.Buffer, p->TypeName.Length / sizeof(WCHAR));
			type->SecurityRequired = p->SecurityRequired;
			type->PoolType = (PoolType)p->PoolType;
			type->WaitableObject = p->WaitableObject;
			type->Objects.reserve(64);
			_allTypeObjects.push_back(type);
		}
		else {
			type = _allTypeObjects[itype++];
			type->Objects.clear();
		}

		type->NumberOfObjects = p->NumberOfObjects;
		type->NumberOfHandles = p->NumberOfHandles;

		auto pObject = (SYSTEM_OBJECT_INFORMATION*)((BYTE*)p + sizeof(*p) + p->TypeName.MaximumLength);
		for (;;) {
			ATLASSERT(((ULONG_PTR)pObject & 7) == 0);
			auto object = std::make_shared<ObjectInfo>();
			object->HandleCount = pObject->HandleCount;
			object->PointerCount = pObject->PointerCount;
			object->Object = pObject->Object;
			object->CreatorProcess = HandleToULong(pObject->CreatorUniqueProcess);
			object->ExclusiveProcessId = HandleToULong(pObject->ExclusiveProcessId);
			object->Flags = pObject->Flags;
			object->NonPagedPoolCharge = pObject->NonPagedPoolCharge;
			object->PagedPoolCharge = pObject->PagedPoolCharge;
			object->Name = CString(pObject->NameInfo.Buffer, pObject->NameInfo.Length / sizeof(WCHAR));
			object->Type = type.get();

			_allObjects.push_back(object);
			type->Objects.push_back(object);
			if (pObject->NextEntryOffset == 0)
				break;

			pObject = (SYSTEM_OBJECT_INFORMATION*)((BYTE*)buffer.get() + pObject->NextEntryOffset);
		}

		if (p->NextEntryOffset == 0)
			break;

		p = (SYSTEM_OBJECTTYPE_INFORMATION*)(buffer.get() + p->NextEntryOffset);
	}

	return STATUS_SUCCESS;
}

const std::vector<std::shared_ptr<ObjectInfo>>& ObjectManager::GetAllObjects() const {
	return _allObjects;
}

const std::vector<std::shared_ptr<ObjectTypeInfo>>& ObjectManager::GetAllTypeObjects() const {
	return _allTypeObjects;
}

bool ObjectManager::EnumProcesses() {
	wil::unique_handle hSnapshot(::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0));
	if (!hSnapshot)
		return false;

	PROCESSENTRY32 pe;
	pe.dwSize = sizeof(pe);

	if (!::Process32First(hSnapshot.get(), &pe))
		return false;

	_processesById.clear();
	_processesById.reserve(256);

	do {
		ProcessInfo pi(pe.th32ProcessID, pe.szExeFile);
		_processesById.emplace(pe.th32ProcessID, pi);
	} while (::Process32Next(hSnapshot.get(), &pe));

	return true;
}

const CString& ObjectManager::GetProcessNameById(DWORD id) const {
	static CString empty;
	auto it = _processesById.find(id);
	return it == _processesById.end() ? empty : it->second.Name;
}

ProcessInfo::ProcessInfo(DWORD id, PCWSTR name) : Id(id), Name(name) {
}
