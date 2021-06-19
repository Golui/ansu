#pragma once

#include "ansu/compression_context.hpp"
#include "ansu/data/compression_table.hpp"
#include "ansu/settings.hpp"

#include <bitset>
#include <cassert>
#include <vector>

#ifdef SOFTWARE
#	include <iostream>
#endif

#define MASK(b) ((1L << (b)) - 1)

namespace ANS
{
	template <typename _Table,
			  template <typename...> class Container = std::vector>
	struct ChannelCompressionContext
		: public CompressionContext<ChannelCompressionContext,
									_Table,
									Container>
	{
		using BaseT =
			CompressionContext<ChannelCompressionContext, _Table, Container>;

		using Table			 = _Table;
		using StateT		 = typename Table::StateT;
		using ReducedSymbolT = typename Table::ReducedSymbolT;
		using NbBitsT		 = typename Table::NbBitsT;
		template <typename... T>
		using ContainerT = Container<T...>;

		// TODO Refer to def in base class
		using DecompressResult = typename BaseT::DecompressResult;

		struct State
		{
			StateT x;
			StateT partial;
			u8 partialBits;

			template <typename Archive>
			void serialize(Archive& ar)
			{
				ar(this->x, this->partial, this->partialBits);
			}
		};

		struct Meta
		{
			u32 channels;
			Container<StateT> controlState;
			s32 symbolsInBlock;

			Meta(u32 channelCount = 0) : channels(channelCount)
			{
				this->controlState = Container<StateT>(channels);
			}

			bool operator==(const Meta other)
			{
				if(this->channels != other.channels) return false;
				if(this->symbolsInBlock != other.symbolsInBlock) return false;
				for(u32 i = 0; i < this->channels; i++)
				{
					if(this->controlState[i] != other.controlState[i])
						return false;
				}
				return true;
			}

			bool operator!=(const Meta other) { return !(*this == other); }

			template <typename Archive>
			void serialize(Archive& ar)
			{
				ar(this->channels, this->controlState, this->symbolsInBlock);
			}
		};

		u32 channelCount;
		Table ansTable;
		Container<State> coders;
		u32 lastChannel;
		State master;
		Container<ReducedSymbolT> reverseMsg;
		s32 symbolsInCurrentBlock = 0;

	private:
	public:
		ChannelCompressionContext(u32 channelCount = 2, Table tbl = Table())
			: channelCount(channelCount), ansTable(tbl)
		{
			this->coders = Container<State>(this->channelCount);
			this->resetEncoder();
		}
		void resetDecompressBuffer()
		{
			reverseMsg.reserve((u64) (this->_checkpointFrequency / 0.6));
		}

		void resetEncoding(State& s)
		{
			s.x			  = ansTable.tableSize();
			s.partial	  = 0;
			s.partialBits = 0;
		}

		void resetEncodingMaster(State& s)
		{
			resetEncoding(s);
			s.partialBits = this->allBitsRemaining;
		}

		void resetEncoderImpl()
		{
			for(u32 i = 0; i < this->channelCount; i++)
			{
				resetEncoding(this->coders[i]);
			}
			resetEncodingMaster(master);
			this->lastChannel = 0;
		}

		void resetDecoderImpl(Meta& meta)
		{
			this->reverseMsg.clear();
			for(u32 i = 0; i < this->channelCount; i++)
			{
				StateT channelState = meta.controlState[i];
				State& a			= coders[i];
				a.x					= channelState;
			}
			this->symbolsInCurrentBlock = meta.symbolsInBlock;
		}

		Meta createMeta() { return Meta(this->channelCount); }

		// Compression

		void encodeSingle(State& s, ReducedSymbolT current)
		{
			PRAGMA_HLS(pipeline)

			NbBitsT nbBits;
			// Recover number of bits
			nbBits = (s.x + ansTable.nb(current))
					 >> (this->ansTable.tableSizeLog() + 1);

			// Mask the output
			s.partial	  = s.x & MASK(nbBits);
			s.partialBits = nbBits;

			// Get next state
			s.x = ansTable.encodingTable(ansTable.start(current)
										 + (s.x >> nbBits));
			this->symbolsInCurrentBlock++;
		}

		void mergeChannels(backend::stream<StateT>& out,
						   backend::stream<Meta>& meta,
						   bool hadLast)
		{
			PRAGMA_HLS(inline)

			auto emitMeta = [&]() {
				PRAGMA_HLS(occurence CHECKPOINT)

				Meta finished(this->channelCount);

				for(u32 i = 0; i < this->channelCount; i++)
				{
					finished.controlState[i] =
						this->coders[i].x - this->ansTable.tableSize();
				}

				finished.symbolsInBlock = this->symbolsInCurrentBlock;

				meta << finished;
			};
			for(u32 j = 0; j < this->channelCount; j++)
			{
				PRAGMA_HLS(unroll)

				u32 k = j + this->lastChannel;
				if(k >= this->channelCount) k -= this->channelCount;
				State& s = this->coders[j];
				if(s.partialBits > master.partialBits)
				{
					// Write them
					master.partial |=
						s.partial >> (s.partialBits - master.partialBits);
					out << master.partial;
					// Reset state of the partial
					master.partialBits = this->allBitsRemaining - s.partialBits
										 + master.partialBits;
					master.partial = s.partial << (master.partialBits);

					this->checkpointCounter++;
					if(this->checkpointCounter == this->_checkpointFrequency)
					{
						this->checkpointCounter = 0;
						emitMeta();
					}
				} else
				{
					// Otherwise, just shift and write to partial
					master.partial |= s.partial
									  << (master.partialBits - s.partialBits);

					master.partialBits -= s.partialBits;
				}

				s.partial	  = 0;
				s.partialBits = 0;
			}

			if(hadLast) emitMeta();
		}

		void compressImpl(backend::side_stream<ReducedSymbolT>& message,
						  backend::stream<StateT>& out,
						  backend::stream<Meta>& meta)
		{
			bool hadLast = false;
			for(u32 i = 0; i < this->_chunkSize / this->channelCount; i++)
			{
				PRAGMA_HLS(pipeline ii = CHANNEL_COUNT)
				for(u32 j = 0; j < this->channelCount; j++)
				{
					PRAGMA_HLS(unroll)
					u32 k = j + this->lastChannel;
					if(k >= this->channelCount) k -= this->channelCount;
					auto cur = message.read();
					// auto oldX = this->coders[k].x;
					this->encodeSingle(this->coders[k], cur.data);
					//		std::cout << k << " " << oldX << " " << coders[k].x
					//<< " "
					//				  << std::endl;
					//		std::cout
					//			<< "Channel: " << k << " OldX: " << oldX
					//			<< " State: " << coders[k].x << " "
					//			<< u32(this->coders[k].partialBits) << "
					// Partial:
					//"
					//			<< std::bitset<16>(this->coders[k].partial) <<
					//"\n";
					if(cur.last)
					{
						this->lastChannel = k;
						hadLast			  = true;
						break;
					}
				}
				this->mergeChannels(out, meta, hadLast);
				//	std::cout << "Master: " << this->master.x << " "
				//			  << std::bitset<32>(this->master.partial) << " "
				//			  << (u32) this->master.partialBits << std::endl;
				if(hadLast) break;
			}
		}

		void redistribute(StateT& newd, StateT& d, u8 available, u8 lowerAmnt)
		{
			// In C++ a n-bit bitshift has undefined behaviour on an n bit type
			// So, we have a special case here...
			if(available == this->allBitsRemaining && lowerAmnt == 0)
			{
				d	 = newd;
				newd = 0;
			} else
			{
				auto negShift = std::min(
					(u8) (this->allBitsRemaining - lowerAmnt), available);
				d |= (newd & MASK(negShift)) << lowerAmnt;
				newd >>= negShift;
			}
		}

		void shift(StateT& newd, StateT& d, u8& available, u8& shiftAmount)
		{
			d >>= shiftAmount;
			available -= shiftAmount;
			this->redistribute(
				newd, d, available, this->allBitsRemaining - shiftAmount);
		}

		// Decompression

		DecompressResult
		decompressImpl(backend::stream<StateT>& data,
					   backend::stream<Meta>& meta,
					   backend::stream<ReducedSymbolT>& message)
		{
			StateT read			= 0;
			auto& curVal		= master.partial;
			auto& availableBits = master.partialBits;

			while(this->symbolsInCurrentBlock > 0)
			{
				State& curDec = this->coders[this->lastChannel];

				if(!data.empty() && availableBits < this->allBitsRemaining)
				{
					read = data.read();
					this->redistribute(read,
									   curVal,
									   availableBits + this->allBitsRemaining,
									   availableBits);
					availableBits += this->allBitsRemaining;
				}

				// auto bs	  = std::bitset<32>(curVal);
				auto oldX = curDec.x;
				if(this->ansTable.nbBitsDelta(oldX) > availableBits)
				{
					if(data.empty())
					{
						break;
					} else
						throw std::runtime_error(
							"The decompression requires more bits than are "
							"available, but there is data still elft in the "
							"stream. This is a bug!");
				}

				curDec.partialBits = this->ansTable.nbBitsDelta(curDec.x);
				reverseMsg.push_back(this->ansTable.states(curDec.x));
				this->symbolsInCurrentBlock--;
				curDec.x = this->ansTable.newX(curDec.x);

				curDec.partial = curVal & MASK(curDec.partialBits);
				curDec.x	   = curDec.x + curDec.partial;
				//		std::cout << this->lastChannel << " "
				//				  << u32(curDec.x + this->ansTable.tableSize())
				//<<
				//"
				//"
				//				  << u32(oldX + this->ansTable.tableSize())
				//				  << std::endl;
				//	std::cout << "Channel: " << this->lastChannel << " NewState:
				//"
				//			  << curDec.x + this->ansTable.tableSize()
				//			  << " State: " << oldX + this->ansTable.tableSize()
				//			  << " " << u32(curDec.partialBits)
				//			  << " Partial: " << std::bitset<16>(curDec.partial)
				//			  << " " << s32(curDec.partial) << " " << bs << " "
				//			  << u32(availableBits) << std::endl;
				this->shift(read, curVal, availableBits, curDec.partialBits);

				if(this->lastChannel == 0)
					this->lastChannel = this->channelCount;
				this->lastChannel--;
			}

			auto size = reverseMsg.size();
			for(auto it = this->reverseMsg.rbegin();
				it != this->reverseMsg.rend();
				it++)
			{
				message << *it;
			}

			this->reverseMsg.clear();

			return size;
		}

		template <typename Archive>
		void save(Archive& ar) const
		{
			ar(this->channelCount,
			   this->ansTable,
			   this->lastChannel,
			   this->master);
		}

		template <typename Archive>
		void load(Archive& ar)
		{
			ar(this->channelCount,
			   this->ansTable,
			   this->lastChannel,
			   this->master);
			this->master.partial >>= this->master.partialBits;
			this->master.partialBits =
				this->allBitsRemaining - this->master.partialBits;
			this->resetDecompressBuffer();
			this->coders.resize(this->channelCount);
		}
	};
} // namespace ANS
