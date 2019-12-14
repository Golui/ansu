#pragma once

#include "stdint.h"

typedef uint8_t state_t;
typedef int8_t state_delta_t;
typedef state_t encoding_table_t;
typedef uint8_t nb_bits_t;

#define table_size_log 5
#define table_size 32
#define alphabet_length 3

const state_t states[table_size];
const state_t new_x[table_size];
const encoding_table_t encoding_table[table_size];
const nb_bits_t nb_bits_delta[table_size];
const nb_bits_t nb[table_size];
const state_delta_t start[alphabet_length];
