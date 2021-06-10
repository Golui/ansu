#pragma once

#include "ansu.hpp"
#include "data/compression_table.hpp"
#include "ints.hpp"
#include "io/serialization.hpp"

#include <array>
#include <cereal/cereal.hpp>
#include <exception>
#include <fstream>
#include <iostream>

namespace ANS
{
	namespace io
	{
		constexpr static u32 ANSU_MAGIC =
			'U' << 24 | 'S' << 16 | 'N' << 8 | 'A';

		class ArchiveException : public std::exception
		{
			std::string reason;
			ArchiveException(std::string what) : reason(what) {}
			virtual const char* what() const throw()
			{
				return this->reason.c_str();
			}

		public:
			static ArchiveException invalid()
			{
				return ArchiveException("Archive is in an invalid state!");
			}

			static ArchiveException wrongSizeData()
			{
				return ArchiveException(
					"Data block was not a multiple of message size!");
			}

			static ArchiveException missingHeader()
			{
				return ArchiveException("File header missing.");
			}

			static ArchiveException spellUnsuccessful()
			{
				return ArchiveException(
					"Archive damaged: Magic bytes do not spell ANSU");
			}

			static ArchiveException damagedArchive()
			{
				return ArchiveException(
					"Archive damaged: data or meta missing.");
			}
		};

		// Current idea: Reserve space for the header. Write the blocks, without
		// resetting the compressor state. Previously blocks were actual
		// entities, now the "block size" is how often the meta is emitted.
		struct SharedHeader
		{
			u64 version = 1;
			tables::Type tableType;

			u64 blocks		  = 0;
			u16 dataTypeWidth = 0;
			// Size in element count
			u64 blockSize	   = 0;
			u64 finalBlockSize = 0;
			// Size in bytes
			u64 metaSize = 0;
			// In bytes
			u64 totalHeaderSize = 0;

			u64 inputSize = 0;

			template <typename Archive>
			void save(Archive& ar) const
			{
				u32 magic = ANSU_MAGIC;
				ar(magic,
				   version,
				   tableType,
				   blocks,
				   dataTypeWidth,
				   blockSize,
				   finalBlockSize,
				   metaSize,
				   totalHeaderSize,
				   inputSize);
			}

			template <typename Archive>
			void load(Archive& ar)
			{
				u32 magic;
				ar(magic);
				if(magic != ANSU_MAGIC)
					throw ArchiveException::spellUnsuccessful();
				ar(version,
				   tableType,
				   blocks,
				   dataTypeWidth,
				   blockSize,
				   finalBlockSize,
				   metaSize,
				   totalHeaderSize,
				   inputSize);
			}
		};

		class ArchiveCommon
		{
		public:
			std::fstream fileHandle;
			std::string filePath;
			void safeOpen(std::ios_base::openmode mode);
			void increment();
			void decrement();

			ArchiveCommon(std::string filePath, std::ios_base::openmode mode)
				: filePath(filePath)
			{
				this->safeOpen(mode);
			}

			virtual ~ArchiveCommon() {}
		};

		template <typename ContextT>
		class ArchiveWriter : public ArchiveCommon
		{
			cereal::BinaryOutputArchive outArchive;
			bool hasHeader = false;
			std::shared_ptr<ContextT> context;

			void writeHeader()
			{
				if(this->context.get() == 0) throw ArchiveException::invalid();
				this->fileHandle.seekp(0);
				if(!this->fileHandle.good()) throw ArchiveException::invalid();
				this->outArchive(this->header);
				this->outArchive(*this->context);
			}

		public:
			SharedHeader header;
			using Meta = typename ContextT::Meta;

			ArchiveWriter(std::string filePath)
				: ArchiveCommon(filePath,
								std::ios::out | std::ios::in | std::ios::trunc),
				  outArchive(this->fileHandle)
			{}

			bool isValid()
			{
				return this->hasHeader && this->context.get() != 0;
			}

			void bindContext(std::shared_ptr<ContextT> ctx)
			{
				this->context = ctx;

				this->header.tableType	   = typename ContextT::TableT().type();
				this->header.dataTypeWidth = sizeof(typename ContextT::StateT)
											 << 3;
				this->header.blockSize = this->context->checkpointFrequency();
				this->header.inputSize = 0;
				this->header.metaSize  = -1;

				// No need to return to where we were, this invalidates the
				// current contents of the archive anyway.
				this->fileHandle.seekp(0);
				this->fileHandle.seekg(0);
				this->outArchive(this->context->createMeta());
				this->header.metaSize =
					this->fileHandle.tellp() * sizeof(std::fstream::char_type);
				this->writeHeader();
				this->hasHeader				 = true;
				this->header.totalHeaderSize = this->fileHandle.tellp();
			}

			u64 writeBlock(u64 blockSize, void* blockData, Meta& blockMeta)
			{
				if(!this->isValid()) throw ArchiveException::invalid();
				if(!this->fileHandle.good()) throw ArchiveException::invalid();
				this->header.blocks++;
				u64 writeSize = blockSize * (this->header.dataTypeWidth >> 3)
								/ sizeof(std::fstream::char_type);
				auto pos = this->fileHandle.tellg();
				this->fileHandle.write((std::fstream::char_type*) blockData,
									   writeSize);
				this->outArchive(blockMeta);
				this->header.finalBlockSize = blockSize;
				return this->fileHandle.tellg() - pos;
			}

			const SharedHeader getHeader() { return this->header; }

			~ArchiveWriter()
			{
				auto pos = this->fileHandle.tellp();
				this->writeHeader();
				this->fileHandle.seekp(pos);
			}
		};

		class ArchiveReader : private ArchiveCommon
		{
			cereal::BinaryInputArchive inArchive;

			u64 currentBlock = -1;
			std::fstream::off_type tableOffset;
			bool hasReadHeader = false;
			SharedHeader header;

		public:
			ArchiveReader(std::string filePath);

			const SharedHeader getHeader()
			{
				if(!this->hasReadHeader)
				{
					this->fileHandle.seekg(0);
					if(!this->fileHandle.good())
						throw ArchiveException::invalid();
					this->inArchive(this->header);
					this->tableOffset	= this->fileHandle.tellg();
					this->currentBlock	= this->header.blocks - 1;
					this->hasReadHeader = true;
				}
				return this->header;
			}

			template <class Context>
			Context readContext()
			{
				auto ctx = Context();
				auto pos = this->fileHandle.tellg();
				this->fileHandle.seekg(this->tableOffset);
				if(!this->fileHandle.good()) throw ArchiveException::invalid();
				this->inArchive(ctx);
				ctx.setCheckpointFrequency(header.blockSize);
				this->fileHandle.seekg(pos);
				return ctx;
			}

			/**
			 * Read a block from the archive into relevant streams.
			 * @param  data Compressed data stream to write to
			 * @param  meta Metadata stream to write to
			 * @return      does the archive have more blocks?
			 */
			template <typename Meta>
			u64 readNextBlock(void* data, Meta& meta)
			{
				if(this->currentBlock == ((decltype(this->currentBlock))(-1)))
					return false;
				auto ret = readBlock(this->currentBlock, data, meta);
				this->currentBlock--;
				return ret;
			}

			template <typename Meta>
			u64
			readBlock(u64 blockNumber, void* data, backend::stream<Meta>& meta)
			{
				constexpr auto charSize = sizeof(std::fstream::char_type);
				u64 blockData;
				if(blockNumber == this->header.blocks - 1)
				{
					blockData = this->header.finalBlockSize;
				} else
				{
					blockData = this->header.blockSize;
				}
				blockData *= (this->header.dataTypeWidth >> 3) / charSize;

				this->fileHandle.seekg(
					this->header.totalHeaderSize
					+ blockNumber
						  * (this->header.blockSize
								 * (this->header.dataTypeWidth >> 3) / charSize
							 + this->header.metaSize));

				if(!this->fileHandle.good()) throw ArchiveException::invalid();
				auto dataReadOffset = this->fileHandle.tellg();
				this->fileHandle.read((std::fstream::char_type*) data,
									  blockData);
				auto read = this->fileHandle.gcount();
				Meta m;
				this->inArchive(m);
				meta << m;
				//		std::cout << " THS: " << this->header.totalHeaderSize
				//				  << " Block: " << blockNumber
				//				  << " Offset: " << dataReadOffset
				//				  << " BS: " << blockData << " Read: "
				//				  << (this->fileHandle.tellg() - dataReadOffset)
				//				  << " bytes. "
				//				  << " Meta had " << m.channels << " ("
				//				  << m.controlState.size() << ") channels" <<
				// std::endl;
				return read * charSize / (this->header.dataTypeWidth >> 3);
			}
		};
	} // namespace io
} // namespace ANS
