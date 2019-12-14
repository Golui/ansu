#include "ansu.h"
#include <stdio.h>

int main(int argc, char const *argv[]) {
	message_t mess[] = {0, 0, 2, 2, 1, 0, 1, 1, 0, 0};
	ANSBlock* block = ANSBlock_new_max();
	int ret = encode_block(block, mess, 10);
	for(u32 i = 0; i < block->length; i++)
	{
		printf(BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(block->block_data[i]));
	}
	printf("\nBlock OBJ stats:\n");
	printf(" - final_state: %d\n", block->final_state);
	printf(" - length: %d\n", block->length);
	printf(" - dead_bits: %d\n", block->dead_bits);
	printf("Read: %d\n", ret);

}
