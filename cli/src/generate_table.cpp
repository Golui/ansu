#include "data/table_generator.hpp"
#include "driver.hpp"
#include "io/table_archive.hpp"

#include <fstream>

int ANS::driver::generate::run(OptionsP opts)
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
	std::ofstream out(opts->outFilePath);

	auto tableGenOpts = TableGeneratorOptions();

	tableGenOpts.tableSizeLog = opts->tableSizeLog;
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

	auto table = ANS::generateTable<u32, u8>(*in, tableGenOpts);
	ANS::io::saveTable(out, table);

	return 0;
}
