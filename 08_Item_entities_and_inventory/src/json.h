#pragma once

#include <nlohmann/json.hpp>
#include <magic_enum.hpp>
#include "db.h"
#include "entityid.h"
#include "entity.h"
#include "level.h"
#include "tilemap.h"
#include "game.h"

// Our own little addition to support reading json data and ignoring missing values (treating all variables as optional)
#define NLOHMANN_JSON_FROM_OPT(v1) if(nlohmann_json_j.find(#v1) != nlohmann_json_j.end()) nlohmann_json_j.at(#v1).get_to(nlohmann_json_t.v1);
#define NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_OPT(Type, ...)  \
    inline void to_json(nlohmann::json& nlohmann_json_j, const Type& nlohmann_json_t) { NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_TO, __VA_ARGS__)) } \
    inline void from_json(const nlohmann::json& nlohmann_json_j, Type& nlohmann_json_t) { NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM_OPT, __VA_ARGS__)) }


namespace glm
{
    void from_json(const nlohmann::json& j, ivec4& v);
    void to_json(nlohmann::json& j, const ivec4& v);
}

using namespace nlohmann;

namespace rlf
{
    void from_json(const nlohmann::json& j, TileData& td);
    void to_json(nlohmann::json& j, const TileData& td);

    void from_json(const nlohmann::json& j, EntityType& e) { e = magic_enum::enum_cast<EntityType>(std::string(j)).value(); }
    void to_json(nlohmann::json& j, const EntityType& e) { j = magic_enum::enum_name(e); }

	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DbIndex, name);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_OPT(ItemConfig, defaultStackSize, weight);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_OPT(CreatureConfig, lineOfSightRadius, hp);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_OPT(ObjectConfig, blocksMovement, blocksVision, defaultState);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_OPT(EntityConfig, type, tileData, itemCfg, creatureCfg, objectCfg, allowRandomSpawn);
}