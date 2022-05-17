#include "dungen.h"

#include <algorithm>
#include <unordered_set>

#include <glm/gtx/hash.hpp>

#include "grid.h"

using namespace glm;
using namespace std;

namespace rlf
{
	static const LevelBgElement bgFloor = { "floor", false, false, false, '.', glm::vec4(.7, .7, .7, 1) };
	static const LevelBgElement bgWall = { "wall", true, true, false, '#', glm::vec4(.7, .7, .7, 1) };
	static const LevelBgElement bgWater = { "water", false, true, true, '=', glm::vec4(0, 0, 1, 1) };

	void dig(Array2D<LevelBgElement>& layout, int& numLeft, const ivec2& point)
	{
		auto& elem = layout(point.x, point.y);
		if (elem.blocksMovement)
		{
			elem = bgFloor;
			--numLeft;
		}
		if (numLeft > 0)
		{
			auto newDir = Nb4()[rand() % 4];
			while( !layout.InBounds(point + 2*newDir)) // test with 2xnewDir because we don't want to dig the border tiles
				newDir = Nb4()[rand() % 4];
			dig(layout, numLeft, point+newDir);
		}
	}

	Array2D<LevelBgElement> generateDungeon(const glm::ivec2& size)
	{
		Array2D<LevelBgElement> layout(size, bgWall);
		auto center = size / 2;
		layout(center.x, center.y) = bgFloor;
		for (const auto& startDir : Nb4())
		{
			int numLeft = (size.x * size.y) / 16;
			dig(layout, numLeft, center +startDir);
		}
		return layout;
	}

	std::vector<std::pair<DbIndex, EntityDynamicConfig>> populateDungeon(const Array2D<LevelBgElement>& layout, int numMonsters, int numFeatures, int numTreasures, bool addStairsDown, bool addStairsUp)
	{
		// Get all available monsters/treasures/features
		const auto& db = Db::Instance().All();
		vector<DbIndex> monsters;
		vector<DbIndex> features;
		vector<DbIndex> treasures;
		for_each(db.begin(), db.end(), [&](const auto& nameAndConfig) {
			if (nameAndConfig.second.type == EntityType::Creature && nameAndConfig.second.allowRandomSpawn)
				monsters.emplace_back(nameAndConfig.first);
			else if (nameAndConfig.second.type == EntityType::Object && nameAndConfig.second.allowRandomSpawn)
				features.emplace_back(nameAndConfig.first);
			else if (nameAndConfig.second.type == EntityType::Item && nameAndConfig.second.allowRandomSpawn)
				treasures.emplace_back(nameAndConfig.first);
		});
		
		// Get all available positions, randomized
		unordered_set<ivec2> availablePositions;
		for (int y = 0; y < layout.Size().y; ++y)
			for (int x = 0; x < layout.Size().x; ++x)
				if (!layout(x, y).blocksMovement)
					availablePositions.emplace(x, y);

		auto fnPopPosition = [&availablePositions]()
		{
			auto position = *availablePositions.begin();
			availablePositions.erase(availablePositions.begin());
			return position;
		};

		std::vector<std::pair<DbIndex, EntityDynamicConfig>> output;
		if (addStairsDown)
			output.emplace_back("stairs_down", EntityDynamicConfig{ fnPopPosition() });
		if (addStairsUp)
			output.emplace_back("stairs_up", EntityDynamicConfig{ fnPopPosition() });

		for(int i=0;i<numMonsters;++i)
			output.emplace_back(monsters[rand()%monsters.size()], EntityDynamicConfig{fnPopPosition()});
		for (int i = 0; i < numFeatures; ++i)
			output.emplace_back(features[rand() % features.size()], EntityDynamicConfig{ fnPopPosition() });
		for (int i = 0; i < numTreasures; ++i)
		{
			auto dcfg = EntityDynamicConfig{ fnPopPosition() };
			dcfg.inventory.push_back(treasures[rand() % treasures.size()]);
			output.emplace_back("item_pile", dcfg);
		}
		return output;
	}
}