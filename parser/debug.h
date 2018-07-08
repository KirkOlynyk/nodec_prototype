#pragma once
#include <stdlib.h>

//---------------------------[ debug_free ]------------------------------------
// debug version of free
//-----------------------------------------------------------------------------

void debug_free(void* block);

//---------------------------[ debug_realloc ]---------------------------------
// debug version of realloc
//-----------------------------------------------------------------------------

void* debug_realloc(void* block, size_t size);

//---------------------------[ debug_memcpy ]----------------------------------
// debug version of memcopy
//-----------------------------------------------------------------------------

void debug_memcpy(void* dst, const void* src, size_t size);

//---------------------------[ hexDump ]---------------------------------------
// Prints a hexidecimal dump of a block of memory
//-----------------------------------------------------------------------------

void hexDump(const void *addr, size_t len, const char* prefix);

//---------------------------[ pause ]-----------------------------------------
// Prints a message and waits for the caller to press ENTER
//-----------------------------------------------------------------------------

void pause(const char* msg);