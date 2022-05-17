#pragma once

#include <nlohmann/json.hpp>
#include <magic_enum.hpp>
#include "array2d.h"
#include "db.h"
#include "entityid.h"
#include "level.h"
#include "tilemap.h"

// Our own little addition to support reading json data and ignoring missing values (treating all variables as optional)
#define NLOHMANN_JSON_FROM_OPT(v1) if(nlohmann_json_j.find(#v1) != nlohmann_json_j.end()) nlohmann_json_j.at(#v1).get_to(nlohmann_json_t.v1);
#define NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_OPT(Type, ...)  \
    inline void to_json(nlohmann::json& nlohmann_json_j, const Type& nlohmann_json_t) { NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_TO, __VA_ARGS__)) } \
    inline void from_json(const nlohmann::json& nlohmann_json_j, Type& nlohmann_json_t) { NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM_OPT, __VA_ARGS__)) }

using namespace nlohmann;

namespace glm
{
    //void from_json(const nlohmann::json& j, ivec2& v);
    //void to_json(nlohmann::json& j, const ivec2& v);
    void from_json(const nlohmann::json& j, vec4& v);
    void to_json(nlohmann::json& j, const vec4& v);
    void from_json(const nlohmann::json& j, ivec4& v);
    void to_json(nlohmann::json& j, const ivec4& v);
}

namespace rlf
{
    template <typename T>
    struct adl_serializer {
        static void to_json(json& j, const Array2D<T>& value) {
            // calls the "to_json" method in T's namespace
        }

        static void from_json(const json& j, Array2D<T>& value) {
            // same thing, but with the "from_json" method
        }
    };

    void from_json(const nlohmann::json& j, TileData& td);
    void to_json(nlohmann::json& j, const TileData& td);

    void from_json(const nlohmann::json& j, Effect& e) { e = magic_enum::enum_cast<Effect>(std::string(j)).value(); }
    void to_json(nlohmann::json& j, const Effect& e) { j = magic_enum::enum_name(e); }

    void from_json(const nlohmann::json& j, ItemCategory& e) { e = magic_enum::enum_cast<ItemCategory>(std::string(j)).value(); }
    void to_json(nlohmann::json& j, const ItemCategory& e) { j = magic_enum::enum_name(e); }

    void from_json(const nlohmann::json& j, EntityType& e) { e = magic_enum::enum_cast<EntityType>(std::string(j)).value(); }
    void to_json(nlohmann::json& j, const EntityType& e) { j = magic_enum::enum_name(e); }

	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DbIndex, name);
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(EntityId, version, id);
    //NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TileData, spriteIndex, color);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_OPT(ItemConfig, defaultStackSize, weight, category, combatStatBonuses, effect, attackRange);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_OPT(CreatureConfig, lineOfSightRadius, hp, combatStats);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_OPT(ObjectConfig, effect, blocksMovement, blocksVision, defaultState);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_OPT(EntityConfig, type, tileData, itemCfg, creatureCfg, objectCfg, allowRandomSpawn);
	//NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Level, bg, fogOfWar, entities);
}