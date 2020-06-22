#include "ansu.hpp"

/**
 * Alias of `ANS::compress` for Vivado HLS compatiblity.
 */
void hls_compress(ANS::backend::stream<message_t>& message,
				  ANS::backend::stream<state_t>& out,
				  ANS::backend::stream<ANS::Meta>& meta,
				  u32 padding,
				  u8& control)
{
	PRAGMA_HLS(inline)
	ANS::compress(message, out, meta, padding, control);
}
