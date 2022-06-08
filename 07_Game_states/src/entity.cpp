#include "entity.h"
#include "game.h"
#include "graphics.h"
#include "commands.h"
#include "grid.h"
#include "signals.h"

using namespace glm;

namespace rlf
{
	int Inventory::Weight() const
	{
		// weight is the sum of weights of all items in inventory
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

		// any other work, now that the basics are done

		// Initialization depending on the entity type
		if (type == EntityType::Creature)
		{
			inventory = std::make_unique<Inventory>(); // Creatures always have inventory
			creatureData = std::make_unique<CreatureData>();
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
			itemData->owner = dcfg.itemOwner;
		}
		
		// items don't have a position
		if (type != EntityType::Item)
			location = { Game::Instance().GetCurrentLevelIndex(), dcfg.position };

		// Add inventory items where applicable
		if (DbCfg() == DbIndex::ItemPile() || !dcfg.inventory.empty())
		{
			if(!inventory)
				inventory = std::make_unique<Inventory>();
			EntityDynamicConfig dcfgItem;
			dcfgItem.itemOwner = id;
			for (const auto& itemCfg : dcfg.inventory)
				inventory->items.push_back( Game::Instance().CreateEntity(itemCfg, dcfgItem,true));
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
			// creatures don't block vision
		case EntityType::Creature:
			return false;
			// objects may block vision
		case EntityType::Object:
			return objectData->blocksVision;
		default:
			assert(false); // don't be here
			break;
		}
		return false;
	}
}