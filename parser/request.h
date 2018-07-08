#pragma once
#include "http_parser.h"
#include "sbuf.h"
#include "kvp.h"

//---------------------------[ http_parser_callback ]--------------------------
// lists all possible http_parser callbacks
//-----------------------------------------------------------------------------

typedef enum _http_parser_callback {
    ON_NONE = 0,
    ON_MESSAGE_BEGIN,
    ON_URL,
    ON_STATUS,
    ON_HEADER_FIELD,
    ON_HEADER_VALUE,
    ON_HEADERS_COMPLETE,
    ON_BODY,
    ON_MESSAGE_COMPLETE,
    ON_CHUNK_HEADER,
    ON_CHUNK_COMPLETE
} http_parser_callback;

//---------------------------[ http_request_t ]--------------------------------
// Contains the state for completing an HTTP request
//-----------------------------------------------------------------------------

typedef struct _http_request_t {
    http_parser parser;
    sbuf_t sbuf;
    size_t sbuf_inc;    // charcter chunk increment
    kvpbuf_t kvpbuf;
    size_t kvpbuf_inc;  // increment of the number of key-value pairs
    pascal_string_t url;
    http_parser_callback previous;
} http_request_t;

#define HTTP_REQUEST_DEFAULT_SBUF_INC 1024
#define HTTP_REQUEST_DEFAULT_KVPBUF_INC 16

//---------------------------[ http_request_alloc ]----------------------------
// Allocates and initializes
//-----------------------------------------------------------------------------

http_request_t* http_request_alloc();

//---------------------------[ http_request_free ]-----------------------------
// deletes all resources and then frees up the memory
//-----------------------------------------------------------------------------

void http_request_free(http_request_t* self);
