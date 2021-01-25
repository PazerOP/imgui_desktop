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

ID::ID(const std::string_view& sv) : ID(sv.data(), sv.data() + sv.size())
{
}

ID::~ID()
{
	ImGui::PopID();
}

StyleColor::StyleColor(ImGuiCol_ color, const ImVec4& value, bool enabled) :
	m_Enabled(enabled)
{
	if (m_Enabled)
		ImGui::PushStyleColor(color, value);
}

StyleColor::~StyleColor()
{
	if (m_Enabled)
		ImGui::PopStyleColor();
}

StyleColor::StyleColor(StyleColor&& other) :
	m_Enabled(other.m_Enabled)
{
	other.m_Enabled = false;
}
StyleColor& StyleColor::operator=(StyleColor&& other)
{
	if (m_Enabled)
		ImGui::PopStyleColor();

	m_Enabled = other.m_Enabled;
	other.m_Enabled = false;
	return *this;
}

TextColor::TextColor(const ImVec4& color, bool enabled) :
	StyleColor(ImGuiCol_Text, color, enabled)
{
}

GlobalAlpha::GlobalAlpha(float newAlpha) :
	m_OldAlpha(ImGui::GetStyle().Alpha)
{
	auto& style = ImGui::GetStyle();
	m_OldAlpha = style.Alpha;
	style.Alpha = newAlpha;
}

GlobalAlpha::~GlobalAlpha()
{
	ImGui::GetStyle().Alpha = m_OldAlpha;
}

Indent::Indent(unsigned count) :
	m_Count(count)
{
	for (unsigned i = 0; i < count; i++)
		ImGui::Indent();
}
Indent::~Indent()
{
	for (unsigned i = 0; i < m_Count; i++)
		ImGui::Unindent();
}

Context::Context(ImGuiContext* newContext) :
	m_OldContext(ImGui::GetCurrentContext()),
	m_NewContext(newContext)
{
	ImGui::SetCurrentContext(m_NewContext);
}
Context::~Context()
{
	assert(ImGui::GetCurrentContext() == m_NewContext);
	ImGui::SetCurrentContext(m_OldContext);
}
