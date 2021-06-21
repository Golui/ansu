#include "ansu.hpp"
#include "cereal/types/array.hpp"
#include "data/table_generator.hpp"
#include "driver.hpp"
#include "io/archive.hpp"
#include "io/table_archive.hpp"
#include "util.hpp"

#include <array>
#include <chrono>
#include <cstdlib>
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

template <typename... T>
using MyVector = std::array<T..., CHANNEL_COUNT>;

template <typename ContextT, typename SymbolT>
int compressTask(ANS::driver::compress::OptionsP opts,
				 std::istream& in,
				 std::shared_ptr<ContextT> mainCtxPtr)
{
	using StateT		 = typename ContextT::StateT;
	using ReducedSymbolT = typename ContextT::ReducedSymbolT;
	using Meta			 = typename ContextT::Meta;
	using InDataT		 = ANS::backend::side<ReducedSymbolT>;

	using CharT					  = std::istream::char_type;
	constexpr auto streamCharSize = sizeof(CharT);

	auto& mainCtx = *mainCtxPtr;

	mainCtx.setCheckpointFrequency(opts->checkpoint);
	mainCtx.setChunkSize(opts->chunkSize);

	ANS::io::ArchiveWriter<ContextT> writer(opts->outFilePath);

	writer.bindContext(mainCtxPtr);

	auto begin = std::chrono::steady_clock::now();

	const auto symbolWidth =
		(ANS::integer::nextPowerOfTwo(mainCtx.ansTable.symbolWidth()) >> 3);
	const auto chunkByteSize = opts->chunkSize * symbolWidth;

	std::array<u64, 256> counts = {0};
	SymbolT* msgbuf				= new SymbolT[opts->chunkSize];
	StateT* statebuf			= new StateT[opts->checkpoint];

	ANS::backend::side_stream<ReducedSymbolT> msg;
	ANS::backend::stream<StateT> out;
	ANS::backend::stream<Meta> ometa;

	// TODO we only support types with power of 2 widths

	u32 j			= 0;
	u64 dataWritten = 0;
	u64 inSize;
	while(in.peek() != EOF)
	{
		u32 i = 0;

		in.read((CharT*) msgbuf, chunkByteSize / streamCharSize);
		u64 read		= in.gcount();
		u64 readSymbols = read / symbolWidth;

		for(decltype(read) k = 0; k < read; k++)
		{
			counts[((u8*) msgbuf)[k]]++;
		}

		u32 validSymbols = 0;
		if(read != chunkByteSize || in.peek() == EOF)
		{
			// Since we may skip symbols, we need to figure out where the last
			// valid one is.
			auto actuaLast = readSymbols - 1;
			while(!mainCtx.ansTable.hasSymbolInAlphabet(msgbuf[actuaLast]))
				actuaLast--;
			// Write the symbols except the last one...
			for(; i < actuaLast; i++)
			{
				if(mainCtx.ansTable.hasSymbolInAlphabet(msgbuf[i]))
				{
					msg << InDataT(mainCtx.ansTable.reverseAlphabet(msgbuf[i]));
					validSymbols++;
				} else if(!opts->skipUnknown)
				{
					std::cerr << "Unknown symbol @ " << inSize + i / symbolWidth
							  << ". Aborting." << std::endl;
					std::exit(0);
				}
			}
			// ...and now write the last one, with the proper flag.
			auto last = InDataT(mainCtx.ansTable.reverseAlphabet(msgbuf[i]));
			last.last = true;
			msg << last;
			validSymbols++;
		} else
		{
			for(; i < readSymbols; i++)
			{
				if(mainCtx.ansTable.hasSymbolInAlphabet(msgbuf[i]))
				{
					msg << InDataT(mainCtx.ansTable.reverseAlphabet(msgbuf[i]));
					validSymbols++;
				} else if(!opts->skipUnknown)
				{
					std::cerr << "Unknown symbol @ " << inSize + i / symbolWidth
							  << ". Aborting. " << std::endl;
					std::exit(0);
				}
			}
			if(msg.empty()) continue;
		}

		inSize += validSymbols * symbolWidth;
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
		if(!ometa.empty())
		{
			std::cerr << "Too many meta objects! Aborting." << std::endl;
			return EXIT_FAILURE;
		}
	}

	inSize *= sizeof(std::istream::char_type);

	// TODO encapsualte header, think of a better way to track inputSize
	writer.header.inputSize = inSize;

	if(inSize % symbolWidth != 0)
	{
		std::cerr
			<< "The given data does not evenly divide into symbols of width "
			<< mainCtx.ansTable.symbolWidth() << " bits. Aborting."
			<< std::endl;
		return EXIT_FAILURE;
	}

	if(opts->printSummary != ANS::driver::SummaryType::None)
	{
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

		auto entropy2percent = 100 / (symbolWidth << 3);

		auto dataWrittenBytes = dataWritten * sizeof(std::fstream::char_type);
		auto percentageFull	  = 100.0 * outSize / ((double) inSize);
		auto percentageData	  = 100.0 * dataWrittenBytes / ((double) inSize);
		auto percentageTheory = entropy * entropy2percent;

		auto entropyFull = percentageFull / entropy2percent;
		auto entropyData = percentageData / entropy2percent;

		auto entrUnit = " bits/byte";

		switch(opts->printSummary)
		{
			case ANS::driver::SummaryType::Human:
			{
				std::cout << std::setprecision(6);
				std::cout << "Done! Took " << timeS << "s \n";
				if(opts->skipUnknown && !opts->quiet)
				{
					std::cout
						<< "CAUTION: The compressor was allowed to skip "
						   "symbols, which is not reflected when calculating "
						   "theoretical entropy and compression ratio. This "
						   "may result in the calculated entropies/compression "
						   "ratios lower than the theoretical best. "
						   "Please keep that in mind when analysing the "
						   "results."
						<< std::endl;
				}
				std::cout << "Stats:"
						  << "\n";
				std::cout << "\t"
						  << "Input size:       " << inSize << "\n";
				std::cout << "\t"
						  << "Output size:      " << outSize << "\n";
				std::cout << "\t\t"
						  << "Header size:      "
						  << writer.getHeader().totalHeaderSize << "\n";
				std::cout << "\t\t"
						  << "Data size:      " << dataWrittenBytes << "\n";
				std::cout << "\t"
						  << "Ratio:\n"
						  << "\t\t Full file:        " << percentageFull
						  << "%\n"
						  << "\t\t Without Header:   " << percentageData
						  << "%\n"
						  << "\t\t Theoretical best: " << percentageTheory
						  << "%\n";
				std::cout << "\t"
						  << "Entropy:\n"
						  << "\t\t Full file:        " << entropyFull
						  << entrUnit << "\n"
						  << "\t\t Without Header:   " << entropyData
						  << entrUnit << "\n"
						  << "\t\t Theoretical best: " << entropy << entrUnit
						  << "\n";
				break;
			}
			case ANS::driver::SummaryType::CSV:
			{
				std::cout << std::setprecision(6);
				std::cout << timeS << ", " << inSize << ", " << outSize << ", "
						  << writer.getHeader().totalHeaderSize << ", "
						  << dataWrittenBytes << ", " << percentageFull << ", "
						  << percentageData << ", " << percentageTheory << ", "
						  << entropyFull << ", " << entropyData << ", "
						  << entropy;
				if(opts->skipUnknown && !opts->quiet)
				{
					std::cout
						<< ", CAUTION: The compressor was allowed to skip "
						   "symbols, which is not reflected when calculating "
						   "theoretical entropy and compression ratio. This "
						   "may result in the calculated entropies/compression "
						   "ratios lower than the theoretical best. "
						   "Please keep that in mind when analysing the "
						   "results."
						<< std::endl;
				} else
				{
					std::cout << std::endl;
				}

				break;
			}
			default: break;
		}
	}

	delete[] msgbuf;
	delete[] statebuf;

	return 0;
}

template <ANS::driver::Alphabet Alph>
int compressWrapperGenerate(ANS::driver::compress::OptionsP opts,
							std::istream& in,
							ANS::TableGeneratorOptions tableGenOpts)
{
	using SymbolT  = typename ANS::integer::fitting<(int) Alph>::type;
	using TableT   = ANS::DynamicCompressionTable<u32, u32>;
	using ContextT = ANS::ChannelCompressionContext<TableT, MyVector>;

	TableT table = ANS::generateTable<u32, u32>(in, tableGenOpts);
	in.seekg(0, std::ios_base::beg);
	in.clear();

	auto mainCtxPtr = std::make_shared<ContextT>(opts->channels, table);
	return compressTask<ContextT, SymbolT>(opts, in, mainCtxPtr);
}

template <ANS::driver::Alphabet Alph>
int compressWrapperLoad(ANS::driver::compress::OptionsP opts, std::istream& in)
{
	using SymbolT  = typename ANS::integer::fitting<(int) Alph>::type;
	using TableT   = ANS::DynamicCompressionTable<>;
	using ContextT = ANS::ChannelCompressionContext<TableT, MyVector>;
	std::ifstream tableFile(opts->tableFilePath);
	TableT table	= ANS::io::loadTable<u32, u32>(tableFile);
	auto mainCtxPtr = std::make_shared<ContextT>(opts->channels, table);
	return compressTask<ContextT, SymbolT>(opts, in, mainCtxPtr);
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
			ANS::ChannelCompressionContext<ANS::StaticCompressionTable,
										   MyVector>;
		auto mainCtxPtr = std::make_shared<ContextT>(opts->channels);
		return compressTask<ContextT, message_t>(opts, *in, mainCtxPtr);
	} else
	{
		if(opts->tableFilePath == "")
		{
			if(opts->inFilePath == "STDIN")
			{
				std::cerr
					<< "Cannot generate table for STDIN. Please save to a file "
					   "first or use a premade table."
					<< std::endl;
				return EXIT_FAILURE;
			}

			auto inSize = getFileSize(*in);
			if(inSize % ((u32) opts->alphabet >> 3) != 0)
			{
				std::cerr << "The given data does not evenly divide into "
							 "symbols of width "
						  << (u32) opts->alphabet << " bits. Aborting."
						  << std::endl;
				return EXIT_FAILURE;
			}

			auto tableGenOpts		  = TableGeneratorOptions();
			tableGenOpts.tableSizeLog = opts->tableSizeLog;
			tableGenOpts.symbolWidth  = (u32) opts->alphabet;
			switch(opts->alphabet)
			{
				case ANS::driver::Alphabet::U8:
				{
					return compressWrapperGenerate<ANS::driver::Alphabet::U8>(
						opts, *in, tableGenOpts);
				}
				case ANS::driver::Alphabet::U16:
				{
					return compressWrapperGenerate<ANS::driver::Alphabet::U16>(
						opts, *in, tableGenOpts);
				}
				default: break;
			}
		} else
		{
			if(opts->inFilePath == "STDIN" && !opts->quiet)
			{
				std::cout << "Compressing stdin. Input some data and then "
							 "press Ctrl+d"
						  << std::endl;
			}
			switch(opts->alphabet)
			{
				case ANS::driver::Alphabet::U8:
				{
					return compressWrapperLoad<ANS::driver::Alphabet::U8>(opts,
																		  *in);
				}
				case ANS::driver::Alphabet::U16:
				{
					return compressWrapperLoad<ANS::driver::Alphabet::U16>(opts,
																		   *in);
				}
				default: break;
			}
		}
	}

	return 0;
}
