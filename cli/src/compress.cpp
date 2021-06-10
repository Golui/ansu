#include "ansu.hpp"
#include "data/table_generator.hpp"
#include "driver.hpp"
#include "io/archive.hpp"
#include "io/table_archive.hpp"

#include <array>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <stdio.h>

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

template <typename ContextT>
int compressTask(ANS::driver::compress::OptionsP opts,
				 std::istream& in,
				 std::shared_ptr<ContextT> mainCtxPtr)
{
	using StateT		= typename ContextT::StateT;
	using MessageT		= u8; // typename ContextT::MessageT;
	using MessageIndexT = typename ContextT::MessageIndexT;
	using Meta			= typename ContextT::Meta;
	using InDataT		= ANS::backend::side<MessageIndexT>;

	auto& mainCtx = *mainCtxPtr;

	mainCtx.setCheckpointFrequency(opts->checkpoint);
	mainCtx.setChunkSize(opts->chunkSize);

	ANS::io::ArchiveWriter<ContextT> writer(opts->outFilePath);

	writer.bindContext(mainCtxPtr);

	auto begin = std::chrono::steady_clock::now();

	std::array<u64, 256> counts = {0};
	MessageT* msgbuf			= new MessageT[opts->chunkSize];
	StateT* statebuf			= new StateT[opts->checkpoint];

	ANS::backend::side_stream<MessageIndexT> msg;
	ANS::backend::stream<StateT> out;
	ANS::backend::stream<Meta> ometa;

	u32 j			= 0;
	u64 dataWritten = 0;
	u64 inSize;
	while(in.peek() != EOF)
	{
		u32 i = 0;

		in.read((std::fstream::char_type*) msgbuf,
				opts->chunkSize * sizeof(MessageT)
					/ sizeof(std::fstream::char_type));
		u64 read = in.gcount();
		inSize += read;

		for(decltype(read) k = 0; k < read; k++)
		{
			counts[((u8*) msgbuf)[k]]++;
		}

		if(read != opts->chunkSize * sizeof(MessageT) || in.peek() == EOF)
		{
			for(; i < read - 1; i++)
			{
				msg << InDataT(mainCtx.ansTable.reverseAlphabet(msgbuf[i]));
			}
			auto last = InDataT(mainCtx.ansTable.reverseAlphabet(msgbuf[i]));
			last.last = true;
			msg << last;
		} else
		{
			for(; i < read; i++)
			{
				msg << InDataT(mainCtx.ansTable.reverseAlphabet(msgbuf[i]));
			}
		}
		mainCtx.compress(msg, out, ometa);

		while(!out.empty())
		{
			for(; !out.empty() && j < opts->checkpoint; j++)
				statebuf[j] = out.read();
			if(!ometa.empty())
			{
				Meta meta	= ometa.read();
				auto result = writer.writeBlock(j, statebuf, meta);
				dataWritten += result;
				//				std::cout << "Wrote " << result << " bytes.\n";

				j = 0;
			}
		}
	}

	if(!ometa.empty())
	{
		Meta meta	= ometa.read();
		auto result = writer.writeBlock(0, statebuf, meta);
		//		std::cout << "Wrote " << result << " bytes.\n";
		dataWritten += result;
		if(!ometa.empty()) std::runtime_error("Too many meta objects!");
	}

	inSize *= sizeof(std::istream::char_type);
	// TODO encapsualte header, think of a better way to track inputSize
	writer.header.inputSize = inSize;

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

	if(opts->printSummary)
	{
		std::cout << std::setprecision(6);
		std::cout << "Done! Took " << timeS << "s \n";
		std::cout << "Stats:"
				  << "\n";
		std::cout << "\t"
				  << "Input size:       " << inSize << "\n";
		std::cout << "\t"
				  << "Output size:      " << outSize << "\n";
		std::cout << "\t\t"
				  << "Header size:      " << writer.getHeader().totalHeaderSize
				  << "\n";
		std::cout << "\t\t"
				  << "Data size:      "
				  << dataWritten * sizeof(std::fstream::char_type) << "\n";
		std::cout << "\t"
				  << "Ratio:            " << 100.0 * outSize / ((double) inSize)
				  << "%"
				  << "\n";
		std::cout << "\t"
				  << "Theoretical best: " << entropy * 12.5 << "%"
				  << "\n";
		std::cout << "\t"
				  << "Entropy:          " << entropy << " bits/symbol\n";
	}

	delete[] msgbuf;
	delete[] statebuf;

	return 0;
}

// TODO efficiency
int ANS::driver::compress::run(OptionsP opts)
{
	std::ifstream inFile = std::ifstream(opts->inFilePath, std::ios::binary);
	std::istream* in;

	if(opts->inFilePath != "STDIN")
	{
		in = &inFile;
	} else
	{
		in = &std::cin;
	}

	if(opts->tableFilePath == "static")
	{
		using ContextT =
			ANS::ChannelCompressionContext<ANS::StaticCompressionTable>;
		auto mainCtxPtr = std::make_shared<ContextT>(opts->channels);
		return compressTask(opts, *in, mainCtxPtr);
	} else
	{
		using TableT   = ANS::DynamicCompressionTable<u32, u8>;
		using ContextT = ANS::ChannelCompressionContext<TableT>;
		TableT table;
		if(opts->tableFilePath == "")
		{
			if(opts->inFilePath == "STDIN")
				throw std::runtime_error(
					"Cannot generate table for STDIN. Please save to a file "
					"first or use a premade table.");
			auto tableGenOpts = TableGeneratorOptions();
			switch(opts->alphabet)
			{
				case ANS::driver::Alphabet::Reduced:
				{
					tableGenOpts.useFullAscii = false;
					break;
				}
				case ANS::driver::Alphabet::Ascii:
				{
					tableGenOpts.useFullAscii = true;
					break;
				}
				default: break;
			}
			tableGenOpts.tableSizeLog = opts->tableSizeLog;
			table = ANS::generateTable<u32, u8>(*in, tableGenOpts);
			in->seekg(0, std::ios_base::beg);
		} else
		{
			std::ifstream tableFile(opts->tableFilePath);
			table = ANS::io::loadTable<u32, u8>(tableFile);
		}
		auto mainCtxPtr = std::make_shared<ContextT>(opts->channels, table);
		return compressTask(opts, *in, mainCtxPtr);
	}

	return 0;
}
