#pragma once
#include <stdlib.h>
#include <stdbool.h>

//---------------------------[ pascal_string_t ]-------------------------------
// Pascal type string where we store a charater reference and length of a
// string. In this case the reference is the staring position within
// a buffer of characters. The length does not include a terminator.
//-----------------------------------------------------------------------------

typedef struct _pascal_string_t {
    size_t start;
    size_t length;
} pascal_string_t;

//---------------------------[ kvp_t ]-----------------------------------------
// A pair of strings representing a key-value pair
//-----------------------------------------------------------------------------

typedef struct _kvp_t {
    pascal_string_t key;
    pascal_string_t value;
} kvp_t;

//---------------------------[ kvpbuf_t ]--------------------------------------
// A collection of key-value pairs stored in an array of kvp_t structures.
//      total: total number kvp_t strutures allocted
//      used: number of kvp_t structures are used. The used structures
//            start from the start of the array and are consecutive.
//      buffer: a pointer to the array of kvp_t structures.
//-----------------------------------------------------------------------------

typedef struct _kvpbuf_t {
    size_t total;
    size_t used;
    kvp_t* buffer;
} kvpbuf_t;

//--------------------[ kvpbuf_add ]-------------------------------------------
// Add a key-value pair to the key-value-pair buffer.
// kvp_inc: chunking size of key-value pairs to be added if necessary.
// A copy of the key-value pair is made so the caller can released the
// memory after the call if so wished.
//-----------------------------------------------------------------------------

void kvpbuf_add(kvpbuf_t* self, const kvp_t* kvp, size_t kvp_inc);

//-----------------------[ kvpbuf_delete ]-------------------------------------
// Releases all resources
//-----------------------------------------------------------------------------

void kvpbuf_delete(kvpbuf_t* self);

//-----------------------[ kvpbuf_init ]---------------------------------------
// Initializes all resources
//-----------------------------------------------------------------------------

void kvpbuf_init(kvpbuf_t* self);
