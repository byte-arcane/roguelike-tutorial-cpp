#pragma once

#include <cstdint>

#include <glm/glm.hpp>

namespace rlf
{
	// Helper class for storing a texture where the pixels store information about the map: sprite and color tint
	class Spritemap
	{
	public:
		// Make sure when we destroy the sprite map, the opengl texture will be released
		~Spritemap() { Dispose(); }
		
		// create the texture, given a size and starting data
		void Init(const glm::ivec2& size, const glm::uvec2 * data);

		// Draw the texture as a quad
		void Draw(uint32_t program) const;

		// Release the buffer
		void Dispose();
	private:
		uint32_t texLayer=0;
	};
}