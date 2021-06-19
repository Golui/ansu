#pragma once

#include "ansu/backend/backend.hpp"
#include "ansu/backend/stream.hpp"
#include "ansu/data/ans_table.hpp"
#include "ansu/data/compression_table.hpp"
#include "ansu/impl/channel_compression_context.ipp"
#include "ansu/ints.hpp"
#include "ansu/settings.hpp"
#include "ansu/util.hpp"

#define CONTROL_RESET_STATE 1
#define CONTROL_ENCODE 2
#define CONTROL_FLUSH 4

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)                                  \
	(byte & 0x80 ? '1' : '0'), (byte & 0x40 ? '1' : '0'),     \
		(byte & 0x20 ? '1' : '0'), (byte & 0x10 ? '1' : '0'), \
		(byte & 0x08 ? '1' : '0'), (byte & 0x04 ? '1' : '0'), \
		(byte & 0x02 ? '1' : '0'), (byte & 0x01 ? '1' : '0')
