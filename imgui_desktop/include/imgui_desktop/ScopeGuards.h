#pragma once

#include <string_view>

enum ImGuiCol_ : int;
struct ImVec4;

namespace ImGuiDesktop::ScopeGuards
{
	namespace detail
	{
		struct DisableCopyMove
		{
			DisableCopyMove() = default;
			DisableCopyMove(const DisableCopyMove&) = delete;
			DisableCopyMove(DisableCopyMove&&) = delete;
			DisableCopyMove& operator=(const DisableCopyMove&) = delete;
			DisableCopyMove& operator=(DisableCopyMove&&) = delete;
		};
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

	struct StyleColor : detail::DisableCopyMove
	{
		StyleColor(ImGuiCol_ color, const ImVec4& value);
		~StyleColor();
	};

	struct TextColor final : StyleColor
	{
		TextColor(const ImVec4& color);
	};

	struct GlobalAlpha final : detail::DisableCopyMove
	{
		explicit GlobalAlpha(float newAlpha);
		~GlobalAlpha();

	private:
		float m_OldAlpha;
	};
}
