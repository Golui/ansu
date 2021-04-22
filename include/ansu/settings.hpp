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
 * if the test vector is sufficiently large. Needs to be divisible by
 * CHANNEL_COUNT.
 */
#define AVG_MESSAGE_LENGTH 256

/**
 * Specify how often to emit ANS::Meta
 * @param state_t size of state
 */
#define MESSAGES_PER_BLOCK 256

/**
 * Number of messages to emit an ANS::Meta after.
 */
#define CHECKPOINT 64

/**
 *  Convenience
 */
#define DATA_BLOCK_SIZE (AVG_MESSAGE_LENGTH * MESSAGES_PER_BLOCK)

#define META_BLOCK_SIZE (CHANNEL_COUNT * MESSAGES_PER_BLOCK / CHECKPOINT)

// TODO
#if AVG_MESSAGE_LENGTH % CHANNEL_COUNT != 0
#	error "AVG_MESSAGE_LENGTH must be divisible by CHANNEL_COUNT"
#endif
