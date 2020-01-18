#include "ansu.hpp"
#include <cstdio>

int main(int argc, char const *argv[]) {
	message_t mess[MESSAGE_SIZE] = {0, 0, 2, 2, 1, 0, 1, 1, 0, 0, 0, 1, 2, 1, 0, 0};

	hls::stream<message_t> message;
	hls::stream<state_t> out;
	hls::stream<ANSBlock> meta;

	for(message_t m : mess) message << m;

	setup_encode();
	encode_stream(message, out, meta);
	end_encode(out, meta);

	state_t CORRECT[2] = {0x1D, 0x06};

	while(!out.empty())
	{
		state_t dt;
		out >> dt;
		printf(BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(dt));
	}

	printf("\n");

	while(!meta.empty())
	{
		ANSBlock block;
		meta >> block;
		printf("\nBlock OBJ stats:\n");
		printf(" - final_state: %d\n", block.final_state);
		printf(" - length: %d\n", block.length);
		printf(" - dead_bits: %d\n", block.dead_bits);
	}


}
