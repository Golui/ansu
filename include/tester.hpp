#include "ints.hpp"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

#define OUT_DIR "res/out/"
#define GOLDEN_OUT_DIR "res/golden/"

using String = std::string;

String location = "./";

template <typename S, typename T>
void writeFile(S& os, T v)
{
	os.write((char*) &v, sizeof(T));
}

template <typename S, typename T>
void readFile(S& os, T& v)
{
	os.read((char*) &v, sizeof(T));
}

class Tester
{
public:
	String inn;
	String goutn;
	String gmetan;
	String outn;
	String metan;
	String testName;
	bool isVerbose;
	Tester(String location, String testName) : testName(testName)
	{
		this->inn		= location + "res/in/" + testName + ".vin";
		this->goutn		= location + GOLDEN_OUT_DIR + testName + ".vout";
		this->gmetan	= location + GOLDEN_OUT_DIR + testName + ".vmeta";
		this->outn		= location + OUT_DIR + testName + ".vout";
		this->metan		= location + OUT_DIR + testName + ".vmeta";
		this->isVerbose = false;
	}

	void setVerbose() { this->isVerbose = true; }

	int run(bool generate = false)
	{
		std::ios_base::fmtflags coutflags(std::cout.flags());

		std::ifstream in(this->inn, std::ios::binary);
		std::ofstream out(generate ? this->goutn : this->outn,
						  std::ios::binary);
		std::ofstream meta(generate ? this->gmetan : this->metan,
						   std::ios::binary);

		std::ifstream gout(this->goutn, std::ios::binary);
		std::ifstream gmeta(this->gmetan, std::ios::binary);

		if(!in || !out || !meta || !gout || !gmeta)
		{
			std::cerr << "One of the test files was not found." << std::endl;
			return 1;
		}

		hls::stream<message_t> message;
		hls::stream<state_t> encout;
		hls::stream<ANSMeta> encmeta;

		bool hadErrors = false, outOfBoundsRead = false;

		auto testOut = [&]() {
			int i = 0;
			if(this->isVerbose)
			{
				if(!encout.empty())
				{
					std::cout << "Encoded: ";
				} else
				{
					std::cout << "(Nothing emitted)";
				}
			}
			while(!encout.empty())
			{
				state_t cur, compare;
				encout >> cur;
				writeFile(out, cur);
				if(!generate)
				{
					if(gout)
					{
						readFile(gout, compare);
					} else
					{
						compare			= -1;
						outOfBoundsRead = hadErrors = true;
					}
				}

				std::cout << std::uppercase << std::hex;

				if(this->isVerbose)
				{
					std::cout << "0x" << std::setfill('0')
							  << std::setw(sizeof(state_t) * 2) << (int) cur
							  << " ";
					if(!generate && cur != compare)
					{
						std::cout << "(0x" << std::setfill('0')
								  << std::setw(sizeof(state_t) * 2)
								  << (int) compare << ") ";
					}

					if((i + 1) % (sizeof(state_t) / sizeof(message_t) * 4) == 0)
					{ std::cout << "\n         "; }
					i++;
				}

				if(!generate && cur != compare) { hadErrors = true; }
			}
			if(this->isVerbose) std::cout << std::endl;
		};

		auto testMeta = [&]() {
			while(!encmeta.empty())
			{
				ANSMeta cur, cmp;
				encmeta >> cur;
				writeFile(meta, cur);
				if(!generate)
				{
					if(gmeta)
						readFile(gmeta, cmp);
					else
					{
						cmp.control_state = -1;
						cmp.partial		  = -1;
						cmp.offset		  = -1;
						cmp.dead_bits	  = -1;
						outOfBoundsRead = hadErrors = true;
					}
				}

				if(this->isVerbose)
				{
					std::cout << std::dec;
					std::cout << (generate ? "Meta:" : "GOT:") << "\n"
							  << "\tState: " << (int) cur.control_state << "\n"
							  << "\tOffse: " << (int) cur.offset << "\n"
							  << "\tDeadB: " << (int) cur.dead_bits << "\n"
							  << "\tParti: " << std::hex << (int) cur.partial
							  << "\n";
					if(!generate)
					{
						std::cout << std::dec;
						std::cout
							<< "EXP: \n"
							<< "\tState: " << (int) cmp.control_state << "\n"
							<< "\tOffse: " << (int) cmp.offset << "\n"
							<< "\tDeadB: " << (int) cmp.dead_bits << "\n"
							<< "\tParti: " << std::hex << (int) cmp.partial
							<< "\n";
					}
				}

				if(!generate && cur != cmp) { hadErrors = true; }
			}
			if(this->isVerbose) std::cout << std::endl;
		};

		if(!generate)
		{
			std::cout << "*** RUNNING " << this->testName << " ***\n";
		} else
		{
			std::cout << "*** GENERATING " << this->testName << " ***\n";
		}

		u8 control = CONTROL_RESET_STATE | CONTROL_ENCODE;

		u32 dropBytes	 = 0;
		int blockCounter = 0;
		while(!in.eof())
		{
			control |= CONTROL_ENCODE;
			if(this->isVerbose)
			{
				std::cout << std::dec << "Block: " << blockCounter << "\n";
				std::cout << std::hex;
				blockCounter++;
			}
			int i = 0;
			for(; i < AVG_MESSAGE_LENGTH && in; i++)
			{
				message_t cur;
				readFile(in, cur);
				message << cur;

				if(this->isVerbose)
				{
					std::cout << std::uppercase << "0x" << std::setfill('0')
							  << std::setw(sizeof(message_t) * 2) << (int) cur
							  << " ";
					if((i + 1) % (sizeof(message_t) * 16) == 0)
					{ std::cout << "\n"; }
				}
			}

			if(this->isVerbose) std::cout << std::endl;

			dropBytes = (AVG_MESSAGE_LENGTH - i) * sizeof(message_t);
			for(; i < AVG_MESSAGE_LENGTH; i++) message << (message_t) 0;

			encode_stream(message, encout, encmeta, 0, control);

			testOut();
			testMeta();

			in.peek();
		}

		control = CONTROL_FLUSH;
		encode_stream(message, encout, encmeta, dropBytes, control);

		testOut();
		testMeta();

		if(!message.empty()) { std::cerr << "MESSAGE WAS NOT ALL CONSUMED\n"; }
		if(!encout.empty()) { std::cerr << "ENCOUT WAS NOT ALL CONSUMED\n"; }
		if(!encmeta.empty()) { std::cerr << "ENCMETA WAS NOT ALL CONSUMED\n"; }

		if(!generate)
		{
			if(hadErrors)
			{
				std::cerr << "!!! TEST " << this->testName << " FAILURE !!!"
						  << std::endl;
				if(outOfBoundsRead)
				{
					std::cerr << "!!! Generated too much data. !!!"
							  << std::endl;
				}
			} else
			{
				std::cout << "*** TEST " << this->testName << " PASS ***"
						  << std::endl;
			}
		}

		std::cout.flags(coutflags);

		return hadErrors;
	}
};
