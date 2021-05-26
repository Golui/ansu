#pragma once

#include "data/compression_table.hpp"
#include "impl/channel_compression_context.ipp"

#include <cereal/cereal.hpp>
#include <cereal/types/vector.hpp>

template <typename Archive, typename T, typename O>
void save(Archive& ar, const ANS::DynamicCompressionTable<T, O>& table)
{
	ar(cereal::base_class<typename ANS::DynamicCompressionTable<T, O>::Base>(
		table));

	ar(table._states,
	   table._newX,
	   table._encodingTable,
	   table._nbBitsDelta,
	   table._nb,
	   table._start,
	   table._adjStart);
}

template <typename Archive>
void load(Archive& ar, ANS::DynamicCompressionTable<u32, u32>& table)
{
	ar(cereal::base_class<
		typename ANS::DynamicCompressionTable<u32, u32>::Base>(table));
	ar(table._states,
	   table._newX,
	   table._encodingTable,
	   table._nbBitsDelta,
	   table._nb,
	   table._start,
	   table._adjStart);
}

template <typename Archive, template <typename...> class Ctx, typename... T>
void serialize(Archive& ar, Ctx<T...>& ctx)
{
	ar(ctx.lastChannel);
}
