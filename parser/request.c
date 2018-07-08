#include <memory.h>
#include "http_parser.h"
#include "request.h"
#include "debug.h"
#include "sbuf.h"

//-----------------[ forward declaration of callbacks ]------------------------

int on_message_begin(http_parser*);
int on_url(http_parser*, const char*, size_t);
int on_status(http_parser*, const char*, size_t);
int on_header_field(http_parser*, const char*, size_t);
int on_header_value(http_parser*, const char*, size_t);
int on_headers_complete(http_parser*);
int on_body(http_parser*, const char*, size_t);
int on_message_complete(http_parser*);
int on_chunk_header(http_parser*);
int on_chunk_complete(http_parser*);

//---------------------------[ init_settings ]---------------------------------

static void init_settings(http_parser_settings* self)
{
	http_parser_settings_init(self);

	self->on_message_begin = on_message_begin;
	self->on_url = on_url;
	self->on_status = on_status;
	self->on_header_field = on_header_field;
	self->on_header_value = on_header_value;
	self->on_headers_complete = on_headers_complete;
	self->on_body = on_body;
	self->on_message_complete = on_message_complete;
	self->on_chunk_header = on_chunk_header;
	self->on_chunk_complete = on_chunk_complete;
}

//---------------------------[ http_request_init ]-----------------------------
// initializes the fields of an http_requsest_t structure
// private function used by http_request_alloc
//-----------------------------------------------------------------------------

static void http_request_init(http_request_t* self)
{
    memset(self, 0, sizeof(*self));
    sbuf_init(&self->sbuf);
    kvpbuf_init(&self->kvpbuf);
    self->sbuf_inc = HTTP_REQUEST_DEFAULT_SBUF_INC;
    self->kvpbuf_inc = HTTP_REQUEST_DEFAULT_KVPBUF_INC;
	http_parser_init(&self->parser, HTTP_REQUEST);
	init_settings(&self->settings);
	self->parser.data = self;
}

//---------------------------[ http_request_alloc ]----------------------------
// Allocates and initializes
//-----------------------------------------------------------------------------

http_request_t* http_request_alloc()
{
    http_request_t* ans = (http_request_t*)debug_realloc(0, sizeof(*ans));
    http_request_init(ans);
    return ans;
}

//---------------------------[ http_request_free ]-----------------------------
// deletes all resources and then frees up the memory
//-----------------------------------------------------------------------------

void http_request_free(http_request_t* self)
{
    sbuf_delete(&self->sbuf);
    kvpbuf_delete(&self->kvpbuf);
	debug_free(self);
}
//---------------------------[ get_request ]-----------------------------------
// returns a pointer to the http_request_t structure
//-----------------------------------------------------------------------------

static http_request_t* get_request(http_parser* parser)
{
	return (http_request_t*)(parser->data);
}

//[ parser callbacks ]

//---------------------------[ on_message_begin ]------------------------------

int on_message_begin(http_parser* parser)
{
	print_parser_only("on_message_begin", parser);
	http_request_t* R = get_request(parser);
	R->previous = ON_MESSAGE_BEGIN;
	return 0;
}

//---------------------------[ on_url ]----------------------------------------

int on_url(http_parser* parser, const char* at, size_t len)
{
	print_all("on_url", parser, at, len);
	http_request_t* R = get_request(parser);
	int ans = 0;
	switch (R->previous)
	{
	case ON_MESSAGE_BEGIN:
		R->url.start = sbuf_add(&R->sbuf, at, len, R->sbuf_inc);
		break;
	case ON_URL:
		sbuf_add(&R->sbuf, at, len, R->sbuf_inc);
		break;
	default:
		ans = 1;
		break;
	}
	R->previous = ON_URL;
	return ans;
}

//---------------------------[ on_status ]-------------------------------------

int on_status(http_parser* parser, const char* at, size_t len)
{
	print_all("on_status", parser, at, len);
	http_request_t* R = get_request(parser);
	R->previous = ON_STATUS;
	return 0;
}

//---------------------------[ on_header_field ]-------------------------------

int on_header_field(http_parser* parser, const char* at, size_t len)
{
	print_all("on_header_field", parser, at, len);
	http_request_t* R = get_request(parser);
	int ans = 0;
	switch (R->previous)
	{
	case ON_URL:
		R->url.length = sbuf_get_string_length(&R->sbuf);
		break;
	case ON_HEADER_FIELD:
		break;
	case ON_HEADER_VALUE:
		break;
	default:
		ans = 1;
		break;
	}
	R->previous = ON_HEADER_FIELD;
	return ans;
}

//---------------------------[ on_header_value ]-------------------------------

int on_header_value(http_parser* parser, const char* at, size_t len)
{
	print_all("on_header_value", parser, at, len);
	http_request_t* R = get_request(parser);
	R->previous = ON_HEADER_VALUE;
	return 0;
}

//---------------------------[ on_headers_complete ]---------------------------

int on_headers_complete(http_parser* parser)
{
	print_parser_only("on_headers_complete", parser);
	http_request_t* R = get_request(parser);
	R->previous = ON_HEADERS_COMPLETE;
	R->headers_are_complete = true;
	R->content_length = parser->content_length;
	return 0;
}

//---------------------------[ on_body ]---------------------------------------

int on_body(http_parser* parser, const char* at, size_t len)
{
	print_all("on_body", parser, at, len);
	http_request_t* R = get_request(parser);
	R->previous = ON_BODY;
	return 0;
}

//---------------------------[ on_message_complete ]---------------------------

int on_message_complete(http_parser* parser)
{
	print_parser_only("on_message_complete", parser);
	http_request_t* R = get_request(parser);
	R->previous = ON_MESSAGE_COMPLETE;
	return 0;
}

//---------------------------[ on_chunk_header ]-------------------------------

int on_chunk_header(http_parser* parser)
{
	print_parser_only("on_chunk_header", parser);
	http_request_t* R = get_request(parser);
	R->previous = ON_CHUNK_HEADER;
	return 0;
}

//---------------------------[ on_chunk_complete ]-----------------------------

int on_chunk_complete(http_parser* parser)
{
	print_parser_only("on_chunk_complete", parser);
	http_request_t* R = get_request(parser);
	R->previous = ON_CHUNK_COMPLETE;
	return 0;
}

//---------------------------[ http_request_execute ]--------------------------

size_t http_request_execute(http_request_t* self, const char* data, size_t len) 
{
	print_all("http_request_execute", &self->parser, data, len);
	size_t ans = http_parser_execute(&self->parser, &self->settings, data, len);
	return ans;
}

//--------------------[ http_request_headers_are_complete ]--------------------

bool http_request_headers_are_complete(const http_request_t* self)
{
	return self->headers_are_complete;
}

//--------------------[ http_request_content_length ]--------------------------
// Answer is valid only if http_request_headers_are_complete is true
//-----------------------------------------------------------------------------

uint64_t http_request_content_length(const http_request_t* self)
{
	return self->content_length;
}

//--------------------[ http_request_http_major ]------------------------------

unsigned short http_request_http_major(const http_request_t * self)
{
	return self->parser.http_major;
}

//--------------------[ http_request_http_minor ]------------------------------

unsigned short http_request_http_minor(const http_request_t * self)
{
	return self->parser.http_minor;
}

//--------------------[ http_request_method ]----------------------------------

enum http_method http_request_method(const http_request_t* self)
{
	return self->parser.method;
}

string_t http_request_url(const http_request_t* self)
{
	string_t ans = { 0, 0 };
	if (self->url.length > 0) {
		ans.len = self->url.length;
		ans.s = self->sbuf.buffer + self->url.start;
	}
	return ans;
}