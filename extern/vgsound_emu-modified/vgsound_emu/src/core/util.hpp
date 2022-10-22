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
#include <iterator>
#include <memory>
#include <string>
#include <vector>

namespace vgsound_emu
{
	typedef unsigned char u8;
	typedef unsigned short u16;
	typedef unsigned int u32;
	typedef unsigned long long u64;
	typedef signed char s8;
	typedef signed short s16;
	typedef signed int s32;
	typedef signed long long s64;
	typedef float f32;
	typedef double f64;

	static constexpr f64 PI = 3.1415926535897932384626433832795;

	// std::clamp is only for C++17 or later; I use my own code
	template<typename T>
	static inline T clamp(T in, T min, T max)
	{
#if defined(_HAS_CXX17) && _HAS_CXX17
		// just use std::clamp if C++17 or above
		return std::clamp(in, min, max);
#else
		// otherwise, use my own implementation of std::clamp
		return std::min(std::max(in, min), max);
#endif
	}

	// get bitfield, bitfield(input, position, len)
	template<typename T>
	static inline T bitfield(T in, u8 pos, u8 len = 1)
	{
		return (in >> pos) & (len ? (T(1 << len) - 1) : 1);
	}

	// get sign extended value, sign_ext<type>(input, len)
	template<typename T>
	static inline T sign_ext(T in, u8 len)
	{
		len = std::max<u8>(0, (8 * sizeof(T)) - len);
		return T(T(in) << len) >> len;
	}

	// convert attenuation decibel value to gain
	static inline f32 dB_to_gain(f32 attenuation) { return std::pow(10.0f, attenuation / 20.0f); }

};	// namespace vgsound_emu

#endif
