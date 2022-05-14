#include "entity.h"
#include "game.h"
#include "graphics.h"
#include "eventhandlers.h"

namespace rlf
{
	void Entity::Initialize(EntityId id, DbIndex dbIndex, const EntityDynamicConfig& dcfg)
	{
		this->id = id;
		this->dbIndex = dbIndex;
		
		const auto& cfg = dbIndex.Cfg();
		type = cfg->type;
		name = dcfg.nameOverride.empty() ? dbIndex.name : dcfg.nameOverride;

		// TODO: any other work, now that the basics are done
		
		const auto baseType = BaseEntityType(type);
		if (baseType == EntityType::Creature)
		{
			creatureData = std::make_unique<CreatureData>();
			inventory = std::make_unique<Inventory>();
		}
		else if (baseType == EntityType::Object)
			objectData = std::make_unique<ObjectData>();
		else if (baseType == EntityType::Item)
			itemData = std::make_unique<ItemData>();
		
		if (baseType != EntityType::Item)
		{
			location = { GameState::Instance().GetCurrentLevelIndex(), dcfg.position };
		}
		else
		{
			itemData->owner = dcfg.itemOwner;
		}

		if (!dcfg.inventory.empty())
		{
			if(!inventory)
				inventory = std::make_unique<Inventory>();
			EntityDynamicConfig dcfgItem;
			dcfgItem.itemOwner = id;
			for (const auto& itemCfg : dcfg.inventory)
				inventory->items.push_back( GameState::Instance().CreateEntity(itemCfg, dcfgItem));
		}

		switch (type)
		{
			case EntityType::Object_Door:
				objectData->blocksMovement = true;
				objectData->blocksVision = true;
				objectData->state = 0;
				break;
			default:
				break;
		}
	}

	const TileData& Entity::CurrentTileData() const
	{
		// If we're an item pile with a single item, show the tile data of that single item
		if (Type() == EntityType::Object_ItemPile && inventory->items.size() == 1)
			return inventory->items[0].Entity()->CurrentTileData();
		else
		{
			int state = BaseType() == EntityType::Object ? objectData->state : 0;
			return DbCfg().Cfg()->tileData[state];
		}
	}

	bool Entity::BlocksMovement() const
	{
		auto baseEntityType = BaseType();
		switch (baseEntityType)
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
		auto baseEntityType = BaseType();
		switch (baseEntityType)
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

	void ObjectData::Handle(const Entity& object, const Entity& handler)
	{
		auto oldState = state;
		if (object.Type() == EntityType::Object_Door)
		{
			state = 1 - state;
			blocksMovement = state == STATE_DOOR_CLOSED;
			blocksVision = state == STATE_DOOR_CLOSED;
		}
		else if (object.Type() == EntityType::Object_StairsUp)
		{
			auto& g = GameState::Instance();
			if (g.GetCurrentLevelIndex() > 0)
				ChangeLevel(g.GetCurrentLevelIndex() - 1);
		}
		else if (object.Type() == EntityType::Object_StairsDown)
		{
			auto& g = GameState::Instance();
			ChangeLevel(g.GetCurrentLevelIndex() + 1);
		}
		else if (object.Type() == EntityType::Object_ItemPile)
		{
			auto& g = GameState::Instance();
			for (const auto& itemId : object.GetInventory()->items)
			{
				// set owner and add to handler
			}
		}

		if (state != oldState)
		{
			GameState::Instance().CurrentLevel().OnObjectStateChanged(object);
			Graphics::Instance().OnObjectStateChanged(object);
		}
	}
}