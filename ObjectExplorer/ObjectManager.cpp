#include "stdafx.h"
#include "ObjectManager.h"
#include "DriverHelper.h"
#include "NtDll.h"

#define STATUS_INFO_LENGTH_MISMATCH      ((NTSTATUS)0xC0000004L)

NTSTATUS ObjectManager::EnumObjects() {
	ULONG len = 1 << 26;
	std::unique_ptr<BYTE[]> buffer;
	do {
		buffer = std::make_unique<BYTE[]>(len);
		auto status = NT::NtQuerySystemInformation(NT::SystemObjectInformation, buffer.get(), len, &len);
		if (status == STATUS_INFO_LENGTH_MISMATCH) {
			len <<= 1;
			continue;
		}
		if (status == 0)
			break;
		return status;
	} while (true);

	_allTypeObjects.clear();
	bool first = _allTypeObjects.empty();
	if (first) {
		_allTypeObjects.reserve(64);
		_allObjects.reserve(1 << 17);
	}
	_allObjects.clear();

	auto p = (NT::SYSTEM_OBJECTTYPE_INFORMATION*)buffer.get();
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

		auto pObject = (NT::SYSTEM_OBJECT_INFORMATION*)((BYTE*)p + sizeof(*p) + p->TypeName.MaximumLength);
		for (;;) {
			ATLASSERT(((ULONG_PTR)pObject & 7) == 0);
			auto object = std::make_shared<ObjectInfo>();
			object->HandleCount = pObject->HandleCount;
			object->PointerCount = pObject->PointerCount;
			object->Object = pObject->Object;
			object->CreatorProcess = HandleToULong(pObject->CreatorUniqueProcess);
			//object->ExclusiveProcessId = HandleToULong(pObject->ExclusiveProcessId);
			object->Flags = pObject->Flags;
			object->NonPagedPoolCharge = pObject->NonPagedPoolCharge;
			object->PagedPoolCharge = pObject->PagedPoolCharge;
			object->Name = CString(pObject->NameInfo.Buffer, pObject->NameInfo.Length / sizeof(WCHAR));
			object->Type = type.get();
			object->CreatorName = GetProcessNameById(object->CreatorProcess);

			_allObjects.push_back(object);
			type->Objects.push_back(object);
			if (pObject->NextEntryOffset == 0)
				break;

			pObject = (NT::SYSTEM_OBJECT_INFORMATION*)((BYTE*)buffer.get() + pObject->NextEntryOffset);
		}

		if (p->NextEntryOffset == 0)
			break;

		p = (NT::SYSTEM_OBJECTTYPE_INFORMATION*)(buffer.get() + p->NextEntryOffset);
	}

	return STATUS_SUCCESS;
}

int ObjectManager::EnumTypes() {
	ULONG len = 1 << 17;
	auto buffer = std::make_unique<BYTE[]>(len);
	if (!NT_SUCCESS(NT::NtQueryObject(nullptr, NT::ObjectTypesInformation, buffer.get(), len, &len)))
		return 0;

	auto p = reinterpret_cast<NT::OBJECT_TYPES_INFORMATION*>(buffer.get());
	bool empty = _types.empty();

	auto count = p->NumberOfTypes;
	if (empty) {
		_types.reserve(count);
		_typesMap.reserve(count);
		_changes.reserve(32);
	}
	else {
		_changes.clear();
		ATLASSERT(count == _types.size());
	}
	auto raw = &p->TypeInformation[0];
	_totalHandles = _totalObjects = 0;

	for (ULONG i = 0; i < count; i++) {
		auto type = empty ? std::make_shared<ObjectTypeInfoEx>() : _typesMap[raw->TypeIndex];
		if (empty) {
			type->GenericMapping = raw->GenericMapping;
			type->TypeIndex = raw->TypeIndex;
			type->DefaultNonPagedPoolCharge = raw->DefaultNonPagedPoolCharge;
			type->DefaultPagedPoolCharge = raw->DefaultPagedPoolCharge;
			type->TypeName = CString(raw->TypeName.Buffer, raw->TypeName.Length / 2);
			type->PoolType = (PoolType)raw->PoolType;
			type->DefaultNonPagedPoolCharge = raw->DefaultNonPagedPoolCharge;
			type->DefaultPagedPoolCharge = raw->DefaultPagedPoolCharge;
			type->ValidAccessMask = raw->ValidAccessMask;
			type->InvalidAttributes = raw->InvalidAttributes;
		}
		else {
			if (type->TotalNumberOfHandles != raw->TotalNumberOfHandles)
				_changes.push_back({ type, ChangeType::TotalHandles, (int32_t)raw->TotalNumberOfHandles - (int32_t)type->TotalNumberOfHandles });
			if (type->TotalNumberOfObjects != raw->TotalNumberOfObjects)
				_changes.push_back({ type, ChangeType::TotalObjects, (int32_t)raw->TotalNumberOfObjects - (int32_t)type->TotalNumberOfObjects });
			if (type->HighWaterNumberOfHandles != raw->HighWaterNumberOfHandles)
				_changes.push_back({ type, ChangeType::PeakHandles, (int32_t)raw->HighWaterNumberOfHandles - (int32_t)type->HighWaterNumberOfHandles });
			if (type->HighWaterNumberOfObjects != raw->HighWaterNumberOfObjects)
				_changes.push_back({ type, ChangeType::PeakObjects, (int32_t)raw->HighWaterNumberOfObjects - (int32_t)type->HighWaterNumberOfObjects });
		}

		type->TotalNumberOfHandles = raw->TotalNumberOfHandles;
		type->TotalNumberOfObjects = raw->TotalNumberOfObjects;
		type->TotalNonPagedPoolUsage = raw->TotalNonPagedPoolUsage;
		type->TotalPagedPoolUsage = raw->TotalPagedPoolUsage;
		type->HighWaterNumberOfObjects = raw->HighWaterNumberOfObjects;
		type->HighWaterNumberOfHandles = raw->HighWaterNumberOfHandles;
		type->TotalNamePoolUsage = raw->TotalNamePoolUsage;

		_totalObjects += raw->TotalNumberOfObjects;
		_totalHandles += raw->TotalNumberOfHandles;

		if (empty) {
			_types.emplace_back(type);
			_typesMap.insert({ type->TypeIndex, type });
		}

		auto temp = (BYTE*)raw + sizeof(NT::OBJECT_TYPE_INFORMATION) + raw->TypeName.MaximumLength;
		temp += sizeof(PVOID) - 1;
		raw = reinterpret_cast<NT::OBJECT_TYPE_INFORMATION*>((ULONG_PTR)temp / sizeof(PVOID) * sizeof(PVOID));
	}
	return static_cast<int>(_types.size());
}

bool ObjectManager::EnumHandlesAndObjects() {
	auto now = ::GetTickCount();

	EnumTypes();

	ULONG len = 1 << 25;
	std::unique_ptr<BYTE[]> buffer;
	do {
		buffer = std::make_unique<BYTE[]>(len);
		auto status = NT::NtQuerySystemInformation(NT::SystemExtendedHandleInformation, buffer.get(), len, &len);
		if (status == STATUS_INFO_LENGTH_MISMATCH) {
			len <<= 1;
			continue;
		}
		if (status == 0)
			break;
		return false;
	} while (true);

	auto p = (NT::SYSTEM_HANDLE_INFORMATION_EX*)buffer.get();
	auto count = p->NumberOfHandles;
	_handles.clear();
	_handles.reserve(count);
	_objects.clear();
	_objectsByAddress.clear();
	_objects.reserve(count / 2);
	_objectsByAddress.reserve(count / 2);

	auto& handles = p->Handles;
	for (decltype(count) i = 0; i < count; i++) {
		auto& handle = handles[i];
		auto hi = std::make_shared<HandleInfo>();
		hi->HandleValue = (ULONG)handle.HandleValue;
		hi->GrantedAccess = handle.GrantedAccess;
		hi->Object = handle.Object;
		hi->HandleAttributes = handle.HandleAttributes;
		hi->ProcessId = (ULONG)handle.UniqueProcessId;
		hi->ObjectTypeIndex = handle.ObjectTypeIndex;

		if (auto it = _objectsByAddress.find(handle.Object); it == _objectsByAddress.end()) {
			auto obj = std::make_shared<ObjectInfoEx>();
			obj->HandleCount = 1;
			obj->Object = handle.Object;
			obj->Handles.push_back(hi);
			obj->TypeIndex = handle.ObjectTypeIndex;
			obj->Name = GetObjectName((HANDLE)handle.HandleValue, (ULONG)handle.UniqueProcessId, handle.ObjectTypeIndex);

			_objects.push_back(obj);
			_objectsByAddress.insert({ handle.Object, obj });
		}
		else {
			it->second->HandleCount++;
			it->second->Handles.push_back(hi);
		}


		_handles.push_back(hi);
	}
	CString msg;
	msg.Format(L"time: %d\n", ::GetTickCount() - now);
	OutputDebugString(msg);

	return true;
}

const std::vector<std::shared_ptr<ObjectInfo>>& ObjectManager::GetAllObjects() const {
	return _allObjects;
}

const std::vector<std::shared_ptr<ObjectTypeInfo>>& ObjectManager::GetAllTypeObjects() const {
	return _allTypeObjects;
}

const std::vector<std::shared_ptr<ObjectInfoEx>>& ObjectManager::GetObjects() const {
	return _objects;
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

CString ObjectManager::GetObjectName(HANDLE hObject, ULONG pid, USHORT type) const {
	auto hDup = DriverHelper::DupHandle(hObject, pid, _typesMap.at(type)->ValidAccessMask);
	if (hDup) {
		BYTE buffer[2048];
		if (NT_SUCCESS(NT::NtQueryObject(hDup, NT::ObjectNameInformation, buffer, sizeof(buffer), nullptr))) {
			auto name = (NT::POBJECT_NAME_INFORMATION)buffer;
			return CString(name->Name.Buffer, name->Name.Length / sizeof(WCHAR));
		}
		::CloseHandle(hDup);
	}
	return L"";
}

std::shared_ptr<ObjectTypeInfoEx> ObjectManager::GetType(USHORT index) const {
	return _typesMap.at(index);
}

ProcessInfo::ProcessInfo(DWORD id, PCWSTR name) : Id(id), Name(name) {
}
