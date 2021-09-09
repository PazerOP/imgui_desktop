cmake_minimum_required(VERSION 3.0)

project(imgui-desktop
	VERSION 1.0
	DESCRIPTION "Desktop application framework using imgui"
	HOMEPAGE_URL "https://github.com/PazerOP/imgui_desktop"
	LANGUAGES CXX
)

set(imgui_USE_OPENGL2 on)
set(imgui_USE_OPENGL3 on)
# set(imgui_USE_GLBINDING on)
set(imgui_USE_SDL2 on)

add_library(${PROJECT_NAME}
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
add_library(mh::imgui-desktop ALIAS ${PROJECT_NAME})

find_package(fmt CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(mh-stuff CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME}
	PUBLIC
		imgui::imgui
	PRIVATE
		mh::mh-stuff
		fmt::fmt
)

target_compile_features(${PROJECT_NAME} PUBLIC "cxx_std_20")

if (imgui_USE_SDL2)
	find_package(SDL2 CONFIG REQUIRED)
	target_compile_definitions(${PROJECT_NAME} PRIVATE "IMGUI_USE_SDL2")
	target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2)
endif()

if (imgui_USE_OPENGL2 OR imgui_USE_OPENGL3)
	find_package(glad2-gl CONFIG REQUIRED)
	target_link_libraries(${PROJECT_NAME} PRIVATE mh::glad2-gl)

	if (imgui_USE_OPENGL2)
		target_compile_definitions(${PROJECT_NAME} PRIVATE "IMGUI_USE_OPENGL2")
	endif()
	if (imgui_USE_OPENGL3)
		target_compile_definitions(${PROJECT_NAME} PRIVATE "IMGUI_USE_OPENGL3")
	endif()
endif()

target_include_directories(${PROJECT_NAME} PRIVATE imgui_desktop/include/imgui_desktop)
target_include_directories(${PROJECT_NAME} PUBLIC "$<INSTALL_INTERFACE:imgui_desktop/include>")

find_package(mh-cmake-common CONFIG REQUIRED)
mh_basic_install(
	PROJ_INCLUDE_DIRS
		"imgui_desktop/include"
)

###########################################
# "install" is intended for vcpkg support #
###########################################
# include(CMakePackageConfigHelpers)
# configure_package_config_file(
# 	cmake/config.cmake.in
# 	"${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
# 	INSTALL_DESTINATION "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}"
# )

# include(GNUInstallDirs)

# write_basic_package_version_file(
# 	"${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
# 	VERSION ${PROJECT_VERSION}
# 	COMPATIBILITY SameMajorVersion
# )

# install(
# 	FILES
# 		"${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
# 		"${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
# 	DESTINATION
# 		"${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}"
# )

# install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}_targets)
# install(DIRECTORY imgui_desktop/include/ DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
# install(
# 	EXPORT ${PROJECT_NAME}_targets
# 	NAMESPACE mh::
# 	DESTINATION "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}"
# )
