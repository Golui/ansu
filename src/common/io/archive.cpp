#include "ansu/io/archive.hpp"

using namespace ANS::io;

void ArchiveCommon::safeOpen(std::ios_base::openmode mode)
{
	this->fileHandle = std::fstream(this->filePath, std::ios::binary | mode);
}

ArchiveReader::ArchiveReader(std::string filePath)
	: ArchiveCommon(filePath, std::ios::in), inArchive(this->fileHandle)
{}
