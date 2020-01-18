#pragma once

#include <stdint.h>

typedef uint8_t state_t;
typedef int8_t state_delta_t;
typedef state_t encoding_table_t;
typedef uint8_t nb_bits_t;

#define TABLE_SIZE_LOG 5
#define TABLE_SIZE 32
#define ALPHABET_LENGTH 3

const state_t states[TABLE_SIZE] = {
    0, 0, 1, 2, 2, 0, 1, 1, 2, 2, 0, 1, 2, 2, 0, 0, 1, 2, 2, 0, 1, 1, 2, 0, 0, 1, 2, 2, 0, 1, 1, 2
};

const state_t new_x[TABLE_SIZE] = {
    8, 12, 8, 16, 20, 16, 12, 16, 24, 28, 20, 20, 0, 2, 24, 28, 24, 4, 6, 0, 28, 0, 8, 2, 4, 2, 10, 12, 6, 4, 6, 14
};

const encoding_table_t encoding_table[TABLE_SIZE] = {
    32, 33, 37, 42, 46, 47, 51, 55, 56, 60, 34, 38, 39, 43, 48, 52, 53, 57, 61, 62, 35, 36, 40, 41, 44, 45, 49, 50, 54, 58, 59, 63
};

const nb_bits_t nb_bits_delta[TABLE_SIZE] = {
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 2, 2, 2, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

const nb_bits_t nb[TABLE_SIZE] = {
    88, 88, 80
};

const state_delta_t start[ALPHABET_LENGTH] = {
    -10, 0, 8
};

const state_t adj_start[ALPHABET_LENGTH] = {
    22, 0, 8
};
