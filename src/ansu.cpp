#include "ansu.hpp"

#define MASK(b) ((1 << b) - 1)

state_t x		= encoding_table[0];
state_t partial = 0;
// *Remaining* bits
u8 partial_bits = sizeof(state_t) << 3;
u32 meta_offset = 0;

void reset_encoding()
{
	x			 = encoding_table[0];
	partial		 = 0;
	partial_bits = sizeof(state_t) << 3;
	meta_offset	 = 0;
}

void encode_single(message_t current,
				   hls::stream<state_t>& out,
				   hls::stream<ANSMeta>& meta)
{
#pragma HLS pipeline

	nb_t nb_bits;

	// Recover number of bits
	nb_bits = (x + nb[current]) >> (TABLE_SIZE_LOG + 1);

	// Mask the output
	state_t masked = x & MASK(nb_bits);

	// Get next state
	x = encoding_table[start[current] + (x >> nb_bits)];

	// Does partial output overflow now?
	if(nb_bits > partial_bits)
	{
		// Write them
		partial |= masked >> (nb_bits - partial_bits);
		out << (state_t) partial;
		meta_offset++;
		// Reset state of the partial
		partial_bits = (sizeof(state_t) << 3) - nb_bits + partial_bits;
		partial		 = masked << (partial_bits);
	} else
	{
		// Otherwise, just shift and write to partial
		partial |= masked << (partial_bits - nb_bits);

		partial_bits -= nb_bits;
	}

	if(meta_offset == CHECKPOINT - 1)
	{
		ANSMeta finished;

		finished.offset		   = meta_offset;
		finished.dead_bits	   = partial_bits;
		finished.control_state = x - TABLE_SIZE;
		finished.partial	   = partial;

		meta << finished;
		meta_offset = 0;
	}
}

void encode_stream(hls::stream<message_t>& message,
				   hls::stream<state_t>& out,
				   hls::stream<ANSMeta>& meta,
				   u8& control)
{
	PRAGMA_HLS(stream variable = message depth = AVG_MESSAGE_LENGTH)

	if(control & CONTROL_RESET_STATE)
	{
		reset_encoding();
		control ^= CONTROL_RESET_STATE;
	}

	if(control & CONTROL_ENCODE)
	{
		while(!message.empty())
		// for(int i = 0; i < AVG_MESSAGE_LENGTH; i++)
		{
			encode_single(message.read(), out, meta);
		}
		control ^= CONTROL_ENCODE;
	}

	if(control & CONTROL_FLUSH)
	{
		if(partial_bits != sizeof(state_t) << 3) { out << (state_t) partial; }

		ANSMeta finished;

		finished.offset		   = meta_offset + 1;
		finished.dead_bits	   = partial_bits;
		finished.control_state = x - TABLE_SIZE;
		finished.partial	   = partial;

		meta << finished;

		control ^= CONTROL_FLUSH;
	}
}
