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
        m->Name = std::wstring((PCWSTR)((BYTE*)p + p->BaseInfo.OffsetToFileName));
    } while (p->NextOffset != 0);

    return uint32_t();
}
