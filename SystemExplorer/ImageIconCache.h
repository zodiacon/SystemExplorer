#pragma once

struct ImageIconCache {
	int GetIcon(const std::wstring& path) const;
	HIMAGELIST GetImageList() const;

	static ImageIconCache& Get();

private:
	ImageIconCache();

	ImageIconCache(const ImageIconCache&) = delete;
	ImageIconCache& operator=(const ImageIconCache&) = delete;

private:
	mutable CImageList _images;
	mutable std::unordered_map<std::wstring, int> _icons;
};
