project(imgui)

FetchContent_Declare(imgui GIT_REPOSITORY https://github.com/PazerOP/imgui.git)
if (NOT imgui_POPULATED)
	FetchContent_MakeAvailable(imgui)
endif()

add_library(imgui
	"${imgui_SOURCE_DIR}/imconfig.h"
	"${imgui_SOURCE_DIR}/imgui_demo.cpp"
	"${imgui_SOURCE_DIR}/imgui_draw.cpp"
	"${imgui_SOURCE_DIR}/imgui_internal.h"
	"${imgui_SOURCE_DIR}/imgui_widgets.cpp"
	"${imgui_SOURCE_DIR}/imgui.cpp"
	"${imgui_SOURCE_DIR}/imgui.h"
	"${imgui_SOURCE_DIR}/imstb_rectpack.h"
	"${imgui_SOURCE_DIR}/imstb_textedit.h"
	"${imgui_SOURCE_DIR}/imstb_truetype.h"
	"${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp"
	"${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.h"
)

target_include_directories(imgui PUBLIC
	"${imgui_SOURCE_DIR}"
	"${imgui_SOURCE_DIR}/misc/cpp"
)
