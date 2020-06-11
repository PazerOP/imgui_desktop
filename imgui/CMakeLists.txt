project(imgui)

add_library(imgui
	repo/imgui_demo.cpp
	repo/imgui_draw.cpp
	repo/imgui_widgets.cpp
	repo/imgui.cpp
	repo/misc/cpp/imgui_stdlib.cpp
)

target_include_directories(imgui PUBLIC repo)
