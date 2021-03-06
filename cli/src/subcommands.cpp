#include "driver.hpp"

void ANS::driver::compress::subRegister(CLI::App& app)
{
	CLI::App* sub = app.add_subcommand("compress");

	auto opts = std::make_shared<Options>();

	sub->add_option("infile", opts->inFilePath, "the file to compress")
		->required()
		->check(CLI::ExistingFile);

	sub->add_option("outfile", opts->outFilePath, "the resultin archive")
		->required();

	sub->final_callback([opts]() {
		ANS::driver::compress::run(opts);
	});
}

void ANS::driver::decompress::subRegister(CLI::App& app)
{
	CLI::App* sub = app.add_subcommand("decompress");

	auto opts = std::make_shared<Options>();

	sub->add_option("infile", opts->inFilePath, "the file to decompress")
		->required()
		->check(CLI::ExistingFile);

	sub->add_option("outfile", opts->outFilePath, "the decompressed data")
		->required();

	sub->final_callback([opts]() {
		ANS::driver::decompress::run(opts);
	});
}
