#include "ans_table.hpp"

#define __PRAGMA_SUB(x) _Pragma (#x)
#define DO_PRAGMA(x) __PRAGMA_SUB(x)
#define PRAGMA_HLS(x) DO_PRAGMA(HLS x)

/**
 * Specify how often to emit ANSMeta
 * @param state_t size of state
 */
#define CHECKPOINT 16384/sizeof(state_t)
// #define CHECKPOINT 2/sizeof(state_t)

/**
 * Specify the "average message length"; this is used excusively for tests.
 * encode_stream will be called every time the FIFO fills up with
 * AVG_MESSAGE_LENGTH symbols; this is due to potential memory issues
 * if the test vector is sufficiently large.
 */
#define AVG_MESSAGE_LENGTH 8
