#include "pch.h"
#include "ObjectTypeFactory.h"
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
#include "WinStationObjectType.h"
#include "WorkerFactoryObjectType.h"
#include "DeviceObjectType.h"

std::unique_ptr<ObjectType> ObjectTypeFactory::CreateObjectType(int typeIndex, const CString& name) {
	static WinSys::ProcessManager procMgr;

	procMgr.EnumProcesses();
	if (name == L"Mutant")
		return std::make_unique<MutexObjectType>(typeIndex, name);
	if (name == L"Process")
		return std::make_unique<ProcessObjectType>(procMgr, typeIndex, name);
	if (name == L"Thread")
		return std::make_unique<ThreadObjectType>(procMgr, typeIndex, name);
	if (name == L"Semaphore")
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
	if (name == L"WindowStation")
		return std::make_unique<WinStationObjectType>(typeIndex, name);
	if (name == L"TpWorkerFactory")
		return std::make_unique<WorkerFactoryObjectType>(procMgr, typeIndex, name);
	if(name == L"Device")
		return std::make_unique<DeviceObjectType>(typeIndex, name);
	return nullptr;
}
