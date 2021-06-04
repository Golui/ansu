#pragma once

#include "data/compression_table.hpp"
#include "impl/channel_compression_context.ipp"

#include <cereal/cereal.hpp>
#include <cereal/types/polymorphic.hpp>
#include <cereal/types/vector.hpp>

namespace ANS
{
	namespace io
	{
		namespace __internal
		{
			// Store the compression table as u32's for now
			template <typename Archive, typename Vector>
			void asVectorU32(Archive& arch, Vector& vec)
			{
				arch(std::vector<u32>(vec.begin(), vec.end()));
			}

		} // namespace __internal
	}	  // namespace io
} // namespace ANS

template <typename Archive, typename... T>
void save(Archive& ar, const ANS::DynamicCompressionTable<T...>& table)
{
	ar(cereal::base_class<typename ANS::DynamicCompressionTable<T...>::Base>(
		table));

	asVectorU32(ar, table._states);
	asVectorU32(ar, table._newX);
	asVectorU32(ar, table._encodingTable);
	asVectorU32(ar, table._nbBitsDelta);
	asVectorU32(ar, table._nb);
	asVectorU32(ar, table._start);
	asVectorU32(ar, table._adjStart);
}

template <typename Archive, typename... T>
void load(Archive& ar, ANS::DynamicCompressionTable<T...>& table)
{
	ar(cereal::base_class<typename ANS::DynamicCompressionTable<T...>::Base>(
		table));
	ar(table._states,
	   table._newX,
	   table._encodingTable,
	   table._nbBitsDelta,
	   table._nb,
	   table._start,
	   table._adjStart);
}

// template <typename Archive,
//		  template <typename, template <typename...> class...>
//		  class ImplT,
//		  class Table,
//		  template <typename...>
//		  class... Rest>
// void serialize(Archive& ar, ANS::CompressionContext<ImplT, Table, Rest...>&
// ctx)
//{}
//
// template <typename Archive>
// void save(Archive& ar,
//		  ANS::ChannelCompressionContext<ANS::StaticCompressionTable>& ctx)
//{
//	ar(ctx.channelCount, ctx.ansTable, ctx.lastChannel);
//}
//
// template <typename Archive>
// void load(Archive& ar,
//		  ANS::ChannelCompressionContext<ANS::StaticCompressionTable>& ctx)
//{
//	ar(ctx.channelCount, ctx.ansTable, ctx.lastChannel);
//	ctx.encoders = (decltype(ctx.encoders))(ctx.channelCount);
//}

