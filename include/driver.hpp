#pragma once

#include "CLI11.hpp"

#include <memory.h>

namespace ANS
{
	namespace driver
	{
		namespace compress
		{
			struct Options
			{
				std::string filepath;
			};

			using OptionsP = std::shared_ptr<Options>;

			void subRegister(CLI::App& app);
			int run(OptionsP opts);
		} // namespace compress

		namespace decompress
		{
			struct Options
			{
				std::string filepath;
			};

			using OptionsP = std::shared_ptr<Options>;

			void subRegister(CLI::App& app);
			int run(OptionsP opts);
		} // namespace decompress
	}	  // namespace driver
} // namespace ANS
