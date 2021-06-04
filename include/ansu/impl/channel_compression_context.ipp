#pragma once

#include "compression_context.hpp"
#include "data/compression_table.hpp"
#include "settings.hpp"

#include <bitset>
#include <cassert>
#include <cereal/types/vector.hpp>
#include <vector>

#ifdef SOFTWARE
#	include <iostream>
#endif

#define MASK(b) ((1 << b) - 1)

namespace ANS
{
	template <typename _Table = DynamicCompressionTable<>,
			  template <typename...> class Container = std::vector>
	struct ChannelCompressionContext
		: public CompressionContext<ChannelCompressionContext,
									_Table,
									Container>
	{
		using BaseT =
			CompressionContext<ChannelCompressionContext, _Table, Container>;

		using Table	   = _Table;
		using StateT   = typename Table::StateT;
		using MessageT = typename Table::MessageT;
		using NbBitsT  = typename Table::NbBitsT;
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

			Meta(u32 channelCount = 0) : channels(channelCount)
			{
				this->controlState = Container<StateT>(channels);
			}

			bool operator==(const Meta other)
			{
				if(this->channels != other.channels) return false;
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
				ar(this->channels, this->controlState);
			}
		};

		Table ansTable;
		u32 channelCount;
		Container<State> coders;
		u32 lastChannel;
		State master;
		Container<MessageT> reverseMsg;
		u8 deadBits;

	private:
	public:
		ChannelCompressionContext(u32 channelCount = 2)
			: channelCount(channelCount)
		{
			this->coders = Container<State>(this->channelCount);
			this->resetImpl();
		}

		void resetDecompressBuffer()
		{
			reverseMsg.reserve((u64)(this->_checkpointFrequency / 0.6));
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

		void resetImpl()
		{
			for(u32 i = 0; i < this->channelCount; i++)
			{
				resetEncoding(this->coders[i]);
			}
			resetEncodingMaster(master);
			this->reverseMsg = Container<MessageT>();
		}

		void encodeSingle(State& s, MessageT current)
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

		// TODO New plan: Implement side channel data with ap_axis or similar,
		// use Vivado header libraries. Write meta on writing the final state
		// onto the stream. No need for requesting the meta - it will be written
		// if we trigger the last element from the side channel!

		template <typename T>
		void compressImpl(backend::side_stream<T>& message,
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
					//					auto oldX = this->coders[k].x;
					this->encodeSingle(this->coders[k],
									   reinterpret_cast<MessageT>(cur.data));
					//					std::cout
					//						<< "Channel: " << k << " OldX: " <<
					// oldX
					//						<< " State: " << coders[k].x << " "
					//						<< u32(this->coders[k].partialBits)
					//<< " Partial: "
					//						<<
					// std::bitset<16>(this->coders[k].partial)
					//<<
					//"\n";
					if(cur.last)
					{
						this->lastChannel = k;
						hadLast			  = true;
						break;
					}
				}
				this->mergeChannels(out, meta, hadLast);
				//				std::cout << "Master: " << this->master.x << " "
				//						  <<
				// std::bitset<16>(this->master.partial)
				//<<
				//"
				//"
				//						  << (u32) this->master.partialBits <<
				// std::endl;
				if(hadLast) break;
			}
		}

		void redistribute(StateT& newd, StateT& d, u8 available, u8 lowerAmnt)
		{
			auto negShift =
				std::min((u8)(this->allBitsRemaining - lowerAmnt), available);
			d |= (newd & MASK(negShift)) << lowerAmnt;
			newd >>= negShift;
		}

		void shift(StateT& newd, StateT& d, u8& available, u8& shiftAmount)
		{
			d >>= shiftAmount;
			available -= shiftAmount;
			this->redistribute(
				newd, d, available, this->allBitsRemaining - shiftAmount);
		}

		void initDecoders(Meta& meta)
		{
			for(u32 i = 0; i < this->channelCount; i++)
			{
				StateT channelState = meta.controlState[i];
				State& a			= coders[i];
				a.x					= channelState;
			}
		}

		DecompressResult decompressImpl(backend::stream<StateT>& data,
										backend::stream<Meta>& meta,
										backend::stream<MessageT>& message)
		{
			StateT read			= 0;
			auto& curVal		= master.partial;
			auto& availableBits = master.partialBits;

			while(!data.empty() || availableBits > 0)
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

				// auto bs = std::bitset<16>(curVal);
				// auto oldX = curDec.x;
				if(this->ansTable.nbBitsDelta(curDec.x) > availableBits)
				{
					if(data.empty())
					{
						//		std::cout << "NEXT BLOCK\n";
						break;
					} else
						throw std::runtime_error("Decompression went booboo");
				}
				curDec.partialBits = this->ansTable.nbBitsDelta(curDec.x);
				reverseMsg.push_back(this->ansTable.states(curDec.x));
				curDec.x = this->ansTable.newX(curDec.x);

				curDec.partial = curVal & MASK(curDec.partialBits);
				// std::cout << "Channel: " << this->lastChannel << " NewState:
				// "
				//		  << curDec.x + this->ansTable.tableSize()
				//		  << " State: " << oldX + this->ansTable.tableSize()
				//		  << " " << u32(curDec.partialBits)
				//		  << " Partial: " << std::bitset<16>(curDec.partial)
				//		  << " " << bs << " " << u32(availableBits)
				//		  << std::endl;
				curDec.x = curDec.x + curDec.partial;
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

		Meta createMeta() { return Meta(this->channelCount); }

		template <typename Archive>
		void save(Archive& ar) const
		{
			ar(this->channelCount,
			   this->ansTable,
			   this->lastChannel,
			   this->master,
			   this->deadBits);
		}

		template <typename Archive>
		void load(Archive& ar)
		{
			ar(this->channelCount,
			   this->ansTable,
			   this->lastChannel,
			   this->master,
			   this->deadBits);
			this->master.partial >>= this->master.partialBits;
			this->master.partialBits =
				this->allBitsRemaining - this->master.partialBits;
			this->resetDecompressBuffer();
			this->coders.resize(this->channelCount);
		}
	};
} // namespace ANS
