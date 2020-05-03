#include "ansu.hpp"

#define MASK(b) ((1 << b) - 1)

namespace ANS::Decompress
{
	ANS::State encoders[CHANNEL_COUNT];

	ANS::State master;
} // namespace ANS::Decompress
