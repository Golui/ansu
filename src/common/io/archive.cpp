#include "io/archive.hpp"

using namespace ANS::io;

void ArchiveCommon::safeOpen(const char* mode)
{
	u32 ret = mtar_open(&this->tarFile, this->filePath.c_str(), mode);
	if(ret != MTAR_ESUCCESS) { throw TARException(ret); }
	this->tarOpen = true;
}

void ArchiveCommon::increment()
{
	auto nextChar = [](char c) {
		if(c == '9') return 'a';
		if(c == 'f')
			return '0';
		else
			return (char) (c + 1);
	};

	if(this->curBlockName.empty())
	{
		this->curBlockName = "0";
		return;
	}

	auto it = this->curBlockName.rbegin();
	while(it != this->curBlockName.rend())
	{
		*it = nextChar(*it);
		if(*it != '0') break;
		it++;
	}

	if(this->curBlockName.front() == '0')
	{ this->curBlockName = "1" + this->curBlockName; }
}

void ArchiveCommon::decrement()
{
	auto prevChar = [](char c) {
		if(c == 'a') return '9';
		if(c == '0')
			return 'f';
		else
			return (char) (c - 1);
	};

	if(this->curBlockName.empty() || this->curBlockName == "0")
	{
		this->curBlockName = "";
		return;
	}

	auto it = this->curBlockName.rbegin();
	while(it != this->curBlockName.rend())
	{
		*it = prevChar(*it);
		if(*it != 'f') break;
		it++;
	}

	if(this->curBlockName.front() == '0')
	{ this->curBlockName = this->curBlockName.substr(1); }
}

ArchiveCommon::~ArchiveCommon()
{
	if(this->tarOpen) { mtar_close(&this->tarFile); }
}

ArchiveWriter::ArchiveWriter(std::string filePath) : ArchiveCommon(filePath)
{
	this->safeOpen("w");
}

void ArchiveWriter::writeBlock(DataBuffer db, MetaBuffer mb)
{
	if(!this->tarOpen) throw ArchiveException::invalid();
	this->increment();
	auto size  = db.size() * sizeof(DataBuffer::value_type);
	auto msize = mb.size() * sizeof(MetaBuffer::value_type);

	mtar_write_file_header(&this->tarFile, this->curBlockName.c_str(), size);
	mtar_write_data(&this->tarFile, &db[0], size);

	std::string metaname = "m" + this->curBlockName;
	mtar_write_file_header(&this->tarFile, metaname.c_str(), msize);
	mtar_write_data(&this->tarFile, &mb[0], msize);
}

void ArchiveWriter::finalize()
{
	if(!this->tarOpen) throw ArchiveException::invalid();

	mtar_write_file_header(&this->tarFile, ".hdr", this->curBlockName.size());
	mtar_write_data(
		&this->tarFile, this->curBlockName.c_str(), this->curBlockName.size());

	u32 ret = mtar_finalize(&this->tarFile);
	if(ret != MTAR_ESUCCESS) { throw TARException(ret); }
}

ArchiveReader::ArchiveReader(std::string filePath) : ArchiveCommon(filePath)
{
	this->safeOpen("r");

	mtar_header_t headerHeader;
	if(mtar_find(&this->tarFile, ".hdr", &headerHeader) != MTAR_ESUCCESS)
		throw ArchiveException::missingHeader();

	this->curBlockName = std::string(headerHeader.size, '0');
	mtar_read_data(
		&this->tarFile, (char*) this->curBlockName.c_str(), headerHeader.size);
}

bool ArchiveReader::readBlock(backend::stream<state_t>& data,
							  backend::stream<Meta>& meta)
{
	if(!this->tarOpen) throw ArchiveException::invalid();

	if(this->curBlockName.empty()) return false;

	mtar_header_t fheader;
	mtar_header_t mheader;

	const u32 blockLength = (MESSAGES_PER_BLOCK + 2) * AVG_MESSAGE_LENGTH;
	const u32 metaSize	  = sizeof(Meta);
	std::vector<state_t> dbuffer(blockLength);

	Meta metaStruct;

	std::cout << this->curBlockName << "\n";

	std::string mname = "m" + this->curBlockName;

	bool ret = mtar_find(&this->tarFile, this->curBlockName.c_str(), &fheader)
			   != MTAR_ESUCCESS;

	if(ret) { throw ArchiveException::damagedArchive(); }

	dbuffer.resize(fheader.size / sizeof(state_t));

	mtar_read_data(&this->tarFile, &dbuffer[0], fheader.size);
	for(auto it = dbuffer.rbegin(); it != dbuffer.rend(); it++) data << *it;

	bool mret =
		mtar_find(&this->tarFile, mname.c_str(), &mheader) != MTAR_ESUCCESS;

	if(mret) { throw ArchiveException::damagedArchive(); }

	if(mheader.size % metaSize != 0) throw ArchiveException::wrongSizeMeta();

	for(u32 i = 0; i < mheader.size / metaSize; i++)
	{
		mtar_read_data(&this->tarFile, &metaStruct, metaSize);
		meta << metaStruct;
	}

	this->decrement();
	return true;
}
