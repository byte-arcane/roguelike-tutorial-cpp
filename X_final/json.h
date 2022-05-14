#pragma once

#include <nlohmann/json.hpp>
#include "array2d.h"
#include "db.h"
#include "entityid.h"
#include "level.h"
#include "tilemap.h"

using namespace nlohmann;

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

    // todo: glm vec4

	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DbIndex, name);
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(EntityId, version, id);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TileData, spriteIndex, color);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(EntityConfig, type, tileData);
	//NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Level, bg, fogOfWar, entities);
}