#pragma once

#include <mh/text/string_insertion.hpp>

#include <string>
#include <string_view>

namespace ImGuiDesktop
{
	void PrintLogMsg(const std::string_view& msg);
	void PrintLogMsg(const char* msg);
}

#define SDL_PRINT_AND_CLEAR_ERROR() \
	[](const char* func, int line) \
	{ \
		auto err = SDL_GetError(); \
		if (err != nullptr && err[0] != '\0') \
		{ \
			using namespace std::string_literals; \
			::ImGuiDesktop::PrintLogMsg(""s << func << ':' << line << ": SDL_GetError() returned " << err); \
			SDL_ClearError(); \
			return false; \
		} \
		return true; \
	}(__FUNCTION__, __LINE__)
