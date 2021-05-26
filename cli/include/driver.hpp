#pragma once

#include "CLI11.hpp"
#include "ansu/ints.hpp"

#include <memory.h>

namespace ANS
{
	namespace driver
	{
		constexpr u64 MAX_BUFFER_SIZE = 256 << 20;
		namespace compress
		{
			struct Options
			{
				std::string inFilePath;
				std::string outFilePath;
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
			};

			using OptionsP = std::shared_ptr<Options>;

			void subRegister(CLI::App& app);
			int run(OptionsP opts);
		} // namespace decompress
	}	  // namespace driver
} // namespace ANS
