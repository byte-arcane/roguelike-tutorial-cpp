SET(SOURCE_FILES
	src/main.cpp
	src/tilemap.cpp
	src/sparsebuffer.cpp
	src/level.cpp
	src/game.cpp
	src/entity.cpp
	src/graphics.cpp
	src/spritemap.cpp
	src/commands.cpp
	src/fov.cpp
	src/entityid.cpp
    src/json.cpp
	src/astar.cpp
	src/grid.cpp
	src/db.cpp
	src/signals.cpp
	src/state/maingame.cpp
	src/state/state.cpp
)

SET(HEADER_FILES
	src/tilemap.h
	src/sparsebuffer.h
	src/level.h
	src/game.h
	src/entity.h
	src/graphics.h
	src/spritemap.h
	src/commands.h
	src/db.h
	src/fov.h
	src/entityid.h
    src/json.h
	src/astar.h
	src/grid.h
	src/signals.h
	src/state/maingame.h
	src/state/state.h
)

SET(ALL_SOURCE_FILES
    ${SOURCE_FILES}
    ${HEADER_FILES}
)

assign_source_group(${ALL_SOURCE_FILES})
add_executable(05_Object_entities ${ALL_SOURCE_FILES})
set_target_properties(05_Object_entities PROPERTIES OUTPUT_NAME 05_Object_entities CLEAN_DIRECT_OUTPUT 1)
target_link_libraries(05_Object_entities PRIVATE ${APP_LINK_LIBRARIES})
target_include_directories(05_Object_entities PRIVATE ../framework)