#pragma once

#include "ansu/backend/stream.hpp"
#include "ansu/data/compression_table.hpp"
#include "ansu/ints.hpp"
#include "ansu/settings.hpp"

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
		u64 _chunkSize			 = AVG_MESSAGE_LENGTH;

	public:
		using DecompressResult = u64;
		using StateT		   = typename Table::StateT;
		using SymbolT		   = typename Table::SymbolT;
		using ReducedSymbolT   = typename Table::ReducedSymbolT;
		using TableT		   = Table;
		// NB We can't alias Meta to ImplFullT, because the type is not defined
		// yet!

		u8 allBitsRemaining = (sizeof(StateT) << 3);

		u64 checkpointFrequency() { return this->_checkpointFrequency; }

		void setCheckpointFrequency(u64 newFreq)
		{
			this->_checkpointFrequency = newFreq;
			this->checkpointCounter	   = 0;
		}

		u64 chunkSize() { return this->_chunkSize; }

		void setChunkSize(u64 n) { this->_chunkSize = n; }

		void resetEncoder() { ((ImplFullT*) (this))->resetEncoderImpl(); }

		template <typename Meta>
		void resetDecoder(Meta& m)
		{
			((ImplFullT*) (this))->resetDecoderImpl(m);
		}

		template <typename Meta>
		void compress(backend::side_stream<ReducedSymbolT>& message,
					  backend::stream<StateT>& out,
					  backend::stream<Meta>& meta)
		{
			((ImplFullT*) (this))->compressImpl(message, out, meta);
		}

		template <typename Meta>
		DecompressResult
		decompress(backend::stream<StateT>& out,
				   backend::stream<Meta>& meta,
				   backend::stream<ReducedSymbolT>& message) const
		{
			return ((ImplFullT*) (this))->decompressImpl(out, meta, message);
		}
	};

} // namespace ANS
