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

		// any other work, now that the basics are done

		// Initialization depending on the entity type
		if (type == EntityType::Creature)
		{
			creatureData = std::make_unique<CreatureData>();
			creatureData->hp = cfg->creatureCfg.hp;
		}
		
		location = { dcfg.position };	
	}

	const TileData& Entity::CurrentTileData() const
	{
		return DbCfg().Cfg()->tileData[0];
	}

	bool Entity::BlocksMovement() const
	{
		switch (Type())
		{
			// Creatures always block movement
		case EntityType::Creature:
			return true;
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
		default:
			assert(false); // don't be here
			break;
		}
		return false;
	}
}