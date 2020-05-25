#include "stdafx.h"
#include "SecurityInfo.h"

#pragma comment(lib, "Aclui")

SecurityInfo::SecurityInfo(HANDLE hObject, PCWSTR name) : _hObject(hObject), _name(name) {
}

HRESULT __stdcall SecurityInfo::QueryInterface(REFIID riid, void** ppvObj) {
	if (riid == __uuidof(ISecurityInformation) || riid == __uuidof(IUnknown)) {
		*ppvObj = static_cast<ISecurityInformation*>(this);
		return S_OK;
	}
	return E_NOINTERFACE;
}

ULONG __stdcall SecurityInfo::AddRef(void) {
	return 2;
}

ULONG __stdcall SecurityInfo::Release(void) {
	return 1;
}

HRESULT __stdcall SecurityInfo::GetObjectInformation(PSI_OBJECT_INFO pObjectInfo) {
	pObjectInfo->dwFlags = SI_ADVANCED | SI_EDIT_ALL;
	pObjectInfo->hInstance = nullptr;
	pObjectInfo->pszPageTitle = nullptr;
	pObjectInfo->pszObjectName = (LPWSTR)(LPCWSTR)_name;

	return S_OK;
}

HRESULT __stdcall SecurityInfo::GetSecurity(SECURITY_INFORMATION RequestedInformation, PSECURITY_DESCRIPTOR* ppSecurityDescriptor, BOOL fDefault) {
	DWORD len;
	if (::GetKernelObjectSecurity(_hObject, RequestedInformation, (PSECURITY_DESCRIPTOR)_buffer, sizeof(_buffer), &len)) {
		*ppSecurityDescriptor = _buffer;
		return S_OK;
	}

	return HRESULT_FROM_WIN32(::GetLastError());
}

HRESULT __stdcall SecurityInfo::SetSecurity(SECURITY_INFORMATION SecurityInformation, PSECURITY_DESCRIPTOR pSecurityDescriptor) {
	if (::SetKernelObjectSecurity(_hObject, SecurityInformation, pSecurityDescriptor))
		return S_OK;

	return HRESULT_FROM_WIN32(::GetLastError());
}

HRESULT __stdcall SecurityInfo::GetAccessRights(const GUID* pguidObjectType, DWORD dwFlags, PSI_ACCESS* ppAccess, ULONG* pcAccesses, ULONG* piDefaultAccess) {
	static SI_ACCESS access[] = {
			{ &GUID_NULL, GENERIC_READ, L"Read", SI_ACCESS_GENERAL },
			{ &GUID_NULL, GENERIC_WRITE, L"Write", SI_ACCESS_GENERAL },
			{ &GUID_NULL, GENERIC_EXECUTE, L"Execute", SI_ACCESS_GENERAL },
			{ &GUID_NULL, READ_CONTROL, L"Read Control", SI_ACCESS_GENERAL },
			{ &GUID_NULL, WRITE_DAC, L"Write DAC", SI_ACCESS_GENERAL },
			{ &GUID_NULL, SYNCHRONIZE, L"Synchronize", SI_ACCESS_GENERAL }
	};

	*ppAccess = access;
	*pcAccesses = _countof(access);
	*piDefaultAccess = 0;

	return S_OK;
}

HRESULT __stdcall SecurityInfo::MapGeneric(const GUID* pguidObjectType, UCHAR* pAceFlags, ACCESS_MASK* pMask) {
	return E_NOTIMPL;
}

HRESULT __stdcall SecurityInfo::GetInheritTypes(PSI_INHERIT_TYPE* ppInheritTypes, ULONG* pcInheritTypes) {
	return E_NOTIMPL;
}

HRESULT __stdcall SecurityInfo::PropertySheetPageCallback(HWND hwnd, UINT uMsg, SI_PAGE_TYPE uPage) {
	return E_NOTIMPL;
}
