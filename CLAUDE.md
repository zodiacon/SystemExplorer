# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

System Explorer (aka SysExp / ObjExp) — a Windows systems tool (like a much-expanded Process Explorer) written in
C++ with WTL. It shows processes, threads, kernel objects, handles, services, devices, COM objects, logon
sessions, and more, and can optionally load a kernel driver for privileged operations.

## Solution layout

`SystemExplorer.sln` (Visual Studio 2022, toolset `v145`) contains four projects with a strict dependency chain:

- **`KObjExp`** — kernel-mode driver (`WindowsKernelModeDriver10.0` toolset, WDK). Exposes a device
  `\Device\KObjExp` (symlink `\??\KObjExp`) via `IRP_MJ_CREATE/CLOSE/DEVICE_CONTROL`. Only lets specific client
  executables open it (checked by image file name in `ObjExpCreateClose` — `SysExp.exe`, `ObjExp.exe`,
  `TotalSys.exe`). Used for privileged operations the app can't do from user mode alone (e.g. duplicating
  handles across processes, reading raw object addresses). Talked to from user mode via `SystemExplorer/DriverHelper.*`.
- **`ObjExpCore`** — static library (C++17), the core/backend: process, thread, service, device, object-manager,
  COM, security/token, and system-information data collection, all built directly on top of the Native API
  (`phnt`) rather than the documented Win32 API where the documented API is insufficient. No UI code here.
- **`WTLHelper`** — git **submodule** (`https://github.com/zodiacon/WTLHelper.git`), a personal WTL utility
  library shared across the author's other tools (custom controls, column manager, dark-mode/theme support,
  settings/INI helpers, list view helpers, etc.). Treat it as a semi-external dependency: prefer fixing
  submodule bugs upstream when the fix is generally reusable, and remember `WTLHelper` shows up in `git status`
  as a dirty submodule pointer whenever its checked-out commit differs from what's recorded in this repo.
- **`SystemExplorer`** (C++20/23) — the WTL GUI application; depends on `KObjExp` (build dependency, not link
  dependency — it's a driver artifact copied for deployment/signing), `ObjExpCore`, and `WTLHelper`.

Two more third-party dependencies live in-repo rather than via NuGet: **`phnt`** (Native API headers, at
`/phnt`) and **WTL** itself (`/WTLHelper/WTL`, vendored headers). NuGet (`packages.config`) is used only for
`Microsoft.Windows.ImplementationLibrary` (WIL) and the `wtl` package under `/packages`.

## Building

This is a Visual Studio / MSBuild C++ project — there is no cross-platform build script, CMake, or CLI test
runner. Build via Visual Studio 2022 (or `msbuild`/`devenv` from a Developer Command Prompt) targeting
`SystemExplorer.sln`.

- Platforms: x86, x64, ARM, ARM64 (x64 is the primary/most-tested platform).
- Configurations: `Debug`, `Release`, `ReleaseSigned`. `ReleaseSigned` produces a driver signed for
  distribution — required for `KObjExp.sys` to load on a machine without test-signing enabled.
- Building `KObjExp` requires the WDK (Windows Driver Kit) installed alongside the matching Visual Studio
  toolset.
- There is no automated test suite in this repo — validate changes by building and running the app manually
  (see below).

Example from a Developer Command Prompt:
```
msbuild SystemExplorer.sln /p:Configuration=Debug /p:Platform=x64
```

Init the WTLHelper submodule after a fresh clone:
```
git submodule update --init --recursive
```

## Running / manual verification

`SystemExplorer.exe` is a normal Win32 GUI app — run the built binary directly. Kernel-driver-dependent
features (privileged handle duplication, some object-address queries) additionally require the driver: use the
in-app "Options > Replace Task Manager" / driver-related menu commands, or `DriverHelper::LoadDriver()` /
`InstallDriver()` paths, which expect `KObjExp.sys` to be discoverable next to the exe or in the configured
location. Loading an unsigned driver requires the machine to be in test-signing mode unless using a
`ReleaseSigned` build with a trusted signature.

Since there's no automated UI test suite, after UI-affecting changes, build and run the app, exercise the
affected view(s) (open the relevant menu item from `CMainFrame`, e.g. `System > Processes`, `Objects > Object
Manager`, `System > Services`), and check both light and dark mode if the change touches rendering/colors.

## Architecture (SystemExplorer app)

**Native API first.** Both `ObjExpCore` and `SystemExplorer` are built with `PHNT_MODE 1` and include
`phnt_windows.h` / `phnt.h` before anything else (see `pch.h` in each project) — undocumented `Nt*`/`Zw*` calls
and internal structures (`SYSTEM_HANDLE_INFORMATION`, `SYSTEM_OBJECTTYPE_INFORMATION`, etc.) are the primary
way data is gathered, not `Toolhelp32`/`PSAPI`. Keep new backend code consistent with this — don't reach for
documented-but-lossy Win32 equivalents when `ObjExpCore` already has NT-API-based primitives.

**MDI-tab document/view via `CMainFrame`.** `SystemExplorer/MainFrm.h`/`.cpp` is the single top-level frame
(`CFrameWindowImpl` + `CAutoUpdateUI` + `CToolBarHelper` + `CNotifyIcon` + `IMainFrame`). It owns a tabbed
container (`CNativeCustomTabView` from WTLHelper) and creates one child view per "page" — processes, threads,
handles, services, object manager, memory map, modules, COM, logon sessions, search, etc. Each menu command in
`CMainFrame` (`ID_SYSTEM_*`, `ID_PROCESS_*`, `ID_OBJECTS_*`, `ID_HANDLES_*`...) creates/activates the
corresponding view and adds it as a tab. Views talk back to the frame only through the `IMainFrame` interface
(`Interfaces.h`) — never assume a concrete `CMainFrame*`; this keeps views independent of frame internals
(icon lookup, popup menus, tab lifecycle, dark-mode notifications via `WM_UPDATE_DARKMODE`).

**Object type plugin pattern.** Kernel object types (Process, Thread, Section, Event, Mutex, Semaphore, Key,
File, Job, ...) each get an `ObjectType`-derived class (e.g. `ProcessObjectType`, `FileObjectType`,
`KeyObjectType`) implementing `GetDetails(HANDLE)` to render type-specific detail text. `ObjectTypeFactory::
CreateObjectType(typeIndex, name)` maps a Windows object-type index/name to the right subclass. When adding
support for a new object type, follow this pattern: add an `XxxObjectType.h/.cpp` pair and register it in the
factory, rather than special-casing type names elsewhere.

**List views implement a shared interface.** Most data grids (`ProcessesView`, `ServicesView`, `HandlesView`,
`ModulesView`, etc.) go through `IListView`/`IView` and use `ColumnManager` for user-configurable/persistable
columns and `ListViewHelper`/`CustomDrawListView` for owner-drawn rendering (including per-process/tree
coloring — see `ProcessColor.*`, `ColorsSelectionDlg`). Reuse these rather than hand-rolling list view
owner-draw logic in a new view.

**Kernel driver bridge.** `DriverHelper` (`SystemExplorer/DriverHelper.*`) is the only place that talks to
`\\.\KObjExp` via `DeviceIoControl`. It wraps operations like `OpenHandle`, `DupHandle`, `OpenProcess`,
`GetObjectAddress` that need privileged kernel help. IOCTLs are defined in `KObjExp/KObjExp.h` and handled in
`ObjExpDeviceControl` (`KObjExp/KObjExp.cpp`) — when adding a new privileged operation, add the IOCTL there and
a corresponding wrapper in `DriverHelper`, keeping the driver's request validation minimal/explicit since it's
a high-privilege attack surface (note the existing client-image-name allowlist check on `IRP_MJ_CREATE`).

**Settings/persistence.** `Settings.h/.cpp` and `IniFile.*` (from WTLHelper) handle app settings, including
per-window layout, column layout, colors, and dark-mode preference, loaded/saved via
`CMainFrame::LoadSettings`/`SaveSettings`.

## Conventions

- Character set is Unicode throughout (`CString` = `CStringW`); avoid introducing ANSI (`char*`) string
  handling in new UI/core code.
- `abstract final` factory/helper classes with only `static` members (e.g. `DriverHelper`, `ObjectTypeFactory`)
  are used in place of namespaces for grouping static utility functions — follow this style for similar
  stateless helpers rather than introducing free functions or namespaces.
- `ObjExpCore` must stay UI-free — it has no dependency on WTL/ATL windowing; UI-related types (`CString`
  aside) belong in `SystemExplorer`.
