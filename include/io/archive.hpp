#pragma once

#include "ansu.hpp"
#include "ints.hpp"
#include "microtar.h"

#include <array>
#include <exception>
#include <iostream>

namespace ANS
{
	namespace io
	{
		using DataBuffer = std::vector<state_t>;
		using MetaBuffer = std::vector<ANS::Meta>;

		struct TARException : public std::exception
		{
			u32 retcode;
			TARException(u32 retcode) : retcode(retcode) {}
			virtual const char* what() const throw()
			{
				return mtar_strerror(this->retcode);
			}
		};

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

			static ArchiveException wrongSizeMeta()
			{
				return ArchiveException("Meta block was not a multiple of "
										"metadata structure size!");
			}

			static ArchiveException missingHeader()
			{
				return ArchiveException("File header missing.");
			}

			static ArchiveException damagedArchive()
			{
				return ArchiveException(
					"Archive damaged: data or meta missing.");
			}
		};

		class ArchiveCommon
		{
		public:
			bool tarOpen = false;
			mtar_t tarFile;
			std::string filePath;
			std::string curBlockName = "";
			void safeOpen(const char* mode);
			void increment();
			void decrement();

			ArchiveCommon(std::string filePath) : filePath(filePath) {}
			virtual ~ArchiveCommon();
		};

		class ArchiveWriter : public ArchiveCommon
		{
			using StateIterator = DataBuffer::const_iterator;
			using MetaIterator	= MetaBuffer::const_iterator;

		public:
			ArchiveWriter(std::string filePath);

			/**
			 * Write compressed data (a block) to the archive.
			 * @param db Block of compressed data
			 * @param mb Accompanying metadata
			 */
			void writeBlock(DataBuffer db, MetaBuffer mb);
			void finalize();
		};

		class ArchiveReader : private ArchiveCommon
		{
		public:
			ArchiveReader(std::string filePath);

			/**
			 * Read a block from the archive into relevant streams.
			 * @param  data Compressed data stream to write to
			 * @param  meta Metadata stream to write to
			 * @return      does the archive have more blocks?
			 */
			bool readBlock(backend::stream<state_t>& data,
						   backend::stream<Meta>& meta);
		};
	} // namespace io
} // namespace ANS
