#pragma once

#include <cstdint>
#include <set>

namespace rlf
{
	// Wrapper class for an OpenGL buffer used for sparse rendering of elements (creatures, items, gui elements, etc. Anything but wall/floor)
	class SparseBuffer
	{
	public:
		~SparseBuffer() { Dispose(); }

		void Init(int stride, int numElementsMax);
		bool IsInitialized() const { return stride > 0; }

		// Add some data (num bytes == stride) at a free slot and return the slot
		int Add(const void* data);
		// update the data at a given slot
		void Update(int slot, const void* data);
		// Free up a slot
		void Remove(int slot);

		// Set up several elements at the same time
		void Set(int numElements, const void* data);

		// Release the buffer
		void Dispose();

		void Draw() const;

		void Clear();

		// access is GL_MAP_WRITE_BIT
		void* MapMemory( int firstElement, int numElements, int access);
		void* MapMemory(int access) { return MapMemory(0, numElementsMax, access); }
		void UnmapMemory();

	private:
		uint32_t buffer=0;
		int stride=0;
		int numElementsMax = 0;

		std::set<int> freeSlots;
		int firstFreeSlotAtEnd=0;
	};
}