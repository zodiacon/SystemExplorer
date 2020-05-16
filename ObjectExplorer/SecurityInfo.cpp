#include "stdafx.h"
#include "SecurityInfo.h"

#pragma comment(lib, "Aclui")

SecurityInfo::SecurityInfo(HANDLE hObject, PCWSTR name) : _hObject(hObject), _name(name) {
}

HRESULT __stdcall SecurityInfo::QueryInterface(REFIID riid, void** ppvObj) {
	if (riid == __uuidof(ISecurityInformation) || riid == __uuidof(IUnknown)) {
		_refCount++;
		*ppvObj = static_cast<ISecurityInformation*>(this);
		return S_OK;
	}
	return E_NOINTERFACE;
}

ULONG __stdcall SecurityInfo::AddRef(void) {
	return ++_refCount;
}

ULONG __stdcall SecurityInfo::Release(void) {
	auto count = --_refCount;
	if (count == 0)
		delete this;
	return count;
}

HRESULT __stdcall SecurityInfo::GetObjectInformation(PSI_OBJECT_INFO pObjectInfo) {
	pObjectInfo->dwFlags = SI_ADVANCED | SI_READONLY;
	pObjectInfo->hInstance = nullptr;
	pObjectInfo->pszPageTitle = nullptr;
	pObjectInfo->pszObjectName = (LPWSTR)(LPCWSTR)_name;

	return S_OK;
}

HRESULT __stdcall SecurityInfo::GetSecurity(SECURITY_INFORMATION RequestedInformation, PSECURITY_DESCRIPTOR* ppSecurityDescriptor, BOOL fDefault) {
	auto error = ::GetSecurityInfo(_hObject, SE_KERNEL_OBJECT, RequestedInformation,
		nullptr, nullptr, nullptr, nullptr, ppSecurityDescriptor);

	return HRESULT_FROM_WIN32(error);
}

HRESULT __stdcall SecurityInfo::SetSecurity(SECURITY_INFORMATION SecurityInformation, PSECURITY_DESCRIPTOR pSecurityDescriptor) {
	return E_NOTIMPL;
}

HRESULT __stdcall SecurityInfo::GetAccessRights(const GUID* pguidObjectType, DWORD dwFlags, PSI_ACCESS* ppAccess, ULONG* pcAccesses, ULONG* piDefaultAccess) {
	static SI_ACCESS access[] = {
			{ &GUID_NULL, GENERIC_READ, L"Read", SI_ACCESS_GENERAL },
			{ &GUID_NULL, GENERIC_WRITE, L"Write", SI_ACCESS_GENERAL },
			{ &GUID_NULL, GENERIC_EXECUTE, L"Execute", SI_ACCESS_GENERAL },
			{ &GUID_NULL, MUTANT_QUERY_STATE, L"Query State", SI_ACCESS_GENERAL },
			{ &GUID_NULL, EVENT_MODIFY_STATE, L"Modify State", SI_ACCESS_GENERAL },
			{ &GUID_NULL, SYNCHRONIZE, L"Synchronize", SI_ACCESS_GENERAL }
	};

	*ppAccess = access;
	*pcAccesses = _countof(access);
	*piDefaultAccess = 0;

	return S_OK;
}

HRESULT __stdcall SecurityInfo::MapGeneric(const GUID* pguidObjectType, UCHAR* pAceFlags, ACCESS_MASK* pMask) {
	return S_OK;
}

HRESULT __stdcall SecurityInfo::GetInheritTypes(PSI_INHERIT_TYPE* ppInheritTypes, ULONG* pcInheritTypes) {
	return E_NOTIMPL;
}

HRESULT __stdcall SecurityInfo::PropertySheetPageCallback(HWND hwnd, UINT uMsg, SI_PAGE_TYPE uPage) {
	return E_NOTIMPL;
}
