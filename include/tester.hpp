#include "ints.hpp"
#include <fstream>
#include <string>

using String = std::string;

String location = "./";

template <typename S, typename T>
void writeFile(S& os, T v)
{
	os.write((char*)&v, sizeof(T));
}

template <typename S, typename T>
void readFile(S& os, T& v)
{
	os.read((char*)&v, sizeof(T));
}

class Tester
{
public:
	String inn;
	String outn;
	String metan;
	String testName;
	bool isVerbose;
	Tester(String location, String testName) : testName(testName)
	{
		this->inn = location + "res/in/" + testName + ".vin";
		this->outn = location + "res/golden/" + testName + ".vout";
		this->metan =  location + "res/golden/" + testName + ".vmeta";
		this->isVerbose = false;
	}

	void setVerbose()
	{
		this->isVerbose = true;
	}

	void generate_vector()
	{
		std::ios_base::fmtflags coutflags(std::cout.flags());

		// char cwd[512];
		// std::cout << getcwd(cwd, 512) << std::endl;
		// chdir(location.c_str());

		std::ifstream in(this->inn, std::ios::binary);
		std::ofstream out(this->outn, std::ios::binary);
		std::ofstream meta(this->metan, std::ios::binary);

		// chdir(cwd);

		if(!in || !out || !meta)
		{
			std::cerr << "One of the test files was not found." << std::endl;
			return;
		}

		hls::stream<message_t> message;
		hls::stream<state_t> encout;
		hls::stream<ANSMeta> encmeta;

		auto saveOut = [&](){
			int i = 0;
			if(this->isVerbose)
			{
				if(!encout.empty())
				{
					std::cout << "Encoded: ";
				}else
				{
					std::cout << "(Nothing emitted)";
				}
			}
			while(!encout.empty())
			{
				state_t cur;
				encout >> cur;
				writeFile(out, cur);

				std::cout << std::uppercase << std::hex;

				if(this->isVerbose)
				{
					std::cout << "0x" << (int)cur << " ";
					if(i + 1 % 8 == 0)
					{
						std::cout << "\n         ";
					}
				}
			}
			std::cout << std::endl;
		};

		auto saveMeta = [&](){
			while(!encmeta.empty())
			{
				ANSMeta cur;
				encmeta >> cur;
				writeFile(meta, cur);

				if(this->isVerbose)
				{
					std::cout << std::dec;
					std::cout << "Meta: \n"
							  << "\tState: " << (int)cur.control_state << "\n"
							  << "\tOffse: " << (int)cur.offset << "\n"
							  << "\tDeadB: " << (int)cur.dead_bits << "\n"
							  << "\tParti: " << std::hex << (int)cur.partial << "\n";
				}
			}
			std::cout << std::endl;
		};

		std::cout << "---- GENERATING VECTOR FOR " << this->testName << " ----\n";

		u8 control = CONTROL_RESET_STATE | CONTROL_ENCODE;

		int blockCounter = 0;
		while(!in.eof())
		{
			control |= CONTROL_ENCODE;
			if(this->isVerbose)
			{
				std::cout << "Block: " << blockCounter << "\n";
				blockCounter++;
			}
			for(int i = 0; i < AVG_MESSAGE_LENGTH && in; i++)
			{
				message_t cur;
				readFile(in, cur);
				message << cur;

				if(this->isVerbose)
				{
					std::cout << std::hex << (int)cur << " ";
					if(i + 1 % 8 == 0)
					{
						std::cout << "\n";
					}
				}
			}

			std::cout << std::endl;

			encode_stream(message, encout, encmeta, control);

			saveOut();
			saveMeta();

			in.peek();

		}

		control |= CONTROL_FLUSH;
		encode_stream(message, encout, encmeta, control);

		saveOut();
		saveMeta();

		std::cout.flags(coutflags);
	}

	int run()
	{
		std::ios_base::fmtflags coutflags(std::cout.flags());

		// char cwd[512];
		// std::cout << getcwd(cwd, 512) << std::endl;
		// chdir(location.c_str());

		std::ifstream in(this->inn, std::ios::binary);
		std::ifstream out(this->outn, std::ios::binary);
		std::ifstream meta(this->metan, std::ios::binary);

		// chdir(cwd);

		if(!in || !out || !meta)
		{
			std::cerr << "One of the test files was not found." << std::endl;
			return 1;
		}

		hls::stream<message_t> message;
		hls::stream<state_t> encout;
		hls::stream<ANSMeta> encmeta;

		bool hadErrors = false, outOfBoundsRead = false;

		auto testOut = [&](){
			int i = 0;
			if(this->isVerbose)
			{
				if(!encout.empty())
				{
					std::cout << "Encoded: ";
				}else
				{
					std::cout << "(Nothing emitted)";
				}
			}
			while(!encout.empty())
			{
				state_t cur, compare;
				encout >> cur;
				if(out) readFile(out, compare);
				else
				{
					compare = -1;
					outOfBoundsRead = hadErrors = true;
				}

				std::cout << std::uppercase << std::hex;

				if(this->isVerbose)
				{
					std::cout << "0x" << (int)cur << " ";
					if(cur != compare)
					{
						std::cout << "(0x" << (int)compare << ") ";
					}

					if(i + 1 % 8 == 0)
					{
						std::cout << "\n         ";
					}
				}

				if(cur != compare)
				{
					hadErrors = true;
				}
			}
			std::cout << std::endl;
		};

		auto testMeta = [&](){

			while(!encmeta.empty())
			{
				ANSMeta cur, compare;
				encmeta >> cur;
				if(meta) readFile(meta, compare);
				else
				{
					compare.control_state = -1;
					compare.partial = -1;
					compare.offset = -1;
					compare.dead_bits = -1;
					outOfBoundsRead = hadErrors = true;
				}

				if(this->isVerbose)
				{
					std::cout << std::dec;
					std::cout << "GOT: \n"
							  << "\tState: " << (int)cur.control_state << "\n"
							  << "\tOffse: " << (int)cur.offset << "\n"
							  << "\tDeadB: " << (int)cur.dead_bits << "\n"
							  << "\tParti: " << std::hex << (int)cur.partial << "\n";
					std::cout << std::dec;
					std::cout << "EXP: \n"
							  << "\tState: " << (int)compare.control_state << "\n"
							  << "\tOffse: " << (int)compare.offset << "\n"
							  << "\tDeadB: " << (int)compare.dead_bits << "\n"
							  << "\tParti: " << std::hex << (int)compare.partial << "\n";
				}

				if(cur != compare)
				{
					hadErrors = true;
				}
			}
			std::cout << std::endl;
		};

		std::cout << "*** RUNNING " << this->testName << " ***\n";

		u8 control = CONTROL_RESET_STATE | CONTROL_ENCODE;

		int blockCounter = 0;
		while(!in.eof())
		{
			control |= CONTROL_ENCODE;
			if(this->isVerbose)
			{
				std::cout << "Block: " << blockCounter << "\n";
				blockCounter++;
			}
			for(int i = 0; i < AVG_MESSAGE_LENGTH && in; i++)
			{
				message_t cur;
				readFile(in, cur);
				message << cur;

				if(this->isVerbose)
				{
					std::cout << std::hex << (int)cur << " ";
					if(i + 1 % 8 == 0)
					{
						std::cout << "\n";
					}
				}
			}

			std::cout << std::endl;

			encode_stream(message, encout, encmeta, control);

			testOut();
			testMeta();

			in.peek();
		}

		control |= CONTROL_FLUSH;
		encode_stream(message, encout, encmeta, control);

		testOut();
		testMeta();

		std::cout.flags(coutflags);

		if(hadErrors)
		{
			std::cerr << "!!! TEST " << this->testName << " FAILURE !!!" << std::endl;
			if(outOfBoundsRead)
			{
				std::cerr << "!!! Generated too much data. !!!" << std::endl;
			}
		}else
		{
			std::cout << "*** TEST " << this->testName << " PASS ***" << std::endl;
		}

		return hadErrors;
	}

};
