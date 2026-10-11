#include "pch.h"
#include "ImageHelper.h"

HICON ImageHelper::GetSystemIcon(SHSTOCKICONID icon, bool big) {
	SHSTOCKICONINFO ssii = { sizeof(ssii) };
	if (FAILED(::SHGetStockIconInfo(icon, (big ? SHGSI_LARGEICON : SHGSI_SMALLICON) | SHGSI_ICON, &ssii)))
		return nullptr;

	return ssii.hIcon;
}

int ImageHelper::AddIcon(HIMAGELIST images, HICON hIcon) {
	if (hIcon == nullptr)
		return -1;
	int index = ::ImageList_ReplaceIcon(images, -1, hIcon);
	::DestroyIcon(hIcon);
	return index;
}
