#pragma once

#include "CLI11.hpp"
#include "ansu/data/table_generator.hpp"
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
			U8	= 8,
			U16 = 16,
		};

		static const std::map<std::string, Alphabet> ALPHABET_STR_TO_ENUM {
			{"8", Alphabet::U8}, {"16", Alphabet::U16}};

		namespace compress
		{
			struct Options
			{
				std::string inFilePath			= "STDIN";
				std::string outFilePath			= "compressed.ansu";
				u64 checkpoint					= CHECKPOINT;
				u32 channels					= CHANNEL_COUNT;
				u64 chunkSize					= AVG_MESSAGE_LENGTH;
				Alphabet alphabet				= Alphabet::U8;
				u32 tableSizeLog				= 10;
				std::string tableFilePath		= "";
				bool printSummary				= false;
				bool skipPrompt					= false;
				bool warnUnknownSymbol			= false;
				strategies::Quantizer quantizer = strategies::Quantizer::Fast;
				strategies::Spreader spreader	= strategies::Spreader::Fast;
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
				std::string inFilePath			= "STDIN";
				std::string outFilePath			= "table.anstbl";
				Alphabet alphabet				= Alphabet::U8;
				u32 tableSizeLog				= 10;
				bool skipPrompt					= false;
				strategies::Quantizer quantizer = strategies::Quantizer::Fast;
				strategies::Spreader spreader	= strategies::Spreader::Fast;
			};

			using OptionsP = std::shared_ptr<Options>;

			void subRegister(CLI::App& app);
			int run(OptionsP opts);
		} // namespace generate
	}	  // namespace driver
} // namespace ANS
