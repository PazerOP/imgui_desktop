#pragma once

#include <compare>
#include <ostream>

namespace ImGuiDesktop
{
	struct GLContextVersion
	{
		GLContextVersion() = default;
		explicit constexpr GLContextVersion(int major, int minor = 0) :
			m_Major(major), m_Minor(minor)
		{
		}

		constexpr auto operator<=>(const GLContextVersion&) const = default;

		int m_Major = -1;
		int m_Minor = -1;
	};

	template<typename CharT, typename Traits>
	std::basic_ostream<CharT, Traits>& operator<<(
		std::basic_ostream<CharT, Traits>& os, const ImGuiDesktop::GLContextVersion& version)
	{
		return os << version.m_Major << '.' << version.m_Minor;
	}
}
