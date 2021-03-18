#pragma once

#include <mh/types/disable_copy_move.hpp>

#include <string_view>

enum ImGuiCol_ : int;
struct ImGuiContext;
struct ImVec4;

namespace ImGuiDesktop
{
	inline namespace ScopeGuards
	{
		struct ID final : mh::disable_copy_move
		{
		public:
			explicit ID(int int_id);
			explicit ID(int64_t int64_id);
			explicit ID(uint64_t int64_id);
			explicit ID(const char* str_id_begin, const char* str_id_end = nullptr);
			explicit ID(const std::string_view& sv);
			explicit ID(const void* ptr_id);

			~ID();

		private:
			uint8_t m_PopCount = 1;
		};

		struct StyleColor : mh::disable_copy
		{
			StyleColor() : m_Enabled(false) {}
			StyleColor(ImGuiCol_ color, const ImVec4& value, bool enabled = true);
			~StyleColor();

			StyleColor(StyleColor&& other);
			StyleColor& operator=(StyleColor&& other);

		private:
			bool m_Enabled = true;
		};

		struct TextColor final : StyleColor
		{
			TextColor(const ImVec4& color, bool enabled = true);
		};

		struct GlobalAlpha final : mh::disable_copy_move
		{
			explicit GlobalAlpha(float newAlpha);
			~GlobalAlpha();

		private:
			float m_OldAlpha;
		};

		struct Indent final : mh::disable_copy_move
		{
			explicit Indent(unsigned count = 1);
			~Indent();

		private:
			unsigned m_Count;
		};

		struct Context final : mh::disable_copy_move
		{
			explicit Context(ImGuiContext* newContext);
			~Context();

		private:
			ImGuiContext* m_OldContext;
			ImGuiContext* m_NewContext;
		};
	}
}
