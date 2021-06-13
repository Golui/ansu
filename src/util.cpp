#include "util.hpp"

u64 ANS::integer::nextPowerOfTwo(u64 v)
{
	// Feeling linear, might use __builtin_clzl later, idk
	// Source: https://jameshfisher.com/2018/03/30/round-up-power-2/
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v |= v >> 32;
	return v + 1;
}
