#include "data/table_generator.hpp"
#include "driver.hpp"
#include "io/table_archive.hpp"

#include <fstream>

int ANS::driver::generate::run(OptionsP opts)
{
	std::ifstream in(opts->inFilePath);
	std::ofstream out(opts->outFilePath);

	auto tablGenOpts = TableGeneratorOptions();
	auto table		 = ANS::generateTable<u32, u32>(in, tablGenOpts);

	ANS::io::saveTable(out, table);

	return 0;
}
