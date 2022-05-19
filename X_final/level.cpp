#include "level.h"

#include "graphics.h"
#include "utility.h"
#include "game.h"
#include "entity.h"
#include "fov.h"
#include "eventhandlers.h"
#include "astar.h"
#include "grid.h"

using namespace glm;

namespace rlf
{
	void Level::Init(const Array2D<LevelBgElement>& bg, const std::vector<std::pair<DbIndex,EntityDynamicConfig>>& entityCfgs, int locationIndex)
	{
		this->bg = bg;
		fogOfWar = Array2D<FogOfWarStatus>(bg.Size(), {});

		for (auto& ecfg : entityCfgs)
		{
			auto entityId = GameState::Instance().CreateEntity(ecfg.first, ecfg.second);
			auto entity = entityId.Entity();
			entity->SetLocation({ locationIndex, entity->GetLocation().position });
			entities.push_back(entityId);
		}
	}

	void Level::OnEntityAdded(const Entity& entity)
	{
		entities.push_back(entity.Id());

		// if it's the player who was added to the level, recalculate visibility
		if (GameState::Instance().IsPlayer(entity))
			UpdateFogOfWar();
	}

	void Level::OnEntityRemoved(const Entity& entity)
	{
		auto id = entity.Id();
		entities.erase(std::remove_if(entities.begin(), entities.end(), [id](const EntityId& eref) { return eref == id; }), entities.end());
	}

	bool Level::PositionIsVisible(const glm::ivec2& p) const
	{
		if (bg(p.x, p.y).blocksVision)
			return false;
		for (const auto& entityId : entities)
		{
			auto entity = entityId.Entity();
			if (entity->GetLocation().position == p && entity->BlocksVision())
				return false;
		}
		return true;
	}

	void Level::UpdateFogOfWar()
	{
		// "reset" the fov map
		auto map_size = bg.Size();
		for (int y = 0; y < map_size.y; ++y)
			for (int x = 0; x < map_size.x; ++x)
			{
				// Set everything previously visible to currently explored
				auto& fowValue = fogOfWar(x, y);
				if (fowValue == FogOfWarStatus::Visible)
					fowValue = FogOfWarStatus::Explored;
			}

		auto player = GameState::Instance().Player().Entity();
		auto posPlayer = player->GetLocation().position;
		auto cb_is_opaque = [&](const glm::ivec2& p) {return !PositionIsVisible(p); };
		auto cb_on_visible = [&](const glm::ivec2& p) { fogOfWar(p.x, p.y) = FogOfWarStatus::Visible; };
		calculate_fov(player->GetLocation().position, player->DbCfg().Cfg()->creatureCfg.lineOfSightRadius, map_size, cb_is_opaque, cb_on_visible);

		Graphics::Instance().OnFogOfWarChanged();
	}

	bool Level::EntityCanMoveTo(const Entity& e, const glm::ivec2& position) const
	{
		// Check if we can! If not, spawn a message
		if (bg(position.x, position.y).blocksMovement)
			return false;

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

		std::vector<glm::ivec2> points;
		Line(points, start, position);
		assert(points.size() > 2); // not adjacent, so minimum of 3 points

		// process all but first/last points, for vision blocking
		for (int i = 1; i< int(points.size()) - 1; ++i) 
			if (!PositionIsVisible(points[i]))
				return false;
		return true;
	}

	Entity* Level::GetEntity(const glm::ivec2& position, bool blocksMovement) const
	{
		if (!bg.InBounds(position))
			return nullptr;
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

	void Level::OnObjectStateChanged(const Entity& e)
	{
		// The change in the object's state might affect visibility, so update it for good measure
		UpdateFogOfWar();
	}

	std::vector<glm::ivec2> Level::CalcPath(const Entity& e, const glm::ivec2& tgt) const
	{
		return calcPath(e.GetLocation().position, tgt, bg.Size(), [&](const glm::ivec2& p) { return EntityCanMoveTo(e, p) ? 1.0f : std::numeric_limits<float>::infinity(); });
	}


	std::pair<Array2D<LevelBgElement>, std::vector<std::pair<DbIndex, EntityDynamicConfig>>> LoadLevelFromTxtFile(const std::string& filename)
	{	
		static const LevelBgElement bgFloor = { "floor", false, false, false, '.', glm::vec4(.7, .7, .7, 1) };
		static const LevelBgElement bgWall = { "wall", true, true, false, '#', glm::vec4(.7, .7, .7, 1) };
		static const LevelBgElement bgWater = { "water", false, true, true, '=', glm::vec4(0, 0, 1, 1) };

		auto text = rlf::readTextFile(filename);
		
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

		

		// Go through the data, and remember that the file starts with highest Y value first (top-to-bottom)
		//for (int y = height - 1; y >= 0; --y)
		for (int y = 0; y < height; ++y)
		{
			for (int x = 0; x < width; ++x)
			{
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
					entityCfgs.emplace_back("door", dcfg);
					break;
				case '>':
					entityCfgs.emplace_back("stairs_down", dcfg);
					break;
				case '<':
					entityCfgs.emplace_back("stairs_up", dcfg);
					break;
				case 'X':
					entityCfgs.emplace_back("goblin", dcfg);
					break;
				case '$':
					dcfg.inventory.emplace_back("gold");
					entityCfgs.emplace_back("item_pile", dcfg);
					break;
				default:
					break;
				}
			}
		}

		std::vector<std::string> spawnableItems = {
			"helmet",
			"sword",
			"hat",
		};
		for (int i = 0; i < 200; ++i)
		{
			auto x = rand()%width;
			auto y = rand() % height;
			ivec2 p = { x,y };
			if (!bg(x, y).blocksMovement && std::find_if(entityCfgs.begin(), entityCfgs.end(), [&](const auto& dbi_dcfg) { return dbi_dcfg.second.position == p; }) == entityCfgs.end())
			{
				EntityDynamicConfig dcfg{ p };
				dcfg.inventory.emplace_back(spawnableItems[rand()% spawnableItems.size()]);
				entityCfgs.emplace_back("item_pile", dcfg);
			}
				
		}

		return { bg, entityCfgs };
	}
}