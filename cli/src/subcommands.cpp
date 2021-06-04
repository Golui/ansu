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

	sub->add_option(
		"-c",
		opts->checkpoint,
		"Specfy how often a checkpoint should be emitted. This is used to "
		"verify data integrity, but increases the file size.");

	sub->add_option(
		"-n",
		opts->channels,
		"Specify how many channels (parallel coders) should be used.");

	sub->add_option(
		"-l", opts->chunkSize, "How much data to consume per function call.");

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

	sub->add_flag("--ignore-size-warn",
				  opts->ignoreSizeWarning,
				  "Ignore the filesize warning");

	sub->final_callback([opts]() {
		ANS::driver::decompress::run(opts);
	});
}
