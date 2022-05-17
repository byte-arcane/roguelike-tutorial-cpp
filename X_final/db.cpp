#include "db.h"

#include "tilemap.h"
#include "entity.h"

using namespace glm;

namespace rlf
{
	// TODO: helper func here
	EntityConfig makeCreature(const std::vector<TileData>& tileData, bool allowRandomSpawn, const CreatureConfig& ccfg)
	{
		EntityConfig cfg;
		cfg.type = EntityType::Creature;
		cfg.tileData = tileData;
		cfg.allowRandomSpawn = allowRandomSpawn;
		cfg.creatureCfg = ccfg;
		return cfg;
	}

	EntityConfig makeObject(const std::vector<TileData>& tileData, bool allowRandomSpawn, const ObjectConfig& ocfg)
	{
		EntityConfig cfg;
		cfg.type = EntityType::Object;
		cfg.tileData = tileData;
		cfg.allowRandomSpawn = allowRandomSpawn;
		cfg.objectCfg = ocfg;
		return cfg;
	}

	EntityConfig makeItem(const std::vector<TileData>& tileData, bool allowRandomSpawn, const ItemConfig& icfg)
	{
		EntityConfig cfg;
		cfg.type = EntityType::Item;
		cfg.tileData = tileData;
		cfg.allowRandomSpawn = allowRandomSpawn;
		cfg.itemCfg = icfg;
		return cfg;
	}

	void Db::LoadFromCode()
	{
		Add("player", makeCreature(
			{ TileData{'@',vec4(1,1,1,1)} }, 
			false, 
			CreatureConfig{ 10 }));

		Add("door", makeObject(
			{ TileData{'+',vec4(1,.545,0,1)}, TileData{'/',vec4(1,.545,0,1)} },
			false,
			{}));
		Add("stairs_up", makeObject(
			{ TileData{'<',vec4(1,1,1,1)}},
			false,
			{}));
		Add("stairs_down", makeObject(
			{ TileData{'>',vec4(1,1,1,1)} },
			false,
			{}));
		Add("item_pile", makeObject(
			{ TileData{';',vec4(.7,.7,.7,1)} },
			false,
			{}));
		Add("gold", makeItem(
			{ TileData{'$',vec4(1,1,.5,1)} },
			true,
			{}));
		Add("hat", makeItem(
			{ TileData{'[',vec4(.7,.7,.7,1)} },
			true,
			{0,1,ItemCategory::Armor}));
		Add("helmet", makeItem(
			{ TileData{'[',vec4(.7,.7,.7,1)} },
			true,
			{ 0,5,ItemCategory::Armor }));
		Add("sword", makeItem(
			{ TileData{'(',vec4(.7,.7,.7,1)} },
			true,
			{ 0,3,ItemCategory::Weapon }));
		Add("goblin", makeCreature(
			{ TileData{'g',vec4(.3,.3,1,1)} },
			true,
			{ 2 }));
	}
}