SET(SOURCE_FILES
    imgui_impl_glfw.cpp
    imgui_impl_opengl3.cpp
    framework.cpp
    utility.cpp
	input.cpp
	camera.cpp
	shader.cpp
)

SET(HEADER_FILES
    imgui_impl_glfw.h
    imgui_impl_opengl3.h
    imgui_impl_opengl3_loader.h
    framework.h
    utility.h
	input.h
	camera.h
	shader.h
	array2d.h
)

SET(ALL_SOURCE_FILES
    ${SOURCE_FILES}
    ${HEADER_FILES}
)

assign_source_group(${ALL_SOURCE_FILES})
INCLUDE_DIRECTORIES( ${APP_INCLUDE_DIRECTORIES} )
add_library(framework STATIC ${ALL_SOURCE_FILES})
target_link_libraries(framework PRIVATE ${APP_LINK_LIBRARIES_FW})
