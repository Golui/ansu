#include "io/archive.hpp"

using namespace ANS::io;

void ArchiveCommon::safeOpen(std::ios_base::openmode mode)
{
	this->fileHandle = std::fstream(this->filePath, std::ios::binary | mode);
}

ArchiveWriter::ArchiveWriter(std::string filePath)
	: ArchiveCommon(filePath, std::ios::out | std::ios::in | std::ios::trunc),
	  outArchive(this->fileHandle)
{}

ArchiveReader::ArchiveReader(std::string filePath)
	: ArchiveCommon(filePath, std::ios::in), inArchive(this->fileHandle)
{
	if(!this->fileHandle.good()) throw ArchiveException::invalid();
	this->inArchive(this->header);
	this->tableOffset  = this->fileHandle.tellg();
	this->currentBlock = this->header.blocks - 1;
}
