#include "commands.h"

#include <fmt/format.h>

#include "game.h"
#include "entity.h"
#include "graphics.h"
#include "signals.h"

using namespace glm;

namespace rlf
{
	// The different door states, to be used with the state variable of the struct ObjectData
	constexpr int STATE_DOOR_CLOSED = 0;
	constexpr int STATE_DOOR_OPEN = 1;

	// Kill a creature
	void DestroyEntity(Entity& e)
	{
		sig::onEntityRemoved.fire(e);
		Game::Instance().RemoveEntity(e);
	}

	void Teleport(Entity& entity, const glm::ivec2& position)
	{
		const auto& level = Game::Instance().CurrentLevel();
		
		// Check if we can! If not, spawn a message
		if (!level.EntityCanMoveTo(entity, position))
			return;

		auto loc = entity.GetLocation();
		loc.position = position;
		entity.SetLocation(loc);
		sig::onEntityMoved.fire(entity);

		if (Game::Instance().IsPlayer(entity))
		{
			Game::Instance().CurrentLevel().UpdateFogOfWar();
			sig::onGuiUpdated.fire();// movement should cause GUI Update
		}
	}

	static const char * DirectionString(const glm::ivec2& direction)
	{
		if (direction == ivec2{ 1, 0 })
			return "east";
		else if (direction == ivec2{ -1, 0 })
			return "west";
		else if (direction == ivec2{ 0, 1 })
			return "north";
		else if (direction == ivec2{ 0, -1 })
			return "south";
		else
			return "unknown";
	}

	void MoveAdj(Entity& entity, const glm::ivec2& direction)
	{
		const auto& level = Game::Instance().CurrentLevel();

		auto position = entity.GetLocation().position + direction;

		// Check if we can move
		if (level.EntityCanMoveTo(entity, position))
		{
			auto loc = entity.GetLocation();
			loc.position = position;
			entity.SetLocation(loc);
			sig::onEntityMoved.fire(entity);

			if (Game::Instance().IsPlayer(entity))
			{
				Game::Instance().WriteToMessageLog(fmt::format("{0} moves {1}", entity.Name(), DirectionString(direction)));
				Game::Instance().CurrentLevel().UpdateFogOfWar();
				sig::onGuiUpdated.fire();// movement should cause GUI Update
			}
		}
		else // ok, we can't move. Get entity at the obstacle tile
		{
			auto entityAtPosition = level.GetEntity(position, true);
			if (entityAtPosition != nullptr)
			{
				switch (entityAtPosition->Type())
				{
					// if it's a creature, attack it
					case EntityType::Creature:
					{
						AttackEntity(entity, *entityAtPosition);
						break;
					}
					// if it's an object, try to handle it
					case EntityType::Object:
					{
						if (Game::Instance().IsPlayer(entity))
							Game::Instance().WriteToMessageLog(fmt::format("{0} handles {1}", entity.Name(), entityAtPosition->Name()));
						Handle(*entityAtPosition, entity);
						break;
					}
				}
			}
		}
	}

	// return true if entity died
	bool ModifyHp(Entity& entity, int hpMod)
	{
		// get the hp and update it
		auto& hp = entity.GetCreatureData()->hp;
		hp = glm::min(hp + hpMod, entity.DbCfg().Cfg()->creatureCfg.hp);
		// check if the entity is dead, and handle accordingly
		auto died = entity.GetCreatureData()->hp <= 0;
		std::string text;
		if (died)
		{
			text = fmt::format("{0} has died!", entity.Name());
			if (!Game::Instance().IsPlayer(entity))
				DestroyEntity(entity);
		}
		else
			text = hpMod <= 0 ? fmt::format("{0} suffers {1} damage", entity.Name(), -hpMod) : fmt::format("{0} heals for {1} HP", entity.Name(), hpMod);
		if(!text.empty())
			Game::Instance().WriteToMessageLog(text);
		return died;
	}

	void AttackEntity(Entity& attacker, Entity& defender)
	{
		// DestroyEntity(*entityAtPosition);
		auto& g = Game::Instance();
		// Super-simple combat - each bump is 1 damage
		auto text = fmt::format("{0} attacks {1}", attacker.Name(), defender.Name());
		g.WriteToMessageLog(text);
		auto defenderDied = ModifyHp(defender, -1);
		if (defenderDied)
			attacker.GetCreatureData()->xp++;
	}

	void HandleOnFloor(Entity& handler)
	{
		auto& g = Game::Instance();
		const auto& level = g.CurrentLevel();
		auto position = handler.GetLocation().position;
		auto entityOnGround = level.GetEntity(position, false);
		if (entityOnGround != nullptr)
			Handle(*entityOnGround, handler);
	}

	void Handle(Entity& handled, Entity& handler)
	{
		auto& objData = *handled.GetObjectData();
		auto oldState = objData.state;
		if (handled.DbCfg() == DbIndex::Door())
		{
			objData.state = 1 - objData.state;
			objData.blocksMovement = objData.state == STATE_DOOR_CLOSED;
			objData.blocksVision = objData.state == STATE_DOOR_CLOSED;
		}
		else if (handled.DbCfg() == DbIndex::StairsUp())
		{
			auto& g = Game::Instance();
			if (g.GetCurrentLevelIndex() > 0)
				ChangeLevel(g.GetCurrentLevelIndex() - 1);
		}
		else if (handled.DbCfg() == DbIndex::StairsDown())
		{
			auto& g = Game::Instance();
			ChangeLevel(g.GetCurrentLevelIndex() + 1);
		}
		if (objData.state != oldState)
		{
			sig::onObjectStateChanged.fire(handled);
		}
	}

	void ChangeLevel(int levelIndex)
	{
		auto& g = Game::Instance();

		auto delveDirectionForward = g.GetCurrentLevelIndex() < levelIndex;

		auto playerId = g.PlayerId();

		// remove player from old level
		if (playerId.Entity() != nullptr && g.GetCurrentLevelIndex() >= 0)
			sig::onEntityRemoved.fire(*playerId.Entity());

		// Change the level
		g.ChangeLevel(levelIndex);

		auto& level = g.CurrentLevel();
		sig::onLevelChanged.fire(level);

		// put player in new level
		auto placeAtStairsEntityType = delveDirectionForward ? DbIndex::StairsUp() : DbIndex::StairsDown();
		if (playerId.Entity() != nullptr)
		{
			for (const auto& entity : level.Entities())
				if (entity.Entity()->DbCfg() == placeAtStairsEntityType)
				{
					auto position = entity.Entity()->GetLocation().position;
					playerId.Entity()->SetLocation({ levelIndex, position });
					break;
				}
			sig::onEntityAdded.fire(*playerId.Entity());
			level.UpdateFogOfWar();
		}

		std::string msgtext = delveDirectionForward ? "You delve deeper into the dungeon" : "You take the stairs up";
		Game::Instance().WriteToMessageLog(msgtext); // This triggers a gui Update
	}
}