#include "ansu.hpp"

#ifdef SOFTWARE
	#include <iostream>
#endif

#define MASK(b) ((1 << b) - 1)

namespace ANS
{
	namespace Compress
	{
		ANS::State encoders[CHANNEL_COUNT];

		ANS::State master;

		void reset_encoding(ANS::State& s)
		{
			s.x			   = encoding_table[0];
			s.partial	   = 0;
			s.partial_bits = 0;
			s.meta_offset  = 0;
		}

		void reset_encoding_master(ANS::State& s)
		{
			reset_encoding(s);
			s.partial_bits = ANS::all_bits_remaining;
		}

		void encode_single(ANS::State& s, message_t current)
		{
#pragma HLS pipeline

			nb_t nb_bits;
			// Recover number of bits
			nb_bits = (s.x + nb[current]) >> (TABLE_SIZE_LOG + 1);

			// Mask the output
			s.partial	   = s.x & MASK(nb_bits);
			s.partial_bits = nb_bits;

			// Get next state
			s.x = encoding_table[start[current] + (s.x >> nb_bits)];
		}

		void merge_channels(backend::stream<state_t>& out,
							backend::stream<ANS::Meta>& meta)
		{
			PRAGMA_HLS(inline)
			for(int j = 0; j < CHANNEL_COUNT; j++)
			{
				PRAGMA_HLS(unroll)
				ANS::State& s = encoders[j];
				if(s.partial_bits > master.partial_bits)
				{
					// Write them
					master.partial |=
						s.partial >> (s.partial_bits - master.partial_bits);
					out << (state_t) master.partial;
					s.meta_offset++;
					// Reset state of the partial
					master.partial_bits = ANS::all_bits_remaining
										  - s.partial_bits
										  + master.partial_bits;
					master.partial = s.partial << (master.partial_bits);
				} else
				{
					// Otherwise, just shift and write to partial
					master.partial |= s.partial
									  << (master.partial_bits - s.partial_bits);

					master.partial_bits -= s.partial_bits;
				}
			}
		}

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
		for(int i = 0; i < CHANNEL_COUNT; i++) { reset_encoding(encoders[i]); }
		reset_encoding_master(master);
		control ^= CONTROL_RESET_STATE;
	}

	if(control & CONTROL_ENCODE)
	{
		for(int i = 0; i < AVG_MESSAGE_LENGTH / CHANNEL_COUNT; i++)
		{
			PRAGMA_HLS(pipeline ii = CHANNEL_COUNT)
			for(int j = 0; j < CHANNEL_COUNT; j++)
			{
				PRAGMA_HLS(unroll)
				// TODO
				#ifdef SOFTWARE
				std::cout << "Channel: " << j << " State: " << encoders[j].x
						  << std::endl;
				#endif
				encode_single(encoders[j], message.read());
			}
			merge_channels(out, meta);
		}
		control ^= CONTROL_ENCODE;
	}

	if(control & CONTROL_FLUSH)
	{
		out << (state_t) master.partial;

		ANS::Meta finished;

		finished.offset		 = master.meta_offset + 1;
		finished.dead_bits	 = master.partial_bits;
		finished.message_pad = padding;
		for(int i = 0; i < CHANNEL_COUNT; i++)
			finished.control_state[i] = encoders[i].x - TABLE_SIZE;

		finished.channels		 = CHANNEL_COUNT;
		finished.current_channel = CHANNEL_COUNT - 1;

		meta << finished;

		control ^= CONTROL_FLUSH;
	}
}

// TODO - This was done because Vivado used to not be able to have
// namespaced functions as accelerated. This's probably since been fixed
#ifndef SOFTWARE

void hls_compress(ANS::backend::stream<message_t>& message,
				  ANS::backend::stream<state_t>& out,
				  ANS::backend::stream<ANS::Meta>& meta,
				  u32 padding,
				  u8& control)
{
	using namespace ANS::Compress;

	PRAGMA_HLS(stream variable = message depth = AVG_MESSAGE_LENGTH)

	if(control & CONTROL_RESET_STATE)
	{
		for(int i = 0; i < CHANNEL_COUNT; i++) { reset_encoding(encoders[i]); }
		reset_encoding_master(master);
		control ^= CONTROL_RESET_STATE;
	}

	if(control & CONTROL_ENCODE)
	{
		for(int i = 0; i < AVG_MESSAGE_LENGTH / CHANNEL_COUNT; i++)
		{
			PRAGMA_HLS(pipeline ii = CHANNEL_COUNT)
			for(int j = 0; j < CHANNEL_COUNT; j++)
			{
				PRAGMA_HLS(unroll)
				encode_single(encoders[j], message.read());
			}
			merge_channels(out, meta);
		}
		control ^= CONTROL_ENCODE;
	}

	if(control & CONTROL_FLUSH)
	{
		out << (state_t) master.partial;

		ANS::Meta finished;

		finished.offset		 = master.meta_offset + 1;
		finished.dead_bits	 = master.partial_bits;
		finished.message_pad = padding;
		for(int i = 0; i < CHANNEL_COUNT; i++)
			finished.control_state[i] = encoders[i].x - TABLE_SIZE;

		finished.channels		 = CHANNEL_COUNT;
		finished.current_channel = CHANNEL_COUNT - 1;

		meta << finished;

		control ^= CONTROL_FLUSH;
	}
}

#endif
