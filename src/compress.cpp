#include "ansu.hpp"

#define MASK(b) ((1 << b) - 1)

namespace ANS::Compress
{
	ANS::State encoders[CHANNEL_COUNT];

	ANS::State master;

	void reset_encoding(ANS::State& s)
	{
		s.x			   = encoding_table[0];
		s.partial	   = 0;
		s.partial_bits = sizeof(state_t) << 3;
		s.meta_offset  = 0;
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

	void encode_single_meta(ANS::State& s, hls::stream<ANS::Meta>& meta)
	{
		if(s.meta_offset == CHECKPOINT - 1)
		{
			ANS::Meta finished;

			finished.offset		   = s.meta_offset;
			finished.dead_bits	   = s.partial_bits;
			finished.control_state = s.x - TABLE_SIZE;
			finished.partial	   = s.partial;
			finished.dead_bytes	   = 0;

			meta << finished;
			s.meta_offset = 0;
		}
	}

	void merge_channels(hls::stream<state_t>& out, hls::stream<ANS::Meta>& meta)
	{
		for(int j = 0; j < CHANNEL_COUNT; j++)
		{
			ANS::State& s = encoders[j];
			if(s.partial_bits > master.partial_bits)
			{
				// Write them
				master.partial |=
					s.partial >> (s.partial_bits - master.partial_bits);
				out << (state_t) master.partial;
				s.meta_offset++;
				// Reset state of the partial
				master.partial_bits = (sizeof(state_t) << 3) - s.partial_bits
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
} // namespace ANS::Compress

void ANS::compress(hls::stream<message_t>& message,
				   hls::stream<state_t>& out,
				   hls::stream<ANS::Meta>& meta,
				   u32 dropBytes,
				   u8& control)
{
	using namespace ANS::Compress;

	PRAGMA_HLS(stream variable = message depth = AVG_MESSAGE_LENGTH)

	if(control & CONTROL_RESET_STATE)
	{
		for(int i = 0; i < CHANNEL_COUNT; i++) { reset_encoding(encoders[i]); }
		reset_encoding(master);
		control ^= CONTROL_RESET_STATE;
	}

	if(control & CONTROL_ENCODE)
	{
		for(int i = 0; i < AVG_MESSAGE_LENGTH / CHANNEL_COUNT; i++)
		{
			message_t buf[CHANNEL_COUNT];
			for(int j = 0; j < CHANNEL_COUNT; j++) message >> buf[j];
			for(int j = 0; j < CHANNEL_COUNT; j++)
				encode_single(encoders[j], buf[j]);
			merge_channels(out, meta);
		}
		control ^= CONTROL_ENCODE;
	}

	if(control & CONTROL_FLUSH)
	{
		ANS::State& a = master;
		if(a.partial_bits != sizeof(state_t) << 3)
		{ out << (state_t) a.partial; }

		ANS::Meta finished;

		finished.offset		   = a.meta_offset + 1;
		finished.dead_bits	   = a.partial_bits;
		finished.control_state = a.x - TABLE_SIZE;
		finished.partial	   = a.partial;
		finished.dead_bytes	   = dropBytes;

		meta << finished;

		control ^= CONTROL_FLUSH;
	}
}
