#pragma once
#include <vector>

#include <glm/glm.hpp>

namespace rlf
{
	// A generic 2D array class, that stores its data contiguous in memory, in a row-major form (C-style) using std::vector
	template<class T>
	class Array2D
	{
	public:
		Array2D() = default;
		
		// Ctor providing both the size and starting data.
		Array2D(const glm::ivec2& size, const std::vector<T>& data):size(size), data(data) { assert(size.x * size.y == data.size());}
		// Ctor providing the size, and initializing the vector to match the size
		Array2D(const glm::ivec2& size)
			:size(size), data(size.x* size.y) {}		
		// Ctor providing the size and a default value, and initializing the vector to match the size using that value
		Array2D(const glm::ivec2& size, const T& value)
			:size(size), data(size.x* size.y, value) {}

		// Get size and the underlying raw data
		const glm::ivec2& Size() const { return size; }
		const std::vector<T>& Data() const { return data; }

		// element access
		T& operator()(int x, int y) { return data.at(x + y * size.x); }
		const T& operator()(int x, int y) const { return data.at(x + y * size.x); }

		// check if a coordinate is inside the bounds of the 2D array
		bool InBounds(int x, int y) const { return x >= 0 && x < size.x&& y >= 0 && y < size.y; }
		bool InBounds(const glm::ivec2& p) const { return p.x >= 0 && p.x < size.x&& p.y >= 0 && p.y < size.y; }

	private:
		// Size of array (width, height)
		glm::ivec2 size = {0,0};
		// array data stored in row-major form
		std::vector<T> data;
	};
}