#pragma once
#include <array>
#include <vector>

#include <glm/glm.hpp>

#include "sparsebuffer.h"
#include "spritemap.h"
#include "entity.h"
#include "array2d.h"

namespace rlf
{
	enum class FogOfWarStatus : uint8_t
	{
		Unexplored=0,
		Explored,
		Visible
	};

	struct LevelBgElement
	{
		std::string name;
		bool blocksVision = false;
		bool blocksMovement = false;
		bool isLiquid = false;
		char glyph = '.';
		glm::vec4 color = { 1, 1, 1, 1 }; // defaults to white
	};

	class Level
	{
	public:
		void Init(const Array2D<LevelBgElement>& data, const std::vector<std::pair<DbIndex, EntityDynamicConfig>>& entityCfgs, int locationIndex);
		
		const Array2D<LevelBgElement>& Bg() const { return bg; }
		const Array2D<FogOfWarStatus> FogOfWar() const { return fogOfWar; }
		const std::vector<EntityId>& Entities() const { return entities; }

		void UpdateFogOfWar();
		bool EntityCanMoveTo(const Entity& e, const glm::ivec2& position) const;
		Entity* GetEntity(const glm::ivec2& position, bool blocksMovement) const;

		void OnEntityAdded(const Entity& entity);
		void OnEntityRemoved(const Entity& e);
		void OnObjectStateChanged(const Entity& e);
	private:

		Array2D<LevelBgElement> bg;
		Array2D<FogOfWarStatus> fogOfWar;

		std::vector<EntityId> entities;
	};

	std::pair<Array2D<LevelBgElement>, std::vector<std::pair<DbIndex, EntityDynamicConfig>>> LoadLevelFromTxtFile(const std::string& filename);
}