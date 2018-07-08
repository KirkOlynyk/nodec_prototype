#pragma once
#include <stdlib.h>

//--------------------------[ sbuf_t ]------------------------------------------
//  A buffer for storing multiple strings. Each string is separted by a
//  zero terminator
//------------------------------------------------------------------------------

typedef struct _sbuf_t {
    size_t total;   // number of char's allocated must have used < total
    size_t used;    // number of chars used = starting offset for next char string
    size_t start;   // start of current string
    char* buffer;   // buffer of chars (size = total)
} sbuf_t;

//----------------------[ sbuf_add ]--------------------------------------------
// Add a new string to the strings buffer. Return the offset of the start
// of the string into the char buffer
//------------------------------------------------------------------------------

size_t sbuf_add(sbuf_t *sbuf, const char* s, size_t len, size_t buf_inc);

//----------------------[ sbuf_append ]-----------------------------------------
// Append to the current string in the strings buffer. sbuf_add must have
// been called at least once before this can be called.
//------------------------------------------------------------------------------

void sbuf_append(sbuf_t *sbuf, const char* s, size_t len, size_t buf_inc);

size_t sbuf_get_string_length(const sbuf_t* sbuf);
const char* sbuf_get_string(const sbuf_t* sbuf, size_t start);

//---------------------------[ sbuf_init ]-------------------------------------
// initializes the sbuf_t structure
//-----------------------------------------------------------------------------

void sbuf_init(sbuf_t* self);

//----------------------[ sbuf_delete ]-----------------------------------------
// Delete all resources that have been allocate by this strings buffer.
//------------------------------------------------------------------------------

void sbuf_delete(sbuf_t* self);
