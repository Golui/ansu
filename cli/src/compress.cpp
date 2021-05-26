#include "ansu.hpp"
#include "driver.hpp"
#include "io/archive.hpp"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <stdio.h>

// NB Two variants needed due to the specifics of Vivado HLS testbench.
#ifdef SOFTWARE
#	define compress_fun ANS::compress
#else
#	define compress_fun hls_compress
#endif

using InDataT = ANS::backend::side<message_t>;

template <typename T>
std::streamsize getFileSize(T& file)
{
	file.clear(); //  Since ignore will have set eof.
	file.seekg(0, std::ios_base::beg);
	file.ignore(std::numeric_limits<std::streamsize>::max());
	std::streamsize length = file.gcount();
	file.clear(); //  Since ignore will have set eof.
	file.seekg(0, std::ios_base::beg);
	return length;
}

// TODO efficiency
int ANS::driver::compress::run(OptionsP opts)
{
	std::ifstream in(opts->inFilePath,
					 std::ios::binary); // Stream nas interesujacy

	ANS::io::ArchiveWriter writer(opts->outFilePath);

	writer.createHeader(ANS::Compress::mainCtx);

	auto begin = std::chrono::steady_clock::now();

	message_t msgbuf[AVG_MESSAGE_LENGTH];
	state_t statebuf[CHECKPOINT];
	u64 counts[256];

	ANS::backend::side_stream<message_t> msg;
	ANS::backend::stream<state_t> out;
	ANS::backend::stream<ANS::Meta> ometa;

	u32 j = 0;
	while(in.good() && !in.eof())
	{
		u32 i = 0;

		in.read((char*) msgbuf, AVG_MESSAGE_LENGTH * sizeof(message_t));
		auto read = in.gcount();

		for(u32 k; k < read; k++) { counts[((u8*) msgbuf)[k]]++; }

		if(read != AVG_MESSAGE_LENGTH * sizeof(message_t) || in.eof())
		{
			for(; i < read - 1; i++) { msg << InDataT(msgbuf[i]); }
			auto last = InDataT(msgbuf[i]);
			last.last = true;
			msg << last;
		} else
		{
			for(; i < read; i++) { msg << InDataT(msgbuf[i]); }
		}

		compress_fun(msg, out, ometa);

		for(; !out.empty() && j < CHECKPOINT; j++) statebuf[j] = out.read();
		while(!ometa.empty())
		{
			ANS::Meta meta = ometa.read();
			writer.writeBlock(j, statebuf, meta);
			j = 0;
		}
	}

	auto end = std::chrono::steady_clock::now();
	auto timeS =
		std::chrono::duration_cast<std::chrono::microseconds>(end - begin)
			.count()
		/ 1.0e6;

	auto inSize = getFileSize(in);
	u32 outSize = getFileSize(writer.fileHandle);

	double entropy = 0.0;

	for(u16 k = 0; k < 256; k++)
	{
		u32 count = counts[k];
		if(count != 0) entropy -= count / 256.0 * (log2(count) - 8);
	}

	std::cout << std::setprecision(6);
	std::cout << "Done! Took " << timeS << "s \n";
	std::cout << "Stats:"
			  << "\n";
	std::cout << "\t"
			  << "Input size:       " << inSize << "\n";
	std::cout << "\t"
			  << "Output size:      " << outSize << "\n";
	std::cout << "\t"
			  << "Ratio:            " << 100 * outSize / ((double) inSize)
			  << "%"
			  << "\n";
	std::cout << "\t"
			  << "Theoretical best: " << entropy * 12.5 << "%" // * 8 / 100
			  << "\n";

	return 0;
}
