#pragma once

#include "backend/stream.hpp"
#include "data/compression_table.hpp"
#include "ints.hpp"
#include "settings.hpp"

#include <memory>

namespace ANS
{
	template <template <typename, template <typename...> class...> class ImplT,
			  class Table,
			  template <typename...>
			  class... Rest>
	class CompressionContext
	{
		using ImplFullT = ImplT<Table, Rest...>;

	protected:
		// Checkpoint every checkpointFrequency state emissions onto the stream.
		u64 _checkpointFrequency = CHECKPOINT;
		u64 checkpointCounter	 = 0;

	public:
		using DecompressResult					   = bool;
		using StateT							   = typename Table::StateT;
		using MessageT							   = typename Table::MessageT;
		using TableT							   = Table;
		constexpr static const u8 allBitsRemaining = (sizeof(StateT) << 3);

		u64 checkpointFrequency() { return this->_checkpointFrequency; }

		void setCheckpointFrequency(u64 newFreq)
		{
			this->_checkpointFrequency = newFreq;
			this->checkpointCounter	   = 0;
		}

		void initialize() { ((ImplFullT*) (this))->initializeImpl(); }

		template <typename Meta>
		void flush(backend::stream<StateT>& out,
				   backend::stream<Meta>& meta,
				   u32 padding)
		{
			((ImplFullT*) (this))->flushImpl(out, meta, padding);
		}

		template <typename Meta,
				  typename T,
				  class = std::enable_if<(sizeof(MessageT) >= sizeof(T))>>
		void compress(backend::side_stream<T>& message,
					  backend::stream<StateT>& out,
					  backend::stream<Meta>& meta)
		{
			((ImplFullT*) (this))->compressImpl(message, out, meta);
		}

		template <typename Meta>
		DecompressResult decompress(backend::stream<StateT>& out,
									backend::stream<Meta>& meta,
									backend::stream<StateT>& message) const
		{
			return ((ImplFullT*) (this))->decompressImpl(out, meta, message);
		}

		//		template <typename Archive>
		//		void serialize(Archive& ar)
		//		{
		//			return ((ImplFullT*) (this))->serializeImpl(ar);
		//		}
	};

} // namespace ANS
