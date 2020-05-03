#include "ansu.hpp"

#define MASK(b) ((1 << b) - 1)

struct ANSState
{
	state_t x		= 0;
	state_t partial = 0;
	// *Remaining* bits
	u8 partial_bits = 0;
	u32 meta_offset = 0;
};

ANSState a;

void reset_encoding(ANSState& s)
{
	s.x			   = encoding_table[0];
	s.partial	   = 0;
	s.partial_bits = sizeof(state_t) << 3;
	s.meta_offset  = 0;
}

void encode_single(message_t current,
				   hls::stream<state_t>& out,
				   hls::stream<ANSMeta>& meta)
{
#pragma HLS pipeline

	ANSState& s = a;

	nb_t nb_bits;
	// Recover number of bits
	nb_bits = (s.x + nb[current]) >> (TABLE_SIZE_LOG + 1);

	// Mask the output
	state_t masked = s.x & MASK(nb_bits);

	// Get next state
	s.x = encoding_table[start[current] + (s.x >> nb_bits)];

	// Does partial output overflow now?
	if(nb_bits > s.partial_bits)
	{
		// Write them
		s.partial |= masked >> (nb_bits - s.partial_bits);
		out << (state_t) s.partial;
		s.meta_offset++;
		// Reset state of the partial
		s.partial_bits = (sizeof(state_t) << 3) - nb_bits + s.partial_bits;
		s.partial	   = masked << (s.partial_bits);
	} else
	{
		// Otherwise, just shift and write to partial
		s.partial |= masked << (s.partial_bits - nb_bits);

		s.partial_bits -= nb_bits;
	}

	if(s.meta_offset == CHECKPOINT - 1)
	{
		ANSMeta finished;

		finished.offset		   = s.meta_offset;
		finished.dead_bits	   = s.partial_bits;
		finished.control_state = s.x - TABLE_SIZE;
		finished.partial	   = s.partial;
		finished.dead_bytes	   = 0;

		meta << finished;
		s.meta_offset = 0;
	}
}

void encode_stream(hls::stream<message_t>& message,
				   hls::stream<state_t>& out,
				   hls::stream<ANSMeta>& meta,
				   u32 dropBytes,
				   u8& control)
{
	PRAGMA_HLS(stream variable = message depth = AVG_MESSAGE_LENGTH)

	if(control & CONTROL_RESET_STATE)
	{
		reset_encoding(a);
		control ^= CONTROL_RESET_STATE;
	}

	if(control & CONTROL_ENCODE)
	{
		for(int i = 0; i < AVG_MESSAGE_LENGTH; i++)
		{ encode_single(message.read(), out, meta); }
		control ^= CONTROL_ENCODE;
	}

	if(control & CONTROL_FLUSH)
	{
		if(a.partial_bits != sizeof(state_t) << 3)
		{ out << (state_t) a.partial; }

		ANSMeta finished;

		finished.offset		   = a.meta_offset + 1;
		finished.dead_bits	   = a.partial_bits;
		finished.control_state = a.x - TABLE_SIZE;
		finished.partial	   = a.partial;
		finished.dead_bytes	   = dropBytes;

		meta << finished;

		control ^= CONTROL_FLUSH;
	}
}
