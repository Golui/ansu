#include "ansu.hpp"
#include "tester.hpp"

#include <fstream>
#include <string>

String tests[] = {
	"zeros",
	"16bytes",
	"16bytes_r",
	"24bytes",
	"37bytes",
	"hamlet",
	"macbeth",
	"thonk",
};

int main(int argc, char const* argv[])
{
	String location;
	if(argc == 2) { location = argv[1]; }

	bool hadErrors = false;
	for(auto tname: tests)
	{
		Tester t(location, tname);
		t.setVerbose();
#ifdef NO_VIVADO
		t.run(true);
		hadErrors |= t.run();
#else
		hadErrors |= t.run();
#endif
	}

	if(!hadErrors)
	{ std::cout << "*** ALL TESTS PASSED SUCCESSFULLY ***" << std::endl; }

	return hadErrors;
}
