#include "ansu.hpp"
#include "tester.hpp"

#include <fstream>
#include <string>

std::vector<String> tests = {
	"zeros",
	"4bytes",
	"16bytes",
	"16bytes_r",
	"24bytes",
	"37bytes",
	"hamlet",
	"macbeth",
	"thonk",
	"fox",
};

int main(int argc, char const* argv[])
{
	String location;
	if(argc == 2) { location = argv[1]; }

	bool hadErrors = false;

	hadErrors |= ANS::Test::runCompressionTests(location, tests);
#ifdef NO_VIVADO
	hadErrors |= ANS::Test::runDecompressionTests(location, tests);
#endif

	if(!hadErrors)
	{ std::cout << "*** ALL TESTS PASSED SUCCESSFULLY ***" << std::endl; }

	return hadErrors;
}
