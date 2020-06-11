project(imgui)

add_library(imgui
	repo/imconfig.h
	repo/imgui_demo.cpp
	repo/imgui_draw.cpp
	repo/imgui_internal.h
	repo/imgui_widgets.cpp
	repo/imgui.cpp
	repo/imgui.h
	repo/imstb_rectpack.h
	repo/imstb_textedit.h
	repo/imstb_truetype.h
	repo/misc/cpp/imgui_stdlib.cpp
	repo/misc/cpp/imgui_stdlib.h
)

target_include_directories(imgui PUBLIC repo)
