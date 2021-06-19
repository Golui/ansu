#pragma once

#include "ansu/data/compression_table.hpp"

#include <cereal/archives/binary.hpp>
#include <cereal/cereal.hpp>
#include <cereal/types/vector.hpp>

namespace ANS
{
	namespace io
	{
		constexpr static u32 ANSU_TABLE_MAGIC =
			'T' << 24 | 'S' << 16 | 'N' << 8 | 'A';

		template <typename... T>
		void saveTable(std::ostream& out,
					   const DynamicCompressionTable<T...>& tbl)
		{
			cereal::BinaryOutputArchive ar(out);
			ar(ANSU_TABLE_MAGIC);
			ar(tbl);
		}

		template <typename... T,
				  typename ResultT = DynamicCompressionTable<T...> >
		ResultT loadTable(std::istream& in)
		{
			cereal::BinaryInputArchive ar(in);
			u32 magic;
			ar(magic);
			if(magic != ANSU_TABLE_MAGIC)
				throw std::runtime_error("Wring magic.");
			ResultT result;
			ar(result);
			return result;
		}
	} // namespace io
} // namespace ANS
