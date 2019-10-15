#include "stdafx.h"
#include "ObjectManager.h"
#include "DriverHelper.h"
#include "NtDll.h"
#include "MutexObjectType.h"
#include "ProcessObjectType.h"
#include "ThreadObjectType.h"
#include "SectionObjectType.h"
#include "SemaphoreObjectType.h"
#include "EventObjectType.h"
#include "SymbolicLinkObjectType.h"
#include "KeyObjectType.h"
#include "JobObjectType.h"
#include "TokenObjectType.h"
#include "FileObjectType.h"
#include "TimerObjectType.h"

#define STATUS_INFO_LENGTH_MISMATCH      ((NTSTATUS)0xC0000004L)

std::vector<std::shared_ptr<ObjectTypeInfo>> ObjectManager::_types;
std::unordered_map<int16_t, std::shared_ptr<ObjectTypeInfo>> ObjectManager::_typesMap;
std::unordered_map<std::wstring, std::shared_ptr<ObjectTypeInfo>> ObjectManager::_typesNameMap;
std::vector<ObjectManager::Change> ObjectManager::_changes;
int64_t ObjectManager::_totalHandles;
int64_t ObjectManager::_totalObjects;

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
		auto type = empty ? std::make_shared<ObjectTypeInfo>() : _typesMap[raw->TypeIndex];
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
			auto typeObject = CreateObjectType(type->TypeIndex, type->TypeName);
			type->TypeDetails = std::move(typeObject);
			_typesNameMap.insert({ std::wstring(type->TypeName), type });
		}

		auto temp = (BYTE*)raw + sizeof(NT::OBJECT_TYPE_INFORMATION) + raw->TypeName.MaximumLength;
		temp += sizeof(PVOID) - 1;
		raw = reinterpret_cast<NT::OBJECT_TYPE_INFORMATION*>((ULONG_PTR)temp / sizeof(PVOID) * sizeof(PVOID));
	}
	return static_cast<int>(_types.size());
}

bool ObjectManager::EnumHandlesAndObjects(PCWSTR type) {
	EnumTypes();
	EnumProcesses();

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

	auto filteredTypeIndex = type == nullptr || ::wcslen(type) == 0 ? -1 : _typesNameMap.at(type)->TypeIndex;

	auto p = (NT::SYSTEM_HANDLE_INFORMATION_EX*)buffer.get();
	auto count = p->NumberOfHandles;
	_objects.clear();
	_objectsByAddress.clear();
	_objects.reserve(count / 2);
	_objectsByAddress.reserve(count / 2);

	auto& handles = p->Handles;
	for (decltype(count) i = 0; i < count; i++) {
		auto& handle = handles[i];
		if (filteredTypeIndex >= 0 && handle.ObjectTypeIndex != filteredTypeIndex)
			continue;

		auto hi = std::make_shared<HandleInfo>();
		hi->HandleValue = (ULONG)handle.HandleValue;
		hi->GrantedAccess = handle.GrantedAccess;
		hi->Object = handle.Object;
		hi->HandleAttributes = handle.HandleAttributes;
		hi->ProcessId = (ULONG)handle.UniqueProcessId;
		hi->ObjectTypeIndex = handle.ObjectTypeIndex;
		if (auto it = _objectsByAddress.find(handle.Object); it == _objectsByAddress.end()) {
			auto obj = std::make_shared<ObjectInfo>();
			obj->HandleCount = 1;
			obj->Object = handle.Object;
			obj->Handles.push_back(hi);
			obj->TypeIndex = handle.ObjectTypeIndex;
			obj->TypeName = GetType(obj->TypeIndex)->TypeName;
			GetObjectInfo(obj.get(), (HANDLE)handle.HandleValue, (ULONG)handle.UniqueProcessId, handle.ObjectTypeIndex);

			_objects.push_back(obj);
			_objectsByAddress.insert({ handle.Object, obj });
		}
		else {
			it->second->HandleCount++;
			it->second->Handles.push_back(hi);
		}
	}

	return true;
}

bool ObjectManager::EnumHandles(PCWSTR type) {
	EnumTypes();
	EnumProcesses();

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

	auto filteredTypeIndex = type == nullptr || ::wcslen(type) == 0 ? -1 : _typesNameMap.at(type)->TypeIndex;

	auto p = (NT::SYSTEM_HANDLE_INFORMATION_EX*)buffer.get();
	auto count = p->NumberOfHandles;
	_handles.clear();
	_handles.reserve(count);
	for (decltype(count) i = 0; i < count; i++) {
		auto& handle = p->Handles[i];
		if (filteredTypeIndex >= 0 && handle.ObjectTypeIndex != filteredTypeIndex)
			continue;

		auto hi = std::make_shared<HandleInfo>();
		hi->HandleValue = (ULONG)handle.HandleValue;
		hi->GrantedAccess = handle.GrantedAccess;
		hi->Object = handle.Object;
		hi->HandleAttributes = handle.HandleAttributes;
		hi->ProcessId = (ULONG)handle.UniqueProcessId;
		hi->ObjectTypeIndex = handle.ObjectTypeIndex;
		_handles.emplace_back(hi);
	}

	return true;
}

const std::vector<std::shared_ptr<ObjectInfo>>& ObjectManager::GetObjects() const {
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

const std::vector<ObjectManager::Change>& ObjectManager::GetChanges() const {
	return _changes;
}

std::unique_ptr<ObjectType> ObjectManager::CreateObjectType(int typeIndex, const CString& name) const {
	if (name == L"Mutant")
		return std::make_unique<MutexObjectType>(typeIndex, name);
	if (name == L"Process")
		return std::make_unique<ProcessObjectType>(*this, typeIndex, name);
	if (name == L"Thread")
		return std::make_unique<ThreadObjectType>(*this, typeIndex, name);
	if(name == L"Semaphore")
		return std::make_unique<SemaphoreObjectType>(typeIndex, name);
	if (name == L"Section")
		return std::make_unique<SectionObjectType>(typeIndex, name);
	if (name == L"Event")
		return std::make_unique<EventObjectType>(typeIndex, name);
	if (name == L"SymbolicLink")
		return std::make_unique<SymbolicLinkObjectType>(typeIndex, name);
	if (name == L"Key")
		return std::make_unique<KeyObjectType>(typeIndex, name);
	if (name == L"Job")
		return std::make_unique<JobObjectType>(typeIndex, name);
	if (name == L"Token")
		return std::make_unique<TokenObjectType>(typeIndex, name);
	if (name == L"File")
		return std::make_unique<FileObjectType>(typeIndex, name);
	if (name == L"Timer")
		return std::make_unique<TimerObjectType>(typeIndex, name);

	return nullptr;
}

HANDLE ObjectManager::DupHandle(ObjectInfo * pObject, ACCESS_MASK access) {
	for (auto& h : pObject->Handles) {
		auto hDup = DriverHelper::DupHandle(ULongToHandle(h->HandleValue), h->ProcessId, //access);
			_typesMap.at(h->ObjectTypeIndex)->ValidAccessMask);
		if (hDup)
			return hDup;
	}
	return nullptr;
}

int64_t ObjectManager::GetTotalHandles() {
	return _totalHandles;
}

int64_t ObjectManager::GetTotalObjects() {
	return _totalObjects;
}

bool ObjectManager::GetObjectInfo(ObjectInfo* info, HANDLE hObject, ULONG pid, USHORT type) const {
	auto hDup = DupHandle(info);
	if (hDup) {
		info->Name = GetObjectName(hDup, type);
		::CloseHandle(hDup);
		return true;
	}
	return false;
}

CString ObjectManager::GetObjectName(HANDLE hObject, ULONG pid, USHORT type) const {
	auto hDup = DriverHelper::DupHandle(hObject, pid, 0);
	return GetObjectName(hDup, type);
}

CString ObjectManager::GetObjectName(HANDLE hDup, USHORT type) const {
	static int processTypeIndex = _typesNameMap.find(L"Process")->second->TypeIndex;
	static int threadTypeIndex = _typesNameMap.find(L"Thread")->second->TypeIndex;
	static int fileTypeIndex = _typesNameMap.find(L"File")->second->TypeIndex;
	ATLASSERT(processTypeIndex > 0 && threadTypeIndex > 0);

	CString sname;
	do {
		if (type == processTypeIndex || type == threadTypeIndex)
			break;

		BYTE buffer[2048];
		if (type == fileTypeIndex) {
			// special case for files in case they're locked
			struct Data {
				HANDLE hDup;
				BYTE* buffer;
			} data = { hDup, buffer };

			wil::unique_handle hThread(::CreateThread(nullptr, 1 << 13, [](auto p) {
				auto d = (Data*)p;
				return (DWORD)NT_SUCCESS(NT::NtQueryObject(d->hDup, NT::ObjectNameInformation, d->buffer, sizeof(buffer), nullptr));
				}, &data, STACK_SIZE_PARAM_IS_A_RESERVATION, nullptr));
			if (::WaitForSingleObject(hThread.get(), 6) == WAIT_TIMEOUT) {
				::TerminateThread(hThread.get(), 1);
			}
			else {
				DWORD code;
				::GetExitCodeThread(hThread.get(), &code);
				if (code == STATUS_SUCCESS) {
					auto name = (NT::POBJECT_NAME_INFORMATION)buffer;
					sname = CString(name->Name.Buffer, name->Name.Length / sizeof(WCHAR));
				}
			}
		}
		else if (NT_SUCCESS(NT::NtQueryObject(hDup, NT::ObjectNameInformation, buffer, sizeof(buffer), nullptr))) {
			auto name = (NT::POBJECT_NAME_INFORMATION)buffer;
			sname = CString(name->Name.Buffer, name->Name.Length / sizeof(WCHAR));
		}
	} while (false);

	return sname;
}

std::shared_ptr<ObjectTypeInfo> ObjectManager::GetType(USHORT index) {
	return _typesMap.at(index);
}

std::shared_ptr<ObjectTypeInfo> ObjectManager::GetType(PCWSTR name) {
	return _typesNameMap.at(name);
}

const std::vector<std::shared_ptr<ObjectTypeInfo>>& ObjectManager::GetObjectTypes() {
	return _types;
}

const std::vector<std::shared_ptr<HandleInfo>>& ObjectManager::GetHandles() const {
	return _handles;
}

ProcessInfo::ProcessInfo(DWORD id, PCWSTR name) : Id(id), Name(name) {
}
