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
	
	http_parser_settings settings;
	init_settings(&settings);
}

//---------------------------[ http_request_alloc ]----------------------------
// Allocates and initializes
//-----------------------------------------------------------------------------

http_request_t* http_request_alloc()
{
    http_request_t* ans = (http_request_t*)debug_realloc(0, sizeof(http_request_t));
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
}
//---------------------------[ on_message_begin ]------------------------------

int on_message_begin(http_parser* parser)
{
	print_parser_only("on_message_begin", parser);
	return 0;
}

//---------------------------[ parser callbacks ]------------------------------

//---------------------------[ on_url ]----------------------------------------

int on_url(http_parser* parser, const char* at, size_t len)
{
	print_all("on_url", parser, at, len);
	return 0;
}

//---------------------------[ on_status ]-------------------------------------

int on_status(http_parser* parser, const char* at, size_t len)
{
	print_all("on_status", parser, at, len);
	return 0;
}

//---------------------------[ on_header_field ]-------------------------------

int on_header_field(http_parser* parser, const char* at, size_t len)
{
	print_all("on_header_field", parser, at, len);
	return 0;
}

//---------------------------[ on_header_value ]-------------------------------

int on_header_value(http_parser* parser, const char* at, size_t len)
{
	print_all("on_header_value", parser, at, len);
	return 0;
}

//---------------------------[ on_headers_complete ]---------------------------

int on_headers_complete(http_parser* parser)
{
	print_parser_only("on_headers_complete", parser);
	return 0;
}

//---------------------------[ on_body ]---------------------------------------

int on_body(http_parser* parser, const char* at, size_t len)
{
	print_all("on_body", parser, at, len);
	return 0;
}

//---------------------------[ on_message_complete ]---------------------------

int on_message_complete(http_parser* parser)
{
	print_parser_only("on_message_complete", parser);
	return 0;
}

//---------------------------[ on_chunk_header ]-------------------------------

int on_chunk_header(http_parser* parser)
{
	print_parser_only("on_chunk_header", parser);
	return 0;
}

//---------------------------[ on_chunk_complete ]-----------------------------

int on_chunk_complete(http_parser* parser)
{
	print_parser_only("on_chunk_complete", parser);
	return 0;
}
