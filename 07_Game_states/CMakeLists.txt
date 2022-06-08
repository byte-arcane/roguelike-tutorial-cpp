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
	src/dungen.cpp
	src/db.cpp
	src/signals.cpp
	src/state/inventory.cpp
	src/state/maingame.cpp
	src/state/menu.cpp
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
	src/dungen.h
	src/signals.h
	src/state/inventory.h
	src/state/maingame.h
	src/state/menu.h
	src/state/state.h
)

SET(ALL_SOURCE_FILES
    ${SOURCE_FILES}
    ${HEADER_FILES}
)

assign_source_group(${ALL_SOURCE_FILES})
add_executable(07_Game_states ${ALL_SOURCE_FILES})
set_target_properties(07_Game_states PROPERTIES OUTPUT_NAME 07_Game_states CLEAN_DIRECT_OUTPUT 1)
target_link_libraries(07_Game_states PRIVATE ${APP_LINK_LIBRARIES})
target_include_directories(07_Game_states PRIVATE ../framework)