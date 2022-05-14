#pragma once

#include <string>
#include <array>
#include <vector>
#include <memory>

#include <glm/glm.hpp>

#include "tilemap.h"
#include "db.h"
#include "entityid.h"

namespace rlf
{
	class GameState;
	class Entity;
	
	// allow 8 bits per type, so a max of 255 subtypes
	enum class EntityType : uint32_t
	{
		// Creatures
		Creature = 0,
		// Objects
		Object = 256,
		Object_Door,
		Object_StairsUp,
		Object_StairsDown,
		Object_ItemPile,
		// Items
		Item = 512
	};

	// clear out the 8 lowest bits that define the subtype
	inline EntityType BaseEntityType(EntityType t) { return EntityType(uint32_t(t) & 0xffffff00); }

	enum class EquipmentSlot : int
	{
		Weapon = 0,
		Shield,
		Armor,
		Accessory,
		NUM
	};

	///---------------------------------------------------------------------------
	// Entity "components"

	// Location: where is the creature/object?
	struct Location
	{
		int levelId = -1;
		glm::ivec2 position;
	};

	// Inventory: does the creature/object store items and/or gold?
	struct Inventory
	{
		int gold = 0;
		std::vector<EntityId> items;
	};

	// Equipment: can the creature equip items?
	struct Equipment
	{
		std::array<EntityId, int(EquipmentSlot::NUM)> items;
	};

	// Creature-specific data
	struct CreatureData
	{
		int lineOfSightRadius = 10;
	};

	// Object-specific data
	struct ObjectData
	{
		int state = 0;
		bool blocksMovement = false;
		bool blocksVision = false;

		void Handle(const Entity& object, const Entity& handler);
	};

	// Item-specific data
	struct ItemData
	{
		int stackSize = 0;
		// owner entity; object or creature
		EntityId owner;

		bool IsStackable() const { return stackSize > 0; }
		//void ChangeOwner();
	};

	struct EntityConfig
	{
		EntityType type = EntityType::Creature;
		std::vector<TileData> tileData;
	};

	struct EntityDynamicConfig
	{
		glm::ivec2 position;
		std::string nameOverride;
		EntityId itemOwner;
		std::vector<DbIndex> inventory;
	};

	// Entities
	class Entity
	{
	public:

		Entity() = default;
		Entity(Entity&&) = default; // movable
		Entity(const Entity&) = delete; // non construction-copyable    
		Entity& operator=(const Entity&) = delete; // non copyable

		EntityType Type() const { return type; }
		EntityType BaseType() const { return BaseEntityType(type); }

		const DbIndex& DbCfg() const { return dbIndex; }
		const EntityId& Id() const { return id; }
		const std::string& Name() { return name; }

		bool BlocksMovement() const;
		bool BlocksVision() const;
		bool CanInteractFromAdjacentTile() const { return BlocksMovement(); }

		void SetLocation(const Location& newLocation) { location = newLocation; }
		const Location& GetLocation() const { return location; }
		const Inventory* GetInventory() const { return inventory.get(); }
		const Equipment* GetEquipment() const { return equipment.get(); }
		const CreatureData* GetCreatureData() const { return creatureData.get(); }
		ObjectData* GetObjectData() const { return objectData.get(); }
		const ItemData* GetItemData() const { return itemData.get(); }

		// how do we render this entity given its current state?
		const TileData& CurrentTileData() const;

	private:
		friend class GameState;
		void Initialize(EntityId id, DbIndex dbIndex, const EntityDynamicConfig& dcfg);
		
	private:

		// shared data
		DbIndex dbIndex;

		// identifier for this entity
		EntityId id;

		// Entity data here
		std::string name;
		EntityType type;

		// Location (level and position), useful for creatures and objects
		Location location;
		// Inventory is useful for creatures and sometimes objects
		std::unique_ptr<Inventory> inventory;
		// Equipment is useful for creatures
		std::unique_ptr<Equipment> equipment;
		
		std::unique_ptr<CreatureData> creatureData;
		std::unique_ptr<ObjectData> objectData;
		std::unique_ptr<ItemData> itemData;
	};
}
