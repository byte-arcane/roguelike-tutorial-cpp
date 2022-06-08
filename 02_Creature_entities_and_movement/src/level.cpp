#include "level.h"

#include "graphics.h"
#include "utility.h"
#include "game.h"
#include "entity.h"
#include "commands.h"
#include "astar.h"
#include "grid.h"
#include "signals.h"

using namespace glm;

namespace rlf
{
	void Level::Init(const Array2D<LevelBgElement>& bg, const std::vector<std::pair<DbIndex,EntityDynamicConfig>>& entityCfgs, int locationIndex)
	{
		this->bg = bg;
		fogOfWar = Array2D<FogOfWarStatus>(bg.Size(), FogOfWarStatus::Unexplored);

		for (auto& ecfg : entityCfgs)
		{
			auto entityId = Game::Instance().CreateEntity(ecfg.first, ecfg.second,false);
			auto entity = entityId.Entity();
			entity->SetLocation({ entity->GetLocation().position });
			entities.push_back(entityId);
		}
	}

	void Level::StartListening()
	{
		sig::onEntityAdded.connect<Level, &Level::OnEntityAdded>(this);
		sig::onEntityRemoved.connect<Level, &Level::OnEntityRemoved>(this);
	}

	void Level::StopListening()
	{
		sig::onEntityAdded.disconnect<Level, &Level::OnEntityAdded>(this);
		sig::onEntityRemoved.disconnect<Level, &Level::OnEntityRemoved>(this);
	}

	void Level::OnEntityAdded(Entity& entity)
	{
		entities.push_back(entity.Id());
	}

	void Level::OnEntityRemoved(Entity& entity)
	{
		auto id = entity.Id();
		// erase-remove idiom, removing all entity IDs that match this entity's id
		entities.erase(std::remove_if(entities.begin(), entities.end(), [id](const EntityId& eref) { return eref == id; }), entities.end());
	}

	bool Level::DoesTileBlockVision(const glm::ivec2& p) const
	{
		// check the background tile
		if (bg(p.x, p.y).blocksVision)
			return false;
		// check all entities on this tile
		for (const auto& entityId : entities)
		{
			auto entity = entityId.Entity();
			if (entity->GetLocation().position == p && entity->BlocksVision())
				return false;
		}
		return true;
	}

	bool Level::EntityCanMoveTo(const Entity& e, const glm::ivec2& position) const
	{
		// Check background first, e.g. if it's a wall
		if (bg(position.x, position.y).blocksMovement)
			return false;
		// check if there are any blocker entities
		for (const auto& entityId : entities)
		{
			auto entity = entityId.Entity();
			if (entity != &e && entity->GetLocation().position == position && entity->BlocksMovement())
				return false;
		}
		return true;
	}

	bool Level::EntityHasLineOfSightTo(const Entity& e, const glm::ivec2& position) const
	{
		// only creatures can see
		if (e.GetCreatureData() == nullptr)
			return false;
		// check if the target is out of our line of sight radius
		auto start = e.GetLocation().position;
		auto distance = length(vec2(position - start));
		if (distance > e.DbCfg().Cfg()->creatureCfg.lineOfSightRadius)
			return false;
		// if it's adjacent (within 8-neighbourhood), return true
		if (distance < 2.0f) 
			return true; 

		// Calculate a line from the entity to the target
		// Now keep in mind that your fov function might not always return the same results as the line function, 
		//	  so make sure they give compatible results if that's important for your game
		std::vector<glm::ivec2> points;
		Line(points, start, position);
		assert(points.size() > 2); // not adjacent, so minimum of 3 points

		// process all but first/last points, for vision blocking
		for (int i = 1; i< int(points.size()) - 1; ++i) 
			if (!DoesTileBlockVision(points[i]))
				return false;
		return true;
	}

	Entity* Level::GetEntity(const glm::ivec2& position, bool blocksMovement) const
	{
		// bounds check
		if (!bg.InBounds(position))
			return nullptr;
		// go through all entities and check if any of them is at the requested position, if it matches the movement blocking parameter
		//	  so that we can distinguish between creatures and item piles on the same tile for example
		for (const auto& entityId : entities)
		{
			auto entity = entityId.Entity();
			if (entity->GetLocation().position == position)
			{
				// if it's not set or if it's an item pile or something like that, give priority to what we get here
				if (entity->BlocksMovement() == blocksMovement)
					return entity;
			}
		}
		return nullptr;
	}

	std::vector<glm::ivec2> Level::CalcPath(const Entity& e, const glm::ivec2& tgt) const
	{
		return CalculatePath(e.GetLocation().position, tgt, bg.Size(), [&](const glm::ivec2& p) { 
			return EntityCanMoveTo(e, p) ? 1.0f : std::numeric_limits<float>::infinity(); 
		});
	}


	std::pair<Array2D<LevelBgElement>, std::vector<std::pair<DbIndex, EntityDynamicConfig>>> LoadLevelFromTxtFile(const std::string& filename)
	{	
		static const LevelBgElement bgFloor = { "floor", false, false, false, '.', glm::vec4(.7, .7, .7, 1) };
		static const LevelBgElement bgWall = { "wall", true, true, false, '#', glm::vec4(.7, .7, .7, 1) };
		static const LevelBgElement bgWater = { "water", false, true, true, '=', glm::vec4(0, 0, 1, 1) };

		auto text = ReadTextFile(filename);
		
		// remove all occurences of \r, for windows-style newlines, so we always split newlines with '\n'
		text.erase(std::remove(text.begin(), text.end(), '\r'), text.end());
		// make sure we end in a newline
		if(text.back() != '\n')
			text.push_back('\n');
		// width is first occurence of newline
		int width = text.find('\n');
		int height = text.size() / (width + 1); // each line contains all chars PLUS the newline

		Array2D<LevelBgElement> bg( glm::ivec2(width, height));

		std::vector<std::pair<DbIndex,EntityDynamicConfig>> entityCfgs;

		// Go through the data
		for (int y = 0; y < height; ++y)
		{
			for (int x = 0; x < width; ++x)
			{
				// Remember that the file starts with highest Y value first (top-to-bottom)
				auto c = text[x + (height-1-y) * (width + 1)];

				// Set the bg element -- the floor is used if we can't find the glyph (e.g. if the glyph represents treasure, under the treasure we have a floor)
				// If we have no symbol (empty space) then it's a wall
				auto bgElement = c == ' ' ? bgWall : bgFloor;
				if (c == bgWall.glyph)
					bgElement = bgWall;
				else if (c == bgWater.glyph)
					bgElement = bgWater;
				bg(x, y) = bgElement;

				// Set the sparse feature if we have any
				EntityDynamicConfig dcfg;
				dcfg.position = { x,y };
				dcfg.nameOverride.clear();
				switch (c)
				{
				case '+':
					break;
				case '>':
					break;
				case '<':
					break;
				case 'X':
					entityCfgs.emplace_back("goblin", dcfg);
					break;
				case '$':
					// Nothing for now
					break;
				default:
					break;
				}
			}
		}

		return { bg, entityCfgs };
	}
}