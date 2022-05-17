#pragma once

#include "Array2D.h"
#include "level.h"

namespace rlf
{
	Array2D<LevelBgElement> generateDungeon(const glm::ivec2& size);
	std::vector<std::pair<DbIndex, EntityDynamicConfig>> populateDungeon(const Array2D<LevelBgElement>& layout, int numMonsters, int numFeatures, int numTreasures, bool addStairsDown, bool addStairsUp);
}