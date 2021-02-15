cmake_minimum_required(VERSION 3.0)

project(imgui_desktop)

set(imgui_USE_OPENGL2 on)
set(imgui_USE_OPENGL3 on)
# set(imgui_USE_GLBINDING on)
set(imgui_USE_GLAD2 on)
set(imgui_USE_SDL2 on)
add_subdirectory(imgui_cmake)

add_library(imgui_desktop STATIC
	"imgui_desktop/include/imgui_desktop/Application.h"
	"imgui_desktop/src/Application.cpp"
	"imgui_desktop/include/imgui_desktop/ImGuiHelpers.h"
	"imgui_desktop/src/GLContext.cpp"
	"imgui_desktop/src/GLContext.h"
	"imgui_desktop/include/imgui_desktop/GLContextVersion.h"
	"imgui_desktop/src/ScopeGuards.cpp"
	"imgui_desktop/include/imgui_desktop/ScopeGuards.h"
	"imgui_desktop/include/imgui_desktop/StorageHelper.h"
	"imgui_desktop/src/Window.cpp"
	"imgui_desktop/include/imgui_desktop/Window.h"
	"imgui_desktop/src/ImGuiDesktopInternal.h"
)

find_package(SDL2 CONFIG REQUIRED)
find_package(glad2 REQUIRED)
find_package(fmt CONFIG REQUIRED)
target_link_libraries(imgui_desktop
	PUBLIC
		imgui
		glad2::glad2
	PRIVATE
		mh::stuff
		SDL2::SDL2
		fmt::fmt
)

target_compile_features(imgui_desktop PUBLIC "cxx_std_20")

target_include_directories(imgui_desktop PRIVATE imgui_desktop/include/imgui_desktop)
target_include_directories(imgui_desktop PUBLIC imgui_desktop/include)

# get_cmake_property(_variableNames VARIABLES)
# list (SORT _variableNames)
# foreach (_variableName ${_variableNames})
#     message(STATUS "${_variableName}=${${_variableName}}")
# endforeach()
