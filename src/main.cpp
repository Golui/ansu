#include "ansu.hpp"
#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include "tester.hpp"

String tests[] = {
	"zeros",
	"16bytes",
	"16bytes_r",
	"24bytes",
	"37bytes",
	// "256bytes",
	// "64kbytes"
};

int main(int argc, char const *argv[])
{
	// std::cout << argc;
	String location;
	if(argc == 2)
	{
		location = argv[1];
	}


	bool hadErrors = false;
	for(auto tname : tests)
	{
		Tester t(location, tname);
		t.setVerbose();
	#ifdef NO_VIVADO
		t.generate_vector();
		hadErrors |= t.run();
	#else
		hadErrors |= t.run();
	#endif
	}

	if(!hadErrors)
	{
		std::cout << "*** ALL TESTS PASSED SUCCESSFULLY ***" << std::endl;
	}

	return hadErrors;
}
