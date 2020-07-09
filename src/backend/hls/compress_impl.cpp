#include "backend/hls/ansu.ipp"
#include <utility>
/**
 * Alias of `ANS::compress` for Vivado HLS compatiblity.
 */
void hls_compress(ANS::backend::stream<message_t>&& message,
				  ANS::backend::stream<state_t>&& out,
				  ANS::backend::stream<ANS::Meta>&& meta,
				  u32&& padding,
				  u8&& control)
{
	PRAGMA_HLS(inline)
	ANS::compress(std::forward<ANS::backend::stream<message_t>&>(message),
			std::forward<ANS::backend::stream<state_t>&>(out),
			std::forward<ANS::backend::stream<ANS::Meta>&>(meta),
			std::forward<u32>(padding),
			std::forward<u8&>(control));
}
