#pragma once

#include <string>
#include <array>
#include <vector>
#include <memory>

#include <glm/glm.hpp>
#include <nlohmann/json_fwd.hpp>

#include "tilemap.h"
#include "db.h"
#include "entityid.h"

namespace rlf
{
	class Game;
	class Entity;
	
	// The entity type. For this project, either a creature, object or item.
	enum class EntityType : uint32_t
	{
		Creature,
		Object
	};

	///---------------------------------------------------------------------------
	// Entity "components"

	// Location: where is the creature/object?
	struct Location
	{
		int levelId = -1;
		glm::ivec2 position;
	};

	// Creature-specific data
	struct CreatureData
	{
		// Current hitpoints
		int hp=0;
		// Total xp, gained from kills
		int xp=0;
	};

	// Object-specific data
	struct ObjectData
	{
		// The "state". What this means depends on the actual entity. E.g. for a door, it can be represent 0==closed/1==open
		int state = 0;
		// Does the object currently block movement?
		bool blocksMovement = false;
		// Does the object currently block vision?
		bool blocksVision = false;
	};

	// Configuration data for creature entities
	struct CreatureConfig
	{
		// max hitpoints of creature
		int hp=10;
		// max LoS radius
		int lineOfSightRadius = 10;
	};

	// Configuration data for object entities
	struct ObjectConfig
	{
		// Does it block movement at its initial state?
		bool blocksMovement = false;
		// Does it block vision at its initial state?
		bool blocksVision = false;
		// what's the default state?
		bool defaultState = 0;
	};

	// An interface to inherit for classes/structs that should not be copied, but only moved
	struct INonCopyable
	{
		INonCopyable() = default;
		INonCopyable(INonCopyable&&)  = default; // movable
		INonCopyable(const INonCopyable&) = delete; // non construction-copyable    
		INonCopyable& operator=(const INonCopyable&) = delete; // non copyable
		INonCopyable& operator=(INonCopyable&&) = default;
	};

	// This represents entity configuration data. To keep things simple, this stores optionally-defined creature/item/object configurations in a monolithic way.
	//	There are better ways to do this, which involves more code, with either inheritance or templates and maybe special json parsing to deal with them.
	struct EntityConfig : INonCopyable
	{
		// The entity type
		EntityType type = EntityType::Creature;
		// tile data for rendering
		std::vector<TileData> tileData;
		// set to false for special objects like stairs and doors, that are not randomly generated
		bool allowRandomSpawn = true;
		// configuration for creatures/objects/items. Only define those if necessary, depending on type
		CreatureConfig creatureCfg;
		ObjectConfig objectCfg;
	};

	// Entity-related data required for instantiation
	struct EntityDynamicConfig
	{
		// Where to spawn the entity on the current level?
		glm::ivec2 position{-1,-1};
		// optional override for the entity name, otherwise use the configuration name
		std::string nameOverride;
		// if the entity is an item, who's the owner?
		EntityId itemOwner;
	};

	// The main class to represent creature, object and item instances
	// It's noncopyable, so we can't accidentally copy entity data
	class Entity : INonCopyable
	{
	public:
		// Accessors
		EntityType Type() const  { return type; }
		const DbIndex& DbCfg() const  { return dbIndex; }
		const EntityId& Id() const  { return id; }
		const std::string& Name()  { return name; }
		void SetLocation(const Location& newLocation)  { location = newLocation; }
		const Location& GetLocation() const  { return location; }
		CreatureData* GetCreatureData() const  { return creatureData.get(); }
		ObjectData* GetObjectData() const  { return objectData.get(); }

		// Does this entity block movement?
		bool BlocksMovement() const;
		// Does this entity block vision?
		bool BlocksVision() const;
		// how do we Render this entity given its current state?
		const TileData& CurrentTileData() const;

	private:
		friend class Game;
		// Initialize this entity given an allocated ID, a static configuration and dynamic data
		void Initialize(EntityId id, DbIndex dbIndex, const EntityDynamicConfig& dcfg);

		// friends for easy serialization
		friend void from_json(const nlohmann::json& j, Entity& entity);
		friend void to_json(nlohmann::json& j, const Entity& entity);
		
	private:

		// shared data
		DbIndex dbIndex;

		// identifier for this entity
		EntityId id;

		// Entity data here
		std::string name;
		EntityType type = EntityType(-1);

		// Location (level and position), useful for creatures and objects
		Location location;
		// data specific to different entity types (a creature has creatureData, etc), null if N/A
		std::unique_ptr<CreatureData> creatureData;
		std::unique_ptr<ObjectData> objectData;
	};
}
