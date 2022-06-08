#include "entity.h"
#include "game.h"
#include "graphics.h"
#include "commands.h"
#include "grid.h"
#include "signals.h"

using namespace glm;

namespace rlf
{
	void Entity::Initialize(EntityId id, DbIndex dbIndex, const EntityDynamicConfig& dcfg)
	{
		this->dbIndex = dbIndex;
		this->id = id;
		name = dcfg.nameOverride.empty() ? dbIndex.name : dcfg.nameOverride;
		const auto& cfg = dbIndex.Cfg();
		type = cfg->type;

		// clear everything else
		location = {};
		creatureData.reset();
		objectData.reset();

		// any other work, now that the basics are done

		// Initialization depending on the entity type
		if (type == EntityType::Creature)
		{
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
		
		location = { Game::Instance().GetCurrentLevelIndex(), dcfg.position };

		if (dbIndex == DbIndex::Door())
		{
			objectData->blocksMovement = true;
			objectData->blocksVision = true;
			objectData->state = 0;
		}	
	}

	const TileData& Entity::CurrentTileData() const
	{
		// get the tiledata according to the state
		int state = Type() == EntityType::Object ? objectData->state : 0;
		return DbCfg().Cfg()->tileData[state];
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