#pragma once

#define IM_VEC2_CLASS_EXTRA \
	bool operator==(const ImVec2& other) const { return x == other.x && y == other.y; } \
	bool operator!=(const ImVec2& other) const { return x != other.x || y != other.y; }
