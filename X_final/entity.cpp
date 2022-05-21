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
		int weight = 0;
		for (const auto& item : items)
			weight += item.Entity()->DbCfg().Cfg()->itemCfg.weight * item.Entity()->GetItemData()->stackSize;
		return weight;
	}

	int Inventory::EquippedItemAtSlot(ItemCategory itemCategory) const
	{
		auto it = std::find_if(items.begin(), items.end(), [&itemCategory](const EntityId& itemId) {
			auto e = itemId.Entity();
			return e->GetItemData()->equipped && e->DbCfg().Cfg()->itemCfg.category == itemCategory;
		});
		return it != items.end() ? int(std::distance(items.begin(), it)) : -1;
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
			location = { Game::Instance().GetCurrentLevelIndex(), dcfg.position };
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
}