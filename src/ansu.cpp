#include "ansu.hpp"


#define MASK(b) ((1 << b) - 1)

state_t x;
state_t partial;
// *Remaining* bits
nb_bits_t partial_bits = sizeof(state_t) << 3;
u32 block_offset;

void setup_encode()
{
	#pragma HLS pipeline

	partial = 0;
	partial_bits = sizeof(state_t) << 3;
	block_offset = 0;

	x = encoding_table[0];
}

void end_encode(
		hls::stream<state_t>& out,
		hls::stream<ANSBlock>& meta)
{
	#pragma HLS stream variable = out
	#pragma HLS stream variable = meta

	#pragma HLS pipeline

	if(partial_bits)
	{
		out << partial;

		ANSBlock finished;

		finished.length = block_offset + 1;
		finished.dead_bits = partial_bits;
		finished.final_state = x - TABLE_SIZE;

		meta << finished;
	}
}

void encode_single(
		message_t current,
		hls::stream<state_t>& out,
		hls::stream<ANSBlock>& meta)
{
	#pragma HLS stream variable = out
	#pragma HLS stream variable = meta
	#pragma HLS pipeline

	nb_bits_t nb_bits;

	// Recover number of bits
	nb_bits = (x + nb[current]) >> (TABLE_SIZE_LOG + 1);
	// Mask the output
	state_t masked = x & MASK(nb_bits);
	// Does partial output overflow now?
	if(nb_bits > partial_bits)
	{
		// Write them
		partial |= masked >> (nb_bits - partial_bits);
		out << partial;
		block_offset++;
		// Reset state of the partial
		partial_bits = (sizeof(state_t) << 3) - nb_bits + partial_bits;
		partial = masked << (partial_bits);
	} else
	{
		// Otherwise, just shift and write to partial
		partial |= masked << (partial_bits - nb_bits);
		partial_bits -= nb_bits;
	}
	// Get next state
	x = encoding_table[start[current] + (x >> nb_bits)];

	if(block_offset == BLOCK_SIZE - 1)
	{
		ANSBlock finished;

		finished.length = block_offset + 1;
		finished.dead_bits = 0;
		finished.final_state = x - TABLE_SIZE;

		meta << finished;
	}
}

void encode_stream(
		hls::stream<message_t>& message,
		hls::stream<state_t>& out,
		hls::stream<ANSBlock>& meta
		)
{
	#pragma HLS stream variable = message
	#pragma HLS stream variable = out
	#pragma HLS stream variable = meta

	message_t current;
	while(!message.empty())
	{
		message >> current;
		encode_single(current, out, meta);
	}
}
