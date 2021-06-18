#pragma once

#include "data/ans_table.hpp"
#include "ints.hpp"
#include "util.hpp"

#include <stdexcept>
#include <unordered_map>
#include <vector>

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

	namespace __internal
	{
		// Store the compression table as u32's for now
		template <typename Archive, typename Vector>
		void asVectorU32(Archive& arch, Vector& vec)
		{
			arch(std::vector<u32>(vec.begin(), vec.end()));
		}

	} // namespace __internal

	template <typename _StateT, typename _SymbolT>
	struct CompressionTable
	{
	protected:
		u32 _alphabetSize;
		u32 _tableSizeLog;
		u32 _tableSize;
		u32 _symbolWidth;

	public:
		constexpr static u32 stateWidth = sizeof(_StateT) << 3;

		using StateT		 = _StateT;
		using StateDeltaT	 = s32;
		using SymbolT		 = _SymbolT;
		using EncodingTableT = StateT;
		using ReducedSymbolT = u32;
		using NbBitsT		 = u32;
		using NbBitsDeltaT	 = s32;

		CompressionTable() {}

		CompressionTable(u32 alphabetSize, u32 tableSizeLog, u32 symbolWidth)
			: _alphabetSize(alphabetSize),
			  _tableSizeLog(tableSizeLog),
			  _tableSize(1 << tableSizeLog),
			  _symbolWidth(symbolWidth)
		{}

		u32 alphabetSize() const { return this->_alphabetSize; };
		u32 tableSizeLog() const { return this->_tableSizeLog; };
		u32 tableSize() const { return this->_tableSize; };
		u32 symbolWidth() const { return this->_symbolWidth; };

		virtual tables::Type type() const						 = 0;
		virtual ReducedSymbolT states(StateT index) const		 = 0;
		virtual StateT newX(StateT index) const					 = 0;
		virtual EncodingTableT encodingTable(StateT index) const = 0;
		virtual NbBitsDeltaT nbBitsDelta(StateT index) const	 = 0;
		virtual NbBitsT nb(StateT index) const					 = 0;
		virtual StateDeltaT start(ReducedSymbolT index) const	 = 0;
		virtual StateT adjStart(ReducedSymbolT index) const		 = 0;

		virtual SymbolT alphabet(ReducedSymbolT index) const		 = 0;
		virtual ReducedSymbolT reverseAlphabet(SymbolT symbol) const = 0;
		virtual bool hasSymbolInAlphabet(SymbolT symbol) const		 = 0;

		virtual ~CompressionTable() {}
	};

	// TODO While this is parametrized, we assume the table is always stored as
	// 32 bit in the archive.
	// _SymbolT is an internal type that is wide enough to encompass the symbol,
	// not necessarily how big the symbol is exactly!
	template <typename _StateT = u32, typename _SymbolT = u32>
	struct DynamicCompressionTable : public CompressionTable<_StateT, _SymbolT>
	{
		using Base			 = CompressionTable<_StateT, _SymbolT>;
		using StateT		 = typename Base::StateT;
		using StateDeltaT	 = typename Base::StateDeltaT;
		using SymbolT		 = typename Base::SymbolT;
		using ReducedSymbolT = typename Base::ReducedSymbolT;
		using EncodingTableT = typename Base::EncodingTableT;
		using NbBitsDeltaT	 = typename Base::NbBitsDeltaT;
		using NbBitsT		 = typename Base::NbBitsT;

		struct Data
		{
			std::vector<StateT> states				  = {0};
			std::vector<StateT> newX				  = {0};
			std::vector<EncodingTableT> encodingTable = {0};
			std::vector<NbBitsDeltaT> nbBitsDelta	  = {0};
			std::vector<NbBitsT> nb					  = {0};
			std::vector<StateDeltaT> start			  = {0};
			std::vector<StateT> adjStart			  = {0};
			std::vector<SymbolT> alphabet			  = {0};

			template <typename Archive>
			void save(Archive& ar) const
			{
				using namespace __internal;
				asVectorU32(ar, this->states);
				asVectorU32(ar, this->newX);
				asVectorU32(ar, this->encodingTable);
				asVectorU32(ar, this->nbBitsDelta);
				asVectorU32(ar, this->nb);
				asVectorU32(ar, this->start);
				asVectorU32(ar, this->adjStart);
				ar(this->alphabet);
			}

			template <typename Archive>
			void load(Archive& ar)
			{
				ar(this->states,
				   this->newX,
				   this->encodingTable,
				   this->nbBitsDelta,
				   this->nb,
				   this->start,
				   this->adjStart,
				   this->alphabet);
			}
		};

	private:
		Data data;
		std::unordered_map<SymbolT, ReducedSymbolT> reverseAlphabetMap;

	public:
		DynamicCompressionTable() : Base() {}

		DynamicCompressionTable(u32 alphabetSize,
								u32 tableSizeLog,
								u32 symbolWidth,
								Data dt = Data())
			: Base(alphabetSize, tableSizeLog, symbolWidth), data(dt)
		{}

		virtual tables::Type type() const override { return tables::Dynamic; }

		virtual ReducedSymbolT states(StateT index) const
		{
			return data.states[index];
		}
		virtual StateT newX(StateT index) const { return data.newX[index]; }

		virtual EncodingTableT encodingTable(StateT index) const
		{
			return data.encodingTable[index];
		}

		virtual NbBitsDeltaT nbBitsDelta(StateT index) const
		{
			return data.nbBitsDelta[index];
		}

		virtual NbBitsT nb(StateT index) const { return data.nb[index]; }

		virtual StateDeltaT start(ReducedSymbolT index) const
		{
			return data.start[index];
		}

		virtual StateT adjStart(ReducedSymbolT index) const
		{
			return data.adjStart[index];
		}

		virtual SymbolT alphabet(ReducedSymbolT index) const
		{
			// Do bounds checking
			return data.alphabet.at(index);
		}

		virtual ReducedSymbolT reverseAlphabet(SymbolT index) const
		{
			return reverseAlphabetMap.at(index);
		}

		virtual bool hasSymbolInAlphabet(SymbolT index) const
		{
			return reverseAlphabetMap.find(index) != reverseAlphabetMap.end();
		}

		void setData(Data dt)
		{
			this->data = dt;
			u32 i	   = 0;
			for(auto& symbol: this->data.alphabet)
			{
				this->reverseAlphabetMap[symbol] = i;
				i++;
			}
		}

		template <typename Archive>
		void load(Archive& ar)
		{
			ar(this->_alphabetSize, this->_tableSizeLog);
			this->_tableSize = 1 << this->_tableSizeLog;
			u32 size;
			ar(size);
			if(size != sizeof(SymbolT))
			{
				throw std::runtime_error("Incompatible table.");
			}
			ar(this->_symbolWidth, this->data);
			this->setData(this->data);
		}

		template <typename Archive>
		void save(Archive& ar) const
		{
			ar(this->_alphabetSize, this->_tableSizeLog);
			u32 size = sizeof(SymbolT);
			ar(size, this->_symbolWidth);
			ar(this->data);
		}
	};

	class StaticCompressionTable : public CompressionTable<state_t, message_t>
	{
		using Base = CompressionTable<state_t, message_t>;

	public:
		using StateT		 = typename Base::StateT;
		using StateDeltaT	 = typename Base::StateDeltaT;
		using SymbolT		 = typename Base::SymbolT;
		using ReducedSymbolT = typename Base::ReducedSymbolT;
		using EncodingTableT = typename Base::EncodingTableT;
		using NbBitsDeltaT	 = typename Base::NbBitsDeltaT;
		using NbBitsT		 = typename Base::NbBitsT;

		StaticCompressionTable()
			: Base(ALPHABET_LENGTH, TABLE_SIZE_LOG, sizeof(message_t) << 3)
		{}

		virtual tables::Type type() const override { return tables::Static; }

		virtual ReducedSymbolT states(StateT index) const override
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

		virtual StateDeltaT start(ReducedSymbolT index) const override
		{
			return StaticTable::start[index];
		}

		virtual StateT adjStart(ReducedSymbolT index) const override
		{
			return StaticTable::adj_start[index];
		}

		virtual SymbolT alphabet(ReducedSymbolT index) const override
		{
			return index;
		}

		virtual ReducedSymbolT reverseAlphabet(SymbolT index) const override
		{
			return index;
		}

		virtual bool hasSymbolInAlphabet(SymbolT index) const
		{
			// TODO STUB
			return true;
		}

		template <typename Archive>
		void serialize(Archive& ar)
		{}
	};

} // namespace ANS

