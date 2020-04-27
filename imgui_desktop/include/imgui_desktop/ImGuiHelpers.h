#pragma once

#define IM_VEC2_CLASS_EXTRA \
	bool operator==(const ImVec2& other) const { return x == other.x && y == other.y; } \
	bool operator!=(const ImVec2& other) const { return x != other.x || y != other.y; }

#define IM_VEC4_CLASS_EXTRA \
	ImVec4 operator+(const ImVec4& other) const { return ImVec4(x + other.x, y + other.y, z + other.z, w + other.w); } \
	ImVec4 operator-(const ImVec4& other) const { return ImVec4(x - other.x, y - other.y, z - other.z, w - other.w); } \
	ImVec4 operator*(float scalar) const { return ImVec4(x * scalar, y * scalar, z * scalar, w * scalar); }
