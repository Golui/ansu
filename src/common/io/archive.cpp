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
	{
		this->curBlockName = "1" + this->curBlockName;
	}
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
	{
		this->curBlockName = this->curBlockName.substr(1);
	}
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
	auto size = db.size() * sizeof(DataBuffer::value_type);
	// TODO
	auto msize = mb.size() * (mb[0].size());

	mtar_write_file_header(&this->tarFile, this->curBlockName.c_str(), size);
	mtar_write_data(&this->tarFile, &db[0], size);

	std::string metaname = "m" + this->curBlockName;
	mtar_write_file_header(&this->tarFile, metaname.c_str(), msize);

	for(auto& met: mb)
	{
		mtar_write_data(&this->tarFile, &met.channels, sizeof(met.channels));
		mtar_write_data(
			&this->tarFile, &met.currentChannel, sizeof(met.currentChannel));
		mtar_write_data(
			&this->tarFile, &met.messagePad, sizeof(met.messagePad));
		for(auto& v: met.controlState)
		{
			mtar_write_data(&this->tarFile, &v, sizeof(v));
		}
		mtar_write_data(&this->tarFile, &met.offset, sizeof(met.offset));
		mtar_write_data(&this->tarFile, &met.deadBits, sizeof(met.deadBits));
	}
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

	// TODO Hardcoded for testing purposes
	Meta metaStruct(2);

	const u32 blockLength = (MESSAGES_PER_BLOCK + 2) * AVG_MESSAGE_LENGTH;
	const u32 metaSize	  = metaStruct.size();
	std::vector<state_t> dbuffer(blockLength);

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
		mtar_read_data(
			&this->tarFile, &metaStruct.channels, sizeof(metaStruct.channels));
		mtar_read_data(&this->tarFile,
					   &metaStruct.currentChannel,
					   sizeof(metaStruct.currentChannel));
		mtar_read_data(&this->tarFile,
					   &metaStruct.messagePad,
					   sizeof(metaStruct.messagePad));
		for(u32 j = 0; j < 2; j++)
		{
			mtar_read_data(
				&this->tarFile, &metaStruct.controlState[j], sizeof(state_t));
		}
		mtar_read_data(
			&this->tarFile, &metaStruct.offset, sizeof(metaStruct.offset));
		mtar_read_data(
			&this->tarFile, &metaStruct.deadBits, sizeof(metaStruct.deadBits));
		meta << metaStruct;
	}

	this->decrement();
	return true;
}
