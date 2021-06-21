#pragma once

#include "ansu/settings.hpp"

#include <ap_axi_sdata.h>

namespace ANS
{
	namespace backend
	{
		template <typename T>
		using side = ap_axiu<sizeof(T) << 3, 0, 0, 0>;
	}
} // namespace ANS
