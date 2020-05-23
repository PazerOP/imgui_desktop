#pragma once

#include <type_traits>

#define IM_VEC2_CLASS_EXTRA \
	bool operator==(const ImVec2& other) const { return x == other.x && y == other.y; } \
	bool operator!=(const ImVec2& other) const { return x != other.x || y != other.y; }

#define IM_VEC4_CLASS_EXTRA \
	template<size_t N, typename = std::enable_if_t<N == 4>> \
	ImVec4(float (&array)[N]) : x(array[0]), y(array[1]), z(array[2]), w(array[3]) {} \
	ImVec4 operator+(const ImVec4& other) const { return ImVec4(x + other.x, y + other.y, z + other.z, w + other.w); } \
	ImVec4 operator-(const ImVec4& other) const { return ImVec4(x - other.x, y - other.y, z - other.z, w - other.w); } \
	ImVec4 operator*(float scalar) const { return ImVec4(x * scalar, y * scalar, z * scalar, w * scalar); }
