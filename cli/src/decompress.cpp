#include "ansu.hpp"
#include "driver.hpp"
#include "fallocate.h"
#include "io/archive.hpp"
#include "mio/mio.hpp"

#include <iomanip>
#include <iostream>
#include <list>

#define FILE_SIZE_WARNING (256 * 1024 * 1024)

template <typename S, typename T>
void writeFile(S& os, T v)
{
	os.write((char*) &v, sizeof(T));
}

template <class Table>
int decompressTask(mio::mmap_sink& out,
				   ANS::io::ArchiveReader& reader,
				   const ANS::io::SharedHeader& head)
{
	using ContextT		= ANS::ChannelCompressionContext<Table>;
	using StateT		= typename ContextT::StateT;
	using MessageT		= typename ContextT::MessageT;
	using MessageIndexT = typename ContextT::MessageIndexT;
	using Meta			= typename ContextT::Meta;

	auto ctx = reader.readContext<ANS::ChannelCompressionContext<Table>>();

	StateT* readbuf = new StateT[head.blockSize];

	ANS::backend::stream<Meta> meta;
	ANS::backend::stream<MessageIndexT> message;

	u32 blockNum = 0;
	u64 lastRead = 0;
	lastRead	 = reader.readNextBlock(readbuf, meta);
	Meta tmpmeta = meta.read();
	ctx.initDecoders(tmpmeta);
	u64 writeOffset = head.inputSize;
	// clang-format off
	do
	// clang-format on
	{
		auto data	= ANS::backend::streamFromBufferReverse(readbuf, lastRead);
		auto result = ctx.decompress(data, meta, message);
		writeOffset = writeOffset - result;
		while(!message.empty())
		{
			out[writeOffset++] = ctx.ansTable.alphabet(message.read());
			//			std::cout << u16(out[writeOffset - 1]) << " ";
		}
		//		std::cout << std::endl;
		writeOffset -= result;
		if(writeOffset > head.inputSize)
		{
			std::cout << "Underflow in writeOffset. \n";
			break;
		}
		blockNum++;
	} while((lastRead = reader.readNextBlock(readbuf, meta)));

	if(writeOffset != 0)
	{
		std::cout << "Warning: Decompression ended prematurely, at offset "
				  << writeOffset << std::endl;
	}

	delete[] readbuf;

	return 0;
}

int handleMioError(std::error_code& error)
{
	std::cerr << "MMIO Error:\n" << error.message() << std::endl;
	return error.value();
}

int ANS::driver::decompress::run(OptionsP opts)
{
	ANS::io::ArchiveReader reader(opts->inFilePath);
	auto head = reader.getHeader();

	if(!opts->ignoreSizeWarning && head.inputSize > FILE_SIZE_WARNING)
	{
		std::cout << "WARNING: This operation will preallocate "
				  << head.inputSize << " bytes (~" << (head.inputSize >> 20)
				  << "MiB) of storage. Press Enter to continue, or any other "
					 "key to abort. You can ignore this warning by specifying "
					 "the \"--ignore-warning\" option."
				  << std::endl;
		if(std::cin.get() != '\n') return 0;
	}

	prepare_fallocate_wrapper();
	auto outHandle =
		open(opts->outFilePath.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0660);
	auto res = fallocate_wrapper(outHandle, head.inputSize);
	if(res != 0)
	{
		std::cerr << "Fallcoate error: " << res << std::endl;
		return 0;
	}

	std::error_code error;
	auto mmap = mio::make_mmap_sink(outHandle, 0, mio::map_entire_file, error);
	if(error) { return handleMioError(error); }

	int returnCode = 0;
	switch(head.tableType)
	{
		case ANS::tables::Static:
			returnCode =
				decompressTask<ANS::StaticCompressionTable>(mmap, reader, head);
			break;
		case ANS::tables::Dynamic:
			returnCode = decompressTask<ANS::DynamicCompressionTable<>>(
				mmap, reader, head);
			break;
		default:
			std::cerr << "Invalid table type detected! This is a bug!"
					  << std::endl;
			return -1;
	}

	mmap.sync(error);
	if(error) { return handleMioError(error); }

	mmap.unmap();
	return returnCode;
}
