#pragma once

#include "data/compression_table.hpp"

#include <iostream>
#include <unordered_map>

namespace ANS
{
	template <typename MessageT>
	using OccurenceMap = std::unordered_map<MessageT, u32>;
	template <typename MessageT>
	using QunatizedMap = std::unordered_map<MessageT, u32>;
	template <typename MessageT>
	using ProbabilityMap = std::unordered_map<MessageT, double>;

	template <typename MessageT>
	struct SampleInformation
	{
		struct Symbol
		{
			MessageT symbol = 0;
			s32 q			= 0;
			double p		= 0;
		};
		std::vector<Symbol> data;
	};

	struct TableGeneratorOptions
	{
		u32 tableSizeLog  = 10;
		bool useFullAscii = false;

		u32 tableSize() { return 1 << this->tableSizeLog; }
	};

	namespace strategies
	{
		template <typename MessageT>
		void quantizeFast(SampleInformation<MessageT>& si,
						  TableGeneratorOptions opts)
		{
			u32 used	= 0;
			double maxv = 0;
			u32 maxp	= 0;
			for(u32 i = 0; i < si.data.size(); i++)
			{
				auto& symbol = si.data[i];
				auto curQ	 = u32(floor(symbol.p * opts.tableSize()));
				if(curQ == 0) curQ += 1;
				used += curQ;
				symbol.q = curQ;
				if(symbol.p > maxv)
				{
					maxv = symbol.p;
					maxp = i;
				}
			}
			si.data[maxp].q += opts.tableSize() - used;
			if(si.data[maxp].q <= 0)
				throw std::runtime_error(
					"Error: The used quantizer is too inaccurate for this "
					"file. Please choose a different quantizer, or adjust the "
					"alphabet size.");
		}

		template <typename MessageT, typename Data>
		void spreadFast(Data& data,
						const SampleInformation<MessageT>& si,
						TableGeneratorOptions opts)
		{
			u32 pos	 = 0;
			u32 step = (opts.tableSize() >> 1) + (opts.tableSize() >> 3) + 3;
			u32 mask = opts.tableSize() - 1;
			data.states.resize(opts.tableSize());
			for(u32 i = 0; i < si.data.size(); i++)
			{
				auto& entry = si.data[i];
				for(s32 j = 0; j < entry.q; j++)
				{
					data.states[pos] = i;
					pos				 = (pos + step) & mask;
				}
			}
		}
	} // namespace strategies

	template <typename MessageT>
	OccurenceMap<MessageT> generateProbabilities(std::istream& sample)
	{
		OccurenceMap<MessageT> result;
		constexpr auto readWidth =
			sizeof(MessageT) / sizeof(std::istream::char_type);
		while(sample.peek() != EOF)
		{
			MessageT symbol;
			sample.read((std::istream::char_type*) &symbol, readWidth);
			auto it = result.find(symbol);
			if(it == result.end())
			{
				result[symbol] = 1;
			} else
			{
				(*it).second += 1;
			}
		}

		return result;
	}

	template <typename MessageT>
	SampleInformation<MessageT>
	createSampleInformation(OccurenceMap<MessageT>&& map,
							TableGeneratorOptions& opts)
	{
		SampleInformation<MessageT> result;
		typename OccurenceMap<MessageT>::mapped_type sum = 0;
		if(opts.useFullAscii)
		{
			result.data.resize(256);
			for(u32 i = 0; i < 256; i++) { result.data[i].symbol = i; }
			for(auto& entry: map)
			{
				result.data[entry.first].symbol = entry.first;
				sum += entry.second;
			}

		} else
		{
			result.data.resize(map.size());
			u32 i = 0;
			for(auto& entry: map)
			{
				result.data[i].symbol = entry.first;
				i++;
				sum += entry.second;
			}
		}

		for(auto& symbol: result.data)
		{
			symbol.p = map[symbol.symbol] / (double) sum;
		}
		return result;
	}

	template <typename StateT,
			  typename MessageT,
			  typename ResultT = DynamicCompressionTable<StateT, MessageT>>
	ResultT generateTable(SampleInformation<MessageT>&& _si,
						  TableGeneratorOptions opts)
	{
		auto si = _si;

		ResultT result(si.data.size(), opts.tableSizeLog);
		typename ResultT::Data data;
		// TODO Make strategies an option
		strategies::quantizeFast(si, opts);
		strategies::spreadFast(data, si, opts);

		data.start.resize(si.data.size());

		auto next = std::vector<u32>(si.data.size());
		for(u32 i = 0; i < si.data.size(); i++) next[i] = si.data[i].q;

		u64 runningTotal = 0;
		for(u32 i = 0; i < data.start.size(); i++)
		{
			auto s		  = si.data[i].q;
			data.start[i] = -s + runningTotal;
			runningTotal += s;
		}

		data.adjStart.resize(data.start.size());
		for(u32 i = 0; i < data.adjStart.size(); i++)
		{
			data.adjStart[i] = data.start[i] % opts.tableSize();
		}

		data.encodingTable.resize(opts.tableSize());
		data.nbBitsDelta.resize(opts.tableSize());
		data.newX.resize(opts.tableSize());

		for(u32 i = 0; i < data.states.size(); i++)
		{
			auto symbol = data.states[i];
			data.encodingTable[data.start[symbol] + next[symbol]] =
				opts.tableSize() + i;
			auto x				= next[symbol];
			data.nbBitsDelta[i] = opts.tableSizeLog - int(floor(log2(x)));
			data.newX[i]		= (x << data.nbBitsDelta[i]) - opts.tableSize();
			next[symbol]++;
		}

		auto nb = [&](u32 s) {
			auto ks = opts.tableSizeLog - int(floor(log2(si.data[s].q)));
			return (ks << (opts.tableSizeLog + 1)) - (si.data[s].q << ks);
		};

		data.nb.resize(opts.tableSize());
		data.alphabet.resize(si.data.size());
		for(u32 i = 0; i < si.data.size(); i++)
		{
			data.nb[i]		 = nb(i);
			data.alphabet[i] = si.data[i].symbol;
		}

		result.setData(data);
		return result;
	}

	template <typename StateT, typename MessageT>
	DynamicCompressionTable<StateT, MessageT>
	generateTable(OccurenceMap<MessageT>&& occurences,
				  TableGeneratorOptions opts)
	{
		return generateTable<StateT, MessageT>(
			createSampleInformation(std::move(occurences), opts), opts);
	}

	template <typename StateT, typename MessageT>
	DynamicCompressionTable<StateT, MessageT>
	generateTable(std::istream& sample, TableGeneratorOptions opts)
	{
		return generateTable<StateT, MessageT>(
			generateProbabilities<MessageT>(sample), opts);
	}

} // namespace ANS
