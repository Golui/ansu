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
	}
} // namespace ANS
