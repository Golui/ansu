#pragma once

#include "data/ans_table.hpp"
#include "ints.hpp"
#include "util.hpp"

namespace ANS
{
	namespace tables
	{
		enum Type
		{
			Static,
			Dynamic
		};
	} // namespace tables

	template <typename _StateT, typename _MessageT>
	struct CompressionTable
	{
	protected:
		u32 _alphabetSize;
		u32 _tableSizeLog;
		u32 _tableSize;

	public:
		constexpr static u32 stateWidth		= sizeof(_StateT) << 3;
		constexpr static u32 mesessageWidth = sizeof(_MessageT) << 3;

		using StateT		 = _StateT;
		using MessageT		 = _MessageT;
		using EncodingTableT = StateT;
		using MessageIndexT	 = u32;
		using NbBitsT		 = u32;
		using NbBitsDeltaT	 = s32;

		CompressionTable() {}

		CompressionTable(u32 alphabetSize, u32 tableSizeLog)
			: _alphabetSize(alphabetSize),
			  _tableSizeLog(tableSizeLog),
			  _tableSize(1 << tableSizeLog)
		{}

		u32 alphabetSize() const { return this->_alphabetSize; };
		u32 tableSizeLog() const { return this->_tableSizeLog; };
		u32 tableSize() const { return this->_tableSize; };

		virtual tables::Type type() const						 = 0;
		virtual MessageT states(StateT index) const				 = 0;
		virtual StateT newX(StateT index) const					 = 0;
		virtual EncodingTableT encodingTable(StateT index) const = 0;
		virtual NbBitsDeltaT nbBitsDelta(StateT index) const	 = 0;
		virtual NbBitsT nb(StateT index) const					 = 0;
		virtual StateT start(MessageIndexT index) const			 = 0;
		virtual StateT adjStart(MessageIndexT index) const		 = 0;

		template <typename Archive>
		void serialize(Archive& ar)
		{}

		virtual ~CompressionTable() {}
	};

	// TODO While this is parametrized, we assume the table is always stored as
	// 32 bit in the archive.
	template <typename _StateT = u32, typename _MessageT = u32>
	struct DynamicCompressionTable : public CompressionTable<_StateT, _MessageT>
	{
		using Base			 = CompressionTable<_StateT, _MessageT>;
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
		DynamicCompressionTable() : Base() {}

		DynamicCompressionTable(u32 alphabetSize, u32 tableSizeLog)
			: Base(alphabetSize, tableSizeLog)
		{}

		virtual tables::Type type() const override { return tables::Dynamic; }

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

	class StaticCompressionTable : public CompressionTable<state_t, message_t>
	{
		using Base = CompressionTable<state_t, message_t>;

	public:
		using StateT		 = typename Base::StateT;
		using MessageT		 = typename Base::MessageT;
		using MessageIndexT	 = typename Base::MessageIndexT;
		using EncodingTableT = typename Base::EncodingTableT;
		using NbBitsDeltaT	 = typename Base::NbBitsDeltaT;
		using NbBitsT		 = typename Base::NbBitsT;

		StaticCompressionTable() : Base(ALPHABET_LENGTH, TABLE_SIZE_LOG) {}

		virtual tables::Type type() const override { return tables::Static; }

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

		template <typename Archive>
		void serialize(Archive& ar)
		{
			u32 dummy = 0xDEADBEEF;
			ar(dummy);
		}
	};

} // namespace ANS

