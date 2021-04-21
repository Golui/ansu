#include "ansu.hpp"
#include "driver.hpp"
#include "io/archive.hpp"

#include <iostream>
#include <list>

template <typename S, typename T>
void writeFile(S& os, T v)
{
	os.write((char*) &v, sizeof(T));
}

int ANS::driver::decompress::run(OptionsP opts)
{
	// TODO Security
	std::ofstream out(opts->outFilePath, std::ios::binary);

	auto reader = ANS::io::ArchiveReader(opts->inFilePath);

	std::list<u64> blockOffs;

	ANS::backend::stream<state_t> data;
	ANS::backend::stream<ANS::Meta> meta;
	ANS::backend::stream<message_t> message;
	u32 blockNum = 0;
	while(reader.readBlock(data, meta))
	{
		if(!ANS::decompress(data, meta, message))
		{
			std::cout << "Block " << blockNum
					  << " did not end at state 0; invalid decompression.";
		}
		while(!message.empty()) writeFile(out, message.read());
		blockNum++;
	}

	return 0;
}
