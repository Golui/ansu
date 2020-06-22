#include "ansu.hpp"
#include "driver.hpp"
#include "io/archive.hpp"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <stdio.h>

ANS::io::DataBuffer buffer;
ANS::io::MetaBuffer meta_buffer;

// NB Two variants needed due to the specifics of Vivado HLS testbench.
#ifdef SOFTWARE
#	define compress_fun ANS::compress
#else
#	define compress_fun hls_compress
#endif

std::streamsize getFileSize(std::ifstream& file)
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
	static const message_t ZERO = 0;

	std::ifstream in(opts->inFilePath, std::ios::binary);

	auto writer = ANS::io::ArchiveWriter(opts->outFilePath);

	auto begin = std::chrono::steady_clock::now();
	auto cur   = ANS::io::DataBuffer();
	cur.reserve(DATA_BLOCK_SIZE);
	auto curMeta = ANS::io::MetaBuffer();
	curMeta.reserve(MESSAGES_PER_BLOCK / CHECKPOINT);

	message_t msgbuf[AVG_MESSAGE_LENGTH];
	u32 padding;

	ANS::backend::stream<message_t> msg;
	ANS::backend::stream<state_t> out;
	ANS::backend::stream<ANS::Meta> ometa;

	u8 control = CONTROL_RESET_STATE;

	u32 i = 0;
	while(in.good() && !in.eof())
	{
		in.read((char*) msgbuf, AVG_MESSAGE_LENGTH * sizeof(message_t));
		auto read = in.gcount();

		for(i = 0; i < read; i++) { msg << msgbuf[i]; }
		padding = (AVG_MESSAGE_LENGTH - i);
		for(; i < AVG_MESSAGE_LENGTH; i++) { msg << ZERO; }

		control |= CONTROL_ENCODE;
		compress_fun(msg, out, ometa, padding, control);

		while(!out.empty()) { cur.push_back(out.read()); }
		while(!ometa.empty()) { curMeta.push_back(ometa.read()); }

		if(cur.size() >= DATA_BLOCK_SIZE)
		{
			control = CONTROL_FLUSH;
			compress_fun(msg, out, ometa, padding, control);

			while(!out.empty()) { cur.push_back(out.read()); }
			while(!ometa.empty()) { curMeta.push_back(ometa.read()); }

			auto tmp = ANS::io::DataBuffer();
			tmp.reserve(DATA_BLOCK_SIZE);
			auto tmpMeta = ANS::io::MetaBuffer();
			tmpMeta.reserve(MESSAGES_PER_BLOCK / CHECKPOINT);
			std::swap(cur, tmp);
			std::swap(curMeta, tmpMeta);

			writer.writeBlock(tmp, tmpMeta);

			control |= CONTROL_RESET_STATE;
		}
	}

	control = CONTROL_FLUSH;
	compress_fun(msg, out, ometa, padding, control);
	while(!out.empty()) { cur.push_back(out.read()); }
	while(!ometa.empty()) { curMeta.push_back(ometa.read()); }

	writer.writeBlock(cur, curMeta);

	writer.finalize();

	auto end = std::chrono::steady_clock::now();
	auto timeS =
		std::chrono::duration_cast<std::chrono::microseconds>(end - begin)
			.count()
		/ 1.0e6;

	auto inSize = getFileSize(in);
	fseek((FILE*) writer.tarFile.stream, 0, SEEK_END);
	u32 outSize = ftell((FILE*) writer.tarFile.stream);

	std::cout << std::setprecision(6);
	std::cout << "Done! Took " << timeS << "s \n";
	std::cout << "Stats:"
			  << "\n";
	std::cout << "\t"
			  << "Input size:    " << inSize << "\n";
	std::cout << "\t"
			  << "Output size:   " << outSize << "\n";
	std::cout << "\t"
			  << "Ratio:         " << 100 * outSize / ((double) inSize) << "%"
			  << "\n";

	return 0;
}
