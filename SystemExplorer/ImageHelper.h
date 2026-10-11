#pragma once

struct ImageHelper final abstract {
	static HICON GetSystemIcon(SHSTOCKICONID icon, bool big = false);
	// adds a copy of the icon to the image list and destroys the icon (an image list keeps its own copy); for icons
	// the caller owns (loaded, extracted, created). Returns the image's index, -1 if hIcon is null or adding failed.
	static int AddIcon(HIMAGELIST images, HICON hIcon);
};


