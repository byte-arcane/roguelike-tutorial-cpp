SET(SOURCE_FILES
    test.cpp
)

SET(HEADER_FILES
)

SET(ALL_SOURCE_FILES
    ${SOURCE_FILES}
    ${HEADER_FILES}
)

assign_source_group(${ALL_SOURCE_FILES})
add_executable(test ${ALL_SOURCE_FILES})
set_target_properties(test PROPERTIES OUTPUT_NAME test CLEAN_DIRECT_OUTPUT 1)
target_link_libraries(test PRIVATE ${APP_LINK_LIBRARIES})
target_include_directories(test PRIVATE framework)