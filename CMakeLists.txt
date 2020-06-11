project(imgui_desktop)

add_subdirectory(imgui)

add_library(imgui_desktop
	imgui_desktop/src/GLContext.cpp
	imgui_desktop/src/ScopeGuards.cpp
	imgui_desktop/src/Window.cpp
	imgui/repo/examples/imgui_impl_opengl3.cpp
	imgui/repo/examples/imgui_impl_sdl.cpp
)

set_property(TARGET imgui_desktop PROPERTY CXX_STANDARD 17)


# get_cmake_property(_variableNames VARIABLES)
# list (SORT _variableNames)
# foreach (_variableName ${_variableNames})
#     message(STATUS "${_variableName}=${${_variableName}}")
# endforeach()

find_package(SDL2 CONFIG REQUIRED)
find_path(SDL2_INCLUDE_DIR SDL2/SDL.h)
target_include_directories(imgui_desktop PRIVATE ${SDL2_INCLUDE_DIR}/SDL2)
find_library(SDL2_LIBRARY_DIR SDL2)
target_link_libraries(imgui_desktop ${SDL2_LIBRARY_DIR})

find_package(glbinding CONFIG REQUIRED)
find_path(GLBINDING_INCLUDE_DIR glbinding/glbinding.h)
target_include_directories(imgui_desktop PRIVATE ${GLBINDING_INCLUDE_DIR})
find_library(GLBINDING_LIBRARY_DIR glbinding)
target_link_libraries(imgui_desktop ${GLBINDING_LIBRARY_DIR})

# message("SDL2_VERSION: " "${SDL2_VERSION}")

target_link_libraries(imgui_desktop imgui)
target_include_directories(imgui_desktop PRIVATE imgui_desktop/include/imgui_desktop)
target_include_directories(imgui_desktop PUBLIC imgui_desktop/include)
