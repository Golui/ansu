#pragma once

#include "data/ans_table.hpp"
#include "ints.hpp"
#include "util.hpp"

namespace ANS
{
	template <u32 MinStateWidth,
			  u32 MinMessageWidth,
			  class = typename std::enable_if<(MinStateWidth > 0)>::type,
			  class = typename std::enable_if<(MinMessageWidth > 0)>::type>
	struct CompressionTable
	{
		constexpr static u32 minStateWidth	 = MinStateWidth;
		constexpr static u32 minMessageWidth = MinMessageWidth;

		using StateT		 = typename integer::fitting<MinStateWidth>::type;
		using MessageT		 = typename integer::fitting<MinMessageWidth>::type;
		using EncodingTableT = StateT;
		using MessageIndexT	 = u32;
		using NbBitsT		 = u32;
		using NbBitsDeltaT	 = s32;

		const u32 alphabetSize;
		const u32 tableSizeLog;
		const u32 tableSize;

		CompressionTable(u32 alphabetSize, u32 tableSizeLog)
			: alphabetSize(alphabetSize),
			  tableSizeLog(tableSizeLog),
			  tableSize(1 << tableSizeLog)
		{}

		virtual MessageT states(StateT index) const				 = 0;
		virtual StateT newX(StateT index) const					 = 0;
		virtual EncodingTableT encodingTable(StateT index) const = 0;
		virtual NbBitsDeltaT nbBitsDelta(StateT index) const	 = 0;
		virtual NbBitsT nb(StateT index) const					 = 0;
		virtual StateT start(MessageIndexT index) const			 = 0;
		virtual StateT adjStart(MessageIndexT index) const		 = 0;

		virtual ~CompressionTable() {}
	};

	// Assumptions:
	// - By default, the StateT is wide enough to contain (1 << TableSizeLog).
	// - `nb_bits_delta` is assumed to be 8 bits wide
	template <u32 MinS = 32, u32 MinM = 32>
	struct DynamicCompressionTable : public CompressionTable<MinS, MinM>
	{
		using Base			 = CompressionTable<MinS, MinM>;
		using StateT		 = typename Base::StateT;
		using MessageT		 = typename Base::MessageT;
		using MessageIndexT	 = typename Base::MessageIndexT;
		using EncodingTableT = typename Base::EncodingTableT;
		using NbBitsDeltaT	 = typename Base::NbBitsDeltaT;
		using NbBitsT		 = typename Base::NbBitsT;

	private:
		std::vector<StateT> _states				   = {0};
		std::vector<StateT> _newX				   = {0};
		std::vector<EncodingTableT> _encodingTable = {0};
		std::vector<NbBitsDeltaT> _nbBitsDelta	   = {0};
		std::vector<NbBitsT> _nb				   = {0};
		std::vector<StateT> _start				   = {0};
		std::vector<StateT> _adjStart			   = {0};

	public:
		DynamicCompressionTable(u32 alphabetSize, u32 tableSizeLog)
			: Base(alphabetSize, tableSizeLog)
		{}

		virtual MessageT states(StateT index) const { return _states[index]; }
		virtual StateT newX(StateT index) const { return _newX[index]; }

		virtual EncodingTableT encodingTable(StateT index) const
		{
			return _encodingTable[index];
		}

		virtual NbBitsDeltaT nbBitsDelta(StateT index) const
		{
			return _nbBitsDelta[index];
		}

		virtual NbBitsT nb(StateT index) const { return _nb[index]; }

		virtual StateT start(MessageIndexT index) const
		{
			return _start[index];
		}

		virtual StateT adjStart(MessageIndexT index) const
		{
			return _adjStart[index];
		}
	};

	class StaticCompressionTable
		: public CompressionTable<sizeof(state_t) << 3, sizeof(message_t) << 3>
	{
		using Base =
			CompressionTable<sizeof(state_t) << 3, sizeof(message_t) << 3>;

	public:
		using StateT		 = typename Base::StateT;
		using MessageT		 = typename Base::MessageT;
		using MessageIndexT	 = typename Base::MessageIndexT;
		using EncodingTableT = typename Base::EncodingTableT;
		using NbBitsDeltaT	 = typename Base::NbBitsDeltaT;
		using NbBitsT		 = typename Base::NbBitsT;

		StaticCompressionTable() : Base(ALPHABET_LENGTH, TABLE_SIZE_LOG) {}

		virtual MessageT states(StateT index) const override
		{
			return StaticTable::states[index];
		}

		virtual StateT newX(StateT index) const override
		{
			return StaticTable::new_x[index];
		}

		virtual EncodingTableT encodingTable(StateT index) const override
		{
			return StaticTable::encoding_table[index];
		}

		virtual NbBitsDeltaT nbBitsDelta(StateT index) const override
		{
			return StaticTable::nb_bits_delta[index];
		}

		virtual NbBitsT nb(StateT index) const override
		{
			return StaticTable::nb[index];
		}

		virtual StateT start(MessageIndexT index) const override
		{
			return StaticTable::start[index];
		}

		virtual StateT adjStart(MessageIndexT index) const override
		{
			return StaticTable::adj_start[index];
		}
	};
} // namespace ANS

