#pragma once

#include "ints.hpp"

#include <cmath>
#include <memory>
#include <type_traits>

#ifdef DEBUG
#	include <iostream>
#	define DEBUG_OUT(x) std::cout << x;
#else
#	define DEBUG_OUT(x)
#endif

namespace ANS
{
	namespace memory
	{
		template <typename T, typename... Args>
		std::unique_ptr<T> make_unique(Args... args)
		{
			return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
		}
	} // namespace memory

	namespace integer
	{
		u64 nextPowerOfTwo(u64 v);

		template <u64 Num, bool _signed = false>
		struct fitting
		{
			// clang-format off
			using type_signed = 	typename std::conditional<(Num <=  7),  s8,
									typename std::conditional<(Num <= 15), s16,
									typename std::conditional<(Num <= 31), s32, s64>::type>::type>::type;

			using type_unsigned =	typename std::conditional<(Num <=  8),  u8,
									typename std::conditional<(Num <= 16), u16,
									typename std::conditional<(Num <= 32), u32, u64>::type>::type>::type;

			using type = typename std::conditional<_signed, type_signed, type_unsigned>::type;
			// clang-format on
		};
	} // namespace integer
} // namespace ANS
