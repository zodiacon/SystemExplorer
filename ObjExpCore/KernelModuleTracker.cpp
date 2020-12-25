#include "pch.h"
#include "KernelModuleTracker.h"

using namespace WinSys;

uint32_t KernelModuleTracker::EnumModules() {
    DWORD size = 1 << 18;
    wil::unique_virtualalloc_ptr<> buffer(::VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));

    if (!NT_SUCCESS(::NtQuerySystemInformation(SystemModuleInformationEx, buffer.get(), size, nullptr)))
        return 0;

    if (_modules.empty()) {
        _modules.reserve(256);
    }

    auto p = (RTL_PROCESS_MODULE_INFORMATION_EX*)buffer.get();
    CHAR winDir[MAX_PATH];
    ::GetWindowsDirectoryA(winDir, _countof(winDir));
    static const std::string root("\\SystemRoot\\");

    for(;;) {
        if (p->BaseInfo.ImageBase == 0)
            break;
        auto m = std::make_shared<KernelModuleInfo>();
        m->Flags = p->BaseInfo.Flags;
        m->FullPath = (const char*)p->BaseInfo.FullPathName;
        if (m->FullPath.find(root) == 0)
            m->FullPath = winDir + m->FullPath.substr(root.size() - 1);
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
        m->Name = std::string((PCSTR)(p->BaseInfo.FullPathName + p->BaseInfo.OffsetToFileName));

        _modules.push_back(std::move(m));

        if (p->NextOffset == 0)
            break;
        p = (RTL_PROCESS_MODULE_INFORMATION_EX*)((BYTE*)p + p->NextOffset);
    }

    return uint32_t(_modules.size());
}

const std::vector<std::shared_ptr<KernelModuleInfo>>& KernelModuleTracker::GetModules() const {
    return _modules;
}