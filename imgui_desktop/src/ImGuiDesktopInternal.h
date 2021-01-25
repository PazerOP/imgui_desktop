#pragma once

#include <mh/source_location.hpp>
#include <mh/text/string_insertion.hpp>

#include <string>
#include <string_view>

namespace ImGuiDesktop
{
	void PrintLogMsg(const std::string_view& msg, MH_SOURCE_LOCATION_AUTO(location));
	void PrintLogMsg(const char* msg, MH_SOURCE_LOCATION_AUTO(location));
}

#define SDL_PRINT_AND_CLEAR_ERROR() \
	[](MH_SOURCE_LOCATION_AUTO(location)) \
	{ \
		auto err = SDL_GetError(); \
		if (err != nullptr && err[0] != '\0') \
		{ \
			using namespace std::string_literals; \
			::ImGuiDesktop::PrintLogMsg("SDL_GetError() returned "s << err, location); \
			SDL_ClearError(); \
			return false; \
		} \
		return true; \
	}()
