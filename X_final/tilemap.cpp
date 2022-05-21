#include "tilemap.h"

#include <GL/glew.h>
#include <utility.h>

using namespace glm;

namespace rlf
{
	// Release the texture
	void Tilemap::Dispose()
	{
		if (texture != 0)
		{
			glDeleteTextures(1, &texture);
			texture = 0;
		}
	}

	// Load the texture
	void Tilemap::Load(const char* imgFilename, const ivec2& tileSize)
	{
		ivec2 textureSize;
		texture = LoadTexture(imgFilename, false, textureSize);
		this->tileSize = tileSize;
		tileNum = textureSize / tileSize;
	}

	uvec2 TileData::PackDense() const
	{
		return uvec2(spriteIndex, PackColor(color));
	}

	uvec4 TileData::PackSparse(const ivec2& position) const
	{
		return uvec4(position, spriteIndex, PackColor(color));
	}
}