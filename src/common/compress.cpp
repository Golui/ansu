#include "ansu.hpp"

#ifdef SOFTWARE
#	include <iostream>
#endif

#define MASK(b) ((1 << b) - 1)

namespace ANS
{
	namespace Compress
	{
		// TODO Remove using
		using namespace ANS::StaticTable;

		std::unique_ptr<ContextT> mainCtx =
			memory::make_unique<ContextT>(CHANNEL_COUNT);

	} // namespace Compress
} // namespace ANS

/**
 * Compress a chunk of the stream.
 * @param message the message to compress
 * @param out     compressed data output
 * @param meta    metadata output
 * @param padding number of padding bytes in the message
 * @param control state control
 */
void ANS::compress(backend::stream<message_t>& message,
				   backend::stream<state_t>& out,
				   backend::stream<ANS::Meta>& meta,
				   u32 padding,
				   u8& control)
{
	using namespace ANS::Compress;

	PRAGMA_HLS(stream variable = message depth = AVG_MESSAGE_LENGTH)

	if(control & CONTROL_RESET_STATE)
	{
		mainCtx->initialize();
		control ^= CONTROL_RESET_STATE;
	}

	if(control & CONTROL_ENCODE)
	{
		mainCtx->compress(message, out, meta);
		control ^= CONTROL_ENCODE;
	}

	if(control & CONTROL_FLUSH)
	{
		mainCtx->flush(out, meta, padding);
		control ^= CONTROL_FLUSH;
	}
}
