#pragma once

#include "Array2D.h"
#include "level.h"

namespace rlf
{
	Array2D<LevelBgElement> GenerateDungeon(const glm::ivec2& size);
	std::vector<std::pair<DbIndex, EntityDynamicConfig>> PopulateDungeon(const Array2D<LevelBgElement>& layout, int numMonsters, int numFeatures, int numTreasures, bool addStairsDown, bool addStairsUp);
}