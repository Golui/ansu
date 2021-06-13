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
	tableGenOpts.symbolWidth  = (u32) opts->alphabet;

	if(opts->inFilePath == "STDIN" && !opts->skipPrompt)
	{
		std::cout << "Compressing stdin. Input some data and then "
					 "press Ctrl+d"
				  << std::endl;
	}
	// TODO
	auto table = ANS::generateTable<u32, u32>(*in, tableGenOpts);
	ANS::io::saveTable(out, table);

	return 0;
}
