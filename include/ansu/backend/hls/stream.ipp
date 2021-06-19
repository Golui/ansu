#pragma once

#include "ansu/ints.hpp"

#include <hls_stream.h>

namespace ANS
{
	namespace backend
	{
		template <typename T>
		using stream = hls::stream<T>;

		template <typename T>
		stream<T> streamFromBuffer(T* buf, u64 count)
		{
			auto ret = stream<T>();
			for(u32 i = 0; i < count; i++) { ret << buf[i]; }
			return ret;
		}

		template <typename T>
		stream<T> streamFromBufferReversed(T* buf, u64 count)
		{
			auto ret = stream<T>();
			for(u32 i = count; i > 0; i--) { ret << buf[i - 1]; }
			return ret;
		}
	} // namespace backend
} // namespace ANS
