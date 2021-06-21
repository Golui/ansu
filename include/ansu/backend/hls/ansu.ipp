#pragma once
#include "ans/settings.hpp"
#include "ansu.hpp"

void hls_compress(ANS::backend::stream<message_t>& message,
				  ANS::backend::stream<state_t>& out,
				  ANS::backend::stream<ANS::Meta>& meta,
				  u32 dropBytes,
				  u8& control);
