#pragma once

#define __PRAGMA_SUB(x) _Pragma(#x)
#define DO_PRAGMA(x) __PRAGMA_SUB(x)
#define PRAGMA_HLS(x) DO_PRAGMA(HLS x)

/**
 * Specify the number of encoding channels, i.e. encoders working in parallel.
 */
#define CHANNEL_COUNT 2

/**
 * Specify the "average message length"; this is used excusively for tests.
 * encode_stream will be called every time the FIFO fills up with
 * AVG_MESSAGE_LENGTH symbols; this is due to potential memory issues
 * if the test vector is sufficiently large. Currently it has to be a multiple
 * of channel count.
 */
#define AVG_MESSAGE_LENGTH 65536

/**
 * An ANS::Meta will be emitted every CHECKPOINT elements written to the output
 * stream
 */
#define CHECKPOINT (65536 * 16)

