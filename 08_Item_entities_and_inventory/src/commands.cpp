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
		if (e.Type() == EntityType::Item) // it's an item -- remove it from its owner!
		{
			auto& items = e.GetItemData()->owner.Entity()->GetInventory()->items;
			items.erase(std::remove_if(items.begin(), items.end(), [&e](const EntityId& eref) {
				return eref == e.Id();
			}), items.end());
		}
		Game::Instance().RemoveEntity(e);
	}

	void Teleport(Entity& entity, const glm::ivec2& position)
	{
		assert(entity.Type() != EntityType::Item);
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
		assert(entity.Type() != EntityType::Item);
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
		if(defenderDied)
			attacker.GetCreatureData()->xp++;
	}

	void TransferItem(const EntityId& takerId, const EntityId itemId, Entity& giver)
	{
		// remove from giver
		auto& items = takerId.Entity()->GetInventory()->items;
		auto& giverItems = giver.GetInventory()->items;
		giverItems.erase(std::remove_if(giverItems.begin(), giverItems.end(), [&itemId](const EntityId& eref) { 
			return eref == itemId; 
		}), giverItems.end());

		// add to taker
		const auto& itemDbCfg = itemId.Entity()->DbCfg();
		auto itFound = std::find_if(items.begin(), items.end(), [&itemDbCfg](const EntityId& takerItemId) {return takerItemId.Entity()->DbCfg() == itemDbCfg; });
		if (itFound != items.end() && itemDbCfg.Cfg()->itemCfg.IsStackable())
			++itFound->Entity()->GetItemData()->stackSize;
		else
		{
			itemId.Entity()->GetItemData()->owner = takerId;
			items.push_back(itemId);
		}

		// if giver is an item pile
		if (giver.DbCfg() == DbIndex::ItemPile())
		{
			// if it just became empty, destroy it
			if (giverItems.empty())
				DestroyEntity(giver);
			// else update its state
			else
				sig::onObjectStateChanged.fire(giver);
		}
		// if taker is an item pile, update its state
		const auto& taker = *takerId.Entity();
		if (taker.DbCfg() == DbIndex::ItemPile())
			sig::onObjectStateChanged.fire(taker);
		
	}

	void PickUpEverythingOrHandle(Entity& handler)
	{
		auto& g = Game::Instance();
		const auto& level = g.CurrentLevel();
		auto position = handler.GetLocation().position;
		auto entityOnGround = level.GetEntity(position, false);
		if (entityOnGround != nullptr)
		{
			if (entityOnGround->GetInventory())
			{
				for (const auto& itemId : entityOnGround->GetInventory()->items)
					TransferItem(handler.Id(), itemId, *entityOnGround);
				Game::Instance().WriteToMessageLog(fmt::format("{0} picks up some items", handler.Name()));
			}
			else
			{
				Handle(*entityOnGround, handler);
			}
		}
	}

	void PickUp(Entity& handler, Entity& itemPile, const EntityId& itemId)
	{
		TransferItem(handler.Id(), itemId, itemPile);
		Game::Instance().WriteToMessageLog(fmt::format("{0} picks up {1}", handler.Name(), itemId.Entity()->Name()));
	}

	void Drop(Entity& handler, const EntityId& itemId)
	{
		const auto& level = Game::Instance().CurrentLevel();
		auto position = handler.GetLocation().position;
		// When dropping an item, if an item pile does not exist under our feet, we need to create one
		auto itemPile = level.GetEntity(position, false);
		if (itemPile == nullptr)
		{
			EntityDynamicConfig dcfg;
			dcfg.position = position;
			itemPile = Game::Instance().CreateEntity(DbIndex::ItemPile(), dcfg, true).Entity();
		}
		TransferItem(itemPile->Id(), itemId, handler);
		Game::Instance().WriteToMessageLog(fmt::format("{0} drops {1}", handler.Name(), itemId.Entity()->Name()));
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