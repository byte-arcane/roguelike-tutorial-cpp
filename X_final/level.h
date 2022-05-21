#pragma once
#include <array>
#include <vector>

#include <glm/glm.hpp>
#include <nlohmann/json_fwd.hpp>

#include "sparsebuffer.h"
#include "spritemap.h"
#include "entity.h"
#include "array2d.h"

namespace rlf
{
	// Visibility status of tile
	enum class FogOfWarStatus : uint8_t
	{
		Unexplored=0, // never seen
		Explored,	  // seen in the past, but not currently
		Visible		  // currently visible
	};

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

	// Represents a game level
	class Level
	{
	public:
		// when the level gets destroyed, it stops listening to any events
		~Level() { StopListening(); }
		
		const Array2D<LevelBgElement>& Bg() const { return bg; }
		const Array2D<FogOfWarStatus> FogOfWar() const { return fogOfWar; }
		const std::vector<EntityId>& Entities() const { return entities; }

		// initialize the level with data
		void Init(const Array2D<LevelBgElement>& data, const std::vector<std::pair<DbIndex, EntityDynamicConfig>>& entityCfgs, int locationIndex);
		// update the fog of war map
		void UpdateFogOfWar();
		// check if an entity can move to a target position
		bool EntityCanMoveTo(const Entity& e, const glm::ivec2& position) const;
		// check if an entity can see a target position
		bool EntityHasLineOfSightTo(const Entity& e, const glm::ivec2& position) const;
		// get an entity at a tile, filter it by if it's blocking movement or not (so we can tell between an item pile and a dragon standing over it)
		Entity* GetEntity(const glm::ivec2& position, bool blocksMovement) const;
		// calculate a path between an entity and a target position
		std::vector<glm::ivec2> CalcPath(const Entity& e, const glm::ivec2& tgt) const;
		// start listening to events
		void StartListening();
		// stop listening to events
		void StopListening();

	private:
		// signal-slots
		void OnEntityAdded(Entity& entity);
		void OnEntityRemoved(Entity& e);
		void OnObjectStateChanged(const Entity& e);

		// Does this tile block vision? Check the bg element and all entities standing on that tile
		bool DoesTileBlockVision(const glm::ivec2& p) const;
	private:

		// friends for easy serialization
		friend void from_json(const nlohmann::json& j, Level& level);
		friend void to_json(nlohmann::json& j, const Level& level);

		// the 2d array of bg elements
		Array2D<LevelBgElement> bg;
		// the 2d array of fow status
		Array2D<FogOfWarStatus> fogOfWar;
		// the list of entities (creatures/objects) in the level
		std::vector<EntityId> entities;
	};

	// helper to load a level from a text file
	std::pair<Array2D<LevelBgElement>, std::vector<std::pair<DbIndex, EntityDynamicConfig>>> LoadLevelFromTxtFile(const std::string& filename);
}