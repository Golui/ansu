#pragma once

#ifdef SOFTWARE
#	include "software/stream.ipp"
#	include "software/sidechannel.ipp"
#else
#	include "hls/stream.ipp"
#endif

namespace ANS
{
	namespace backend
	{
		template <typename T>
		using side_stream = stream<side<T>>;

		/**
		 * count - number of elements, not bytes
		 */
		template <typename T>
		stream<T> streamFromBuffer(T* buf, u64 count);
	} // namespace backend
} // namespace ANS
