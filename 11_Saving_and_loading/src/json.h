#pragma once

#include <nlohmann/json.hpp>
#include <magic_enum.hpp>
#include "array2d.h"
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
    void from_json(const nlohmann::json& j, ivec2& v);
    void to_json(nlohmann::json& j, const ivec2& v);
    void from_json(const nlohmann::json& j, vec4& v);
    void to_json(nlohmann::json& j, const vec4& v);
    void from_json(const nlohmann::json& j, ivec4& v);
    void to_json(nlohmann::json& j, const ivec4& v);
}

namespace nlohmann {

    // support unique pointers
    template <typename T>
    struct adl_serializer<std::unique_ptr<T>> {
        static void to_json(json& j, const std::unique_ptr<T>& opt) {
            if (opt.get()) {
                j = *opt;
            }
            else {
                j = nullptr;
            }
        }
        
        static void from_json(const json& j, std::unique_ptr<T>& opt) {
            if (!j.is_null())
            {
                opt.reset(new T(j));
            }
        }
    };

    template <typename T>
    struct adl_serializer<rlf::Array2D<T>> {
        static void to_json(json& j, const rlf::Array2D<T>& value) {
            j["size"] = value.Size();
            j["data"] = value.Data();
        }

        static void from_json(const json& j, rlf::Array2D<T>& value) {
            glm::ivec2 size = j.at("size");
            std::vector<T> data = j.at("data");
            value = rlf::Array2D<T>(size, data);
        }
    };
}

using namespace nlohmann;

namespace rlf
{
    void from_json(const nlohmann::json& j, TileData& td);
    void to_json(nlohmann::json& j, const TileData& td);

    void from_json(const nlohmann::json& j, Effect& e) { e = magic_enum::enum_cast<Effect>(std::string(j)).value(); }
    void to_json(nlohmann::json& j, const Effect& e) { j = magic_enum::enum_name(e); }

    void from_json(const nlohmann::json& j, ItemCategory& e) { e = magic_enum::enum_cast<ItemCategory>(std::string(j)).value(); }
    void to_json(nlohmann::json& j, const ItemCategory& e) { j = magic_enum::enum_name(e); }

    void from_json(const nlohmann::json& j, EntityType& e) { e = magic_enum::enum_cast<EntityType>(std::string(j)).value(); }
    void to_json(nlohmann::json& j, const EntityType& e) { j = magic_enum::enum_name(e); }

    void from_json(const nlohmann::json& j, FogOfWarStatus& e) { e = magic_enum::enum_cast<FogOfWarStatus>(std::string(j)).value(); }
    void to_json(nlohmann::json& j, const FogOfWarStatus& e) { j = magic_enum::enum_name(e); }

	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DbIndex, name);
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(EntityId, version, id);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_OPT(ItemConfig, defaultStackSize, weight, category, combatStatBonuses, effect, attackRange);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_OPT(CreatureConfig, lineOfSightRadius, hp, combatStats);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_OPT(ObjectConfig, effect, blocksMovement, blocksVision, defaultState);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_OPT(EntityConfig, type, tileData, itemCfg, creatureCfg, objectCfg, allowRandomSpawn);

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(LevelBgElement, name, blocksVision, blocksMovement,isLiquid, glyph,color);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Inventory, items);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CreatureData, hp,xp);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ObjectData, state, blocksMovement, blocksVision);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ItemData, stackSize, owner, equipped);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Location, levelId, position);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Level, bg, entities, fogOfWar);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Entity, dbIndex, id, name, inventory, location, type, itemData, creatureData, objectData);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(SaveData, poolEntities, invalidPoolIndices, playerId, levels, currentLevelIndex, messageLog);
}