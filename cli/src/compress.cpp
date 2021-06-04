#include "ansu.hpp"
#include "driver.hpp"
#include "io/archive.hpp"

#include <array>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <stdio.h>

using InDataT  = ANS::backend::side<message_t>;
using ContextT = ANS::ChannelCompressionContext<ANS::StaticCompressionTable>;

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
	using StateT   = typename ContextT::StateT;
	using MessageT = typename ContextT::MessageT;
	using Meta	   = typename ContextT::Meta;

	std::ifstream in(opts->inFilePath,
					 std::ios::binary); // Stream nas interesujacy

	auto inSize = getFileSize(in);

	auto mainCtxPtr = std::make_shared<ContextT>(opts->channels);
	auto& mainCtx	= *mainCtxPtr;

	mainCtx.setCheckpointFrequency(opts->checkpoint);
	mainCtx.setChunkSize(opts->chunkSize);

	ANS::io::ArchiveWriter<ContextT> writer(opts->outFilePath);

	writer.bindContext(mainCtxPtr, inSize);

	auto begin = std::chrono::steady_clock::now();

	std::array<u64, 256> counts = {0};
	MessageT* msgbuf			= new MessageT[opts->chunkSize];
	StateT* statebuf			= new StateT[opts->checkpoint];

	ANS::backend::side_stream<MessageT> msg;
	ANS::backend::stream<StateT> out;
	ANS::backend::stream<Meta> ometa;

	u32 j = 0;
	while(in.peek() != EOF)
	{
		u32 i = 0;

		in.read((char*) msgbuf, opts->chunkSize * sizeof(message_t));
		u64 read = in.gcount();

		for(decltype(read) k = 0; k < read; k++)
		{
			counts[((u8*) msgbuf)[k]]++;
		}

		if(read != opts->chunkSize * sizeof(message_t) || in.peek() == EOF)
		{
			for(; i < read - 1; i++) { msg << InDataT(msgbuf[i]); }
			auto last = InDataT(msgbuf[i]);
			last.last = true;
			msg << last;
		} else
		{
			for(; i < read; i++) { msg << InDataT(msgbuf[i]); }
		}
		mainCtx.compress(msg, out, ometa);

		while(!out.empty())
		{
			for(; !out.empty() && j < opts->checkpoint; j++)
				statebuf[j] = out.read();
			if(!ometa.empty())
			{
				Meta meta = ometa.read();
				/*auto result =*/writer.writeBlock(j, statebuf, meta);
				//				std::cout << "Wrote " << result << " bytes.\n";

				j = 0;
			}
		}
	}

	if(!ometa.empty())
	{
		Meta meta = ometa.read();
		/*auto result =*/writer.writeBlock(0, statebuf, meta);
		if(!ometa.empty()) std::runtime_error("Too many meta objects!");
	}

	auto end = std::chrono::steady_clock::now();
	auto timeS =
		std::chrono::duration_cast<std::chrono::microseconds>(end - begin)
			.count()
		/ 1.0e6;

	auto outSize = getFileSize(writer.fileHandle);

	double entropy = 0.0;

	u64 sum = 0;

	for(u16 k = 0; k < 256; k++) sum += counts[k];
	for(u16 k = 0; k < 256; k++)
	{
		u32 count = counts[k];
		if(count != 0)
			entropy -= count / double(sum) * (log2(count / double(sum)));
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
			  << "Ratio:            " << 100.0 * outSize / ((double) inSize)
			  << "%"
			  << "\n";
	std::cout << "\t"
			  << "Theoretical best: " << entropy * 12.5 << "%"
			  << "\n";
	std::cout << "\t"
			  << "Entropy:          " << entropy << " bits/symbol\n";

	delete[] msgbuf;
	delete[] statebuf;

	return 0;
}
