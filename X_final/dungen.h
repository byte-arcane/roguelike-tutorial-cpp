#pragma once

#include "Array2D.h"
#include "level.h"

namespace rlf
{
	// Generate the dungeon layout (floor/wall/liquid/etc)
	Array2D<LevelBgElement> GenerateDungeon(const glm::ivec2& size);
	// Populate the dungeon with monsters, treasures, dungeon features, stairs, etc. Return a vector of (entity configuration, dynamic entity configuration) data
	std::vector<std::pair<DbIndex, EntityDynamicConfig>> PopulateDungeon(const Array2D<LevelBgElement>& layout, int numMonsters, int numFeatures, int numTreasures, bool addStairsDown, bool addStairsUp);
}