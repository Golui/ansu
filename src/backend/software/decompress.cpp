#include "ansu.hpp"

#include <algorithm>
#include <stack>

#define MASK(b) ((1 << b) - 1)

namespace ANS
{
	namespace Decompress
	{
		ANS::State decoders[CHANNEL_COUNT];

		ANS::Meta master;
	} // namespace Decompress
} // namespace ANS

// TODO Remove the using declaration
using namespace ANS::StaticTable;
using namespace ANS::Decompress;

void init_decoders(ANS::backend::stream<ANS::Meta>& meta)
{
	master = meta.read();

	// TODO
	// if(master.channels != CHANNEL_COUNT)
	// {
	// 	std::cout << "This file was generated with a different version of "
	// 				 "ansu. A crash is imminent.\n";
	// }
	for(int i = 0; i < CHANNEL_COUNT; i++)
	{
		state_t channel_state = master.controlState[i];
		ANS::State& a		  = decoders[i];
		a.x					  = new_x[channel_state];
		a.partialBits		  = nb_bits_delta[channel_state];
	}
}

void redistribute(state_t& newd, state_t& d, u8 available, u8 lower_amnt)
{
	auto neg_shift =
		std::min((u8)(ANS::allBitsRemaining - lower_amnt), available);
	d |= (newd & MASK(neg_shift)) << lower_amnt;
	newd >>= neg_shift;
}

void shift(state_t& newd, state_t& d, u8& available, u8& shift_amount)
{
	d >>= shift_amount;
	available -= shift_amount;
	redistribute(newd, d, available, ANS::allBitsRemaining - shift_amount);
}

/**
 * Perform ANS decompression. NB Expects the input data to be reversed.
 * @param data    Compressed data stream - reversed
 * @param meta    Metadata stream
 * @param message Stream to output the message to
 */
ANS::DecompressResult ANS::decompress(backend::stream<state_t>& data,
									  backend::stream<Meta>& meta,
									  backend::stream<message_t>& message)
{
	init_decoders(meta);
	int cur_channel	  = master.currentChannel;
	u8 available_bits = ANS::allBitsRemaining;

	state_t cur_val = data.read();
	state_t read	= 0;
	if(master.deadBits > 0)
	{
		cur_val >>= master.deadBits;
		available_bits -= master.deadBits;
	}

	std::stack<message_t> reverse_msg;

	while(!data.empty() || available_bits > 0)
	{
		ANS::State& cur_dec = decoders[cur_channel];
		if(!data.empty() && available_bits < ANS::allBitsRemaining)
		{
			read = data.read();
			redistribute(read,
						 cur_val,
						 available_bits + ANS::allBitsRemaining,
						 available_bits);
			available_bits += ANS::allBitsRemaining;
		}

		auto advance = cur_val & MASK(cur_dec.partialBits);

		cur_dec.x = cur_dec.x + advance;
		shift(read, cur_val, available_bits, cur_dec.partialBits);

		reverse_msg.push(states[cur_dec.x]);

		cur_dec.partialBits = nb_bits_delta[cur_dec.x];
		cur_dec.x			= new_x[cur_dec.x];
		cur_channel			= (CHANNEL_COUNT + cur_channel - 1) % CHANNEL_COUNT;
	}

	// After we are done, let's verify the block
	for(int i = 0; i < CHANNEL_COUNT; i++)
	{
		// We always start from 0 state, so if we ended up somewhere else
		// something broke along the way.
		if(decoders[i].x != 0) return false;
	}

	// Account for the compressor "jump start"
	for(int i = 0; i < CHANNEL_COUNT && !reverse_msg.empty(); i++)
		reverse_msg.pop();

	// Same here; since we popped CHANNEL_COUNT elements off at the beginning
	master.messagePad -= CHANNEL_COUNT;
	while(!reverse_msg.empty() && reverse_msg.size() > (master.messagePad))
	{
		message << reverse_msg.top();
		reverse_msg.pop();
	}

	return true;
}
