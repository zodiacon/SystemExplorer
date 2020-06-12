#include "pch.h"
#include "ProcessHeaps.h"
#include <TlHelp32.h>

using namespace WinSys;

std::vector<ProcessHeap> ProcessHeaps::EnumHeaps() {
	std::vector<ProcessHeap> heaps;
	_hSnapshot.reset(::CreateToolhelp32Snapshot(TH32CS_SNAPHEAPLIST, _pid));
	if (!_hSnapshot)
		return heaps;

	HEAPLIST32 list;
	list.dwSize = sizeof(list);

	if (!::Heap32ListFirst(_hSnapshot.get(), &list))
		return heaps;

	heaps.reserve(8);
	do {
		ProcessHeap heap;
		heap.Address = reinterpret_cast<PVOID>(list.th32HeapID);
		heap.Size = list.dwSize;

		heaps.push_back(heap);
	} while (::Heap32ListNext(_hSnapshot.get(), &list));

	return heaps;
}

std::vector<ProcessHeapEntry> WinSys::ProcessHeaps::EnumHeapEntries(const ProcessHeap& heap) {
	std::vector<ProcessHeapEntry> entries;
	if (!_hSnapshot)
		return entries;

	::HEAPENTRY32 he;
	he.dwSize = sizeof(he);

	auto id = reinterpret_cast<ULONG_PTR>(heap.Address);
	if (!::Heap32First(&he, _pid, id))
		return entries;

	entries.reserve(256);
	do {
		ProcessHeapEntry entry;
		entry.Address = reinterpret_cast<void*>(he.dwAddress);
		entry.BlockSize = he.dwBlockSize;
		entry.Flag = (ProcessHeapEntryFlags)he.dwFlags;
		entries.push_back(entry);

	} while (::Heap32Next(&he));

	return entries;
}
