#pragma once
#include <vector>

#include <glm/glm.hpp>

namespace rlf
{
	template<class T>
	class Array2D
	{
	public:
		Array2D() = default;
		Array2D(const glm::ivec2& size, const std::vector<T>& data)
			:size(size), data(data) {}
		Array2D(const glm::ivec2& size)
			:size(size), data(size.x* size.y) {}		
		Array2D(const glm::ivec2& size, const T& value)
			:size(size), data(size.x* size.y, value) {}

		// Get size and the underlying raw data
		const glm::ivec2& Size() const { return size; }
		const std::vector<T>& Data() const { return data; }
		const T* Raw() const { return data.data(); }

		// element access
		T& operator()(int x, int y) { return data[x + y * size.x]; }
		const T& operator()(int x, int y) const { return data[x + y * size.x]; }

		// helpers
		bool InBounds(int x, int y) const { return x >= 0 && x < size.x&& y >= 0 && y < size.y; }
		bool InBounds(const glm::ivec2& p) const { return p.x >= 0 && p.x < size.x&& p.y >= 0 && p.y < size.y; }

	private:
		glm::ivec2 size = {0,0};
		std::vector<T> data;
	};
}