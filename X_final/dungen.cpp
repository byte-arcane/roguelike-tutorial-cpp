#include "dungen.h"

#include <algorithm>
#include <random>

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
		std::vector<ivec2> availablePositions;
		availablePositions.reserve(layout.Size().x * layout.Size().y);
		for (int y = 0; y < layout.Size().y; ++y)
			for (int x = 0; x < layout.Size().x; ++x)
				if (!layout(x, y).blocksMovement)
					availablePositions.emplace_back(x, y);
#ifndef _DEBUG
		std::random_device rd;
		std::mt19937 g(rd());
#else
		std::mt19937 g(2);
#endif
		std::shuffle(availablePositions.begin(), availablePositions.end(),g);

		auto fnPopPosition = [&availablePositions]()
		{
			auto position = availablePositions.back();
			availablePositions.pop_back();
			return position;
		};

		std::vector<std::pair<DbIndex, EntityDynamicConfig>> output;
		if (addStairsDown)
			output.emplace_back("stairs_down", EntityDynamicConfig{ fnPopPosition() });
		if (addStairsUp)
			output.emplace_back("stairs_up", EntityDynamicConfig{ fnPopPosition() });

		// monsters first
		for(int i=0;i<numMonsters;++i)
			output.emplace_back(monsters[rand()%monsters.size()], EntityDynamicConfig{fnPopPosition()});
		// treasures afterwards
		for (int i = 0; i < numTreasures; ++i)
		{
			auto dcfg = EntityDynamicConfig{ fnPopPosition() };
			dcfg.inventory.push_back(treasures[rand() % treasures.size()]);
			output.emplace_back("item_pile", dcfg);
		}
		// now features, but careful as they have the potential to be blocking narrow passageways!
		for (int i = 0; i < numFeatures; ++i)
		{
			// get a feature
			const auto& feature = features[rand() % features.size()];
			// only attempt to place if we have enough positions available
			while (!availablePositions.empty())
			{
				// pop a position
				auto position = fnPopPosition();
				// if the feature blocks movement, need further checks to ensure passability
				if (feature.Cfg()->objectCfg.blocksMovement)
				{
					// calculate the number of walkable neighbours (4-connected). We need at least 3!
					int numFloorNbs = 0;
					for (const auto& nb4 : Nb4())
					{
						auto pnb = position + nb4;
						if (layout.InBounds(pnb) && !layout(pnb.x, pnb.y).blocksMovement)
							numFloorNbs++;							
					}
					// if we don't have 3 walkable neighbours, then skip this one
					if (numFloorNbs < 3)
						continue;
				}
				// if we arrived here, either the feature is not a blocker, or it has enough walkable neighbours
				output.emplace_back(feature, EntityDynamicConfig{ position });
				break;
			}
			
		}
		return output;
	}
}