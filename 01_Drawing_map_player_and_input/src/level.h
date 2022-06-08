#pragma once
#include <array>
#include <vector>

#include <glm/glm.hpp>
#include <nlohmann/json_fwd.hpp>

#include "sparsebuffer.h"
#include "spritemap.h"
#include "array2d.h"

namespace rlf
{
	// data for a background element of a map (e.g. floor, wall or liquid)
	// This data should be static configuration and use something akin to DbIndex to access it: exercise for the reader!
	struct LevelBgElement
	{
		std::string name;
		bool blocksVision = false;
		bool blocksMovement = false;
		bool isLiquid = false;
		char glyph = '.'; // what ascii symbol do we use to display this element
		glm::vec4 color = { 1, 1, 1, 1 }; // glyph color, defaults to white
	};


	// helper to load a level from a text file
	Array2D<LevelBgElement> LoadLevelFromTxtFile(const std::string& filename);
}