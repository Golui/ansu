#pragma once

#include <hls_stream.h>

namespace ANS
{
	namespace backend
	{
		template <typename T>
		using stream = hls::stream<T>;
	}
} // namespace ANS
