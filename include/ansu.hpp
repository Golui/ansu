#pragma once

#include "ints.hpp"
#include "ans_table.hpp"
#ifdef NO_VIVADO
	#include "binstream.hpp"
#else
	#include <hls_stream.h>
#endif


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

typedef u8 message_t;

// #define BLOCK_SIZE 16384/sizeof(state_t)
// #define MESSAGE_SIZE 1024*sizeof(message_t)

#define BLOCK_SIZE 2/sizeof(state_t)
#define MESSAGE_SIZE 16*sizeof(message_t)


struct ANSBlock {
	state_t final_state;
	u32 length;
	u32 dead_bits;
};

void setup_encode();

void encode_stream(
		hls::stream<message_t>& message,
		hls::stream<state_t>& out,
		hls::stream<ANSBlock>& meta);

void end_encode(
		hls::stream<state_t>& out,
		hls::stream<ANSBlock>& meta);
