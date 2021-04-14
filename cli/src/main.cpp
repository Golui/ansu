#include "CLI11.hpp"
#include "ansu.hpp"
#include "driver.hpp"

#include <string>

int main(int argc, char const* argv[])
{
	CLI::App app {"ansu - a tANS implementation targeting FPGAs."};

	std::string str = "NOPE";
	ANS::driver::compress::subRegister(app);
	ANS::driver::decompress::subRegister(app);
	app.require_subcommand(1);

	try
	{
		app.parse(argc, argv);
	} catch(const CLI::ParseError& e)
	{
		return app.exit(e);
	}

	return 0;
}
