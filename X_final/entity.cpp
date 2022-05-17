#include "entity.h"
#include "game.h"
#include "graphics.h"
#include "eventhandlers.h"
#include "grid.h"

using namespace glm;

namespace rlf
{
	int Inventory::Weight() const
	{
		int weight = 0;
		for (const auto& item : items)
			weight += item.Entity()->DbCfg().Cfg()->itemCfg.weight * item.Entity()->GetItemData()->stackSize;
		return weight;
	}

	void Entity::Initialize(EntityId id, DbIndex dbIndex, const EntityDynamicConfig& dcfg)
	{
		this->dbIndex = dbIndex;
		this->id = id;
		name = dcfg.nameOverride.empty() ? dbIndex.name : dcfg.nameOverride;
		const auto& cfg = dbIndex.Cfg();
		type = cfg->type;

		// clear everything else
		location = {};
		inventory.reset();
		creatureData.reset();
		objectData.reset();
		itemData.reset();

		// TODO: any other work, now that the basics are done
		
		if (type == EntityType::Creature)
		{
			creatureData = std::make_unique<CreatureData>();
			inventory = std::make_unique<Inventory>();

			creatureData->hp = cfg->creatureCfg.hp;
		}
		else if (type == EntityType::Object)
		{
			objectData = std::make_unique<ObjectData>();
			objectData->blocksMovement = cfg->objectCfg.blocksMovement;
			objectData->blocksVision = cfg->objectCfg.blocksVision;
			objectData->state = cfg->objectCfg.defaultState;
		}
		else if (type == EntityType::Item)
		{
			itemData = std::make_unique<ItemData>();
			itemData->stackSize = cfg->itemCfg.IsStackable() ? cfg->itemCfg.defaultStackSize : 1;
		}
		
		if (type != EntityType::Item)
		{
			location = { GameState::Instance().GetCurrentLevelIndex(), dcfg.position };
		}
		else
		{
			itemData->owner = dcfg.itemOwner;
		}

		if (DbCfg() == DbIndex::ItemPile() || !dcfg.inventory.empty())
		{
			if(!inventory)
				inventory = std::make_unique<Inventory>();
			EntityDynamicConfig dcfgItem;
			dcfgItem.itemOwner = id;
			for (const auto& itemCfg : dcfg.inventory)
				inventory->items.push_back( GameState::Instance().CreateEntity(itemCfg, dcfgItem));
		}

		if (dbIndex == DbIndex::Door())
		{
			objectData->blocksMovement = true;
			objectData->blocksVision = true;
			objectData->state = 0;
		}	
	}

	const TileData& Entity::CurrentTileData() const
	{
		// If we're an item pile with a single item, show the tile data of that single item
		if (DbCfg() == DbIndex::ItemPile() && inventory->items.size() == 1)
			return inventory->items[0].Entity()->CurrentTileData();
		else
		{
			int state = Type() == EntityType::Object ? objectData->state : 0;
			return DbCfg().Cfg()->tileData[state];
		}
	}

	bool Entity::BlocksMovement() const
	{
		switch (Type())
		{
			// Creatures always block movement
		case EntityType::Creature:
			return true;
			// Objects MAY block movement
		case EntityType::Object:
			return objectData->blocksMovement;
		default:
			assert(false); // don't be here
			break;
		}
		return false;
	}

	bool Entity::BlocksVision() const
	{
		switch (Type())
		{
		case EntityType::Creature:
			return false;
		case EntityType::Object:
			return objectData->blocksVision;
		default:
			assert(false); // don't be here
			break;
		}
		return false;
	}

	// Interaction functions

	constexpr int STATE_DOOR_CLOSED = 0;
	constexpr int STATE_DOOR_OPEN = 1;

	void ObjectData::Handle(Entity& object, Entity& handler)
	{
		auto oldState = state;
		if (object.DbCfg() == DbIndex::Door())
		{
			state = 1 - state;
			blocksMovement = state == STATE_DOOR_CLOSED;
			blocksVision = state == STATE_DOOR_CLOSED;
		}
		else if (object.DbCfg() == DbIndex::StairsUp())
		{
			auto& g = GameState::Instance();
			if (g.GetCurrentLevelIndex() > 0)
				ChangeLevel(g.GetCurrentLevelIndex() - 1);
		}
		else if (object.DbCfg() == DbIndex::StairsDown())
		{
			auto& g = GameState::Instance();
			ChangeLevel(g.GetCurrentLevelIndex() + 1);
		}
		else if (int(object.DbCfg().Cfg()->objectCfg.effect) >= 0)
		{
			auto& g = GameState::Instance();
			ApplyEffect(handler, object.DbCfg().Cfg()->objectCfg.effect);
		}
		if (state != oldState)
		{
			GameState::Instance().CurrentLevel().OnObjectStateChanged(object);
			Graphics::Instance().OnObjectStateChanged(object);
		}
	}
}