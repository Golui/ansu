#pragma once

#include "ints.h"
#include "ans_table.h"

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0')

#define MAX_BLOCK_SIZE 65536/sizeof(state_t)

typedef u8 message_t;

typedef struct _ANSBlock {
	state_t final_state;
	u32 length;
	u32 dead_bits;
	u8* block_data;
} ANSBlock;

typedef struct _ANSContext
{
	state_t x;
} ANSContext;

ANSBlock* ANSBlock_new_max();
ANSBlock* ANSBlock_new(u32 length);
ANSBlock* ANSBlock_alloc_block_data(ANSBlock* this);
void ANSBlock_free(ANSBlock* this);

/**
 * Encodes the message into an ANS block
 * @param  block   the block to encode into
 * @param  message the message to encode
 * @param  len     length of the message
 * @return         number of alphabet characters consumed
 */
u32 encode_block(ANSBlock* block, message_t* message, u32 len);
void decode_block(ANSBlock* block, message_t* out, u32 len);
