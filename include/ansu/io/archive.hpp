#pragma once

#include "ansu.hpp"
#include "data/compression_table.hpp"
#include "ints.hpp"
#include "io/serialization.hpp"

#include <array>
#include <cereal/archives/binary.hpp>
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

			u64 blocks;
			u16 dataTypeWidth;
			// Size in element count
			u64 blockSize;
			u64 finalBlockSize;
			u8 unusedBitsInFinalState;
			// Size in bytes
			u64 metaSize;
			// In bytes
			u64 totalHeaderSize;

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
				   totalHeaderSize);
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
				   totalHeaderSize);
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

		class ArchiveWriter : public ArchiveCommon
		{
			cereal::BinaryOutputArchive outArchive;
			bool hasHeader = false;
			SharedHeader header;

			void writeHeader()
			{
				if(!this->hasHeader) throw ArchiveException::invalid();
				this->fileHandle.seekp(0);
				if(!this->fileHandle.good()) throw ArchiveException::invalid();
				this->outArchive(this->header);
			}

		public:
			ArchiveWriter(std::string filePath);

			template <typename ContextT>
			void createHeader(ContextT& ctx)
			{
				this->hasHeader			   = true;
				this->header.tableType	   = typename ContextT::TableT().type();
				this->header.dataTypeWidth = sizeof(typename ContextT::StateT)
											 << 3;
				this->header.blockSize = CHECKPOINT;
				this->header.metaSize  = -1;

				// No need to return to where we were, this invalidates the
				// current contents of the archive anyway.
				this->fileHandle.flush();
				this->fileHandle.seekp(0);
				this->writeHeader();
				this->outArchive(ctx);

				this->header.totalHeaderSize = this->fileHandle.tellp();
			}

			template <typename Meta>
			void writeBlock(u64 blockSize, void* blockData, Meta& blockMeta)
			{
				if(!this->hasHeader) throw ArchiveException::invalid();
				if(!this->fileHandle.good()) throw ArchiveException::invalid();
				this->header.blocks++;
				u64 writeSize = blockSize * (this->header.dataTypeWidth >> 3)
								/ sizeof(std::fstream::char_type);
				this->fileHandle.write((std::fstream::char_type*) blockData,
									   writeSize);
				if(this->header.metaSize
				   == (decltype(this->header.metaSize))(-1))
				{
					auto pos = this->fileHandle.tellg();
					this->outArchive(blockMeta);
					this->header.metaSize = (this->fileHandle.tellg() - pos)
											* sizeof(std::fstream::char_type);
				} else
				{
					this->outArchive(blockMeta);
				}
			}

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
			SharedHeader header;

		public:
			ArchiveReader(std::string filePath);
			template <typename Table>
			void readTable(Table& tbl)
			{
				auto pos = this->fileHandle.tellg();
				this->fileHandle.seekg(this->tableOffset);
				if(!this->fileHandle.good()) throw ArchiveException::invalid();
				this->inArchive(tbl);
				this->fileHandle.seekg(pos);
			}

			/**
			 * Read a block from the archive into relevant streams.
			 * @param  data Compressed data stream to write to
			 * @param  meta Metadata stream to write to
			 * @return      does the archive have more blocks?
			 */
			template <typename Meta>
			bool readNextBlock(void* data, Meta& meta)
			{
				if(this->currentBlock == -1) return false;
				auto ret = readBlock(this->currentBlock, data, meta);
				this->currentBlock--;
				return ret;
			}

			template <typename Meta>
			bool readBlock(u64 blockNumber, void* data, Meta& meta)
			{
				auto blockData = this->header.blockSize
								 * (this->header.dataTypeWidth >> 3)
								 / sizeof(std::fstream::char_type);
				this->fileHandle.seekg(
					this->header.totalHeaderSize
					+ blockNumber * (blockData + this->header.metaSize));
				if(!this->fileHandle.good()) throw ArchiveException::invalid();
				this->fileHandle.read((std::fstream::char_type*) data,
									  blockData);
				this->inArchive(meta);
				return true;
			}
		};
	} // namespace io
} // namespace ANS
