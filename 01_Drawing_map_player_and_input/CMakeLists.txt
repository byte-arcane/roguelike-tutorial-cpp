SET(SOURCE_FILES
	src/main.cpp
	src/tilemap.cpp
	src/sparsebuffer.cpp
	src/level.cpp
	src/game.cpp
	src/graphics.cpp
	src/spritemap.cpp
	src/grid.cpp
	src/signals.cpp
	src/state/maingame.cpp
)

SET(HEADER_FILES
	src/tilemap.h
	src/sparsebuffer.h
	src/level.h
	src/game.h
	src/graphics.h
	src/spritemap.h
	src/grid.h
	src/signals.h
	src/state/maingame.h
)

SET(ALL_SOURCE_FILES
    ${SOURCE_FILES}
    ${HEADER_FILES}
)

assign_source_group(${ALL_SOURCE_FILES})
add_executable(01_Drawing_map_player_and_input ${ALL_SOURCE_FILES})
set_target_properties(01_Drawing_map_player_and_input PROPERTIES OUTPUT_NAME 01_Drawing_map_player_and_input CLEAN_DIRECT_OUTPUT 1)
target_link_libraries(01_Drawing_map_player_and_input PRIVATE ${APP_LINK_LIBRARIES})
target_include_directories(01_Drawing_map_player_and_input PRIVATE ../framework)