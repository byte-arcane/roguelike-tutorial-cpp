#pragma once

#include <string>
#include <array>
#include <vector>
#include <memory>

#include <glm/glm.hpp>

#include "tilemap.h"
#include "db.h"
#include "entityid.h"
#include "effect.h"

namespace rlf
{
	class GameState;
	class Entity;
	
	// allow 8 bits per type, so a max of 255 subtypes
	enum class EntityType : uint32_t
	{
		Creature,
		Object,
		Item
	};

	enum class ItemCategory : int
	{
		Weapon = 0,
		Shield,
		Armor,
		Accessory,
		Consumable,
		Other
	};

	constexpr int NUM_EQUIPMENT_SLOTS = int(ItemCategory::Other) -1;
	constexpr int NUM_ITEM_CATEGORIES = int(ItemCategory::Other)+1;

	enum class CombatStat
	{
		Attack=0,
		Defense,
		Damage,
		Resist
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
		std::vector<EntityId> items;

		int Weight() const;
	};

	// Creature-specific data
	struct CreatureData
	{
		int hp=0;
		int xp=0;
	};

	// Object-specific data
	struct ObjectData
	{
		int state = 0;
		bool blocksMovement = false;
		bool blocksVision = false;

		void Handle(Entity& object, Entity& handler);
	};

	// Item-specific data
	struct ItemData
	{
		int stackSize = 1;
		// owner entity; object or creature
		EntityId owner;

		// is this item currently equipped?
		bool equipped = false;
	};

	struct ItemConfig
	{
		bool defaultStackSize = 0;
		int weight = 1; // in stones
		ItemCategory category = ItemCategory::Other;

		// add this to creature's stats
		glm::ivec4 combatStatBonuses = {0,0,0,0};

		// Set this to > 1 for ranged weapons, like a bow
		int attackRange = 1;
		
		// consumable-specific
		Effect effect; 

		bool IsStackable() const { return defaultStackSize != 0; }
	};

	struct CreatureConfig
	{
		int hp=10;
		int lineOfSightRadius = 10;
		// att/def/dmg/res
		glm::ivec4 combatStats = { 10,5,1,0 };
	};

	struct ObjectConfig
	{
		Effect effect = Effect(-1);
		bool blocksMovement = false;
		bool blocksVision = false;
		bool defaultState = 0;
	};

	// An interface to inherit for classes/structs that should not be copied, but only moved
	struct INonCopyable
	{
		INonCopyable() = default;
		INonCopyable(INonCopyable&&) noexcept = default; // movable
		INonCopyable(const INonCopyable&) = delete; // non construction-copyable    
		INonCopyable& operator=(const INonCopyable&) = delete; // non copyable
	};

	struct EntityConfig : INonCopyable
	{
		EntityConfig() = default;
		EntityConfig(const EntityType zType, const std::vector<TileData>& zTileData, const ItemConfig& zItemCfg = {}, const CreatureConfig& zCreatureCfg = {}, const ObjectConfig& zObjectCfg = {})
			:type(zType), tileData(zTileData), itemCfg(zItemCfg), creatureCfg(zCreatureCfg), objectCfg(zObjectCfg) {}

		EntityType type = EntityType::Creature;
		std::vector<TileData> tileData;
		ItemConfig itemCfg;
		CreatureConfig creatureCfg;
		ObjectConfig objectCfg;
		// set to false for special objects like stairs and doors, that are not randomly generated
		bool allowRandomSpawn = true;
	};

	struct EntityDynamicConfig
	{
		glm::ivec2 position;
		std::string nameOverride;
		EntityId itemOwner;
		std::vector<DbIndex> inventory;
	};

	// Entities
	class Entity : INonCopyable
	{
	public:

		EntityType Type() const { return type; }

		const DbIndex& DbCfg() const { return dbIndex; }
		const EntityId& Id() const { return id; }
		const std::string& Name() { return name; }

		bool BlocksMovement() const;
		bool BlocksVision() const;
		bool CanInteractFromAdjacentTile() const { return BlocksMovement(); }

		void SetLocation(const Location& newLocation) { location = newLocation; }
		const Location& GetLocation() const { return location; }
		Inventory* GetInventory() const { return inventory.get(); }
		CreatureData* GetCreatureData() const { return creatureData.get(); }
		ObjectData* GetObjectData() const { return objectData.get(); }
		ItemData* GetItemData() const { return itemData.get(); }

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
		std::unique_ptr<CreatureData> creatureData;
		std::unique_ptr<ObjectData> objectData;
		std::unique_ptr<ItemData> itemData;
	};
}
