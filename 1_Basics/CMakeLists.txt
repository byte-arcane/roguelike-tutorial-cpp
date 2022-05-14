SET(SOURCE_FILES
    1_Basics.cpp
)

SET(HEADER_FILES
)

SET(ALL_SOURCE_FILES
    ${SOURCE_FILES}
    ${HEADER_FILES}
)

assign_source_group(${ALL_SOURCE_FILES})

INCLUDE_DIRECTORIES( ${APP_INCLUDE_DIRECTORIES})
add_executable(1_Basics ${ALL_SOURCE_FILES})
target_link_libraries(1_Basics framework)
target_link_libraries(1_Basics debug ${APP_LIBRARIES_DBG})
target_link_libraries(1_Basics optimized ${APP_LIBRARIES})
set_target_properties(1_Basics PROPERTIES OUTPUT_NAME 1_Basics CLEAN_DIRECT_OUTPUT 1)