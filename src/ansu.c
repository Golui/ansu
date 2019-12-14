#include "ansu.h"
#include <stdlib.h>
#include <stdio.h>


#define MASK(b) ((1 << b) - 1)

ANSBlock* ANSBlock_new_max()
{
	return ANSBlock_new(MAX_BLOCK_SIZE);
}

ANSBlock* ANSBlock_new(u32 length)
{
	ANSBlock* the_block = malloc(sizeof(ANSBlock));
	the_block->length = length;
	the_block->dead_bits = 0;
	the_block->block_data = 0;
	ANSBlock_alloc_block_data(the_block);
	return the_block;
}

ANSBlock* ANSBlock_alloc_block_data(ANSBlock* this)
{
	if(this->block_data)
	{
		free(this->block_data);
	}

	this->block_data = calloc(sizeof(state_t), this->length);
	return this;
}

void ANSBlock_free(ANSBlock* this)
{
	if(this->block_data)
	{
		free(this->block_data);
	}

	free(this);
}

u32 encode_block(ANSBlock* block, message_t* message, u32 len)
{
	// Define variables
	state_t x = 0;
	state_t partial = 0;
	// *Remaining* bits
	nb_bits_t partial_bits = sizeof(state_t) << 3;
	message_t current;
	u32 block_offset = 0;
	u32 i = 0;
	nb_bits_t nb_bits;

	// Prepare states
	state_delta_t start_state = start[message[0]];
	while(start_state < 0) start_state += table_size;
	x = encoding_table[start_state];

	for(; i < len; i++)
	{
		// Get the current symbol
		current = message[i];
		// Recover number of bits
		// TODO Bake '>> (table_size_log + 1)' into the table
		nb_bits = (x + nb[current]) >> (table_size_log + 1);
		// Maks the output
		state_t masked = x & MASK(nb_bits);
		// Does partial output overflow now?
		if(nb_bits > partial_bits)
		{
			// Write them
			partial |= masked >> (nb_bits - partial_bits);
			block->block_data[block_offset] = partial;
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
		// Are we running out of block space?
		if(block_offset == block->length - 1) break;
	}
	// Dump what was left in partial, write final state.
	block->block_data[block_offset] = partial;
	block->length = block_offset + 1;
	block->dead_bits = partial_bits;
	block->final_state = x - table_size;
	return i;
}
