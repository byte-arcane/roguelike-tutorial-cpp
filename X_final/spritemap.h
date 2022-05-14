#pragma once

#include <cstdint>

#include <glm/glm.hpp>

namespace rlf
{
	class Spritemap
	{
	public:
		~Spritemap() { Dispose(); }
		
		void Init(const glm::ivec2& size, const glm::uvec2 * data);
		void Draw(uint32_t program) const;

		// Release the buffer
		void Dispose();
	private:
		uint32_t texLayer=0;
	};
}