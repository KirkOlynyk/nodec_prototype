#pragma once
#include <stdbool.h>
#include "http_parser.h"
#include "sbuf.h"
#include "kvp.h"

//---------------------------[ string_t ]--------------------------------------
// A safe string where we specify the location of the string and the length
// not including any terminator
//-----------------------------------------------------------------------------

typedef struct _string_t {
	const char* s;
	size_t len;
} string_t;

//---------------------------[ string_t ]--------------------------------------
//  For iterating through the headers
//-----------------------------------------------------------------------------

typedef struct _header_t {
    string_t field;
    string_t value;
} header_t;

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
	http_parser_settings settings;
	bool headers_are_complete;
	uint64_t content_length;
    sbuf_t sbuf;
    size_t sbuf_inc;    // charcter chunk increment
	kvp_t kvp;
    kvpbuf_t kvpbuf;
    size_t kvpbuf_inc;  // increment of the number of key-value pairs
    pascal_string_t url;
    http_parser_callback previous;
} http_request_t;

#define HTTP_REQUEST_DEFAULT_SBUF_INC 32
#define HTTP_REQUEST_DEFAULT_KVPBUF_INC 2

//---------------------------[ http_request_alloc ]----------------------------
// Allocates and initializes
//-----------------------------------------------------------------------------

http_request_t* http_request_alloc();

//---------------------------[ http_request_free ]-----------------------------
// deletes all resources and then frees up the memory
//-----------------------------------------------------------------------------

void http_request_free(http_request_t* self);


//---------------------------[ http_request_execute ]--------------------------
// Runs the request parser on a sequence of characters
//-----------------------------------------------------------------------------

size_t http_request_execute(http_request_t* self, const char* data, size_t lenght);

//--------------------[ http_request_headers_are_complete ]--------------------
// If the headers are complete then you can ask about the content length,
// http version, and method. Otherwise the values are invalid
//-----------------------------------------------------------------------------

bool http_request_headers_are_complete(const http_request_t* self);

//--------------------[ http_request_content_length ]--------------------------
// Answer is valid only if http_request_headers_are_complete is true
//-----------------------------------------------------------------------------

uint64_t http_request_content_length(const http_request_t* self);

//--------------------[ http_request_http_major ]------------------------------
// Answer is valid only if http_request_headers_are_complete is true
//-----------------------------------------------------------------------------

unsigned short http_request_http_major(const http_request_t* self);

//--------------------[ http_request_http_minor ]------------------------------
// Answer is valid only if http_request_headers_are_complete is true
//-----------------------------------------------------------------------------

unsigned short http_request_http_minor(const http_request_t* self);

//--------------------[ http_request_method ]----------------------------------
// Answer is valid only if http_request_headers_are_complete is true
//-----------------------------------------------------------------------------

enum http_method http_request_method(const http_request_t* self);

//---------------------------[ http_request_url ]------------------------------

string_t http_request_url(const http_request_t* self);

//----------------------[ http_iter_headers ]----------------------------------

void http_request_iter_headers(const http_request_t* self, void(*callback)(const header_t*, void*), void* data);