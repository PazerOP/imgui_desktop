cmake_minimum_required(VERSION 3.0)

project(mh-imgui-desktop
	VERSION 1.0
	DESCRIPTION "Desktop application framework using imgui"
	HOMEPAGE_URL "https://github.com/PazerOP/imgui_desktop"
	LANGUAGES CXX
)

set(imgui_USE_OPENGL2 on)
set(imgui_USE_OPENGL3 on)
set(imgui_USE_SDL2 on)

file(GLOB_RECURSE
	IMGUI_DESKTOP_FILES
	CONFIGURE_DEPENDS
	LIST_DIRECTORIES false
	imgui_desktop/*.h imgui_desktop/*.cpp
)
add_library(${PROJECT_NAME} STATIC "${IMGUI_DESKTOP_FILES}")

find_package(fmt CONFIG REQUIRED)
find_package(mh-imgui CONFIG REQUIRED)
find_package(mh-stuff CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME}
	PUBLIC
		mh::mh-imgui
	PRIVATE
		mh::mh-stuff
		fmt::fmt
)

target_compile_features(${PROJECT_NAME} PUBLIC "cxx_std_20")

if (imgui_USE_SDL2)
	find_package(SDL2 CONFIG REQUIRED)
	target_compile_definitions(${PROJECT_NAME} PUBLIC "IMGUI_USE_SDL2")
	target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2)
endif()

if (imgui_USE_OPENGL2 OR imgui_USE_OPENGL3)
	find_package(mh-glad2-gl CONFIG REQUIRED)
	target_link_libraries(${PROJECT_NAME} PRIVATE mh::mh-glad2-gl)
	target_compile_definitions(${PROJECT_NAME} PUBLIC "IMGUI_USE_GLAD2")

	if (imgui_USE_OPENGL2)
		target_compile_definitions(${PROJECT_NAME} PUBLIC "IMGUI_USE_OPENGL2")
	endif()
	if (imgui_USE_OPENGL3)
		target_compile_definitions(${PROJECT_NAME} PUBLIC "IMGUI_USE_OPENGL3")
	endif()
endif()

target_include_directories(${PROJECT_NAME} PRIVATE imgui_desktop/include/imgui_desktop)
target_include_directories(${PROJECT_NAME}
	PUBLIC
		"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/imgui_desktop/include/>"
		"$<INSTALL_INTERFACE:include/>"
)

find_package(mh-cmake-common CONFIG REQUIRED)
mh_basic_install(
	PROJ_INCLUDE_DIRS
		"imgui_desktop/include/"
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
