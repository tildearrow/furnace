/*
	License: Zlib
	see https://gitlab.com/cam900/vgsound_emu/-/blob/main/LICENSE for more details

	Copyright holder(s): cam900
	Various core utilities for vgsound_emu
*/

#ifndef _VGSOUND_EMU_SRC_CORE_UTIL_HPP
#define _VGSOUND_EMU_SRC_CORE_UTIL_HPP

#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <iterator>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

namespace vgsound_emu
{
	// type defines
	using u8  = unsigned char;
	using u16 = unsigned short;
	using u32 = unsigned int;
	using u64 = unsigned long long;
	using s8  = signed char;
	using s16 = signed short;
	using s32 = signed int;
	using s64 = signed long long;
	using f32 = float;
	using f64 = double;

	template<typename T>
	using make_unsigned_t = typename std::make_unsigned<T>::type;

	template<typename T>
	using make_signed_t = typename std::make_signed<T>::type;

	// constant defines
	static constexpr f64 PI = 3.1415926535897932384626433832795;

	// std::clamp is only for C++17 or later; I use my own code
	template<typename T>
	static inline T clamp(const T in, const T min, const T max)
	{
#if defined(_HAS_CXX17) && _HAS_CXX17
		// just use std::clamp if C++17 or above
		return std::clamp(in, min, max);
#else
		// otherwise, use my own implementation of std::clamp
		return std::min(std::max(in, min), max);
#endif
	}

	// get bitmask, bitmask(len)
	template<typename T>
	static inline make_unsigned_t<T> bitmask(const T len)
	{
		return (len > 0) ? (T(1 << len) - 1) : 0;
	}

	// get boolmask, boolmask(input)
	template<typename T>
	static inline T boolmask(const bool in)
	{
		return in ? 1 : 0;
	}

	// get bitfield, bitfield(input, position[, len])
	template<typename T>
	static inline T bitfield(const T in, const u8 pos)
	{
		return (in >> pos) & 1;
	}

	template<typename T>
	static inline T bitfield(const T in, const u8 pos, const u8 len)
	{
		return (in >> pos) & bitmask<T>(len);
	}

	// merge data with mask, merge_data(src, input[, mask])
	template<typename T, typename U>
	static inline T merge_data(T &src, const U data)
	{
		src = data;
		return src;
	}

	template<typename T, typename U>
	static inline T merge_data(T &src, const U data, const U mask)
	{
		if (mask != 0)
		{
			src = (src & ~mask) | (data & mask);
		}
		return src;
	}

	// get sign extended value, sign_ext<type>(input, len)
	template<typename T>
	static inline make_signed_t<T> sign_ext(const T in, u8 len)
	{
		len = std::max<u8>(0, (8 * sizeof(T)) - len);
		return T(T(in) << len) >> len;
	}

	// convert attenuation decibel value to gain
	static inline f32 dB_to_gain(const f32 attenuation)
	{
		return std::pow(10.0f, attenuation / 20.0f);
	}

};	// namespace vgsound_emu

#endif
