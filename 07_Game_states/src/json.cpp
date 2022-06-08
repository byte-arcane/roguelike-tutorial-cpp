#include "json.h"

#include <nlohmann/json.hpp>

#include "utility.h"
#include "signals.h"

namespace glm
{
	void from_json(const nlohmann::json& j, ivec2& v)
	{
		v.x = j[0]; v.y = j[1];
	}
	void to_json(nlohmann::json& j, const ivec2& v)
	{
		j[0] = v.x; j[1] = v.y;
	}

	void from_json(const nlohmann::json& j, vec4& v) 
	{ 
		v.x = j[0]; v.y = j[1]; v.z = j[2]; v.w = j[3]; 
	}
	void to_json(nlohmann::json& j, const vec4& v)
	{
		j[0] = v.x; j[1] = v.y; j[2] = v.z; j[3] = v.w;
	}

	void from_json(const nlohmann::json& j, ivec4& v)
	{
		v.x = j[0]; v.y = j[1]; v.z = j[2]; v.w = j[3];
	}
	void to_json(nlohmann::json& j, const ivec4& v)
	{
		j[0] = v.x; j[1] = v.y; j[2] = v.z; j[3] = v.w;
	}
}


namespace rlf
{
	void from_json(const nlohmann::json& j, TileData& td)
	{
		std::string sprite;
		j.at("sprite").get_to(sprite);
		td.spriteIndex = sprite[0]; // single char
		j.at("color").get_to(td.color);
	}

	void to_json(nlohmann::json& j, const TileData& td)
	{
		char c = char(td.spriteIndex);
		j = json{ {"sprite", std::string(1,c)}, {"color", td.color} };
	}

	void Db::LoadFromDisk()
	{
		auto filename = MediaSearch("json/db_no_effects_stats.json");
		auto text = ReadTextFile(filename);
		auto j = json::parse(text);
		db = j;
	}

	void Game::New()
	{
		currentLevelIndex = -1;
		invalidPoolIndices.clear();
		levels.clear();
		messageLog.clear();
		playerId = {};
		poolEntities.clear();
	}
}