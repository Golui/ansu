#include "driver.hpp"

struct SpecialValidator : public CLI::Validator
{
	SpecialValidator()
	{
		name_ = "ANSUSPECIAL";
		func_ = [](const std::string& str) {
			if(str != "" && str != "static")
				return std::string("Not a special table type.");
			return std::string("");
		};
	}
};

void ANS::driver::compress::subRegister(CLI::App& app)
{
	CLI::App* sub = app.add_subcommand("compress");

	auto opts = std::make_shared<Options>();

	sub->add_option("-f,--infile",
					opts->inFilePath,
					"the file to compress. Use \"STDIN\" to capture data from "
					"the standard input.")
		->check(SpecialValidator() | CLI::ExistingFile);

	sub->add_option("-o,--outfile", opts->outFilePath, "the resulting archive");

	sub->add_option(
		"-t",
		opts->tableFilePath,
		"Path to the table file. If omitted, a table will be generated for the "
		"input file. (Legacy) If set to \"static\", table present in this "
		"version of ANSU will be used.");

	sub->add_flag("-s",
				  opts->printSummary,
				  "Whether to print a short summary after compressing or not.");

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

	sub->add_option(
		   "-a", opts->alphabet, "The length of the alphabet when generating")
		->transform(
			CLI::CheckedTransformer(ALPHABET_STR_TO_ENUM, CLI::ignore_case));

	sub->add_option("-k", opts->tableSizeLog, "Logarithm of the table size.");

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

void ANS::driver::generate::subRegister(CLI::App& app)
{
	CLI::App* sub = app.add_subcommand("generate");

	auto opts = std::make_shared<Options>();

	sub->add_option("-f,--infile",
					opts->inFilePath,
					"the file to compress. Use \"STDIN\" to capture data from "
					"the standard input.")
		->check(SpecialValidator() | CLI::ExistingFile);

	sub->add_option("-o,--outfile", opts->outFilePath, "the resulting archive");

	sub->add_option(
		   "-a", opts->alphabet, "The length of the alphabet when generating")
		->transform(
			CLI::CheckedTransformer(ALPHABET_STR_TO_ENUM, CLI::ignore_case));

	sub->add_option("-k", opts->tableSizeLog, "Logarithm of the table size.");

	sub->final_callback([opts]() {
		ANS::driver::generate::run(opts);
	});
}
