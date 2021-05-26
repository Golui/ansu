#pragma once

#include "compression_context.hpp"
#include "data/compression_table.hpp"
#include "settings.hpp"

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
		const u32 channelCount;

		using Table	   = _Table;
		using StateT   = typename Table::StateT;
		using MessageT = typename Table::MessageT;
		using NbBitsT  = typename Table::NbBitsT;

		// TODO Refer to def in base class
		using DecompressResult = bool;

		struct State
		{
			StateT x;
			StateT partial;
			u8 partialBits;
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
			void serialize(Archive& ar) const
			{
				ar(this->channels, this->controlState);
			}
		};

		Table ansTable;
		Container<State> encoders;
		u32 lastChannel;
		State master;

	private:
	public:
		ChannelCompressionContext(u32 channelCount) : channelCount(channelCount)
		{
			this->encoders = Container<State>(this->channelCount);
		}

		void resetEncoding(State& s)
		{
			s.x			  = ansTable.encodingTable(0);
			s.partial	  = 0;
			s.partialBits = 0;
		}

		void resetEncodingMaster(State& s)
		{
			resetEncoding(s);
			s.partialBits = this->allBitsRemaining;
		}

		void initializeImpl()
		{
			for(u32 i = 0; i < this->channelCount; i++)
			{
				resetEncoding(this->encoders[i]);
			}
			resetEncodingMaster(master);
		}

		void flushImpl(backend::stream<StateT>& out,
					   backend::stream<Meta>& meta,
					   u32 padding)
		{
			//			out << master.partial;
			//
			//			Meta finished(this->channelCount);
			//
			//			finished.offset		= master.metaOffset + 1;
			//			finished.deadBits	= master.partialBits;
			//			finished.messagePad = padding;
			//			for(u32 i = 0; i < this->channelCount; i++)
			//				finished.controlState[i] =
			//					this->encoders[i].x -
			// this->ansTable.tableSize();
			//
			//			finished.currentChannel = this->channelCount - 1;
			//
			//			meta << finished;
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
			for(u32 j = 0; j < this->channelCount; j++)
			{
				PRAGMA_HLS(unroll)

				u32 k = j + this->lastChannel;
				if(k >= this->channelCount) k -= this->channelCount;
				State& s = this->encoders[k];
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
					if(hadLast
					   || this->checkpointCounter == this->_checkpointFrequency)
					{
						PRAGMA_HLS(occurence CHECKPOINT)
						this->checkpointCounter = 0;
						hadLast					= false;

						Meta finished(this->channelCount);

						for(u32 i = 0; i < this->channelCount; i++)
						{
							u32 k2 = i + this->lastChannel;
							if(k2 >= this->channelCount)
								k2 -= this->channelCount;

							finished.controlState[i] =
								this->encoders[k].x
								- this->ansTable.tableSize();
						}

						meta << finished;
					}
				} else
				{
					// Otherwise, just shift and write to partial
					master.partial |= s.partial
									  << (master.partialBits - s.partialBits);

					master.partialBits -= s.partialBits;
				}
			}
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
			for(u32 i = 0; i < AVG_MESSAGE_LENGTH / this->channelCount; i++)
			{
				PRAGMA_HLS(pipeline ii = CHANNEL_COUNT)
				for(u32 j = 0; j < this->channelCount; j++)
				{
					PRAGMA_HLS(unroll)
					u32 k = j + this->lastChannel;
					if(k >= this->channelCount) k -= this->channelCount;
					auto cur = message.read();
// TODO
#ifdef SOFTWARE
					std::cout << "Channel: " << k << " State: " << encoders[k].x
							  << std::endl;
#endif
					encodeSingle(this->encoders[k],
								 reinterpret_cast<MessageT>(cur.data));
					if(cur.last)
					{
						this->lastChannel = k;
						hadLast			  = true;
						break;
					}
				}
				mergeChannels(out, meta, hadLast);
				if(hadLast) break;
			}
		}

		DecompressResult
		decompressImpl(backend::stream<StateT>& out,
					   backend::stream<Meta>& meta,
					   backend::stream<MessageT>& message) const
		{
			return false;
		}

		template <typename Archive>
		void serialize(Archive& ar)
		{
			ar(this->ansTable, this->lastChannel);
		}
	};
} // namespace ANS
