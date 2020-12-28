#include "pch.h"
#include "ImageIconCache.h"

int ImageIconCache::GetIcon(const std::wstring& path) const {
	auto it = _icons.find(path);
	if (it != _icons.end())
		return it->second;

	HICON hIcon = nullptr;
	::ExtractIconEx(path.c_str(), 0, nullptr, &hIcon, 1);
	if (hIcon) {
		int index = _images.AddIcon(hIcon);
		_icons.insert({ path, index });
		return index;
	}
	return 0;
}

HIMAGELIST ImageIconCache::GetImageList() const {
	return _images;
}

ImageIconCache& ImageIconCache::Get() {
	static ImageIconCache cache;
	return cache;
}

ImageIconCache::ImageIconCache() {
	_images.Create(16, 16, ILC_COLOR32 | ILC_COLOR, 32, 32);
	_images.AddIcon(AtlLoadSysIcon(IDI_APPLICATION));
	_icons.reserve(64);
}
