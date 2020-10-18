#include "pch.h"
#include "KernelModuleTracker.h"

uint32_t WinSys::KernelModuleTracker::EnumModules() {
    BYTE buffer[1 << 12];
    if (!NT_SUCCESS(::NtQuerySystemInformation(SystemModuleInformationEx, buffer, sizeof(buffer), nullptr)))
        return 0;

    auto p = (RTL_PROCESS_MODULE_INFORMATION_EX*)buffer;
    do {
        auto m = std::make_shared<KernelModuleInfo>();
        m->Flags = p->BaseInfo.Flags;
        m->FullPath = (const char*)p->BaseInfo.FullPathName;
        m->MappedBase = p->BaseInfo.MappedBase;
        m->ImageBase = p->BaseInfo.ImageBase;
        m->ImageSize = p->BaseInfo.ImageSize;
        m->InitOrderIndex = p->BaseInfo.InitOrderIndex;
        m->LoadOrderIndex = p->BaseInfo.LoadOrderIndex;
        m->LoadCount = p->BaseInfo.LoadCount;
        m->hSection = p->BaseInfo.Section;
        m->DefaultBase = p->DefaultBase;
        m->ImageChecksum = p->ImageChecksum;
        m->TimeDateStamp = p->TimeDateStamp;
        m->Name = std::wstring((PCWSTR)((BYTE*)p + p->BaseInfo.OffsetToFileName));
    } while (p->NextOffset != 0);

    return uint32_t();
}
