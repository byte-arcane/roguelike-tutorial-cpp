#pragma once

#include <cstdint>
#include <set>

namespace rlf
{
	// Wrapper class for an OpenGL buffer used for sparse rendering of elements (creatures, items, gui elements, etc. Anything but wall/floor)
	class SparseBuffer
	{
	public:
		// Make sure the buffer is released before destroying the object
		~SparseBuffer()  { Dispose(); }

		// Initialize the buffer, using the size of each elements in bytes (stride) and the max number of elements that we can allocate
		void Init(int stride, int numElementsMax);
		// check if our buffer is initialized
		bool IsInitialized() const { return buffer != 0; }

		// Add some data (num bytes == stride) at a free slot and return the slot
		int Add(const void* data);
		// Update the data at a given slot
		void Update(int slot, const void* data);
		// Free up a slot
		void Remove(int slot);

		// Set up several elements at the same time
		void Set(int numElements, const void* data);

		// Release the buffer
		void Dispose() ;

		// Draw a number of quad instances, as many as the buffer elements
		void Draw() const;

		// Clear the buffer
		void Clear();

		// Get a memory pointer from the buffer, access should be GL_MAP_WRITE_BIT if we want to write to it
		void* MapMemory( int firstElement, int numElements, int access);
		// Map the entire buffer with given access
		void* MapMemory(int access) { return MapMemory(0, numElementsMax, access); }
		// Unmap the mapped buffer memory after MapMemory() -- don't forget to call this! Even better, write a struct that uses RAII to automatically map/unmap within scope
		void UnmapMemory();

	private:
		// the opengl buffer
		uint32_t buffer=0;
		// the size of each buffer element, in bytes
		int stride=0;
		// max number of elements that can be written to the buffer
		int numElementsMax = 0;
		// store here any free slots, if we want to remove a buffer element
		std::set<int> freeSlots;
		// Store the first index into contiguous free space -- nothing is written at/after this index
		int firstFreeSlotAtEnd=0;
	};
}