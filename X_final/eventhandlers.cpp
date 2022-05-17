#include "eventhandlers.h"

#include <fmt/format.h>

#include "game.h"
#include "entity.h"
#include "graphics.h"

using namespace glm;

namespace rlf
{
	Entity& SpawnEntity(const DbIndex& cfg, const EntityDynamicConfig& dcfg)
	{
		auto& e = *GameState::Instance().CreateEntity(cfg, dcfg).Entity();
		if (e.Type() != EntityType::Item)
		{
			GameState::Instance().CurrentLevel().OnEntityAdded(e);
			Graphics::Instance().OnEntityAdded(e);
		}
		return e;
	}

	// Kill a creature
	void DestroyEntity(Entity& e)
	{
		if (e.Type() != EntityType::Item)
		{
			GameState::Instance().CurrentLevel().OnEntityRemoved(e);
			Graphics::Instance().OnEntityRemoved(e);
		}
		else // it's an item -- remove it from its owner!
		{
			auto& items = e.GetItemData()->owner.Entity()->GetInventory()->items;
			items.erase(std::remove_if(items.begin(), items.end(), [&e](const EntityId& eref) {
				return eref == e.Id();
			}), items.end());
		}
		GameState::Instance().RemoveEntity(e);
	}

	void Teleport(Entity& entity, const glm::ivec2& position)
	{
		assert(entity.Type() != EntityType::Item);
		const auto& level = GameState::Instance().CurrentLevel();
		
		// Check if we can! If not, spawn a message
		if (!level.EntityCanMoveTo(entity, position))
			return;

		auto loc = entity.GetLocation();
		loc.position = position;
		entity.SetLocation(loc);
		Graphics::Instance().OnEntityMoved(entity);

		if (GameState::Instance().IsPlayer(entity))
		{
			GameState::Instance().CurrentLevel().UpdateFogOfWar();
			Graphics::Instance().OnGuiUpdated(); // movement should cause GUI update
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
			return "unkwnown";
	}

	const Entity * EquippedItemAtSlot(const Entity& entity, ItemCategory itemCategory)
	{
		const auto& items = entity.GetInventory()->items;
		auto itFound = std::find_if(items.begin(), items.end(), [&itemCategory](const EntityId& itemId) {
			return itemId.Entity()->GetItemData()->equipped && itemId.Entity()->DbCfg().Cfg()->itemCfg.category == itemCategory;
		});
		return itFound != items.end() ? itFound->Entity() : nullptr;
	}

	void MoveAdj(Entity& entity, const glm::ivec2& direction)
	{
		assert(entity.Type() != EntityType::Item);
		const auto& level = GameState::Instance().CurrentLevel();

		auto position = entity.GetLocation().position + direction;

		// Check if we can move
		if (level.EntityCanMoveTo(entity, position))
		{
			auto loc = entity.GetLocation();
			loc.position = position;
			entity.SetLocation(loc);
			Graphics::Instance().OnEntityMoved(entity);

			if (GameState::Instance().IsPlayer(entity))
			{
				GameState::Instance().WriteToMessageLog(fmt::format("You move {0}", DirectionString(direction)));
				GameState::Instance().CurrentLevel().UpdateFogOfWar();
				Graphics::Instance().OnGuiUpdated(); // movement should cause GUI update
			}
		}
		else // ok, we can't move. Get entity at the tile
		{
			auto entityAtPosition = level.GetEntity(position, true);
			if (entityAtPosition != nullptr)
			{
				switch (entityAtPosition->Type())
				{
					case EntityType::Creature:
					{
						// Attack if we're attacking with bare hands or a melee weapon
						auto weapon = EquippedItemAtSlot(entity, ItemCategory::Weapon);
						if (weapon == nullptr || weapon->DbCfg().Cfg()->itemCfg.attackRange == 1)
							AttackEntity(entity, *entityAtPosition);
						break;
					}
					case EntityType::Object:
					{
						GameState::Instance().WriteToMessageLog(fmt::format("You handle {0}", entityAtPosition->Name()));
						entityAtPosition->GetObjectData()->Handle(*entityAtPosition, entity);
						break;
					}
				}
			}
		}
	}

	// return true if entity died
	bool ModifyHp(Entity& entity, int hpMod)
	{
		auto& hp = entity.GetCreatureData()->hp;
		hp = glm::min(hp + hpMod, entity.DbCfg().Cfg()->creatureCfg.hp);
		auto died = entity.GetCreatureData()->hp <= 0;
		std::string text;
		if (died)
		{
			text = GameState::Instance().IsPlayer(entity) 
				? "You have died" 
				: fmt::format("{0} has died!", entity.Name());
			DestroyEntity(entity);
		}
		else
		{
			if (hpMod <= 0)
				text = GameState::Instance().IsPlayer(entity) 
					? fmt::format("You suffer {0} damage",-hpMod) 
					: fmt::format("{0} suffers {1} damage", entity.Name(), -hpMod);
			else
				text = GameState::Instance().IsPlayer(entity) 
					? fmt::format("You heal for {0} HP", hpMod) 
					: fmt::format("{0} heals for {1} HP", entity.Name(), hpMod);
		}
		GameState::Instance().WriteToMessageLog(text);
		return died;
	}
	
	ivec4 AccumulateCombatStats(const Entity& creature)
	{
		auto stats = creature.DbCfg().Cfg()->creatureCfg.combatStats;
		for (const auto& item : creature.GetInventory()->items)
			if (item.Entity()->GetItemData()->equipped)
				stats += item.Entity()->DbCfg().Cfg()->itemCfg.combatStatBonuses;
		return stats;
	}

	void AttackEntity(Entity& attacker, Entity& defender)
	{
		// DestroyEntity(*entityAtPosition);
		auto& g = GameState::Instance();
#if 0	// Super-simple combat
		auto text = g.IsPlayer(attacker)
			? fmt::format("You attack {0}", defender.Name())
			: fmt::format("{0} attacks {1}", attacker.Name(), defender.Name());
		g.WriteToMessageLog(text);
		auto defenderDied = ModifyHp(defender, -1);
#else
		bool defenderDied = false;
		auto attackerStats = AccumulateCombatStats(attacker);
		auto defenderStats = AccumulateCombatStats(defender);
		// check if attack lands!
		std::string text = fmt::format("{0} attacks {1}. ", attacker.Name(), defender.Name());
		auto attRoll = rand() % max(attackerStats[int(CombatStat::Attack)],1);
		auto defRoll = rand() % max(defenderStats[int(CombatStat::Defense)],1);
		text += fmt::format("{0}(d{1}) vs {2}(d{3}): ", attRoll + 1, attackerStats[int(CombatStat::Attack)], defRoll + 1, defenderStats[int(CombatStat::Defense)]);
		if (defRoll < attRoll) // does the attack land?
		{
			auto damage = max(attackerStats[int(CombatStat::Damage)] - defenderStats[int(CombatStat::Resist)], 0);
			text += fmt::format("HIT for {0} damage",damage);
			g.WriteToMessageLog(text);
			if (damage > 0)
			{
				auto defenderDied = ModifyHp(defender, -1);
				if (defenderDied)
					attacker.GetCreatureData()->xp++;
			}
		}
		else
		{
			text += "MISS";
			g.WriteToMessageLog(text);
		}
#endif
		
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

		if (giver.DbCfg() == DbIndex::ItemPile())
		{
			if (giverItems.empty())
				DestroyEntity(giver);
			else if (giverItems.size() == 1)
				Graphics::Instance().OnObjectStateChanged(giver);
		}
		const auto& taker = *takerId.Entity();
		if (taker.DbCfg() == DbIndex::ItemPile())
			Graphics::Instance().OnObjectStateChanged(taker);
		
	}

	void PickUpEverythingOrHandle(Entity& handler)
	{
		auto& g = GameState::Instance();
		const auto& level = g.CurrentLevel();
		auto position = handler.GetLocation().position;
		auto entityAtPosition = level.GetEntity(position, false);
		if (entityAtPosition != nullptr)
		{
			if (entityAtPosition->GetInventory())
			{
				for (const auto& itemId : entityAtPosition->GetInventory()->items)
					TransferItem(handler.Id(), itemId, *entityAtPosition);
				if (g.IsPlayer(handler))
					GameState::Instance().WriteToMessageLog("You pick up some items");
			}
			else
			{
				entityAtPosition->GetObjectData()->Handle(*entityAtPosition, handler);
			}
		}
	}

	void PickUp(Entity& handler, Entity& itemPile, const EntityId& itemId)
	{
		TransferItem(handler.Id(), itemId, itemPile);
		if (GameState::Instance().IsPlayer(handler))
			GameState::Instance().WriteToMessageLog(fmt::format("You pick up {0}", itemId.Entity()->Name()));
	}

	void Drop(Entity& handler, const EntityId& itemId)
	{
		const auto& level = GameState::Instance().CurrentLevel();
		auto position = handler.GetLocation().position;
		auto itemPile = level.GetEntity(position, false);
		if (itemPile == nullptr)
		{
			EntityDynamicConfig dcfg;
			dcfg.position = position;
			itemPile = &SpawnEntity(DbIndex{"item_pile"}, dcfg);
		}
		TransferItem(itemPile->Id(), itemId, handler);
		if (GameState::Instance().IsPlayer(handler))
			GameState::Instance().WriteToMessageLog(fmt::format("You drop {0}", itemId.Entity()->Name()));
	}

	void ChangeLevel(int levelIndex)
	{
		auto& g = GameState::Instance();

		auto delveDirectionForward = g.GetCurrentLevelIndex() < levelIndex;

		auto playerId = g.Player();

		// remove player from old level
		if (playerId.Entity() != nullptr && g.GetCurrentLevelIndex() >= 0)
			g.CurrentLevel().OnEntityRemoved(*playerId.Entity());

		// Change the level
		g.SetCurrentLevelIndex(levelIndex);

		auto& level = g.CurrentLevel();
		Graphics::Instance().OnLevelChanged(level);		

		// put player in new level
		auto placeAtStairsEntityType = delveDirectionForward ? DbIndex::StairsUp() : DbIndex::StairsDown();
		if (playerId.Entity() != nullptr)
		{
			for (const auto& entity : level.Entities())
				if (entity.Entity()->DbCfg() == placeAtStairsEntityType)
				{
					auto position = entity.Entity()->GetLocation().position;
					playerId.Entity()->SetLocation({ levelIndex, position });
				}

			level.OnEntityAdded(*playerId.Entity());
			Graphics::Instance().OnEntityAdded(*playerId.Entity());
			level.UpdateFogOfWar();
		}

		std::string msgtext = delveDirectionForward ? "You delve deeper into the dungeon" : "You take the stairs up";
		GameState::Instance().WriteToMessageLog(msgtext); // This triggers a gui update
	}

	void ChangeEquippedItem(Entity& owner, int newEquippedIdx, int oldEquippedIdx)
	{
		auto& items = owner.GetInventory()->items;
		if (newEquippedIdx >= 0)
			items[newEquippedIdx].Entity()->GetItemData()->equipped = true;
		if (oldEquippedIdx >= 0)
			items[oldEquippedIdx].Entity()->GetItemData()->equipped = false;
	}

	void UseItem(Entity& owner, int itemIdx)
	{
		auto& items = owner.GetInventory()->items;
		auto& item = *items[itemIdx].Entity();
		ApplyEffect(owner, item.DbCfg().Cfg()->itemCfg.effect);
		if(GameState::Instance().IsPlayer(owner))
			GameState::Instance().WriteToMessageLog("You used " + item.Name());
		auto& stackSize = item.GetItemData()->stackSize;
		--stackSize;
		if (stackSize == 0)
			DestroyEntity(item);
	}
}