#pragma once

#include "ints.h"
#include "ansu.h"
#include "ans_table.h"

typedef struct _ANSFileBlockEntry {
	ANSBlock block;
} ANSFileBlockEntry;

typedef struct _ANSFile {
	u32 block_count;
	ANSFileBlockEntry* blocks;
} ANSFile;

ANSFile* load_file(FILE* file);
