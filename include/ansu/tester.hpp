#include "backend/hls/ansu.ipp"
#include "ints.hpp"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <stack>
#include <string>
#include <vector>

#define IN_DIR "res/in/"
#define COMPRESSED_OUT_DIR "res/compressed/"
#define DECOMPRESSED_OUT_DIR "res/decompressed/"
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
	os.peek();
}

#ifdef SOFTWARE
#	define __compress_function ANS::compress
#else
#	define __compress_function hls_compress
#endif

namespace ANS
{
	namespace Test
	{
		class Compression
		{
		public:
			String inn;
			String goutn;
			String gmetan;
			String outn;
			String metan;
			String testName;
			bool isVerbose;

			Compression(String location, String testName) : testName(testName)
			{
				this->inn	 = location + IN_DIR + testName + ".vin";
				this->goutn	 = location + GOLDEN_OUT_DIR + testName + ".vout";
				this->gmetan = location + GOLDEN_OUT_DIR + testName + ".vmeta";
				this->outn = location + COMPRESSED_OUT_DIR + testName + ".vcmp";
				this->metan =
					location + COMPRESSED_OUT_DIR + testName + ".vmeta";
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
					std::cerr << "One of the test files was not found."
							  << std::endl;
					return 1;
				}

				backend::stream<message_t> message;
				backend::stream<state_t> encout;
				backend::stream<ANS::Meta> encmeta;

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
									  << std::setw(sizeof(state_t) * 2)
									  << (int) cur << " ";
							if(!generate && cur != compare)
							{
								std::cout << "(0x" << std::setfill('0')
										  << std::setw(sizeof(state_t) * 2)
										  << (int) compare << ") ";
							}

							if((i + 1)
								   % (sizeof(state_t) / sizeof(message_t) * 4)
							   == 0)
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
						ANS::Meta cur, cmp;
						encmeta >> cur;
						writeFile(meta, cur);
						if(!generate)
						{
							if(gmeta)
								readFile(gmeta, cmp);
							else
							{
								cmp.channels = -1;
								// outOfBoundsRead = hadErrors = true;
							}
						}

						if(this->isVerbose)
						{
							std::cout << std::dec;
							std::cout
								<< (generate ? "Meta:" : "GOT:") << "\n"
								<< "\tChannel Count: " << (int) cur.channels
								<< "\n"
								<< "\tFin Channel: "
								<< (int) cur.current_channel << "\n"
								<< "\tOffse: " << (int) cur.offset << "\n"
								<< "\tPadding: " << (int) cur.message_pad
								<< "\n"
								<< "\tDeadB: " << (int) cur.dead_bits << "\n"
								<< "\tStates:\n";
							for(int i = 0; i < CHANNEL_COUNT; i++)
							{
								std::cout << "\t " << (i + 1) << ". "
										  << cur.control_state[i] << "\n";
							}
							std::cout << "\n";
							if(!generate)
							{
								std::cout << std::dec;
								std::cout
									<< "EXP: \n"
									<< "\tChannel Count: " << (int) cmp.channels
									<< "\n"
									<< "\tFin Channel: "
									<< (int) cmp.current_channel << "\n"
									<< "\tOffse: " << (int) cmp.offset << "\n"
									<< "\tPadding: " << (int) cmp.message_pad
									<< "\n"
									<< "\tDeadB: " << (int) cmp.dead_bits
									<< "\n"
									<< "\tStates:\n";
								for(int i = 0; i < CHANNEL_COUNT; i++)
								{
									std::cout << "\t " << (i + 1) << ". "
											  << cmp.control_state[i] << "\n";
								}
								std::cout << "\n";
							}
						}

						// if(!generate && cur != cmp)
						// {
						// 	std::cout << "Metadata mismatch!\n";
						// 	hadErrors = true;
						// }
					}
					if(this->isVerbose) std::cout << std::endl;
				};

				if(!generate)
				{
					std::cout << "*** RUNNING " << this->testName << " ***\n";
				} else
				{
					std::cout << "*** GENERATING " << this->testName
							  << " ***\n";
				}

				u8 control = CONTROL_RESET_STATE | CONTROL_ENCODE;

				u32 padding		 = 0;
				int blockCounter = 0;
				in.peek();
				while(!in.eof())
				{
					control |= CONTROL_ENCODE;
					if(this->isVerbose)
					{
						std::cout << std::dec << "Block: " << blockCounter
								  << "\n";
						std::cout << std::hex;
						blockCounter++;
					}
					int i = 0;
					for(; (i < AVG_MESSAGE_LENGTH) && !in.eof(); i++)
					{
						message_t cur = 0xFF;
						readFile(in, cur);
						message << cur;

						if(this->isVerbose)
						{
							std::cout
								<< std::uppercase << "0x" << std::setfill('0')
								<< std::setw(sizeof(message_t) * 2) << (int) cur
								<< " ";
							if((i + 1) % (sizeof(message_t) * 16) == 0)
							{ std::cout << "\n"; }
						}
					}

					if(this->isVerbose) std::cout << std::endl;

					padding = (AVG_MESSAGE_LENGTH - i);
					for(; i < AVG_MESSAGE_LENGTH; i++) message << (message_t) 0;

					__compress_function(message, encout, encmeta, 0, control);

					testOut();
					testMeta();
				}

				control = CONTROL_FLUSH;
				__compress_function(message, encout, encmeta, padding, control);

				testOut();
				testMeta();

				if(!message.empty())
				{ std::cerr << "MESSAGE WAS NOT ALL CONSUMED\n"; }
				if(!encout.empty())
				{ std::cerr << "ENCOUT WAS NOT ALL CONSUMED\n"; }
				if(!encmeta.empty())
				{ std::cerr << "ENCMETA WAS NOT ALL CONSUMED\n"; }

				if(!generate)
				{
					if(hadErrors)
					{
						std::cerr << "!!! TEST " << this->testName
								  << " FAILURE !!!" << std::endl;
						if(outOfBoundsRead)
						{
							std::cerr << "!!! Generated too much data. !!!"
									  << std::endl;
						}
					} else
					{
						std::cout << "*** TEST " << this->testName
								  << " PASS ***" << std::endl;
					}
				}

				std::cout.flags(coutflags);

				return hadErrors;
			}
		};

		bool runCompressionTests(String location, std::vector<String> cases)
		{
			bool hadErrors = false;
			for(auto tname: cases)
			{
				Compression t(location, tname);
				t.setVerbose();
#ifdef SOFTWARE
				t.run(true);
				hadErrors |= t.run();
#else
				hadErrors |= t.run();
#endif
			}

			return hadErrors;
		}

#ifdef SOFTWARE

		class Decompression
		{
		public:
			String inn;
			String inmn;
			String goutn;
			String outn;
			String testName;
			bool isVerbose;

			Decompression(String location, String testName) : testName(testName)
			{
				this->inn = location + COMPRESSED_OUT_DIR + testName + ".vcmp";
				this->inmn =
					location + COMPRESSED_OUT_DIR + testName + ".vmeta";
				this->goutn = location + IN_DIR + testName + ".vin";
				this->outn =
					location + DECOMPRESSED_OUT_DIR + testName + ".vout";
				this->isVerbose = false;
			}

			void setVerbose() { this->isVerbose = true; }

			int run(bool generate = false)
			{
				generate = false;
				std::ios_base::fmtflags coutflags(std::cout.flags());

				std::ifstream in(this->inn, std::ios::binary);
				std::ifstream inm(this->inmn, std::ios::binary);
				std::ofstream out(generate ? this->goutn : this->outn,
								  std::ios::binary);
				std::ifstream gout(this->goutn, std::ios::binary);

				if(!in || !inm || !out || !gout)
				{
					std::cerr << "One of the test files was not found."
							  << std::endl;
					return 1;
				}

				backend::stream<state_t> compressed;
				backend::stream<ANS::Meta> encmeta;
				backend::stream<message_t> message;

				bool hadErrors = false, outOfBoundsRead = false;

				if(!generate)
				{
					std::cout << "*** RUNNING " << this->testName << " ***\n";
				} else
				{
					std::cout << "*** GENERATING " << this->testName
							  << " ***\n";
				}

				std::cout << std::hex << std::uppercase << std::setfill('0')
						  << std::setw(sizeof(message_t) * 2);

				while(!inm.eof())
				{
					ANS::Meta m;
					readFile(inm, m);
					encmeta << m;
					inm.peek();
					std::cout << "Meta" << std::endl;
				}

				std::stack<state_t> tmps;
				while(!in.eof())
				{
					state_t cur;
					readFile(in, cur);
					std::cout << cur << " ";
					tmps.push(cur);
					in.peek();
				}

				std::cout << "\n";

				while(!tmps.empty())
				{
					state_t cur = tmps.top();
					tmps.pop();
					std::cout << cur << " ";
					compressed << cur;
				}

				std::cout << "\n";

				decompress(compressed, encmeta, message);
				std::cout << "Message: ";

				while(!message.empty())
				{
					message_t cur;
					message >> cur;
					out << cur;
					// std::cout << std::setfill('0')
					// 		  << std::setw(sizeof(message_t) * 2) << (u32) cur
					// 		  << " ";
				}

				std::cout << "\n";

				if(!message.empty())
				{ std::cerr << "MESSAGE WAS NOT ALL CONSUMED\n"; }
				if(!encmeta.empty())
				{ std::cerr << "ENCMETA WAS NOT ALL CONSUMED\n"; }

				if(!generate)
				{
					if(hadErrors)
					{
						std::cerr << "!!! TEST " << this->testName
								  << " FAILURE !!!" << std::endl;
						if(outOfBoundsRead)
						{
							std::cerr << "!!! Generated too much data. !!!"
									  << std::endl;
						}
					} else
					{
						std::cout << "*** TEST " << this->testName
								  << " PASS ***" << std::endl;
					}
				}

				std::cout.flags(coutflags);

				return hadErrors;
			}
		};

		bool runDecompressionTests(String location, std::vector<String> cases)
		{
			bool hadErrors = false;
			for(auto tname: cases)
			{
				Decompression t(location, tname);
				t.setVerbose();
				// t.run(true);
				hadErrors |= t.run();
			}

			return hadErrors;
		}
#endif
	} // namespace Test
} // namespace ANS
