#pragma once

#include "data/compression_table.hpp"

#include <algorithm>
#include <iostream>
#include <unordered_map>

namespace ANS
{
	template <typename SymbolT>
	using OccurenceMap = std::unordered_map<SymbolT, u32>;
	template <typename SymbolT>
	using QuantizedMap = std::unordered_map<SymbolT, u32>;
	template <typename SymbolT>
	using ProbabilityMap = std::unordered_map<SymbolT, double>;

	template <typename SymbolT>
	struct SampleInformation
	{
		struct Symbol
		{
			SymbolT symbol = 0;
			s32 q		   = 0;
			double p	   = 0;
		};
		std::vector<Symbol> data;
	};

	namespace strategies
	{
		enum struct Quantizer
		{
			Fast
		};

		enum struct Spreader
		{
			Fast
		};
	} // namespace strategies

	struct TableGeneratorOptions
	{
		u32 tableSizeLog = 10;
		// TODO
		bool useFull = false;
		u32 symbolWidth;

		strategies::Quantizer quantizer = strategies::Quantizer::Fast;
		strategies::Spreader spreader	= strategies::Spreader::Fast;

		u32 tableSize() { return 1 << this->tableSizeLog; }
	};

	namespace strategies
	{
		template <typename SymbolT>
		struct QuantizerBase
		{
			void operator()(SampleInformation<SymbolT>& si,
							TableGeneratorOptions opts)
			{
				this->quantize(si, opts);
			}
			virtual void quantize(SampleInformation<SymbolT>& si,
								  TableGeneratorOptions opts) = 0;
			virtual ~QuantizerBase() {}
		};

		template <typename SymbolT>
		struct QuantizeFast : public QuantizerBase<SymbolT>
		{
			virtual void quantize(SampleInformation<SymbolT>& si,
								  TableGeneratorOptions opts) override
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
						"file. Please choose a different quantizer, or adjust "
						"the "
						"alphabet size.");
			}
		};

		template <typename SymbolT, typename Data>
		struct SpreaderBase
		{
			void operator()(Data& data,
							const SampleInformation<SymbolT>& si,
							TableGeneratorOptions opts)
			{
				this->spread(data, si, opts);
			}
			virtual void spread(Data& data,
								const SampleInformation<SymbolT>& si,
								TableGeneratorOptions opts) = 0;
			virtual ~SpreaderBase() {}
		};

		template <typename SymbolT, typename Data>
		struct SpreadFast : public SpreaderBase<SymbolT, Data>
		{
			virtual void spread(Data& data,
								const SampleInformation<SymbolT>& si,
								TableGeneratorOptions opts) override
			{
				u32 pos = 0;
				u32 step =
					(opts.tableSize() >> 1) + (opts.tableSize() >> 3) + 3;
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
		};

		template <typename SymbolT>
		void quantize(Quantizer method,
					  SampleInformation<SymbolT>& si,
					  TableGeneratorOptions opts)
		{
			switch(method)
			{
				case Quantizer::Fast:
				{
					QuantizeFast<SymbolT>()(si, opts);
					break;
				}
				default:
				{
					break;
				}
			}
		}

		template <typename SymbolT, typename Data>
		void spread(Spreader method,
					Data& data,
					SampleInformation<SymbolT>& si,
					TableGeneratorOptions opts)
		{
			switch(method)
			{
				case Spreader::Fast:
				{
					SpreadFast<SymbolT, Data>()(data, si, opts);
					break;
				}
				default:
				{
					break;
				}
			}
		}

	} // namespace strategies

	template <typename SymbolT>
	OccurenceMap<SymbolT> generateProbabilities(std::istream& sample,
												TableGeneratorOptions& opts)
	{
		OccurenceMap<SymbolT> result;
		const auto readWidth =
			(ANS::integer::nextPowerOfTwo(opts.symbolWidth) >> 3)
			/ sizeof(std::istream::char_type);
		while(sample.peek() != EOF)
		{
			SymbolT symbol = 0;
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

	template <typename SymbolT>
	SampleInformation<SymbolT>
	createSampleInformation(OccurenceMap<SymbolT>&& map,
							TableGeneratorOptions& opts)
	{
		SampleInformation<SymbolT> result;
		typename OccurenceMap<SymbolT>::mapped_type sum = 0;
		if(opts.useFull)
		{
			throw std::runtime_error("NYI");
			auto fullSize = 1UL << (sizeof(SymbolT) << 3);
			result.data.resize(fullSize);
			for(u32 i = 0; i < fullSize; i++) { result.data[i].symbol = i; }
			for(auto& entry: map)
			{
				// Just to make sure, I guess...
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
			  typename SymbolT,
			  typename ResultT = DynamicCompressionTable<StateT, SymbolT>>
	ResultT generateTable(SampleInformation<SymbolT>&& _si,
						  TableGeneratorOptions opts)
	{
		auto si = _si;

		ResultT result(si.data.size(), opts.tableSizeLog, opts.symbolWidth);
		typename ResultT::Data data;
		// TODO Make strategies an option

		strategies::quantize(opts.quantizer, si, opts);
		strategies::spread(opts.spreader, data, si, opts);

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

	template <typename StateT, typename SymbolT>
	DynamicCompressionTable<StateT, SymbolT>
	generateTable(OccurenceMap<SymbolT>&& occurences,
				  TableGeneratorOptions opts)
	{
		return generateTable<StateT, SymbolT>(
			createSampleInformation(std::move(occurences), opts), opts);
	}

	template <typename StateT, typename SymbolT>
	DynamicCompressionTable<StateT, SymbolT>
	generateTable(std::istream& sample, TableGeneratorOptions opts)
	{
		return generateTable<StateT, SymbolT>(
			generateProbabilities<SymbolT>(sample, opts), opts);
	}

} // namespace ANS
