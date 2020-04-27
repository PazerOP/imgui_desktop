#include "ScopeGuards.h"

#include <imgui.h>

using namespace ImGuiDesktop::ScopeGuards;

ID::ID(int int_id)
{
	ImGui::PushID(int_id);
}

ID::ID(const void* ptr_id)
{
	ImGui::PushID(ptr_id);
}

ID::ID(const char* str_id_begin, const char* str_id_end)
{
	ImGui::PushID(str_id_begin, str_id_end);
}

ID::~ID()
{
	ImGui::PopID();
}

StyleColor::StyleColor(ImGuiCol_ color, const ImVec4& value)
{
	ImGui::PushStyleColor(color, value);
}

StyleColor::~StyleColor()
{
	ImGui::PopStyleColor();
}
