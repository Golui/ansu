#pragma once

#include "CLI11.hpp"
#include "ansu/ints.hpp"
#include "ansu/settings.hpp"

#include <map>
#include <memory.h>

namespace ANS
{
	namespace driver
	{
		constexpr u64 MAX_BUFFER_SIZE = 256 << 20;

		enum class Alphabet : int
		{
			Reduced,
			Ascii
		};

		static const std::map<std::string, Alphabet> ALPHABET_STR_TO_ENUM {
			{"reduced", Alphabet::Reduced}, {"ascii", Alphabet::Ascii}};

		namespace compress
		{
			struct Options
			{
				std::string inFilePath;
				std::string outFilePath;
				u64 checkpoint			  = CHECKPOINT;
				u32 channels			  = CHANNEL_COUNT;
				u64 chunkSize			  = AVG_MESSAGE_LENGTH;
				Alphabet alphabet		  = Alphabet::Reduced;
				u32 tableSizeLog		  = 10;
				std::string tableFilePath = "";
				bool printSummary		  = false;
			};

			using OptionsP = std::shared_ptr<Options>;

			void subRegister(CLI::App& app);
			int run(OptionsP opts);
		} // namespace compress

		namespace decompress
		{
			struct Options
			{
				std::string inFilePath;
				std::string outFilePath;
				bool ignoreSizeWarning = false;
			};

			using OptionsP = std::shared_ptr<Options>;

			void subRegister(CLI::App& app);
			int run(OptionsP opts);
		} // namespace decompress

		namespace generate
		{
			struct Options
			{
				std::string inFilePath;
				std::string outFilePath;
			};

			using OptionsP = std::shared_ptr<Options>;

			void subRegister(CLI::App& app);
			int run(OptionsP opts);
		} // namespace generate
	}	  // namespace driver
} // namespace ANS
