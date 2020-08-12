#pragma once

#include <string_view>

enum ImGuiCol_ : int;
struct ImVec4;

namespace ImGuiDesktop::ScopeGuards
{
	namespace detail
	{
		struct DisableCopy
		{
			DisableCopy() = default;
			DisableCopy(const DisableCopy&) = delete;
			DisableCopy& operator=(const DisableCopy&) = delete;
		};

		struct DisableMove
		{
			DisableMove() = default;
			DisableMove(DisableMove&&) = delete;
			DisableMove& operator=(DisableMove&&) = delete;
		};

		struct DisableCopyMove : DisableCopy, DisableMove {};
	}

	struct ID final : detail::DisableCopyMove
	{
	public:
		explicit ID(int int_id);
		explicit ID(const void* ptr_id);
		explicit ID(const char* str_id_begin, const char* str_id_end = nullptr);
		explicit ID(const std::string_view& sv);
		~ID();
	};

	struct StyleColor : detail::DisableCopy
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

	struct GlobalAlpha final : detail::DisableCopyMove
	{
		explicit GlobalAlpha(float newAlpha);
		~GlobalAlpha();

	private:
		float m_OldAlpha;
	};

	struct Indent final : detail::DisableCopyMove
	{
		explicit Indent(unsigned count = 1);
		~Indent();

	private:
		unsigned m_Count;
	};
}
