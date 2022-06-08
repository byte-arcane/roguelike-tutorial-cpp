#pragma once

#include <cstdint>
#include <glm/glm.hpp>

namespace rlf
{
	// A class that stores a grid of equally-sized tiles (square or rectangular), loaded from an image
	class Tilemap
	{
	public:
		// Release the texture
		~Tilemap()  { Dispose(); }

		// Load the texture
		void Load(const char* imgFilename, const glm::ivec2& tileSize);

		// Accessors
		uint32_t Texture() const  { return texture; }
		const glm::ivec2& TileSize() const  { return tileSize; }
		const glm::ivec2& TileNum() const  { return tileNum; }

		void Dispose() ;

	private:
		// texture object
		uint32_t texture=0;
		// pixels for each tile
		glm::ivec2 tileSize = {0,0};
		// number of tiles in each dimension
		glm::ivec2 tileNum = {0,0};

	};

	// Reference to a tile in the global tilemap, and a color
	struct TileData
	{
		int spriteIndex = -1;
		glm::vec4 color = {0,0,0,0};

		TileData() = default;
		TileData(int si, const glm::vec4& c)  :spriteIndex(si), color(c) {}

		// Pack sprite data (which sprite and what color tint) for use with a dense 2D grid (position of sprite is given implicitly by grid cell)
		glm::uvec2 PackDense() const;
		// Pack sprite data (position, which sprite and what color tint) for use with a sparse 2D grid
		glm::uvec4 PackSparse(const glm::ivec2& position) const;
	};
}