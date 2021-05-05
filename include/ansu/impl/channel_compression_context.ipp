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
			u64 metaOffset;
		};

		struct Meta
		{
			u32 channels;
			u32 currentChannel;
			u32 messagePad;
			Container<StateT> controlState;
			u32 offset;
			u32 deadBits;

			Meta(u32 channelCount = 0) : channels(channelCount)
			{
				this->controlState = Container<StateT>(channels);
			}

			// TODO Currenly seems broken
			bool operator==(const Meta other)
			{
				if(this->channels != other.channels) return false;
				for(u32 i = 0; i < this->channels; i++)
				{
					if(this->controlState[i] != other.controlState[i])
						return false;
				}
				return this->messagePad == other.messagePad
					   && this->controlState == other.controlState
					   && this->offset == other.offset
					   && this->deadBits == other.deadBits;
			}

			bool operator!=(const Meta other) { return !(*this == other); }

			u32 size() const
			{
				return sizeof(channels) + sizeof(currentChannel)
					   + sizeof(messagePad) + channels * sizeof(StateT)
					   + sizeof(offset) + sizeof(deadBits);
			}
		};

		Table ansTable;
		Container<State> encoders;
		State master;

	private:
		u32 writeStateSize;

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
			s.metaOffset  = 0;
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
			out << master.partial;

			Meta finished(this->channelCount);

			finished.offset		= master.metaOffset + 1;
			finished.deadBits	= master.partialBits;
			finished.messagePad = padding;
			for(u32 i = 0; i < this->channelCount; i++)
				finished.controlState[i] =
					this->encoders[i].x - this->ansTable.tableSize;

			finished.currentChannel = this->channelCount - 1;

			meta << finished;
		}

		void encodeSingle(State& s, MessageT current)
		{
			PRAGMA_HLS(pipeline)

			NbBitsT nbBits;
			// Recover number of bits
			nbBits = (s.x + ansTable.nb(current))
					 >> (this->ansTable.tableSizeLog + 1);

			// Mask the output
			s.partial	  = s.x & MASK(nbBits);
			s.partialBits = nbBits;

			// Get next state
			s.x = ansTable.encodingTable(ansTable.start(current)
										 + (s.x >> nbBits));
		}

		void mergeChannels(backend::stream<StateT>& out,
						   backend::stream<Meta>& meta)
		{
			PRAGMA_HLS(inline)
			for(u32 j = 0; j < this->channelCount; j++)
			{
				PRAGMA_HLS(unroll)
				State& s = this->encoders[j];
				if(s.partialBits > master.partialBits)
				{
					// Write them
					master.partial |=
						s.partial >> (s.partialBits - master.partialBits);
					out << master.partial;
					s.metaOffset++;
					// Reset state of the partial
					master.partialBits = this->allBitsRemaining - s.partialBits
										 + master.partialBits;
					master.partial = s.partial << (master.partialBits);
				} else
				{
					// Otherwise, just shift and write to partial
					master.partial |= s.partial
									  << (master.partialBits - s.partialBits);

					master.partialBits -= s.partialBits;
				}
			}
		}

		template <typename T>
		void compressImpl(backend::stream<T>& message,
						  backend::stream<StateT>& out,
						  backend::stream<Meta>& meta)
		{
			// TODO Generalize (policies?)
			for(u32 i = 0; i < AVG_MESSAGE_LENGTH / this->channelCount; i++)
			{
				PRAGMA_HLS(pipeline ii = CHANNEL_COUNT)
				for(u32 j = 0; j < this->channelCount; j++)
				{
					PRAGMA_HLS(unroll)
// TODO
#ifdef SOFTWARE
					std::cout << "Channel: " << j << " State: " << encoders[j].x
							  << std::endl;
#endif
					encodeSingle(this->encoders[j],
								 reinterpret_cast<MessageT>(message.read()));
				}
				mergeChannels(out, meta);
			}
		}

		DecompressResult
		decompressImpl(backend::stream<StateT>& out,
					   backend::stream<Meta>& meta,
					   backend::stream<MessageT>& message) const
		{
			return false;
		}
	};
} // namespace ANS
