#include "sparsebuffer.h"

#include <cassert>

#include <gl/glew.h>

#include <utility.h>

namespace rlf
{
	void SparseBuffer::Init(int stride, int numElementsMax)
	{
		assert(buffer == 0);
		numElementsMax = 1024;
		buffer = cgf::createBuffer(numElementsMax *stride, nullptr, GL_DYNAMIC_DRAW);
		this->stride = stride;
		this->numElementsMax = numElementsMax;
	}

	// Add some data (num bytes == stride) at a free slot and return the slot
	int SparseBuffer::Add(const void* data)
	{
		// calculate the slot for the data
		int slot = -1;
		if (freeSlots.empty())
		{
			slot = firstFreeSlotAtEnd;
			++firstFreeSlotAtEnd;
		}
		else
		{
			// get last free set element
			auto it = (freeSlots.end()--);
			slot = *it;
			freeSlots.erase(it);
		}

		// update the data and return the slot
		Update(slot, data);
		return slot;
	}

	// update the data at a given slot
	void SparseBuffer::Update(int slot, const void* data)
	{
		cgf::updateSSBO(buffer, slot*stride, stride, data);
	}

	// Free up a slot
	void SparseBuffer::Remove(int slot)
	{
		freeSlots.insert(slot);
		auto memory = MapMemory(GL_MAP_WRITE_BIT);
		memset((char*)memory+ firstFreeSlotAtEnd * stride, 0, stride);
		UnmapMemory();
	}

	// Free up a slot
	void SparseBuffer::Dispose()
	{
		if (buffer != 0)
		{
			glDeleteBuffers(1, &buffer);
			buffer = 0;
		}
	}

	void SparseBuffer::Draw() const
	{
		if (firstFreeSlotAtEnd == 0)
			return;
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffer);
		int numInstances = firstFreeSlotAtEnd;
		glDrawArraysInstanced(GL_TRIANGLES, 0, 6, numInstances); // 6: num of quad vertices (2 triangles)
	}

	void* SparseBuffer::MapMemory(int firstElement, int numElements, int access)
	{
		return glMapNamedBufferRange(buffer, firstElement*stride, numElements*stride, access);
	}

	void SparseBuffer::UnmapMemory()
	{
		glUnmapNamedBuffer(buffer);
	}

	void SparseBuffer::Clear()
	{
		if (firstFreeSlotAtEnd > 0)
		{
			auto memory = MapMemory(GL_MAP_WRITE_BIT);
			memset(memory, 0, firstFreeSlotAtEnd * stride);
			UnmapMemory();
		}
		freeSlots.clear();
		firstFreeSlotAtEnd = 0;
	}
}