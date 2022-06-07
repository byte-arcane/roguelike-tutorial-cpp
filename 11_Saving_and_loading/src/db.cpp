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

	EntityConfig MakeObject(const std::vector<TileData>& tileData, bool allowRandomSpawn, const ObjectConfig& ocfg)
	{
		EntityConfig cfg;
		cfg.type = EntityType::Object;
		cfg.tileData = tileData;
		cfg.allowRandomSpawn = allowRandomSpawn;
		cfg.objectCfg = ocfg;
		return cfg;
	}

	EntityConfig MakeItem(const std::vector<TileData>& tileData, bool allowRandomSpawn, const ItemConfig& icfg)
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
		Add("player", MakeCreature(
			{ TileData{'@',vec4(1,1,1,1)} }, 
			false, 
			CreatureConfig{ 10 }));

		Add("door", MakeObject(
			{ TileData{'+',vec4(1,.545,0,1)}, TileData{'/',vec4(1,.545,0,1)} },
			false,
			{}));
		Add("stairs_up", MakeObject(
			{ TileData{'<',vec4(1,1,1,1)}},
			false,
			{}));
		Add("stairs_down", MakeObject(
			{ TileData{'>',vec4(1,1,1,1)} },
			false,
			{}));
		Add("item_pile", MakeObject(
			{ TileData{';',vec4(.7,.7,.7,1)} },
			false,
			{}));
		Add("gold", MakeItem(
			{ TileData{'$',vec4(1,1,.5,1)} },
			true,
			{}));
		Add("hat", MakeItem(
			{ TileData{'[',vec4(.7,.7,.7,1)} },
			true,
			{0,1,ItemCategory::Armor}));
		Add("helmet", MakeItem(
			{ TileData{'[',vec4(.7,.7,.7,1)} },
			true,
			{ 0,5,ItemCategory::Armor }));
		Add("sword", MakeItem(
			{ TileData{'(',vec4(.7,.7,.7,1)} },
			true,
			{ 0,3,ItemCategory::Weapon }));
		Add("goblin", MakeCreature(
			{ TileData{'g',vec4(.3,.3,1,1)} },
			true,
			{ 2 }));
	}
}