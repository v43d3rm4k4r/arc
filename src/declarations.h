#pragma once

#include <stdint.h>

typedef enum Action
{
    Create,
    Extract
} Action;

void initArc(char* path, char* real_bin_file);

void packing(char* path, char** files, uint32_t files_count, char* real_bin_file);

void unpacking(char* binary, char* path);
