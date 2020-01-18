#pragma once

#include "ints.hpp"
#include "ansu.hpp"
#include "ans_table.hpp"

typedef struct _ANSFileBlockEntry {
	ANSBlock block;
} ANSFileBlockEntry;

typedef struct _ANSFile {
	u32 block_count;
	ANSFileBlockEntry* blocks;
} ANSFile;

ANSFile* load_file(FILE* file);
