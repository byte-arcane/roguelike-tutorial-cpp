#include "db.h"

#include "tilemap.h"
#include "entity.h"

using namespace glm;

namespace rlf
{
	// helper functions here
	EntityConfig MakeCreature(const std::vector<TileData>& tileData, bool allowRandomSpawn, const CreatureConfig& ccfg)
	{
		EntityConfig cfg;
		cfg.type = EntityType::Creature;
		cfg.tileData = tileData;
		cfg.allowRandomSpawn = allowRandomSpawn;
		cfg.creatureCfg = ccfg;
		return cfg;
	}

	void Db::LoadFromCode()
	{
		Add("player", MakeCreature(
			{ TileData{'@',vec4(1,1,1,1)} }, 
			false, 
			CreatureConfig{ 10 }));

		Add("goblin", MakeCreature(
			{ TileData{'g',vec4(.3,.3,1,1)} },
			true,
			{ 2 }));
	}
}