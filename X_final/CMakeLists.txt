SET(SOURCE_FILES
    main.cpp
	tilemap.cpp
	sparsebuffer.cpp
	level.cpp
	game.cpp
	entity.cpp
	graphics.cpp
	spritemap.cpp
	commands.cpp
	fov.cpp
	entityid.cpp
    json.cpp
	astar.cpp
	grid.cpp
	turn.cpp
	effect.cpp
	dungen.cpp
	db.cpp
	signals.cpp
	state/inventory.cpp
	state/maingame.cpp
	state/menu.cpp
	state/selecttarget.cpp
	state/state.cpp
	state/death.cpp
	state/createchar.cpp
)

SET(HEADER_FILES
	tilemap.h
	sparsebuffer.h
	level.h
	game.h
	entity.h
	graphics.h
	spritemap.h
	commands.h
	db.h
	fov.h
	entityid.h
    json.h
	astar.h
	grid.h
	turn.h
	effect.h
	dungen.h
	signals.h
	state/inventory.h
	state/maingame.h
	state/menu.h
	state/selecttarget.h
	state/state.h
	state/death.h 
	state/createchar.h
)

SET(ALL_SOURCE_FILES
    ${SOURCE_FILES}
    ${HEADER_FILES}
)

assign_source_group(${ALL_SOURCE_FILES})
add_executable(X_final ${ALL_SOURCE_FILES})
set_target_properties(X_final PROPERTIES OUTPUT_NAME X_final CLEAN_DIRECT_OUTPUT 1)
target_link_libraries(X_final PRIVATE ${APP_LINK_LIBRARIES})
target_include_directories(X_final PRIVATE ../framework)