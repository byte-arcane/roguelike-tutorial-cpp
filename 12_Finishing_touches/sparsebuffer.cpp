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
		buffer = rlf::CreateBuffer(numElementsMax *stride, nullptr, GL_DYNAMIC_DRAW);
		this->stride = stride;
		this->numElementsMax = numElementsMax;
	}

	// Add some data (num bytes == stride) at a free index and return the slot
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

		// Update the data and return the slot
		Update(slot, data);
		return slot;
	}

	void SparseBuffer::Update(int idx, const void* data)
	{
		rlf::UpdateSSBO(buffer, idx *stride, stride, data);
	}

	void SparseBuffer::Set(int numElements, const void* data)
	{
		numElements = glm::min(numElements, numElementsMax);
		auto mem = MapMemory(GL_MAP_WRITE_BIT);
		memcpy(mem, data, numElements * stride);
		UnmapMemory();
		freeSlots.clear();
		firstFreeSlotAtEnd = numElements;
	}

	void SparseBuffer::Remove(int idx)
	{
		freeSlots.insert(idx);
		auto memory = MapMemory(GL_MAP_WRITE_BIT);
		memset((char*)memory+ firstFreeSlotAtEnd * stride, 0, stride);
		UnmapMemory();
	}

	void SparseBuffer::Dispose()
	{
		DeleteBuffer(buffer);
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