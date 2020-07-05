cmake_minimum_required(VERSION 3.0)

project(imgui_desktop)

set(imgui_USE_OPENGL2 on)
set(imgui_USE_GLBINDING on)
set(imgui_USE_SDL2 on)
add_subdirectory(submodules/imgui)

add_library(imgui_desktop
	"imgui_desktop/include/imgui_desktop/ImGuiHelpers.h"
	"imgui_desktop/src/GLContext.cpp"
	"imgui_desktop/src/GLContext.h"
	"imgui_desktop/src/ScopeGuards.cpp"
	"imgui_desktop/include/imgui_desktop/ScopeGuards.h"
	"imgui_desktop/src/Window.cpp"
	"imgui_desktop/include/imgui_desktop/Window.h"
)

find_package(SDL2 CONFIG REQUIRED)
# find_package(glbinding CONFIG REQUIRED)
target_link_libraries(imgui_desktop SDL2::SDL2)
# target_link_libraries(imgui_desktop glbinding::glbinding)

set_property(TARGET imgui_desktop PROPERTY CXX_STANDARD 17)

target_link_libraries(imgui_desktop imgui)
target_include_directories(imgui_desktop PRIVATE imgui_desktop/include/imgui_desktop)
target_include_directories(imgui_desktop PUBLIC imgui_desktop/include)

# get_cmake_property(_variableNames VARIABLES)
# list (SORT _variableNames)
# foreach (_variableName ${_variableNames})
#     message(STATUS "${_variableName}=${${_variableName}}")
# endforeach()
